#define TINY_GSM_MODEM_SIM900

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define GPS_BAUD 9600
#define ARDUINO_GPS_TX 3 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_RX 2 // GPS RX, Arduino TX pin
#define ARDUINO_GSM_RX 7 // GPS TX, Arduino RX pin
#define ARDUINO_GSM_TX 8 // GPS RX, Arduino TX pin

TinyGPSPlus gps;

SoftwareSerial SerialAT(ARDUINO_GSM_RX, ARDUINO_GSM_TX); // RX, TX

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

PubSubClient mqtt(client);

SoftwareSerial gpSerial(ARDUINO_GPS_RX, ARDUINO_GPS_TX); // Create a SoftwareSerial

#define gpsPort gpSerial

double latitude, longitude, speed=0.0, alt;

char data[50];
// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "djezzy.internet";
const char user[] = "";
const char pass[] = "";

const char* broker = "broker.hivemq.com";//"iot.eclipse.org"
const char* topic_GPS = "Tracker/coord";
static char lati[10], longi[10], vitesse[10], altitude[10];

long lastReconnectAttempt = 0;

void reconnect(void);
void pubGPSInfo(float latitude, float longitude);
static void smartDelay(unsigned long ms);

void setup()
{
  gpsPort.begin(GPS_BAUD); // port de comunication avec le capteur GPS
  delay(1000);

  Serial.begin(115200);
  delay(1000);

  // Set GSM module baud rate
  SerialAT.begin(115200);
  delay(3000);

  Serial.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.print("Waiting for network...");
  while (!modem.waitForNetwork()) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");

  Serial.print("Connecting to ");
  Serial.print(apn);

  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" Fail");
    while (true);
  }
  Serial.println(" OK");

  // MQTT Broker setup
  mqtt.setServer(broker, 1883);
}

void loop()
{
  gpsPort.listen();
  latitude = gps.location.lat();
  longitude = gps.location.lng();
  speed = gps.speed.kmph();
  alt = gps.altitude.meters();
  smartDelay(3000);

  SerialAT.listen();
  if (mqtt.connected()) {
    pubGPSInfo(latitude, longitude, speed, alt);
    mqtt.loop();
  } else {
    reconnect();
  }
}

void pubGPSInfo(double latitude, double longitude, double speed, double alt)
{
  dtostrf(latitude, 8, 6, lati);
  dtostrf(longitude, 8, 6, longi);
  dtostrf(speed, 8, 6, vitesse);
  dtostrf(alt, 8, 6, altitude);
  sprintf(data, "%s,%s,%s,%s", lati, longi, vitesse, altitude);
  Serial.println(data);
  mqtt.publish(topic_GPS, data);
}

// This custom version of delay() ensures that the tinyGPS object
// is being "fed". From the TinyGPS++ examples.
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpsPort .available())
      gps.encode(gpsPort .read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

void reconnect(void) {
  // Loop until we're reconnected
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect("oha")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
