#include "pitches.h"
#include "DHT.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Stepper.h>

#define LEDC_CHANNEL_0     0
#define LEDC_TIMER_12_BIT  12
#define LEDC_BASE_FREQ     5000
#define LED_PIN            18
#define DHTPIN 23
#define DHTTYPE DHT11
#define OLED_ADDR   0x3C

Adafruit_SSD1306 display(128, 64, &Wire, -1);
DHT dht(DHTPIN, DHTTYPE); 

const int stepsPerRevolution = 128;
Stepper myStepper(stepsPerRevolution, 32, 25, 33, 26);

const int motorPin = 4;
const int Pas_1=32; 
const int Pas_2=33;  
const int Pas_3=25; 
const int Pas_4=26; 
int signal_moteur = 0;
int signal_moteur_pas = 0;
int i;
int buzzer_stop = 1;


const char* ssid = //Your SSID;
const char* password = //Your Password;

const char* mqtt_server = "mqtt.ci-ciad.utbm.fr";


WiFiClient espClientLucas;
PubSubClient client(espClientLucas); 
long lastMsg = 0;

void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);

  ledcWrite(channel, duty);
}

int melody[] = {
  0, NOTE_DS4
};

void setup() {
  Serial.begin(9600);
  dht.begin();

  setup_wifi(); 
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback); 

  pinMode(motorPin, OUTPUT); 
  pinMode(Pas_1, OUTPUT); 
  pinMode(Pas_2, OUTPUT); 
  pinMode(Pas_3, OUTPUT);  
  pinMode(Pas_4, OUTPUT);
  pinMode(18, OUTPUT);

  myStepper.setSpeed(10);
  Serial.println("ESP32 Button Test");
  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);


}

void setup_wifi() {
  delay(10);
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

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic); 
  Serial.print(". Message: ");
  
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println("probleme");

  if (String(topic) == "esp32/moteur") {
    Serial.print("Changing output to ");

    if(messageTemp == "on"){
      signal_moteur = 1;
    }
    
    else if(messageTemp == "off"){
      signal_moteur = 0;
    }
  }
  else if (String(topic) == "esp32/moteur_pas"){
    if(messageTemp == "haut"){
      signal_moteur_pas = 1;
    }
    
    else if(messageTemp == "bas"){
      signal_moteur_pas = 2;
    }
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("espClientLucas")) {
      Serial.println("connected");
      client.subscribe("esp32/moteur");
      client.subscribe("esp32/moteur_pas");
      client.subscribe("esp32/capteurs");
    } 
	else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void testdrawtriangle(void) {
  display.clearDisplay();

  for(int16_t i=20; i<21; i++) {
    display.drawTriangle(
    display.width()/2  , display.height()/2-i,
    display.width()/2-i, display.height()/2+i,
    display.width()/2+i, display.height()/2+i, WHITE);
    display.setCursor(60, 32);
    display.setTextSize(2);
    display.println("!");
    display.display();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (signal_moteur == 0){
    digitalWrite(motorPin, LOW);
  }
    else if (signal_moteur == 1){
    digitalWrite(motorPin, HIGH);
  }

  if (signal_moteur_pas == 1){
    myStepper.step(stepsPerRevolution);
    signal_moteur_pas = 0;
  }
  else if (signal_moteur_pas == 2){
    myStepper.step(-stepsPerRevolution);
    signal_moteur_pas = 0;
  }


  

  long now = millis(); 
  if (now - lastMsg > 2000) { 
		lastMsg = now; 
  }

  Serial.println("ok");
  delay(10000);
}
