#define TINY_GSM_MODEM_SIM900

#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <TinyGsmClient.h>
#include <PubSubClient.h>

#define GPS_BAUD  9600
#define GPRS_BAUD 115200
#define ARDUINO_GPS_TX 3 // GPS TX, Arduino RX pin
#define ARDUINO_GPS_RX 2 // GPS RX, Arduino TX pin
#define ARDUINO_GSM_RX 7 // GSM TX, Arduino RX pin
#define ARDUINO_GSM_TX 8 // GSM RX, Arduino TX pin

TinyGPSPlus gps;

SoftwareSerial SerialAT(ARDUINO_GSM_RX, ARDUINO_GSM_TX); // RX, TX

SoftwareSerial gpSerial(ARDUINO_GPS_RX, ARDUINO_GPS_TX); // Create a SoftwareSerial for the GPS

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

PubSubClient mqtt(client);

double latitude, longitude, speed=0.0, alt;
static char lati[10], longi[10], vitesse[10], altitude[10];

char data[50];

// Your GPRS credentials leave empty, if missing user or pass
// Vos informations d'identification GPRS laissez vide, s'il manque un utilisateur ou mot de passe

const char apn[]  = "";
const char user[] = "";
const char pass[] = "";

//const char* broker = "iot.eclipse.org" 
//const char* broker = "test.mosquitto.org"
const char* broker = "broker.hivemq.com";

const char* topic_GPS = "Tracker/coord";

long lastReconnectAttempt = 0;

void reconnect(void);
void pubGPSInfo(float latitude, float longitude);
static void smartDelay(unsigned long ms);

void setup()
{
  // communication port with GPS sensor
  // port de communication avec le capteur GPS
  gpSerial.begin(GPS_BAUD);      
  delay(3000);

  // communication port with the GPRS shield
  // port de comunication avec le shield GPRS
  SerialAT.begin(GPRS_BAUD);    
  delay(3000);

  // initializing the GPRS shield
  // initialisation du shield GPRS
  modem.restart();              
  
  // wait for the connection to the network of the mobile operator
  // attendre la connexion au réseau de l'oprateur mobile
  while (!modem.waitForNetwork()) {
  }

  // wait for the Internet connection
  // attendre la connexion à Internet
  while (!modem.gprsConnect(apn, user, pass)) {
  }
  
  // MQTT broker setup
  // Initialisation du broker MQTT
  mqtt.setServer(broker, 1883);
}

void loop()
{
  gpSerial.listen();
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
  mqtt.publish(topic_GPS, data);
}

void reconnect(void) {
  // Loop until we're reconnected
  // Boucle jusqu'à ce que nous soyons reconnectés
  while (!mqtt.connected()) {
    // Attempt to connect
    if (mqtt.connect("oha")) {
    } else {
      // Wait 5 seconds before retrying
      // Attendez 5 secondes avant de réessayer
      delay(5000);
    }
  }
}

// This custom version of delay() ensures that the tinyGPS object
// is being "fed". From the TinyGPS++ examples.

// Cette version personnalisée de delay () garantit que l'objet tinyGPS
// est "nourri". A partir des exemples TinyGPS ++.

static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    // If data has come in from the GPS module
    while (gpSerial .available())
      gps.encode(gpSerial .read()); // Send it to the encode function
    // tinyGPS.encode(char) continues to "load" the tinGPS object with new
    // data coming in from the GPS module. As full NMEA strings begin to come in
    // the tinyGPS library will be able to start parsing them for pertinent info
  } while (millis() - start < ms);
}

