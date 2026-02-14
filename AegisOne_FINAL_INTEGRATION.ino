#define BLYNK_TEMPLATE_ID "TMPL3ANQhnNZ_"
#define BLYNK_TEMPLATE_NAME "Aegis One"
#define BLYNK_AUTH_TOKEN "mOckwfjgx6Rxw2c8a-nWugl20UHlCPXq"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include <PubSubClient.h>

#define GAS_PIN 34
#define VIB_PIN 27

char ssid[] = "Aegis_One_Network"; 
char pass[] = "abcd1234";        
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

String tsUrl = "http://api.thingspeak.com/update?api_key=XNBTMOD5VJ18BQYT&field1=";

volatile bool machineKilled = false;
volatile unsigned long lastVibTime = 0; 
unsigned long lastTsTime = 0;

void IRAM_ATTR handleVibration() {
  unsigned long currentTime = millis();
  if ((currentTime - lastVibTime) > 5000) {
    machineKilled = true;
    lastVibTime = currentTime;
  }
}

void setup() {
  Serial.begin(9600); 
  pinMode(GAS_PIN, INPUT);
  pinMode(VIB_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(VIB_PIN), handleVibration, RISING);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  mqttClient.setServer(mqtt_server, 1883);
  Serial.println("Aegis One: Final Integrated System Online.");
}

void reconnectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("AegisOne_ESP32_Final")) {
      Serial.println("Connected to Telegram MQTT Broker");
    } else {
      delay(2000);
    }
  }
}

void loop() {
  Blynk.run();
  if (!mqttClient.connected()) { reconnectMQTT(); }
  mqttClient.loop();

  int gasLevel = analogRead(GAS_PIN);
  Blynk.virtualWrite(V1, gasLevel);
  
  if (millis() - lastTsTime > 15000) {
    if(WiFi.status() == WL_CONNECTED){
      HTTPClient http;
      http.begin(tsUrl + String(gasLevel));
      http.GET();
      http.end();
      Serial.println("ThingSpeak Logged: " + String(gasLevel));
    }
    lastTsTime = millis();
  }
  
  if (machineKilled) {
    Serial.println("=========================================");
    Serial.println("EMERGENCY: KILL SWITCH TRIGGERED");
    Serial.println("=========================================");
    
    mqttClient.publish("aegis/machine01/alert", "🚨 Aegis One ALERT: Severe Vibration Detected. Machine Halted.");
    
    Blynk.logEvent("machine_failure", "High Vibration Detected! Power Cut.");
    Blynk.virtualWrite(V2, 1); 
    
    delay(5000); 
    machineKilled = false; 
    Blynk.virtualWrite(V2, 0); 
  }
  delay(500); 
}