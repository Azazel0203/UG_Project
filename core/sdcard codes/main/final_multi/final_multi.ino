// #include <SD.h>
// #include <SPI.h>
// #include <Wire.h>
// // #include "MAX30105.h"
// // #include "spo2_algorithm.h"
// // #include "heartRate.h"
// // #include <U8glib.h>

// // // void tcaseselect(uint8_t i2c_bus);
// // // void Displayinit();
// // // void spo2Setup();
// // // void heartRateSetup();
// // // void spo2Loop();
// // // void heartRateLoop();
// // // void logData(int mody, int32_t irValue, float bpm, int avgBpm, int32_t redValue, int32_t irValueSPO2, int32_t heartrate, int32_t validHeartRate, int32_t spo2Value, int8_t validSPO2);

// // MAX30105 spo2sensor;
// // MAX30105 heartratesensor;

// // unsigned long lastBeat = 0;

// // U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST);

// // #define MUX_Address 0x70 // TCA9548A Encoders address

// // const byte RATE_SIZE = 4;
// // byte rates[RATE_SIZE];
// // byte rateSpot = 0;
// // float beatsPerMinute;
// // int beatAvg;
// // uint32_t irBuffer[100];
// // uint32_t redBuffer[100];
// // int32_t bufferLength;
// // int32_t spo2;
// // int8_t validSPO2;
// // int32_t heartRate;
// // int8_t validHeartRate;
// // const byte modeSwitchBtn = 0;
// // #define MODE_HEART_RATE 0
// // #define MODE_SPO2 1
// // byte mode = MODE_HEART_RATE;

// // unsigned long lastPress = 0;
// // int toggleState = MODE_HEART_RATE;
// // int lastButtonState = HIGH;
// // int debounceTime = 20;
// // int buttonState = HIGH;

// int cs = 10;

// File dataFile;

// // unsigned long spo2PreviousMillis = 0;
// // const unsigned long spo2Interval = 1000; // 1 second

// // unsigned long heartRatePreviousMillis = 0;
// // const unsigned long heartRateInterval = 1000; // 1 second

// // void tcaseselect(uint8_t i2c_bus) {
// //     if (i2c_bus > 7) return;
// //     Wire.beginTransmission(MUX_Address);
// //     Wire.write(1 << i2c_bus);
// //     Wire.endTransmission(); 
// // }

// // void Displayinit(){
// //   tcaseselect(0); //DISPLAY
// //   u8g.firstPage();
// //   do{
// //     u8g.begin();
// //   } while (u8g.nextPage());
// // }

// // void spo2Setup(){
// //   Serial.println("SpO2 Mode Setup");
// //   tcaseselect(1);
// //   if (!spo2sensor.begin(Wire, I2C_SPEED_FAST)) {
// //     Serial.println(F("MAX30105 was not found. Please check wiring/power."));
// //     while (1);
// //   }
// //   byte ledBrightness = 60;
// //   byte sampleAverage = 8;
// //   byte ledMode = 2;
// //   byte sampleRate = 200;
// //   int pulseWidth = 411;
// //   int adcRange = 4096;
// //   spo2sensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
// // }

// // void heartRateSetup(){
// //   Serial.println("Heart Rate Mode Setup");
// //   tcaseselect(2);
// //   if (!heartratesensor.begin(Wire, I2C_SPEED_FAST)) {
// //     Serial.println(F("MAX30105 was not found. Please check wiring/power."));
// //     while (1);
// //   }
// //   heartratesensor.setup();
// //   heartratesensor.setPulseAmplitudeRed(0x2A);
// //   heartratesensor.setPulseAmplitudeGreen(0);
// // }

// void setup() {
//   Serial.begin(115200);

//   Serial.println("NOOOOO");
//   // Serial.println("Intializing SD Card..."); // MEMORY CARD
//   if (!SD.begin(cs)){
//     Serial.println("Initialization failed!");
//     while(1);
//   }
//   delay(1000);
//   // Serial.println("Initialization Done.");

//   // if (!SD.exists("DAT.TXT")){
//   //   dataFile = SD.open("DAT.TXT", FILE_WRITE);
//   //   if (dataFile){
//   //     dataFile.println("TimeStamp, DATA");
//   //     dataFile.close();
//   //     Serial.println("File 'DAT.TXT' created.");
//   //   }
//   //   else{
//   //     Serial.println("Error Creating 'DAT.TXT'");
//   //   }
//   // }
//   // else{
//   //   Serial.println("File 'DAT.TXT' already exits");
//   // }

//   // tcaseselect(0); //DISPLAY
//   // Displayinit();

//   // Serial.println("Start");

//   // spo2Setup(); // SPO2

//   // heartRateSetup(); //Heart Rate
// }

// void loop(){
//   Serial.println("DONE");
//   Serial.println("ISSSS");
//   Serial.println("NOTTT");
//   delay(1000);
//   // unsigned long currentMillis = millis();

//   // // Run the spo2Loop() function every spo2Interval milliseconds
//   // if (currentMillis - spo2PreviousMillis >= spo2Interval) {
//   //   spo2Loop();
//   //   spo2PreviousMillis = currentMillis;
//   // }

//   // // Run the heartRateLoop() function every heartRateInterval milliseconds
//   // if (currentMillis - heartRatePreviousMillis >= heartRateInterval) {
//   //   heartRateLoop();
//   //   heartRatePreviousMillis = currentMillis;
//   // }

//   // Displayinit();

//   // tcaseselect(0);
//   // u8g.firstPage();
//   // do {
//   //   u8g.setFont(u8g_font_ncenB10);
//   //   u8g.drawStr(0, 0, "Taking Reading");
//   //   // u8g.drawStr(0, 50, "PPG -> ");
//   //   // u8g.setFont(u8g_font_ncenB18);
//   //   // u8g.drawStr(25, 50, tmp_string);
//   // } while( u8g.nextPage() );
// }

// // void spo2Loop(){
// //   Serial.println("SPO2 LOOP");
// //   // Code for SpO2 measurement goes here
// // }

// // void heartRateLoop(){
// //   Serial.println("Heart Rate Loop");
// //   // Code for heart rate measurement goes here
// // }


void setup(){
  Serial.begin(115200);
  for (int i = 0; i < 100; i++)
    Serial.println("IN setup");
}
void loop(){
  Serial.println("IN LOOP");
}