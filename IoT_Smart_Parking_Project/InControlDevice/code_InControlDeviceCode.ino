

#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <NewPing.h>
#include <Keypad.h>

#define trigPinIn 5  // Trigger (emission)
#define echoPinIn 18  // Echo    (réception)

#define trigPinOut 13  // Trigger (emission)
#define echoPinOut 12  // Echo    (réception)

#define LedRed 27
#define LedGreen 14
#define SERVO_PIN  15// ESP32 pin GIOP18 connected to servo motor


#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns
char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pin_rows[ROW_NUM]      = {19, 33, 26, 17}; // GIOP19, GIOP18, GIOP5, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 4, 0, 2};   // GIOP16, GIOP4, GIOP0, GIOP2 connect to the column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );


/***********************************************************************************************************/
//"YOUR_MQTT_BROKER_IP_ADDRESS";
WiFiClient espClient;
PubSubClient client(espClient);

/***********************************************************************************************************/
// crée un objet servoMotor pour contrôler un servo, 12 objets servo peuvent être créés
Servo servoMotor;

/***********************************************************************************************************/
// définit le type d'écran lcd 16 x 2 et l'adresse
LiquidCrystal_I2C LCD(0x27, 16, 2);

/***********************************************************************************************************/
// définit le ssid et le passxord de WiFi
const char* ssid = "Asmaa";
const char* password = "123456789";

/***********************************************************************************************************/
// définit le MQTT broker, le port et le topic
const char* mqtt_server = "test.mosquitto.org";
const char* topic_pub = "pCheckIN";
const int mqtt_port = 1883;

int pos = 0;  //  variable pour stocker la position du servo
int msg;
char input_password[20];

// Variables utiles
long dureeIn;   // durée de l'echo
int distanceIn; // distance

long dureeOut;   // durée de l'echo
int distanceOut; // distance

long long now;
long long lastMsg = 0;
int x;
int y;

int Din;
int Dout;

/*********************************************** Enumerate **************************************************/
typedef enum Boolean_e{
  BarrOuver = 1,
  BarrFerm = 0
}Boolean_E;
/***********************************************************************************************************/
/*********************************************** Setup ******************************************************/
void setup()
{
  Serial.begin(115200);
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  pinMode(trigPinIn, OUTPUT); // Configuration du port du Trigger comme une SORTIE
  pinMode(echoPinIn, INPUT);  // Configuration du port de l'Echo  comme une ENTREE
  
  pinMode(trigPinOut, OUTPUT); // Configuration du port du Trigger comme une SORTIE
  pinMode(echoPinOut, INPUT);  // Configuration du port de l'Echo  comme une ENTREE
    
  pinMode(LedRed, OUTPUT);
  pinMode(LedGreen, OUTPUT);
  
  LCD.init(); // initialisation de l'afficheurs
  LCD.backlight(); // active le rétro-éclairag
  LCD.setCursor(1, 0);
  LCD.print("Hello User");
  
  servoMotor.attach(SERVO_PIN);  // attache le servo sur la broche 9 à l'objet servo

  digitalWrite(LedRed, HIGH);
  Serial.println("LedRed Allume");
}
/***********************************************************************************************************/
/*********************************************** Loop ******************************************************/
void loop()
{
  char key;
  int str_len=0;

  long now = millis();
  if(now - lastMsg > 1000)
  {
    Din = CapteurIn();
    Serial.println(Din);
    if(Din < 3)
    {
      Serial.println("présence voiture");
      key = keypad.getKey(); 
      while(key != '#' && x == 0)
      {
        if (key) 
        {      
          Serial.println(key);
          input_password[str_len]= key; // append new character to input password string
          str_len++;
        } 
      key = keypad.getKey() ;  
      }  
      x=1;
      Din = CapteurIn();
      Serial.println(Din);
    }
    
    if (!client.connected())
    {
      reconnect();
    }
    client.loop();
    client.publish(topic_pub, input_password);   
  }

  Serial.println("pas voiture");
  
}
/***********************************************************************************************************/
/******************************************** WiFi Function ************************************************/
void setup_wifi()
{
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi Connected !");
  Serial.println(WiFi.localIP());

}
/***********************************************************************************************************/
/*************************************** Receive Function **************************************************/
void callback(char* topic, byte* payload, unsigned int length)
{
  String messageConf;
  
  Serial.println("Avant : ");
  Serial.println(messageConf); 
    
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) 
  {
    Serial.println((char)payload[i]);
    messageConf += (char)payload[i];
  }
  
  Serial.println("Apres : ");
  Serial.println(messageConf);

  if(messageConf == "True")
  {
    LCD.setCursor(0,0);
    LCD.print("Le code Valide.");
    LCD.setCursor(4,1);
    LCD.print("Bienvenue ^_^");

    Serial.println("Le code valide");

    controlServo(BarrOuver);
    
    digitalWrite(LedRed, LOW);
    digitalWrite(LedGreen, HIGH);

    delay(5000);
    Dout = CapteurOut();
    while(Dout < 5)
    {
      Dout = CapteurOut();
      Serial.println(Dout);
    }

    delay(5000);
    controlServo(BarrFerm);
    
    digitalWrite(LedGreen, LOW);
    digitalWrite(LedRed, HIGH);
  }
  
  if(messageConf == "False")
  {
    LCD.clear();
    LCD.setCursor(0,0);
    LCD.print("Le code Invalide.");
    LCD.setCursor(1,1);
    LCD.print("Try again");
    Serial.println("Le code Invalide.");
  }

  Serial.println("-----------------------");
  x=0;
}
/***********************************************************************************************************/
/************************************* Connect server Function *********************************************/
void reconnect()
{
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client"))
    {
      Serial.println("connected");
      client.subscribe("pInConfirmation");
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
/***********************************************************************************************************/
/************************************* Montant/Descendant Function *****************************************/
void controlServo(Boolean_E sens)
{
    //  Ouverture de la barrière
  if(sens)
  {
    for (pos = 0; pos < 90; pos++)
    {
      servoMotor.write(pos);
      delay(15);
    }    
  }

    //  Fermeture de la barrière
  else
  {
    for (pos = 90; pos >= 0; pos--)
    {
      servoMotor.write(pos);
      delay(15);
    }    
  }
}
/***********************************************************************************************************/
/****************************************** Read code Function *********************************************/
int CapteurOut()
{
    // Émission d'un signal de durée 10 microsecondes
  digitalWrite(trigPinOut, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPinOut, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinOut, LOW);

  // Écoute de l'écho 
  dureeOut = pulseIn(echoPinOut, HIGH);

  // Calcul de la distance
  distanceOut = dureeOut*0.034/2;

  return distanceOut;
}

int CapteurIn()
{
    // Émission d'un signal de durée 10 microsecondes
  digitalWrite(trigPinIn, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPinIn, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPinIn, LOW);

  // Écoute de l'écho 
  dureeIn = pulseIn(echoPinIn, HIGH);

  // Calcul de la distance
  distanceIn = dureeIn*0.034/2;

  return distanceIn;
}
/***********************************************************************************************************/
