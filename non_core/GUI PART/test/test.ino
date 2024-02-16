
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
#include "DFRobot_Heartrate.h"
MAX30105 particleSensor;
#define REPORTING_PERIOD_MS 1000
#define debug Serial
uint32_t tsLastReport = 0; 
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
#define heartratePin A1
#define ECG_pin A2
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DFRobot_Heartrate heartrate(DIGITAL_MODE);
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
int32_t heart_rate_df = 0;

long ecg = 0;





char ssid[] = "AzAzEl_1234";
char pass[] = "aadarsh@1234";




/*The `blynk_send` function is responsible for sending data to the Blynk app. 
It checks if the Blynk connection is established and then uses the `Blynk.virtualWrite` function to send the values of 
`irSensorValue`, `spo2Avg`, and `floatAvg` to the corresponding virtual pins (`V0`, `V4`, and `V3`) in the Blynk app. 
If the Blynk connection is not established, it prints a message to the Serial monitor indicating that the data was not sent.*/
void blynk_send(long irSensorValue)
{
  if (Blynk.connected()) {
    Blynk.virtualWrite(V0, irSensorValue);
    Blynk.virtualWrite(V4, spo2Avg);
    Blynk.virtualWrite(V3, floatAvg);

  } else {
    Serial.println("Blynk not connected. Data not sent.");
  }
}

/*The `BlynkSetup()` function is responsible for setting up the Blynk connection and initializing the virtual pins in the Blynk app.*/
void BlynkSetup(){
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Blynk.virtualWrite(V0, 0);
  Blynk.virtualWrite(V3, 0);
  Blynk.virtualWrite(V4, 0);
}

void get_ecg(){
  int heartValue = analogRead(ECG_pin);
  ecg = heartValue;
}


/*The `setup()` function is a standard Arduino function that is called once when the program starts. 
In this function, the following tasks are performed:*/
void setup()
{
  debug.begin(9600);
  Serial.println();
  Serial.println("START");
  if (particleSensor.begin() == false)
  {
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1)
      ;
  }
  Serial.println("STARTED");
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


/*The `clearAllVar()` function is used to reset all the variables used in the program to their initial values. 
This ensures that the variables are clean and ready to be used for the next measurement cycle.*/
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


/*The `print_stuff` function is responsible for printing the values of various variables to the Serial monitor. It takes the `irValue` as a parameter and prints the values of `irValue`, `heartRate`, `beatAvg`, `floatAvg`, and `spo2Avg`. 
This function is used for debugging purposes to monitor the values of these variables during the program execution.*/
void print_stuff(long irValue, long redValue){
  // Serial.print("I");
  Serial.print(irValue);
  Serial.print(",");
  Serial.print(redValue);
  Serial.print(",");
  Serial.print(heartRate);
  Serial.print(",");
  Serial.print(beatAvg);
  Serial.print(",");
  Serial.print(floatAvg);
  Serial.print(",");
  Serial.print(spo2Avg);
  Serial.print(",");
  Serial.print(heart_rate_df);
  Serial.print(",");
  Serial.println(ecg);
}

/*The `heart_rate()` function is responsible for calculating the average heart rate based on the current heart rate value.*/
void heart_rate(){
  rates[rateSpot++] = (byte)heartRate;
  rateSpot %= RATE_SIZE;
  beatAvg = 0;
  for (byte x = 0 ; x < RATE_SIZE ; x++) beatAvg += rates[x];
  beatAvg /= RATE_SIZE;
  floatAvg = float(beatAvg) * (93.00 / 136.00);
}

/*The `oxygen()` function is responsible for calculating the average oxygen saturation (spo2) based on the current spo2 value.*/
void oxygen(){
  rates_OX[rateSpot_OX++] = (byte)spo2;
  rateSpot_OX %= RATE_SIZE_OX;
  spo2Avg = 0;
  for (byte x = 0 ; x < RATE_SIZE_OX ; x++) spo2Avg += rates_OX[x];
  spo2Avg /= RATE_SIZE_OX;
}

/*The `no_finger()` function is responsible for displaying a message on the OLED display when the sensor does not detect a finger.*/
void no_finger(){
  ready_display(10, 24);
  Serial.println("No finger?");
  display.setTextSize(2);
  display.setTextColor(1);
  display.println("NO FINGER");
  display.display();
  delay(1000);
}


void df_robot_heart_rate(){
  uint8_t rateValue;
  heartrate.getValue(heartratePin);
  rateValue = heartrate.getRate();
  if (rateValue)
    heart_rate_df = rateValue;
}


void loop()
{
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
    long redValue = redBuffer[i];
    // for (int i = 0; i < 10; i++)
    df_robot_heart_rate();
    get_ecg();
    print_stuff(irValue, redValue);
    bool f=1;
    int startTimeVal=-1;
    while (irValue < irThresh) {
      no_finger();
      while (particleSensor.available() == false) particleSensor.check();
      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      irValue = irBuffer[i];
      redValue = redBuffer[i];
      if(f)
        {
          f=0;
          startTimeVal= millis();
        }
        else
          if(millis()-startTimeVal>NOFingerTime) goto victor;        
    }
    // Serial.println();
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
      long redValue = redBuffer[i];
      maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
      if (heartRate < 255 && heartRate > 20) heart_rate();
      if (spo2 > 0) oxygen();
      // for (int i = 0; i < 10; i++)
      df_robot_heart_rate();
      get_ecg();
      print_stuff(irValue, redValue);
      bool f=1;
      int startTimeVal=-1;
      while (irValue < irThresh) {
        finger = false;
        no_finger();
        while (particleSensor.available() == false) particleSensor.check();
        redBuffer[i] = particleSensor.getRed();
        irBuffer[i] = particleSensor.getIR();
        irValue = irBuffer[i];
        redValue = redBuffer[i];
        if(f)
        {
          f=0;
          startTimeVal= millis();
        }
        else
          if(millis()-startTimeVal>NOFingerTime) goto victor;        
      }
      // Serial.println();
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



