//#include <ESP32QRCodeReader.h> // Use this lib if you have an ESP-32CAM
#include <Wire.h>
#include <WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Servo.h>

//ESP32QRCodeReader reader(CAMERA_MODEL_AI_THINKER); Uncomment this line if you use QRCodeReader

char* ssid = "Aa";
const char* password = "123456765";

Servo myservo;  // create servo object to control the barrier
int pos = 0;    // variable to store the servo position

// The pins commented are for ESP-32 Cam
#define IR_SENSOR_IN  14   // #define IR_SENSOR_IN  14
#define IR_SENSOR_OUT 27   // #define IR_SENSOR_OUT 16
#define ledPin  33         // #define ledPin  17
#define SDApin  12         // #define SDApin  12
#define SCLpin  13         // #define SCLpin  13
#define servoPin  34       // #define servoPin  15

bool irIn = 0;
bool irOut = 0;

// Define the address of the I2C LCD display
#define LCD_ADDRESS 0x27

// Create an instance of the I2C LCD display
LiquidCrystal_I2C lcd(LCD_ADDRESS, 16, 2);
String messageToScroll = "Thank you for using our Parking   See you soon";
String messageToScroll2 = "Please, Show your QR code near to the scanner";
int delayTime = 100;

// Define the MQTT broker and topic
const char *mqttBroker = "test.mosquitto.org";
const char *mqttTopicFromServer = "pOutConfirmation";
const char *mqttTopicToServer = "pCheckOUT";

// Create an instance of the MQTT client
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// Define the MQTT message buffer
//char mqttMessage[100];

void setup() {
  myservo.attach(servoPin, 15, 0, 180);  // attaches the servo on pin 15 channel : 15 min angle 0 max angle 180
  
  Wire.begin(SDApin, SCLpin);   // Initialize the I2C LCD display
  lcd.begin();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("-Smart Parking-");
  lcd.setCursor(0, 1);
  lcd.print("Have a Nice Day!");

  // Initialize the serial port
  Serial.begin(115200);

  // Set the digital pins as input
  pinMode(IR_SENSOR_IN, INPUT);
  pinMode(IR_SENSOR_OUT, INPUT);
  pinMode(ledPin, OUTPUT);

  setup_wifi();

  // set the Qr reader for ESP-32CAM
  // reader.setup();
  // Serial.println("Setup QRCode Reader");
  // reader.beginOnCore(1);
  // Serial.println("Begin on Core 1");

  // Connect to the MQTT broker
  mqttClient.setServer(mqttBroker, 1883);
  mqttClient.setCallback(callback);
  connectMqtt();
}

void loop() {
  irIn = digitalRead(IR_SENSOR_IN);
  const char* qrcode;

  // Check if the MQTT client is connected
  if (!mqttClient.connected()) {
      connectMqtt(); // Connect to the MQTT broker
  }
  
  if (!irIn) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Show your QrCode"); // scrollText(1, messageToScroll2, delayTime, 16);

    Serial.println(Serial.available());    
    if (Serial.available()) { // if there is data comming
      Serial.println("Serial is available");
      String ser = Serial.readStringUntil('\n'); // read string until newline character
      qrcode = ser.c_str();
      Serial.println(qrcode);
      sendQrCodeData(qrcode); // Use scanQrCode() if you have ESP32-CAM
    }
  }

  // Process MQTT messages
  mqttClient.loop();
}

//-------------------------functions-----------------------------

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

// Connect to the MQTT broker and subscribe to the MQTT topic
void connectMqtt() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect("OutControlDevice")) {
      Serial.println("connected");
      // Subscribe
      mqttClient.subscribe(mqttTopicFromServer);
    } 
    else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 1 second");
      delay(1000);  // Wait 1 seconds before retrying
    }
  }
}

// void scanQrCode() {
//   struct QRCodeData qrCodeData;
//   if (reader.receiveQrCode(&qrCodeData, 100))
//   {
//     Serial.println("Found QRCode");
//     if (qrCodeData.valid)
//     {
//       Serial.print("qrCodeData : ");
//       Serial.println((char *)qrCodeData.payload);
//       sendQrCodeData((char *)qrCodeData.payload);
//     }
//     else
//     {
//       Serial.print("Invalid: ");
//       Serial.println((char *)qrCodeData.payload);
//     }
//   }
// }

void sendQrCodeData(const char* qrCode) {
  char qrCodeJson[100];
  // Create a JSON object to store the QR code information
  StaticJsonDocument<100> doc;
  doc["outAccessKey"] = qrCode;

  // Serialize the JSON object
  serializeJson(doc, qrCodeJson);  // qrCodeJson = {"outAccessKey":qrCode}

  // Publish the QR code information to the MQTT broker
  mqttClient.publish(mqttTopicToServer, qrCodeJson);
  Serial.println("Data sent");
}

void callback(char* topic, byte* message, unsigned int length) {
  if (strstr(topic, mqttTopicFromServer)) {
    Serial.println("msg received");
    char mqttMessage[100];
    memcpy(mqttMessage, message, length);
    mqttMessage[length] = '\0';
    Serial.print("Message arrived on topic: ");
    Serial.print(topic);
    Serial.print(".Message : ");
    
    for (int i = 0; i < length; i++) {
      Serial.print((char)message[i]);
      mqttMessage[i] += (char)message[i];
    }
    Serial.println();
    
    // Parse the JSON-formatted message
    StaticJsonDocument<100> doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
      Serial.println("Failed to parse JSON message");
      return;
    }
    const char* checkOut = doc["checkOut"]; 
    const char* payment = doc["payment"]; 
    Serial.println("Success to parse JSON message");
    Serial.println("Our mqttTopic is : ");
    Serial.println(mqttTopicFromServer);
    Serial.println("Our checkOut is : ");
    Serial.println(checkOut);
    Serial.println("Our payment is : ");
    Serial.println(payment);

    if(strstr(checkOut, "open")){
      Serial.println("Valid Qr Code");
      digitalWrite(ledPin, LOW);
      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print("Valid Qr code");
      lcd.setCursor(0, 1);
      lcd.print("Have a Nice Day");
      openCloseBarrier();
    }
    else {
      Serial.println("Invalid Qr code");
      digitalWrite(ledPin, HIGH);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Invalid Qr code");
      lcd.setCursor(0, 1);
      lcd.print("Or pay your taxe");
    }
    if(payment != ""){
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Or pay your taxe");
      lcd.setCursor(0, 1);
      lcd.print("price :         ");
      lcd.setCursor(8, 1);
      lcd.print(payment);
    }
  }
}

void openCloseBarrier() {
  for (pos = 0; pos <= 90; pos += 1) { // goes from 0 degrees to 90 degrees
      myservo.write(pos);
      delay(15);
  }
  irOut = digitalRead(IR_SENSOR_OUT);
  Serial.println("Barriere opened");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("--Parking Exit--");
  while(irOut){ 
    scrollText(1, messageToScroll, delayTime, 16);
    irOut = digitalRead(IR_SENSOR_OUT);
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("--Parking Exit--");
  lcd.setCursor(0, 1);
  lcd.print("Have a Nice Day!");
  for (pos = 90; pos >= 0; pos -= 1) { // goes from 90 degrees to 0 degrees
      myservo.write(pos);
      delay(15);
  } 
  Serial.println("Barriere closed");
}

void scrollText(int row, String message, int delayTime, int lcdColumns) {
  for (int i=0; i < lcdColumns; i++) {
    message = " " + message;  
  } 
  message = message + " "; 
  for (int pos = 0; pos < message.length(); pos++) {
    lcd.setCursor(0, row);
    lcd.print(message.substring(pos, pos + lcdColumns));
    delay(delayTime);
  }
}
