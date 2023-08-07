
#include <WiFi.h>
#include <PubSubClient.h>
#include <HCSR04.h>
#include <string.h>

#define LED_1_PIN 15
#define LED_2_PIN 2
#define LED_3_PIN 4
#define LED_4_PIN 5
#define LED_5_PIN 18
#define LED_6_PIN 19
#define LED_NUMBER 6
const int buzzer=13;

int j;
byte triggerPin = 12;
byte echoCount = 6;
byte* echoPins = new byte[echoCount] { 27, 26, 25, 33, 32, 35};
byte LEDPinArray[LED_NUMBER] = { LED_1_PIN,       //tableau des leds
                                 LED_2_PIN,
                                 LED_3_PIN,
                                 LED_4_PIN,
                                 LED_5_PIN,
                                 LED_6_PIN};

const char* ssid ="wifimanal";// "REPLACE_WITH_YOUR_SSID";
const char* password ="manal1234";// "REPLACE_WITH_YOUR_PASSWORD";
// Add your MQTT Broker IP address, example:
const char* mqtt_server ="test.mosquitto.org";//"192.168.1.6";
//"YOUR_MQTT_BROKER_IP_ADDRESS";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
     

void initAllLEDs()        //Déclarations des leds
{
  for (int j = 0; j < LED_NUMBER; j++) {
    pinMode(LEDPinArray[j], OUTPUT);
  }
}

void setup() {
 Serial.begin(115200);
 // default settings
 setup_wifi();
 client.setServer(mqtt_server, 1883);
 client.setCallback(callback);
//HCSR04
  HCSR04.begin(triggerPin, echoPins, echoCount);
  initAllLEDs();
//Buzzer
  pinMode(buzzer, OUTPUT);

 
}
void setup_wifi() {
 delay(10);
 // We start by connecting to a WiFi network
 Serial.println();
 Serial.print("Connecting to ");
 Serial.println(ssid);
 WiFi.begin(ssid, password);
 while (WiFi.status() != WL_CONNECTED) {
 delay(500);
 Serial.print(".");
 }
 Serial.println("");
 Serial.println("WiFi connected");
 Serial.println("IP address: ");
 Serial.println(WiFi.localIP());
}

void callback(char* pCarsStatusdevice, byte* message, unsigned int length) {
 Serial.print("Message arrived on topic: ");
 Serial.print(pCarsStatusdevice);
 Serial.print(". Message: ");
 String messageTemp;
 for (int i = 0; i < length; i++) {
 Serial.print((char)message[i]);
 messageTemp += (char)message[i];
 } 
 Serial.println();
 // à ajouter ici qlq chose
 
 }

void reconnect() {
 // Loop until we're reconnected
 while (!client.connected()) {
 Serial.print("Attempting MQTT connection...");
 // Attempt to connect
 if (client.connect("ESP8266Client")) {
 Serial.println("connected");
 // Subscribe
 client.subscribe("pCarsStatusdevice ");
 } else {
 Serial.print("failed, rc=");
 Serial.print(client.state());
 Serial.println(" try again in 5 seconds");
 // Wait 5 seconds before retrying
 delay(5000);
 }
 }
}

void loop() {

double* distances = HCSR04.measureDistanceCm();     //fonction de mesure de distance
 
  for (int j = 0; j < echoCount; j++) {           //afficher les distances mesurées en moniteur serie
    Serial.print(j + 1);
    Serial.print(": ");
    Serial.print(distances[j]);
    Serial.println(" cm");
  }
  if (!client.connected()) {
 reconnect();
 }
 client.loop();
 long now = millis();
if (now - lastMsg > 1000) {
 lastMsg = now;
 
  bool pCarsStatusdevice; 
  for ( j = 0; j < echoCount && j < LED_NUMBER; j++) {    //Déciser est ce que l'emplacement est occupé ou disponible et actyaliser l'état des leds
    if (distances[j]<50) {         //si distance<50 ==) c'est occupé
    Serial.print(j + 1);
    Serial.print(": ");
    Serial.print("Occupied");
   pCarsStatusdevice = 1;                 // 1 signifie que l'emplacement est occupé
    Serial.print("\n");

 char situationString[8];
 dtostrf(pCarsStatusdevice, 1, 2, situationString);
 Serial.print("situation: ");
 Serial.println(situationString);
 client.publish("pCarsStatusdevice ", situationString);  //publication au serveur

    digitalWrite(LEDPinArray[j], HIGH);        //allumer donc la led de signalisation
    
    digitalWrite(buzzer, HIGH);   //activer le buzzer
    delay(1000);                       
    digitalWrite(buzzer, LOW);    //desactiver le buzzer
    delay(1000);                       
    
      }
      else {
    Serial.print(j + 1);
    Serial.print(": ");
    Serial.print("Available");        //l'emplacement est disponible 
    Serial.print("\n");
   pCarsStatusdevice = 0;                    // 0 signifie que l'emplacement est disponible

 char situationString[9];
 dtostrf(pCarsStatusdevice, 1, 2, situationString);
 Serial.print("situation: ");
 Serial.println(situationString);
 client.publish("pCarsStatusdevice ", situationString);   //Publication au serveur
  
    digitalWrite(LEDPinArray[j], LOW);  //étteindre la led de signalisation
      }
  }
  Serial.println("--- ANOTHER LOOP----");
  delay(250);  

}
}
