#define BLYNK_TEMPLATE_ID "TMPL3oNlhfgFU"
#define BLYNK_TEMPLATE_NAME "Heartrate and Spo2"
#define BLYNK_AUTH_TOKEN "Iy8nLl5vp1STRfBY-Q8FJohdLcbMTX1p"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"
#include "heartRate.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "spo2_algorithm.h"
// #include "DFRobot_Heartrate.h"
#include "HeartSpeed.h"
MAX30105 particleSensor;
#define REPORTING_PERIOD_MS 1000
#define debug Serial
uint32_t tsLastReport = 0; 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
// #define heartratePin A1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// DFRobot_Heartrate heartrate(DIGITAL_MODE);
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
char ssid[] = "AzAzEl_1234";
char pass[] = "aadarsh@1234";
static uint16_t df_heart_rate;
HeartSpeed heartspeed(A1);    

void mycb(uint8_t rawData, int value)
{
  df_heart_rate = value;
  if(rawData){
    Serial.println(value);
  }else{
    Serial.print("HeartRate Value = "); Serial.println(value);
  }
}

static uint16_t df_heartrate(){
  // unsigned char rateValue;
  // static uint16_t heartrateValueLast = 0;
  // uint16_t heartrateValue = heartrate.getValue(heartratePin);
  // uint8_t count = heartrate.getCnt();

  // rateValue = heartrate.getRate();
  // heartrateValueLast = heartrateValue;
  // // delay(20);
  // return heartrateValueLast;
  heartspeed.recv(&heartspeed);

}


void blynk_send(long irSensorValue){
  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, irSensorValue);
    Blynk.virtualWrite(V4, spo2Avg);
    Blynk.virtualWrite(V3, floatAvg);
    Blynk.virtualWrite(V2, df_heart_rate);

  } else {
    Serial.println("Blynk not connected. Data not sent.");
  }
}
void BlynkSetup(){
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V4, 0);
  Blynk.virtualWrite(V2, 0);
}
void setup(){
  debug.begin(9600);
  heartspeed.setCB(mycb);    ///Callback function.
  heartspeed.begin();///The pulse test.
  if (particleSensor.begin() == false)
  {
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
  {
    debug.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  BlynkSetup();
  ready_display(0, 0);
  lastBeat = millis(); 
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  byte ledBrightness = 0x5A;
  byte sampleAverage = 16;
  byte ledMode = 3;
  int sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 8192;
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
  
}
void ready_display(int x, int y){
  display.clearDisplay();
  display.setCursor(x, y);
}
void clearAllVar() {
  tsLastReport = 0;  //stores the time the last update was sent to the blynk app
  // Sensor Variables
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
  for (int i = 0; i < RATE_SIZE_OX ; i++) rates_OX[i] = spo2;
  rateSpot_OX = 0;
  spo2Avg = 0.0;
  preva = 50;
  a = 0;
  finger = true;
}
void print_stuff(long irValue){
  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(heartRate);
  Serial.print(", Avg BPM=");
  Serial.print(beatAvg);
  Serial.print(", Cal. BPM=");
  Serial.print(floatAvg);
  Serial.print(", Cal. spO2=");
  Serial.print(spo2Avg);
  Serial.print(", Df_heartrate = ");
  Serial.print(df_heart_rate);
}
void heart_rate(){
  rates[rateSpot++] = (byte)heartRate;
  rateSpot %= RATE_SIZE;
  beatAvg = 0;
  for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
  beatAvg /= RATE_SIZE;
  floatAvg = float(beatAvg) * (93.00 / 136.00);
}
void oxygen(){
  rates_OX[rateSpot_OX++] = (byte)spo2;
  rateSpot_OX %= RATE_SIZE_OX;
  spo2Avg = 0;
  for (byte x = 0 ; x < RATE_SIZE_OX ; x++) spo2Avg += rates_OX[x];
  spo2Avg /= RATE_SIZE_OX;
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
void loop(){
victor:
  clearAllVar();
  Blynk.run();
  bufferLength = 100;
  long initialTime = millis();
  long waitingTime = 6000;
  for (byte i = 0 ; i < bufferLength ; i++)
  {
    while (particleSensor.available() == false) particleSensor.check();
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    long irValue = irBuffer[i];
    print_stuff(irValue);
    bool f=1;
    int startTimeVal=-1;
    while (irValue < irThresh) {
      no_finger();
      while (particleSensor.available() == false) particleSensor.check();
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      irValue = irBuffer[i];
      if(f)
        {
          f=0;
          startTimeVal= millis();
        }
        else
          if(millis()-startTimeVal>NOFingerTime) goto victor;        
    }
    Serial.println();
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
        // HeartAnimation();
      }
      preva = a;
    }
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  if (heartRate < 255 && heartRate > 20) heart_rate();
  if (spo2 > 0) oxygen();
  while (1)
  {
    finger = true;
    for (byte i = 25; i < 100; i++)
    {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
    for (byte i = 75; i < 100; i++)
    {
      redBuffer[i - 75] = redBuffer[i - 50];
      irBuffer[i - 75] = irBuffer[i - 50];
      while (particleSensor.available() == false) particleSensor.check();
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      long irValue = irBuffer[i];
      maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
      if (heartRate < 255 && heartRate > 20) heart_rate();
      if (spo2 > 0) oxygen();
      print_stuff(irValue);
      bool f=1;
      int startTimeVal=-1;
      while (irValue < irThresh) {
        finger = false;
        no_finger();
        while (particleSensor.available() == false) particleSensor.check();
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        irValue = irBuffer[i];
        if(f)
        {
          f=0;
          startTimeVal= millis();
        }
        else
          if(millis()-startTimeVal>NOFingerTime) goto victor;        
      }
      Serial.println();
      particleSensor.nextSample();
      if (millis() - tsLastReport > REPORTING_PERIOD_MS)
      {
        // Blynk.virtualWrite(V0, !finger);
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
            // HeartAnimation();
          }
          preva = a;
        } 
        else {
          df_heart_rate = df_heartrate();
          blynk_send(irValue);
          if (!finger) no_finger();
//            goto victor;
          else {

            
            if (finger && int(spo2Avg) < 90) {
              Blynk.logEvent("lowspo2", "Your SPO2 is below 90%");
            }
            display.clearDisplay();
            display.setTextSize(2);
            display.setTextColor(1);
            display.setCursor(0, 0);
            display.println("BPM:");
            display.setTextSize(2);
            display.setTextColor(1);
            display.setCursor(0, 15);
            display.println(int(floatAvg));
            display.setTextSize(2);
            display.setTextColor(1);
            display.setCursor(0, 31);
            display.println("SPO2:");
            display.setTextSize(2);
            display.setTextColor(1);
            display.setCursor(0, 46);
            display.println(int(spo2Avg));
            display.display();
            // HeartAnimation();
          }
        }
        tsLastReport = millis();
      }
    }
  }
}



