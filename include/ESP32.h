

#define MODEM_RST            25
#define MODEM_TX             27
#define MODEM_RX             26
#define GPS_RX               32
#define GPS_TX               33
#define LED_GPIO             17
#define LED_ON               HIGH
#define LED_OFF              LOW


void setupModem(){

    pinMode(MODEM_RST, OUTPUT);
    digitalWrite(MODEM_RST, LOW);
    delay(2000);
    digitalWrite(MODEM_RST, HIGH);

  
}
void ledStatus()
{
  pinMode(LED_GPIO, OUTPUT);
  digitalWrite(LED_GPIO, LED_ON);
  delay(1000);
  digitalWrite(LED_GPIO, LED_OFF);
}