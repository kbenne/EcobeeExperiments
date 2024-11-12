#include <Arduino.h>

void setup() {
  // Adjust baudrate here
  Serial.begin(115200);
  Serial1.begin(115200);
  //Wait until USB CDC port connects
  while (!Serial) {}
}

void loop() {
  //Copy byte incoming via TTL serial
  if (Serial1.available() > 0) {
    Serial.write(Serial1.read());
  }
  //Copy byte incoming via CDC serial
  if (Serial.available() > 0) {
    Serial1.write(Serial.read());
  }
}