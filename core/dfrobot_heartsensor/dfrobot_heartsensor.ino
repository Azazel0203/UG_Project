#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DFRobot_Heartrate.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define debug Serial

#define heartratePin A1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

DFRobot_Heartrate heartrate(DIGITAL_MODE);

void setup() {
  Serial.begin(115200);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    debug.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  display.display(); // Display initialization

  delay(2000);
  display.clearDisplay();   // Clears the screen and buffer
  display.display();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  display.setCursor(0, 0);
  unsigned char rateValue;
  static uint16_t heartrateValueLast = 0;
  uint16_t heartrateValue = heartrate.getValue(heartratePin);
  uint8_t count = heartrate.getCnt();
  display.println(count);
  display.println(heartrateValue);
  display.println(heartrateValueLast);
  heartrateValueLast = heartrateValue;
  Serial.println("Count: " + String(count) + ", h_rate: " + String(heartrateValue) + ", h_rate_last: " + String(heartrateValueLast));
  display.display(); 
  delay(2000);
  display.clearDisplay();
}

