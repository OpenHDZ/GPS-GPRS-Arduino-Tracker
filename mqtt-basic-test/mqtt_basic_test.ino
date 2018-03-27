// Définition des librairies nécessaire
// Definition of the necessary libraries

#include <SPI.h>
#include <DHT.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Définition de constants
// Definition of constants
#define DHTPIN 7        // Le pin de connexion du capteur 
#define DHTTYPE DHT22   // Le type de capteur DHT 22  

float h;  // pour stocké la valeur de l'humidité 
float t;  // pour stocké la valeur de la température 

// les topic pour la température et de l'humidité
// the topic for temperature and humidity

const char* topic_t="Capteur/temperature";
const char* topic_h="Capteur/humidity";

// les chaine de caractère pour transmettre des données avec MQTT
// the string to transmit data with MQTT

char hum[10],temp[10];

// instance de la classe DHT
// instance of the DHT class

DHT dht(DHTPIN, DHTTYPE); 

// les paramètres de connexion du shield Ethernet
// Ethernet shield connection parameters

byte mac[] = {  0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }; // The Mac adress of your ethernet shield
                                                      // L'adresse Mac de votre shield ethernet
                                                      
IPAddress ip(192, 168, 1, 4);                         // the IP adress of the arduino shield
                                                      // l'adresse IP de votre Arduino

// L'adresse de du Broker
// the Broker adress

const char* server = "test.mosquitto.org";

// instance de la classe EthernetClient
// instance of the EthernetClient class

EthernetClient ethClient;

// l'instance de la classe MQTT
// instance of the MQTT class

PubSubClient mqtt(ethClient);


void reconnect(void);
void pubCapteur(void);

void setup()
{
  // Initialisation de la liaison série
  // Initializing the serial 
  
  Serial.begin(9600);

  // Initialisation du capteur DHT22
  // Initializing the DHT22 sensor
  
  dht.begin();

  // Initialisation de la connexion TCP
  // Initializing the TCP connection
  
  Ethernet.begin(mac, ip);
  
  // Initialisation de la connexion au Broker
  // Initializing the Broker connection
  
  mqtt.setServer(server, 1883);
 
  delay(1500);
}

void loop()
{
  if(mqtt.connected())
  {
    // Si la connexion au Broker est réalisé envoyer les données
    // If the Broker connection is made send the data
    pubCapteur();
    mqtt.loop();
  }else {
    // Sinon se reconnecté au Broker
    // Otherwise connect to the Broker
    reconnect();
  }
}

void pubCapteur(void)
{
  // récupération des valeurs depuis le capteur 
  // recovering values from the sensor
  
  h = dht.readHumidity();
  t = dht.readTemperature();

  // convertion des valeurs capturé en chaine de caractére 
  // converting captured values to a string 
  
  dtostrf(h, 7, 4, hum);
  dtostrf(t, 7, 4, temp);

  // publication des donnée sur la Broker
  // publication of data on the Broker
  
  mqtt.publish(topic_h,hum);
  mqtt.publish(topic_t,temp);

  delay(10000); 
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
