#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "DFRobot_Heartrate.h"
#include "DFRobot_BloodOxygen_S.h"

MAX30105 particleSensor;

#define I2C_ADDRESS    0x57
DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);


const byte RATE_SIZE = 4; 
byte rates[RATE_SIZE];      
byte rateSpot = 0;
long lastBeat = 0; 
float beatsPerMinute;
int beatAvg = 0;
float floatAvg;
bool flag = false;
const byte RATE_SIZE_OX = 100;
byte rates_OX[RATE_SIZE_OX];
byte rateSpot_OX = 0;
float spo2Avg;
int32_t bufferLength;
int32_t spo2 = 0; 
int8_t validSPO2; 
int32_t heartRate; 
int8_t validHeartRate; 
uint32_t irBuffer[100] = {0}; 
uint32_t redBuffer[100] = {0};  
const int irThresh = 50000;
long preva = 50;
long a;
long NOFingerTime= 5000;
bool finger = true;
uint32_t tsLastReport = 0; 
long df_heart_rate = 0;


#define debug Serial
#define REPORTING_PERIOD_MS 1000



#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define heartratePin A1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DFRobot_Heartrate heartrate(DIGITAL_MODE);

void ready_display(int x, int y) {
  display.clearDisplay();
  display.setCursor(x, y);
}

void setup() {
  while (false == MAX30102.begin())
  {
    Serial.println("init fail!");
    delay(1000);
  }
  Serial.println("init success!");
  Serial.println("start measuring...");
  MAX30102.sensorStartCollect();

  debug.begin(115200);
  if (!particleSensor.begin()) {
    debug.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  byte ledBrightness = 0x5A;
  byte sampleAverage = 16;
  byte ledMode = 3;
  int sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 8192;
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    debug.println(F("SSD1306 allocation failed"));
    while (1);
  }
  ready_display(0, 0);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  lastBeat = millis();
}

void no_finger(){
  ready_display(10, 24);
  Serial.println(" No finger?");
  display.setTextSize(2);
  display.setTextColor(1);
  display.println("NO FINGER");
  display.display();
  delay(1000);
}

void clearAllVar() {
  for (int i = 0; i < 100; i++) {
    irBuffer[i] = 0;
    redBuffer[i] = 0;
  }
  spo2 = 98;
  validSPO2 = 0;
  heartRate = 125;
  validHeartRate = 0;
  beatsPerMinute = 0;
  beatAvg = 0;
  floatAvg = 0.0;
  for (int i = 0; i < RATE_SIZE; i++) rates[i] = heartRate;
  rateSpot = 0;
  lastBeat = 0;
  spo2Avg = 0.0;
  preva = 50;
  a = 0;
  finger = true;
}


void heart_rate(){
  rates[rateSpot++] = (byte)heartRate;
  rateSpot %= RATE_SIZE;
  beatAvg = 0;
  for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
  beatAvg /= RATE_SIZE;
  floatAvg = float(beatAvg) * (93.00 / 136.00);
}


void df_heart(){
  uint8_t rateValue;
  heartrate.getValue(heartratePin); ///< A1 foot sampled values
  rateValue = heartrate.getRate(); ///< Get heart rate value 
  if(rateValue)  {
    df_heart_rate = rateValue;
  }
}
void oxygen(){
  rates_OX[rateSpot_OX++] = (byte)spo2;
  rateSpot_OX %= RATE_SIZE_OX;
  spo2Avg = 0;
  for (byte x = 0 ; x < RATE_SIZE_OX ; x++) spo2Avg += rates_OX[x];
  spo2Avg /= RATE_SIZE_OX;
}

void loop() {
  victor:
  clearAllVar();
  int bufferLength = 100;
  long initialTime = millis();
  long waitingTime = 6000;
  
  for (byte i = 0 ; i < bufferLength ; i++) {
    while (!particleSensor.available()) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    long irValue = irBuffer[i];
    bool f = true;
    int startTimeVal = -1;
    while (irValue < irThresh) {
      no_finger();
      while (!particleSensor.available()) particleSensor.check();
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      irValue = irBuffer[i];
      if (f) {
        f = false;
        startTimeVal = millis();
      } else if (millis() - startTimeVal > NOFingerTime) {
        goto victor;
      }
    }
    particleSensor.nextSample();
    if (millis() - initialTime < waitingTime) {
      a = waitingTime / 1000 - (millis() - initialTime) / 1000;
      if (preva != a) {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(1);
        display.setCursor(0, 0);
        display.println("Please wait...");
        display.setCursor(0, 36);
        display.println("Time Left: ");
        display.setCursor(0, 47);
        display.setTextSize(2);
        display.print(a);
        display.display();
      }
      preva = a;
    }
  }
  
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  if (heartRate < 255 && heartRate > 20) heart_rate();
  if (spo2 > 0) oxygen();
  
  while (1) {
    finger = true;
    for (byte i = 25; i < 100; i++) {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
      df_heart();
    }
    for (byte i = 75; i < 100; i++) {
      df_heart();
      MAX30102.getHeartbeatSPO2();

      redBuffer[i - 75] = redBuffer[i - 50];
      irBuffer[i - 75] = irBuffer[i - 50];
      while (!particleSensor.available()) particleSensor.check();
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      long irValue = irBuffer[i];
      maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
      if (heartRate < 255 && heartRate > 20) heart_rate();
      if (spo2 > 0) oxygen();
      bool f = true;
      int startTimeVal = -1;
      while (irValue < irThresh) {
        finger = false;
        no_finger();
        df_heart();
        while (!particleSensor.available()) particleSensor.check();
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        irValue = irBuffer[i];
        if (f) {
          f = false;
          startTimeVal = millis();
        } else if (millis() - startTimeVal > NOFingerTime) {
          goto victor;
        }
      }
      particleSensor.nextSample();
      if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
        if (millis() - initialTime < waitingTime) {
          a = waitingTime / 1000 - (millis() - initialTime) / 1000;
          if (preva != a) {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(1);
            display.setCursor(0, 0);
            display.println("Please wait...");
            display.setCursor(0, 36);
            display.println("Time Left: ");
            display.setCursor(0, 47);
            display.setTextSize(2);
            display.print(a);
            display.display();
          }
          preva = a;
        } else {
          display.clearDisplay();
          display.setTextSize(1);
          display.setTextColor(1);
          display.setCursor(0, 0);
          display.print("IR:");
          display.println(irBuffer[i]);
          display.print("BPM:");
          display.println(int(floatAvg));
          display.print("SpO2:");
          display.println(int(spo2Avg));
          display.print("df_heart:");
          display.println(df_heart_rate);
          display.print("df_heart_oxy.spo2");
          display.println(MAX30102._sHeartbeatSPO2.SPO2);
          display.print("df_heart_oxy.heartbeat");
          display.println(MAX30102._sHeartbeatSPO2.Heartbeat);
          display.display();
        }
        tsLastReport = millis();
      }
    }
  }
}

