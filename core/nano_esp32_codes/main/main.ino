#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include <U8glib.h>
#include <SD.h>



/*
Performs the loop for SpO2 (blood oxygen saturation) sensor readings.
This function continuously reads data from the SpO2 sensor and logs it.
It first selects the appropriate I2C bus for the SpO2 sensor, then reads sensor
data and calculates SpO2 and heart rate values using the spo2_algorithm library.
The calculated values are then logged to the SD card using the logData function.
The loop runs indefinitely until interrupted by a button press.
*/


const int cs = 10;

void tcaselect(uint8_t i2c_bus);
void Displayinit();
void spo2Setup();
void heartRateSetup();
void spo2Loop();
void heartRateLoop();
void logData(int mody, int32_t irValue, float bpm, int avgBpm, int32_t redValue, int32_t irValueSPO2, int32_t heartrate, int32_t validHeartRate, int32_t spo2Value, int8_t validSPO2);


MAX30105 particleSensor1; // heartrate - tcase -> 0
MAX30105 particleSensor2; // SPO2 - tcase -> 2
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST);
#define MUX_Address 0x70 // TCA9548A

byte rates[RATE_SIZE];
byte rateSpot = 0;
float beatsPerMinute;
int beatAvg;
uint32_t irBuffer[100];
uint32_t redBuffer[100];
int32_t bufferLength;
int32_t spo2;
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;
File dataFile;


void Displayinit() {
  tcaselect(1);
  u8g.firstPage();
  do {
    u8g.begin();  
  } while(u8g.nextPage());
}




void setup() {
  Serial.begin(115200);

  Serial.print("Initializing SD card...");
  if (!SD.begin(cs)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");

  if (!SD.exists("DAT.TXT")) {
    dataFile = SD.open("DAT.TXT", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Timestamp, Data");
      dataFile.close();
      Serial.println("File 'DAT.TXT' created.");
    } else {
      Serial.println("Error creating 'DAT.TXT'");
    }
  } else {
    Serial.println("File 'DAT.TXT' already exists.");
  }

  tcaselect(1);
  u8g.firstPage();
  do {
    u8g.begin();
  } while(u8g.nextPage());
  Serial.println("Start");

  spo2Setup(); // Initialize SpO2 mode
  heartRateSetup();
}


void loop() {
  Displayinit();

  tcaselect(1);
  u8g.firstPage();
  do {
    /******** Display Something *********/
    u8g.setFont(u8g_font_ncenB10);
    u8g.drawStr(0, 13, "PPG -> ");
    u8g.drawStr(0, 50, "PPG -> ");
    // itoa(i, tmp_string, 10);  
    u8g.setFont(u8g_font_ncenB18);
    u8g.drawStr(25, 50, tmp_string);
    /************************************/
  } while( u8g.nextPage() );



  // tcaselect(0);
  // int buttonState = digitalRead(buttonPin);
  // if ((millis() - lastPress) > debounceTime ) {
  //    lastPress = millis();
  //    if (buttonState == LOW && lastButtonState == HIGH) {
  //     toggleState = !toggleState;
  //     digitalWrite(ledPin1, toggleState);
  //     spo2Setup();
  //     spo2Loop();
  //     lastButtonState = LOW;
  //     delay(500);
  //    }
  //    if (buttonState == HIGH && lastButtonState == LOW) { 
  //     heartRateSetup();
  //     digitalWrite(ledPin2, toggleState);
  //     heartRateLoop();
  //     lastButtonState = HIGH;
  //    }
  // }
}

void tcaselect(uint8_t i2c_bus) {
    if (i2c_bus > 7) return;
    Wire.beginTransmission(MUX_Address);
    Wire.write(1 << i2c_bus);
    Wire.endTransmission(); 
}

void spo2Setup() {
  tcaselect(2);
  Serial.println("SpO2 Mode Setup");
  tcaselect(0);
  if (!particleSensor2.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  byte ledBrightness = 60;
  byte sampleAverage = 8;
  byte ledMode = 2;
  byte sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 4096;
  particleSensor2.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void spo2Loop() {
  tcaselect(2);
  Serial.println("IN SPO2 LOOP");
  tcaselect(0);
  digitalWrite(ledPin2, HIGH);
  digitalWrite(ledPin1, LOW);
  unsigned long startTime = millis();
  bufferLength = 100;
  for (byte i = 0; i < bufferLength; i++) {
    while (particleSensor2.available() == false)
      particleSensor2.check();
    buttonState = digitalRead(buttonPin);
    long currenttime = millis();
    if (buttonState == HIGH && ((currenttime - startTime) > 1000))
      return;
    redBuffer[i] = particleSensor2.getRed();
    irBuffer[i] = particleSensor2.getIR();
    particleSensor2.nextSample();
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);

  while (1) {
   buttonState = digitalRead(buttonPin);
   long currenttime = millis();
   if (buttonState == HIGH && ((currenttime - startTime) > 1000))
     return;
   for (byte i = 25; i < 100; i++) {
     redBuffer[i - 25] = redBuffer[i];
     irBuffer[i - 25] = irBuffer[i];
   }
   for (byte i = 75; i < 100; i++) {
    while (particleSensor2.available() == false)
      particleSensor2.check();
    digitalWrite(readLED, !digitalRead(readLED));
    redBuffer[i] = particleSensor2.getRed();
    irBuffer[i] = particleSensor2.getIR();
    particleSensor2.nextSample();
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.print(irBuffer[i], DEC);
    Serial.print(F(", HR="));
    Serial.print(heartRate, DEC);
    Serial.print(F(", HRvalid="));
    Serial.print(validHeartRate, DEC);
    Serial.print(F(", SPO2="));
    Serial.print(spo2, DEC);
    Serial.print(F(", SPO2Valid="));
    Serial.println(validSPO2, DEC);
    Serial.println("SpO2 Mode Loop");
    logData(0, 0, 0, 0, redBuffer[i], irBuffer[99], heartRate, validHeartRate, spo2, validSPO2); // Log Spo2 data
   }
 }
}

void heartRateSetup() {
  tcaselect(0);
  Serial.println("Heart Rate Mode Setup");
  if (!particleSensor1.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  particleSensor1.setup();
  particleSensor1.setPulseAmplitudeRed(0x2A);
  particleSensor1.setPulseAmplitudeGreen(0);
}

void heartRateLoop() {
  tcaselect(0);
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
  unsigned long startTime = millis();
  Serial.println("Heart Rate Mode Loop intial");
  while (1) {
    buttonState = digitalRead(buttonPin);
    long currenttime = millis();
    if (buttonState == HIGH && ((currenttime - startTime) > 1000))
      return;
    long irValue = particleSensor1.getIR();
    if (irValue < 50000)
      Serial.println(" No finger?");
    else{  
      if (checkForBeat(irValue) == true) {
        long delta = millis() - lastBeat;
        lastBeat = millis();
        beatsPerMinute = 60 / (delta / 1000.0);
        Serial.println(beatsPerMinute);
        if (beatsPerMinute < 255 && beatsPerMinute > 20) {
          rates[rateSpot++] = (byte)beatsPerMinute;
          rateSpot %= RATE_SIZE;
          beatAvg = 50;
          for (byte x = 0 ; x < RATE_SIZE ; x++)
            beatAvg += rates[x];
          beatAvg /= (RATE_SIZE+1);
        }
      }
      Serial.print("IR=");
      Serial.print(irValue);
      Serial.print(", BPM=");
      Serial.print(beatsPerMinute);
      Serial.print(", Avg BPM=");
      Serial.print(beatAvg);
      Serial.println("Heart Rate Mode Loop");
      logData(1, irValue, beatsPerMinute, beatAvg, 0, 0, 0, 0, 0, 0); // Log Heart rate data
    }
  }
}

void logData(int mody, int32_t irValue, float bpm, int avgBpm, int32_t redValue, int32_t irValueSPO2, int32_t heartrate, int32_t validHeartRate, int32_t spo2Value, int8_t validSPO2) {
  dataFile = SD.open("DAT.TXT", FILE_WRITE);
  if (dataFile) {
    unsigned long currentTime = millis();
    // Convert millis to days, hours, minutes, and seconds
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    unsigned long days = hours / 24;

    // Calculate the remaining hours, minutes, and seconds
    hours = hours % 24;
    minutes = minutes % 60;
    seconds = seconds % 60;

    // Print the timestamp in the desired format
    dataFile.print(hours);
    dataFile.print(':');
    if (minutes < 10) dataFile.print('0');
    dataFile.print(minutes);
    dataFile.print(':');
    if (seconds < 10) dataFile.print('0');
    dataFile.print(seconds);
    dataFile.print(' ');
    dataFile.print(days);
    dataFile.print(' ');
    dataFile.print("00:00:00"); // For month and year, we'll just print placeholders
    dataFile.print(", ");

    if (mody == 0) {
      dataFile.print("red=");
      dataFile.print(redValue);
      dataFile.print(", ir=");
      dataFile.print(irValueSPO2);
      dataFile.print(", heartrate=");
      dataFile.print(heartrate);
      dataFile.print(", validheartRate=");
      dataFile.print(validHeartRate);
      dataFile.print(", SPO2=");
      dataFile.print(spo2Value);
      dataFile.print(", SPO2Valid=");
      dataFile.println(validSPO2);
    } else {
      dataFile.print("IR=");
      dataFile.print(irValue);
      dataFile.print(", BPM=");
      dataFile.print(bpm);
      dataFile.print(", Avg BPM=");
      dataFile.println(avgBpm);
    }
    dataFile.close();
  } else {
    Serial.println("Error opening file for writing!");
  }
}





