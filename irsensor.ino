const int IR_SENSOR_PIN = 2;
const int LED_PIN = 3;

void setup() {
  Serial.begin(9600);
  pinMode(IR_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  int sensorValue = digitalRead(IR_SENSOR_PIN);
  if (sensorValue != HIGH) {
    Serial.println("IR Sensor: Detected");
    digitalWrite(LED_PIN, HIGH);
  } else {
    Serial.println("IR Sensor: Not Detected");
    digitalWrite(LED_PIN, LOW);
  }
  delay(1000);
}
