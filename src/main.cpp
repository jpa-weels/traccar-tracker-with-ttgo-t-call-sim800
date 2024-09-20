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
static const uint32_t GSM_RATE = 115200; // Padrão, nao deve ser alterado
static const uint32_t GPS_RATE = 230400; // O padrão dos modulos gps é de 9600. para alterar vc precisa de um software https://content.u-blox.com/sites/default/files/2024-06/u-centersetup_v24.05.zip e uma plada de conexao usb to ttl FT232l

const unsigned long RESET_INTERVAL = 6UL * 60UL * 60UL * 1000UL; //Reset modulo a cada 6 horas
const unsigned long SEND_INTERVAL = 2000; // Tempo de envio da localização 2 segundos
const unsigned long RECONNECT_INTERVAL = 160000; //Verifica se esta conectado a cada 2 minutos 

static unsigned long lastResetTime = 0;
static unsigned long lastSendTime = 0;
static unsigned long lastReconnectAttempt = 0;

#define APN "claro.com.br"
#define USER "claro"
#define PASS "claro"

const char server[] = "104.237.3.XXX"; //demo4.traccar.org | Você pode criar seu proprio servidor em traccar.org
const int port = 5055;
const char deviceId[] = "7XX155";      //Mude de acordo com sua preferencia

const int BUFFER_SIZE = 50;  // Número máximo de dados a serem armazenados no buffer
String buffer[BUFFER_SIZE];  // Array de strings para armazenar as URLs
int bufferCount = 0;


TinyGsm modem(SerialGPRS);
TinyGsmClient client(modem);
TinyGPSPlus gps;

void connectGPRS() {
    const int maxAttempts = 5;
    int attempt = 0;

    while (attempt < maxAttempts) {
        if (modem.gprsConnect(APN, USER, PASS)) {
            SerialMon.println("\nConectado!");
            SerialMon.println(modem.localIP());
            return;
        } else {
            SerialMon.printf("Falha ao conectar. Tentativa %d de %d\n", ++attempt, maxAttempts);
            delay(5000);
        }
    }
    SerialMon.println("Falha ao conectar após várias tentativas. Reiniciando modem...");
    modem.restart(); // Reinicia o modem se falhar em todas as tentativas
}

void resetModulo() { // Uma segurança caso o modulo trave, talves nao seja necessario.
    if (millis() - lastResetTime >= RESET_INTERVAL) {
        SerialMon.println("Reiniciando ESP32...");
        lastResetTime = millis();
        ESP.restart();
    }
}

float ajustSpeed(float speed) {
    return speed <= 3.00 ? 0.00 : speed;
}
 // Converter a data/hora do GPS para timestamp Unix
unsigned long getUnixTimestamp() {
    if (gps.date.isValid() && gps.time.isValid()) {
        tmElements_t tm;
        tm.Year = gps.date.year() - 1970;
        tm.Month = gps.date.month();
        tm.Day = gps.date.day();
        tm.Hour = gps.time.hour();
        tm.Minute = gps.time.minute();
        tm.Second = gps.time.second();
        return makeTime(tm); // - 3 * 3600; // Ajustar o fuso horário, aqui na minha localidade não foi necessario  (-3 horas para UTC-3)
    }
    return 0;
}

void addToBuffer(String data) {
    if (bufferCount >= BUFFER_SIZE) {
        SerialMon.println("Buffer cheio. Dados mais antigos serão sobrescritos.");
        bufferCount = (bufferCount + 1) % BUFFER_SIZE; // Move o início
    }
    buffer[bufferCount] = data;
    bufferCount++;
    bufferCount %= BUFFER_SIZE; // Buffer circular
}

void sendHttpRequest(String data) {
    if (client.connect(server, port)) {
        client.print("GET " + data + " HTTP/1.1\r\n" +
                     "Host: " + server + "\r\n" +
                     "Connection: close\r\n\r\n");
        ledStatus();             
        client.stop();
        SerialMon.println("Dados enviados com sucesso: " + data);
    } else {
        SerialMon.println("Falha ao conectar ao servidor.");
    }
}

void sendLocation() {
    if (millis() - lastSendTime > SEND_INTERVAL) {
        lastSendTime = millis();
        if (gps.location.isValid()) {
            String url = "/?id=" + String(deviceId) +
                         "&timestamp=" + String(getUnixTimestamp()) +
                         "&lat=" + String(gps.location.lat(), 6) +
                         "&lon=" + String(gps.location.lng(), 6) +
                         "&sat=" + String(gps.satellites.value()) +
                         "&speed=" + String(ajustSpeed(gps.speed.kmph()), 1) +
                         "&bearing=" + String(gps.course.deg(), 1) +
                         "&altitude=" + String(gps.altitude.meters(), 1) +
                         "&battery=" + String(modem.getBattVoltage() / 1000.0F) +
                         "&rssi=" + String(modem.getSignalQuality()) +
                         "&ignition=1";

            addToBuffer(url);
            SerialMon.println("Dados armazenados no buffer: " + url);
        }
    }

    while (bufferCount > 0) {
        sendHttpRequest(buffer[0]); // Envia o primeiro item do buffer
        for (int i = 0; i < bufferCount - 1; i++) {
            buffer[i] = buffer[i + 1]; // Desloca itens para a esquerda
        }
        bufferCount--;
    }
}

void setup(){
  SerialMon.begin(BAUD_RATE);
  setupModem(); // Inicializa o moden go ttgo call em incluide/utilities.h
  delay(10);

  Serial_GPS.begin(GPS_RATE, SERIAL_8N1, GPS_RX, GPS_TX);
  SerialMon.println("Inicializando GPS...");
  delay(2000);

  SerialGPRS.begin(GSM_RATE, SERIAL_8N1, MODEM_RX, MODEM_TX);
  SerialMon.println("Inicializando GSM...");
  delay(2000);

  SerialMon.println("Restarting modem...");
  modem.restart();
  delay(5000);

  String imei = modem.getIMEI();
  SerialMon.println("IMEI: " + imei);

  SerialMon.print("Conectando a Internet... ");
  connectGPRS();
}

void loop() {
    while (Serial_GPS.available() > 0) {
        gps.encode(Serial_GPS.read());
        sendLocation();
    }

    if (!modem.isGprsConnected() && (millis() - lastReconnectAttempt > RECONNECT_INTERVAL)) {
        lastReconnectAttempt = millis();
        SerialMon.println("Reconectando ao GPRS...");
        connectGPRS();
    }

    resetModulo();
}
