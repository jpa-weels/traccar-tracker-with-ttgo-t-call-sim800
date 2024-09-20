#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_TEST_BATTERY true
#define TINY_GSM_RX_BUFFER 1024 // Tamanho do buffer de recepção
#define SerialMon Serial
#define SerialGPRS Serial1
#define Serial_GPS Serial2

#include <TinyGsmClient.h>
#include <TinyGPS++.h>
#include <TimeLib.h>  // Biblioteca para manipular tempo
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

const char server[] = "104.237.XXX.186"; //demo4.traccar.org | Você pode criar seu proprio servidor em traccar.org
const int port = 5055;
const char deviceId[] = "739155";      //Mude de acordo com sua preferencia

// Defina o tamanho do buffer e as variáveis
const int BUFFER_SIZE = 50;  // Número máximo de dados a serem armazenados no buffer
String buffer[BUFFER_SIZE];  // Array de strings para armazenar as URLs
int bufferStart = 0;         // Posição inicial do buffer (primeira mensagem a ser enviada)
int bufferEnd = 0;           // Posição final do buffer (última mensagem armazenada)

TinyGsm modem(SerialGPRS);
TinyGsmClient client(modem);
TinyGPSPlus gps;

void connectGPRS(){
  const int maxAttempts = 5;
  int attempt = 0;
  unsigned long retryDelay = 5000; // Aumentar progressivamente

  while (attempt < maxAttempts){
    if (modem.gprsConnect(APN, USER, PASS)){
      SerialMon.println("\nConectado!");
      IPAddress local = modem.localIP();
      SerialMon.println(local);
      return;
    } else {
      attempt++;
      SerialMon.print("Falha ao conectar. Tentativa ");
      SerialMon.print(attempt);
      SerialMon.print(" de ");
      SerialMon.println(maxAttempts);

      // Aumentar o intervalo entre tentativas a cada falha
      delay(retryDelay);
      retryDelay *= 2; // Dobra o intervalo após cada falha
    }
  }
  SerialMon.println("Falha ao conectar após várias tentativas. Reiniciando modem...");
  modem.restart(); // Reinicia o modem se falhar em todas as tentativas
}

void resetModulo(){
  unsigned long currentTime = millis();
  if (currentTime - lastResetTime >= RESET_INTERVAL){
    SerialMon.println("Reiniciando ESP32...");
    lastResetTime = currentTime;
    ESP.restart();
  }
}

float ajustaVelocidade(float speed){
  if (speed <= 3.00){
    return 0.00;
  } else {
    return speed;
  }
}

unsigned long getUnixTimestamp(){
  if (gps.date.isValid() && gps.time.isValid()){
    // Obter os valores do GPS
    int year = gps.date.year();
    int month = gps.date.month();
    int day = gps.date.day();
    int hour = gps.time.hour();
    int minute = gps.time.minute();
    int second = gps.time.second();

    // Converter a data/hora do GPS para timestamp Unix
    tmElements_t tm;
    tm.Year = year - 1970;  // O timestamp Unix começa em 1970
    tm.Month = month;
    tm.Day = day;
    tm.Hour = hour;
    tm.Minute = minute;
    tm.Second = second;

    // Obter o timestamp em UTC
    unsigned long unixTime = makeTime(tm);

    // Ajustar o fuso horário (-3 horas para UTC-3)
    unixTime -= 3 * 3600;
    return unixTime;
  }
  // Se a data ou hora não for válida, retorne 0 ou outro valor indicativo de erro
  return 0;
}

void addToBuffer(String data) {
  if ((bufferEnd + 1) % BUFFER_SIZE == bufferStart) {
    SerialMon.println("Buffer está cheio. Dados mais antigos serão sobrescritos.");
    bufferStart = (bufferStart + 1) % BUFFER_SIZE; // Move o início para liberar espaço
  }
  buffer[bufferEnd] = data;
  bufferEnd = (bufferEnd + 1) % BUFFER_SIZE;
}

void sendLocation() {
  if (millis() - lastSendTime > SEND_INTERVAL) {
    lastSendTime = millis();
    if (gps.location.isValid()) {
      unsigned long timestamp = getUnixTimestamp();
      double latitude = gps.location.lat();
      double longitude = gps.location.lng();
      int satellites = gps.satellites.value();
      float speed = ajustaVelocidade(gps.speed.kmph());
      float course = gps.course.deg();
      float altitude = gps.altitude.meters();
      float battery = modem.getBattVoltage() / 1000.0F;
      int rssi = modem.getSignalQuality();

      String url = "/?id=" + String(deviceId) +
                   "&timestamp=" + String(timestamp) +
                   "&lat=" + String(latitude, 6) +
                   "&lon=" + String(longitude, 6) +
                   "&sat=" + String(satellites) +
                   "&speed=" + String(speed, 1) +
                   "&bearing=" + String(course, 1) +
                   "&altitude=" + String(altitude, 1) +
                   "&battery=" + String(battery) +
                   "&rssi=" + String(rssi) +
                   "&ignition=1";

      addToBuffer(url);  // Adicione a URL ao buffer
      SerialMon.println("Dados armazenados no buffer: " + url);
    }
  }

  if (modem.isGprsConnected()) {
    while (bufferStart != bufferEnd) {
      if (client.connect(server, port)) {
        String data = buffer[bufferStart];
        client.print(String("GET ") + data + " HTTP/1.1\r\n" +
                     "Host: " + server + "\r\n" +
                     "Connection: close\r\n\r\n");
        ledStatus();             
        client.stop();
        SerialMon.println("Dados enviados com sucesso: " + data);
        bufferStart = (bufferStart + 1) % BUFFER_SIZE;
      } else {
        SerialMon.println("Falha ao conectar ao servidor.");
        break;
      }
    }
  }
}

void setup(){
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
void loop(){
  // Processa dados do GPS
  while (Serial_GPS.available() > 0){
    gps.encode(Serial_GPS.read());
    sendLocation();
  }
  // Verifica se o GPRS ainda está conectado
  if (!modem.isGprsConnected()){
    if (millis() - lastReconnectAttempt > RECONNECT_INTERVAL){
      lastReconnectAttempt = millis();
      SerialMon.println("Reconectando ao GPRS...");
      connectGPRS();
    }
  }
  resetModulo();
}
