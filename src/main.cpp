#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_TEST_BATTERY true
#define TINY_GSM_RX_BUFFER 1024 // Tamanho do buffer de recepção
#define SerialMon Serial
#define SerialGPRS Serial1
#define Serial_GPS Serial2
#define BUFFER_SIZE 500 // Tamanho do buffer

#include <TinyGsmClient.h>
#include <TinyGPS++.h>
#include "utilities.h"

static const uint32_t BAUD_RATE = 9600;
static const uint32_t GSM_RATE = 115200; // por padrão 
static const uint32_t GPS_RATE = 230400; // o padrao dos modulos gps e de 9600. para alterar vc precisa de um software https://content.u-blox.com/sites/default/files/2024-06/u-centersetup_v24.05.zip e uma plada de conexao usb to ttl FT232l

const unsigned long RESET_INTERVAL = 6UL * 60UL * 60UL * 1000UL; //Reset modulo a cada 6 horas
const unsigned long SEND_INTERVAL = 2000; // Tempo de envio da localização 2 segundos
const unsigned long RECONNECT_INTERVAL = 160000; //Verifica se esta conectado a cada 2 minutos 

static unsigned long lastResetTime = 0;
static unsigned long lastSendTime = 0;
static unsigned long lastReconnectAttempt = 0;

#define APN "claro.com.br"
#define USER "claro"
#define PASS "claro"

const char server[] = "104.237.xxx.xxx"; //demo4.traccar.org | Você pode criar seu proprio servidor em traccar.org
const int port = 5055;
const char deviceId[] = "739155";      //Mude de acordo com sua preferencia

TinyGsm modem(SerialGPRS);
TinyGsmClient client(modem);
TinyGPSPlus gps;

void connectGPRS()
{
  const int maxAttempts = 5;
  int attempt = 0;

  while (attempt < maxAttempts)
  {
    if (modem.gprsConnect(APN, USER, PASS))
    {
      SerialMon.println("\nConectado!");
      IPAddress local = modem.localIP();
      SerialMon.println(local);
      return;
    }
    else
    {
      attempt++;
      SerialMon.print("Falha ao conectar. Tentativa ");
      SerialMon.print(attempt);
      SerialMon.print(" de ");
      SerialMon.println(maxAttempts);
      delay(5000);
    }
  }
  SerialMon.println("Falha ao conectar após várias tentativas. Reiniciando modem...");
  modem.restart();
}

void resetModulo()
{
  unsigned long currentTime = millis();
  if (currentTime - lastResetTime >= RESET_INTERVAL)
  {
    SerialMon.println("Reiniciando ESP32...");
    lastResetTime = currentTime;
    ESP.restart();
  }
}

float ajustaVelocidade(float speed)
{
  if (speed <= 3.00)
  {
    return 0.00;
  }
  else
  {
    return speed;
  }
}

// Estrutura para armazenar os dados da localização
struct LocationData
{
  unsigned long timestamp;
  double latitude;
  double longitude;
  int satellites;
  float speed;
  float course;
  float altitude;
  float battery;
  int rssi;
};

LocationData buffer[BUFFER_SIZE]; // Buffer para armazenar os dados
int bufferStart = 0;              // Índice do dado mais antigo
int bufferEnd = 0;                // Próximo índice disponível no buffer
int bufferCount = 0;              // Contador para saber quantos itens estão no buffer

// Função para enviar os dados armazenados no buffer do mais antigo para o mais novo
void sendBufferedData()
{
  if (modem.isGprsConnected() && bufferCount > 0)
  {
    while (bufferCount > 0)
    {
      LocationData data = buffer[bufferStart];

      String url = "/?id=" + String(deviceId) +
                   "&timestamp=" + String(data.timestamp) +
                   "&lat=" + String(data.latitude, 6) +
                   "&lon=" + String(data.longitude, 6) +
                   "&sat=" + String(data.satellites) +
                   "&speed=" + String(data.speed, 1) +
                   "&bearing=" + String(data.course, 1) +
                   "&altitude=" + String(data.altitude, 1) +
                   "&battery=" + String(data.battery) +
                   "&rssi=" + String(data.rssi) +
                   "&ignition=1";

      if (client.connect(server, port))
      {
        client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                     "Host: " + server + "\r\n" +
                     "Connection: close\r\n\r\n");
        client.stop();
        Serial.println("Dados do buffer enviados com sucesso.");

        // Atualiza o índice e contador do buffer
        bufferStart = (bufferStart + 1) % BUFFER_SIZE;
        bufferCount--; // Diminui a quantidade de dados no buffer
      }
      else
      {
        Serial.println("Falha ao conectar ao servidor.");
        break; // Se a conexão falhar, interrompa o envio
      }
    }
  }
}

// Função para armazenar os dados no buffer de forma circular
void storeInBuffer(LocationData data)
{
  buffer[bufferEnd] = data;                  // Armazena os dados no próximo índice disponível
  bufferEnd = (bufferEnd + 1) % BUFFER_SIZE; // Atualiza o índice de onde armazenar os próximos dados

  if (bufferCount < BUFFER_SIZE)
  {
    bufferCount++; // Incrementa o contador se o buffer não estiver cheio
  }
  else
  {
    // Se o buffer estiver cheio, mova o início para o próximo dado
    bufferStart = (bufferStart + 1) % BUFFER_SIZE;
    Serial.println("Buffer cheio. Sobrescrevendo o dado mais antigo.");
  }
}

void sendLocation()
{
  if (millis() - lastSendTime > SEND_INTERVAL)
  {
    lastSendTime = millis();

    if (gps.location.isValid())
    {
      LocationData data;
      data.timestamp = gps.time.value();
      data.latitude = gps.location.lat();
      data.longitude = gps.location.lng();
      data.satellites = gps.satellites.value();
      data.speed = ajustaVelocidade(gps.speed.kmph());
      data.course = gps.course.deg();
      data.altitude = gps.altitude.meters();
      data.battery = modem.getBattVoltage() / 1000.0F;
      data.rssi = modem.getSignalQuality();

      if (modem.isGprsConnected())
      {
        sendBufferedData(); // Tenta enviar os dados armazenados no buffer

        // Criar o URL para enviar os dados atuais
        String url = "/?id=" + String(deviceId) +
                     "&timestamp=" + String(data.timestamp) +
                     "&lat=" + String(data.latitude, 6) +
                     "&lon=" + String(data.longitude, 6) +
                     "&sat=" + String(data.satellites) +
                     "&speed=" + String(data.speed, 1) +
                     "&bearing=" + String(data.course, 1) +
                     "&altitude=" + String(data.altitude, 1) +
                     "&battery=" + String(data.battery) +
                     "&rssi=" + String(data.rssi) +
                     "&ignition=1";

        // Enviar os dados via HTTP
        if (client.connect(server, port))
        {
          client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                       "Host: " + server + "\r\n" +
                       "Connection: close\r\n\r\n");
          client.stop();
          SerialMon.println("Dados enviados com sucesso.");
          ledStatus(); // Função para indicar sucesso no envio
        }
        else
        {
          SerialMon.println("Falha ao conectar ao servidor.");
          storeInBuffer(data); // Armazena os dados no buffer se falhar o envio
        }
      }
      else
      {
        storeInBuffer(data); // Armazena os dados no buffer se não houver conexão
      }

      SerialMon.println("Dados: " + String(data.latitude, 6) + ", " + String(data.longitude, 6) + " | " + String(data.timestamp));
    }
  }
}

void setup()
{
  SerialMon.begin(BAUD_RATE);
  setupModem();
  delay(10);

  Serial_GPS.begin(GPS_RATE, SERIAL_8N1, GPS_RX, GPS_TX);
  SerialMon.println("Inicializando GPS...");
  delay(2000);

  SerialGPRS.begin(GSM_RATE, SERIAL_8N1, MODEM_RX, MODEM_TX);
  SerialMon.println("Inicializando GSM...");
  delay(2000);

  SerialMon.println("Restarting modem...");
  modem.restart();
  delay(4000);

  String imei = modem.getIMEI();
  SerialMon.println("IMEI: " + imei);

  SerialMon.print("Conectando a Internet... ");
  connectGPRS();
}

void loop()
{
  while (Serial_GPS.available() > 0)
  {
    gps.encode(Serial_GPS.read());
    sendLocation();
  }

  if (!modem.isGprsConnected())
  {
    if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL)
    {
      lastReconnectAttempt = millis();
      SerialMon.println("Reconectando ao GPRS...");
      connectGPRS();
    }
  }
  resetModulo();
}
