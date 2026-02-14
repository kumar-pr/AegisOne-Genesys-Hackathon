#define BLYNK_TEMPLATE_ID "TMPL3ANQhnNZ_"
#define BLYNK_TEMPLATE_NAME "Aegis One"
#define BLYNK_AUTH_TOKEN "mOckwfjgx6Rxw2c8a-nWugl20UHlCPXq"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// pinout
#define GAS_PIN 34
#define VIB_PIN 27
#define RELAY_PIN 26
#define BUZZER_PIN 25
#define SERVO_PIN 13

// wifi stuff
char ssid[] = "Aegis_One_Network"; 
char pass[] = "abcd1234";

// globals for state tracking
volatile bool machineKilled = false; 
bool manualOverride = false;         
bool eventLogged = false; // flag so blynk doesnt ban us for spamming

// relay is active low, using these to avoid confusion later
#define RELAY_SAFE HIGH
#define RELAY_TRIGGER LOW
#define GAS_THRESHOLD 2000

Servo exhaustVent;
LiquidCrystal_I2C lcd(0x27, 16, 2); // default i2c address

// interrupt for the vibration sensor (needs to be instant)
void IRAM_ATTR handleVibration() {
  machineKilled = true;
}

// blynk app V3 button (manual kill switch override)
BLYNK_WRITE(V3) {
  int buttonState = param.asInt(); 
  if (buttonState == 1) { 
    manualOverride = true; // cut power
  } else { 
    // reset everything if armed again
    manualOverride = false;
    machineKilled = false; 
    eventLogged = false;   
    lcd.clear(); 
  }
}

void setup() {
  Serial.begin(115200);
  
  // setup pins
  pinMode(GAS_PIN, INPUT);
  pinMode(VIB_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // default states on boot
  digitalWrite(RELAY_PIN, RELAY_SAFE);
  digitalWrite(BUZZER_PIN, LOW);
  exhaustVent.attach(SERVO_PIN);
  exhaustVent.write(0); // keep vents closed at start
  
  // init lcd
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("AEGIS ONE V2");
  lcd.setCursor(0,1);
  lcd.print("Booting Core...");
  
  attachInterrupt(digitalPinToInterrupt(VIB_PIN), handleVibration, RISING);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  lcd.clear();
  lcd.print("SYSTEM ARMED");
  delay(2000);
}

void loop() {
  Blynk.run();
  
  int gasLevel = analogRead(GAS_PIN);
  Blynk.virtualWrite(V1, gasLevel);
  
  // check gas and move servo if too high
  if (gasLevel > GAS_THRESHOLD) {
    exhaustVent.write(90); // open vents
  } else {
    exhaustVent.write(0);  
  }
  
  // emergency cutoff check
  if (machineKilled || manualOverride) {
    digitalWrite(RELAY_PIN, RELAY_TRIGGER);
    digitalWrite(BUZZER_PIN, HIGH);
    Blynk.virtualWrite(V2, 1); // update app UI to red
    
    // show error on screen
    lcd.setCursor(0,0);
    lcd.print("CRITICAL HALT!  ");
    lcd.setCursor(0,1);
    lcd.print("VIB DETECTED    ");
    
    // only log once so blynk doesnt time us out
    if (machineKilled && !manualOverride && !eventLogged) {
      Blynk.logEvent("machine_failure", "High Vibration Detected! Power Cut.");
      eventLogged = true; 
    }
  } else {
    // else machine is safe
    digitalWrite(RELAY_PIN, RELAY_SAFE);
    digitalWrite(BUZZER_PIN, LOW);
    Blynk.virtualWrite(V2, 0); 
    
    // update lcd with current stats
    lcd.setCursor(0,0);
    lcd.print("STATUS: ONLINE  ");
    lcd.setCursor(0,1);
    lcd.print("GAS PPM: ");
    lcd.print(gasLevel);
    lcd.print("    "); // extra spaces to overwrite old text
  }
  
  delay(250); 
}