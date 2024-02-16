#include <Wire.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("\nI2C Scanner");

  Wire.begin();

  for (uint8_t i = 8; i < 120; i++) {
    Wire.beginTransmission(i);
    if (Wire.endTransmission() == 0) {
      Serial.print("Found address: 0x");
      Serial.println(i, HEX);
    }
  }
}

void loop() {}
