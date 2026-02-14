#define GAS_SENSOR_PIN 34
#define BAUD_RATE 115200
#define GAS_WARNING_THRESHOLD 2000 

void setup() {
  Serial.begin(BAUD_RATE);
  pinMode(GAS_SENSOR_PIN, INPUT);
  delay(1000); 
  Serial.println("Aegis One: Environmental Monitoring Initialized...");
}

void loop() {
  int rawGasLevel = analogRead(GAS_SENSOR_PIN);
  
  Serial.print("Gas_Level:");
  Serial.print(rawGasLevel);
  Serial.print(",");
  Serial.print("Threshold:");
  Serial.println(GAS_WARNING_THRESHOLD);

  if (rawGasLevel > GAS_WARNING_THRESHOLD) {
    Serial.println("WARNING: TOXIC GAS DETECTED");
  }

  delay(500); 
}