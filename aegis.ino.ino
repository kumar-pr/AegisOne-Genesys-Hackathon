/*
   AEGIS ONE V2 - DUAL CORE + MQTT EDITION
   Core 0 → WiFi + Blynk + MQTT
   Core 1 → Sensors + TinyML
*/

#define BLYNK_TEMPLATE_ID "TMPL3ANQhnNZ_"
#define BLYNK_TEMPLATE_NAME "Aegis One"
#define BLYNK_AUTH_TOKEN "mOckwfjgx6Rxw2c8a-nWugl20UHlCPXq"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include <Aegis_Vibration_ML_inferencing.h>

/************ HARDWARE PINS ************/
#define VIB_PIN 27
#define RELAY_PIN 26
#define BUZZER_PIN 25
#define GAS_PIN 34
#define SERVO_PIN 13

/************ SYSTEM CONFIG ************/
#define GAS_THRESHOLD 2000
#define ML_SAMPLES 100
#define ANOMALY_CONFIDENCE_THRESHOLD 0.85

/************ WIFI + MQTT ************/
char ssid[] = "Aegis_One_Network";
char pass[] = "abcd1234";

const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "aegis/machine01/alert";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/************ GLOBAL STATE ************/
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo ventServo;

volatile bool machineKilled = false;
volatile bool gasEventFlag = false;
volatile bool vibrationEventFlag = false;

float features[ML_SAMPLES];
float latestGas = 0;
float latestAnomaly = 0;

/************ MQTT CONNECT ************/
void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting MQTT...");
    if (mqttClient.connect("AegisMachine01")) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed");
      delay(2000);
    }
  }
}

/************ BLYNK OVERRIDE ************/
BLYNK_WRITE(V3) {
  int overrideState = param.asInt();

  if (overrideState == 1) {
    machineKilled = true;
  } else {
    machineKilled = false;
    ventServo.write(0);
    gasEventFlag = false;
    vibrationEventFlag = false;
  }
}

/************ CORE 0 TASK - CLOUD + MQTT ************/
void CloudTask(void *pvParameters) {

  while (true) {

    if (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass);
      vTaskDelay(2000 / portTICK_PERIOD_MS);
      continue;
    }

    if (!mqttClient.connected()) {
      connectMQTT();
    }

    mqttClient.loop();

    if (!Blynk.connected()) {
      Blynk.connect(1000);
    }

    Blynk.run();

    // Push dashboard values
    Blynk.virtualWrite(V1, latestGas);
    Blynk.virtualWrite(V2, latestAnomaly * 100);

    // Handle gas event
    if (gasEventFlag) {
      mqttClient.publish(mqtt_topic, "Toxic gas detected!");
      gasEventFlag = false;
    }

    // Handle vibration event
    if (vibrationEventFlag) {
      mqttClient.publish(mqtt_topic,
                         "Catastrophic vibration detected!");
      vibrationEventFlag = false;
    }

    // Outputs
    if (machineKilled) {
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      lcd.setCursor(0, 0);
      lcd.print("CRITICAL HALT!  ");
      lcd.setCursor(0, 1);
      lcd.print("Await Reset     ");
    } else {
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);

      lcd.setCursor(0, 0);
      lcd.print("STATUS: ONLINE  ");
      lcd.setCursor(0, 1);
      lcd.print("Gas:");
      lcd.print((int)latestGas);
      lcd.print("     ");
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}

/************ CORE 1 TASK - SENSOR + ML ************/
void SensorTask(void *pvParameters) {

  while (true) {

    if (machineKilled) {
      vTaskDelay(200 / portTICK_PERIOD_MS);
      continue;
    }

    /******** GAS ********/
    latestGas = analogRead(GAS_PIN);

    if (latestGas > GAS_THRESHOLD) {
      ventServo.write(90);
      machineKilled = true;
      gasEventFlag = true;
      continue;
    } else {
      ventServo.write(0);
    }

    /******** ML SAMPLING ********/
    for (int i = 0; i < ML_SAMPLES; i++) {
      features[i] = digitalRead(VIB_PIN);
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    signal_t signal;
    numpy::signal_from_buffer(features, ML_SAMPLES, &signal);

    ei_impulse_result_t result = {0};
    EI_IMPULSE_ERROR res = run_classifier(&signal, &result, false);

    if (res == EI_IMPULSE_OK) {

      latestAnomaly = result.classification[0].value;

      if (latestAnomaly > ANOMALY_CONFIDENCE_THRESHOLD) {
        machineKilled = true;
        vibrationEventFlag = true;
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

/************ SETUP ************/
void setup() {

  Serial.begin(115200);

  pinMode(VIB_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  ventServo.attach(SERVO_PIN);
  ventServo.write(0);

  lcd.init();
  lcd.backlight();
  lcd.print("AEGIS BOOTING");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Blynk.config(BLYNK_AUTH_TOKEN);

  mqttClient.setServer(mqtt_server, 1883);

  xTaskCreatePinnedToCore(
      CloudTask,
      "CloudTask",
      10000,
      NULL,
      1,
      NULL,
      0);

  xTaskCreatePinnedToCore(
      SensorTask,
      "SensorTask",
      20000,
      NULL,
      1,
      NULL,
      1);
}

void loop() {
  // RTOS handles everything
}