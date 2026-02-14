#define BLYNK_TEMPLATE_ID "TMPL3ANQhnNZ_"
#define BLYNK_TEMPLATE_NAME "Aegis One"
#define BLYNK_AUTH_TOKEN "mOckwfjgx6Rxw2c8a-nWugl20UHlCPXq"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

#define GAS_PIN 34
#define VIB_PIN 27
#define GAS_THRESHOLD 2000

char ssid[] = "Aegis_One_Network";
char pass[] = "abcd1234";

volatile bool machineKilled = false;

void IRAM_ATTR handleVibration() {
  machineKilled = true;
}

void setup() {
  Serial.begin();
  
  pinMode(GAS_PIN, INPUT);
  pinMode(VIB_PIN, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(VIB_PIN), handleVibration, RISING);

  Serial.println("Connecting to Wi-Fi and Blynk Cloud...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Aegis One: Edge Safety Node Online.");
}

void loop() {
  Blynk.run();
  
  int gasLevel = analogRead(GAS_PIN);
  
  Blynk.virtualWrite(V1, gasLevel);
  
  if (machineKilled) {
    Serial.println("=========================================");
    Serial.println("CRITICAL: SEVERE VIBRATION! MACHINE HALTED");
    Serial.println("=========================================");
    
    Blynk.logEvent("machine_failure", "High Vibration Detected! Power Cut.");
    Blynk.virtualWrite(V2, 1); 
    
    delay(5000); 
    
    machineKilled = false; 
    Blynk.virtualWrite(V2, 0); 
  }
  
  delay(500); 
}