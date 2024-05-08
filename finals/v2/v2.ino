#include <Wire.h>
#include "DFRobot_BloodOxygen_S.h"
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"


// #include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// #define MUX_Address 0x70 

MAX30105 particleSensor;
// #define I2C_ADDRESS    0x57
// DFRobot_BloodOxygen_S_I2C df_sensor(&Wire, I2C_ADDRESS);



// float df_spo2;
// float df_heartrate;


// NECESSARY
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;

uint32_t irBuffer[100]; //infrared LED sensor data
uint32_t redBuffer[100];  //red LED sensor data

int32_t bufferLength; //data length
int32_t spo2; //SPO2 value
int8_t validSPO2; //indicator to show if the SPO2 calculation is valid
int32_t heartRate; //heart rate value
int8_t validHeartRate; //indicator to show if the heart rate calculation is valid

//MY
long ppg_irvalue;
float ppg_beatsperminute;
int ppg_beatavg;




void get_heartrate_ppg(){
  // tcaselect(0);
  ppg_irvalue = particleSensor.getIR();
  if (checkForBeat(ppg_irvalue) == true)
  {
    long delta = millis() - lastBeat;
    lastBeat = millis();
    ppg_beatsperminute = 60 / (delta / 1000.0);
    if (ppg_beatsperminute < 255 && ppg_beatsperminute > 20)
    {
      rates[rateSpot++] = (byte)ppg_beatsperminute;
      rateSpot %= RATE_SIZE;
      ppg_beatavg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        ppg_beatavg += rates[x];
      ppg_beatavg /= RATE_SIZE;
    }
  }
}

void setup() {

  Serial.begin(115200);
  Serial.println("begin");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  display.clearDisplay();

  // tcaselect(0);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  // tcaselect(0);
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop() {
  display.clearDisplay();
  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // White text
  display.setCursor(0, 0);     // Start at top-left corner
  get_heartrate_ppg();





  Serial.print(" || PPG -> ");
  Serial.print(ppg_irvalue);
  Serial.print(" | ");
  Serial.print(ppg_beatsperminute);
  Serial.print(" | ");
  Serial.println(ppg_beatavg);
  display.print("IR -> ");
  display.println(ppg_irvalue);
  display.print("BPM -> ");
  display.print(ppg_beatsperminute);
  display.print("BeatAvg -> ");
  display.print(ppg_beatavg);
  display.display();
}


