#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include <RTClib.h> // Include the RTClib library
#include <U8glib.h>


// Pin definitions
const int buttonPin = 7; // Pushbutton pin
const int ledPin1 =  3; // LED pin 1
const int ledPin2 =  2; // LED pin 2

MAX30105 particleSensor;

unsigned long lastBeat = 0;

char tmp_string[8];  // Temp string to convert numeric values to string before print to OLED display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST);

#define MUX_Address 0x70 // TCA9548A Encoders address

const byte RATE_SIZE = 4;
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
const byte pulseLED = 11;
const byte readLED = 13;
const byte modeSwitchBtn = 0;
#define MODE_HEART_RATE 0
#define MODE_SPO2 1
byte mode = MODE_HEART_RATE;

unsigned long lastPress = 0;
int toggleState = MODE_HEART_RATE;
int lastButtonState = HIGH;
int debounceTime = 20;
int buttonState = HIGH;

File dataFile;

RTC_DS3231 rtc; // Create an instance of the RTC_DS3231 class

void tcaselect(uint8_t i2c_bus) {
    if (i2c_bus > 7) return;
    Wire.beginTransmission(MUX_Address);
    Wire.write(1 << i2c_bus);
    Wire.endTransmission(); 
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);

  Serial.print("Initializing SD card...");
  if (!SD.begin()) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");

  if (!SD.exists("DATA.TXT")) {
    dataFile = SD.open("DATA.TXT", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Timestamp, Data");
      dataFile.close();
      Serial.println("File 'DATA.TXT' created.");
    } else {
      Serial.println("Error creating 'DATA.TXT'");
    }
  } else {
    Serial.println("File 'DATA.TXT' already exists.");
  }

  tcaselect(1);
  u8g.firstPage();
  do {
    u8g.begin();
  } while(u8g.nextPage());
  Serial.println("Start");

  tcaselect(0);
  spo2Setup(); // Initialize SpO2 mode

  // Initialize the real-time clock
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
}

void Displayinit() {
  tcaselect(1);
  u8g.firstPage();
  do {
    u8g.begin();  
  } while(u8g.nextPage());
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

  tcaselect(0);
  int buttonState = digitalRead(buttonPin);
  if ((millis() - lastPress) > debounceTime ) {
     lastPress = millis();
     if (buttonState == LOW && lastButtonState == HIGH) {
      toggleState = !toggleState;
      digitalWrite(ledPin1, toggleState);
      spo2Setup();
      spo2Loop();
      lastButtonState = LOW;
      delay(500);
     }
     if (buttonState == HIGH && lastButtonState == LOW) { 
      heartRateSetup();
      digitalWrite(ledPin2, toggleState);
      heartRateLoop();
      lastButtonState = HIGH;
     }
  }
}

void spo2Setup() {
  Serial.println("SpO2 Mode Setup");
  tcaselect(0);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  byte ledBrightness = 60;
  byte sampleAverage = 8;
  byte ledMode = 2;
  byte sampleRate = 200;
  int pulseWidth = 411;
  int adcRange = 4096;
  particleSensor.setup(ledBrightness, sampleAverage, ledMode, sampleRate, pulseWidth, adcRange);
}

void heartRateSetup() {
  Serial.println("Heart Rate Mode Setup");
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30105 was not found. Please check wiring/power."));
    while (1);
  }
  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x2A);
  particleSensor.setPulseAmplitudeGreen(0);
}

void spo2Loop() {
  Serial.println("IN SPO2 LOOP");
  tcaselect(0);
  digitalWrite(ledPin2, HIGH);
  digitalWrite(ledPin1, LOW);
  unsigned long startTime = millis();
  bufferLength = 100;
  for (byte i = 0; i < bufferLength; i++) {
    while (particleSensor.available() == false)
      particleSensor.check();
    buttonState = digitalRead(buttonPin);
    long currenttime = millis();
    if (buttonState == HIGH && ((currenttime - startTime) > 1000))
      return;
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
    Serial.print(F("red="));
    Serial.print(redBuffer[i], DEC);
    Serial.print(F(", ir="));
    Serial.println(irBuffer[i], DEC);
  }
  maxim_heart_rate_and_oxygen_saturation(irBuffer, bufferLength, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  logData(irBuffer[99], 0, 0, redBuffer[99], irBuffer[99], spo2, validSPO2); // Log Spo2 data
}

void heartRateLoop() {
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
  unsigned long startTime = millis();
  Serial.println("Heart Rate Mode Loop intial");
  while (1) {
    buttonState = digitalRead(buttonPin);
    long currenttime = millis();
    if (buttonState == HIGH && ((currenttime - startTime) > 1000))
      return;
    long irValue = particleSensor.getIR();
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
      logData(irValue, beatsPerMinute, beatAvg, 0, 0, 0, 0); // Log Heart rate data
    }
  }
}

void logData(int32_t irValue, float bpm, int avgBpm, int32_t redValue, int32_t irValueSPO2, int32_t spo2Value, int8_t validSPO2) {
  dataFile = SD.open("DATA.TXT", FILE_WRITE);
  if (dataFile) {
    DateTime now = rtc.now(); // Get the current date and time

    // Print the timestamp in the desired format
    dataFile.print(now.hour(), DEC);
    dataFile.print(':');
    if (now.minute() < 10) dataFile.print('0'); // Add leading zero if minute is less than 10
    dataFile.print(now.minute(), DEC);
    dataFile.print(':');
    if (now.second() < 10) dataFile.print('0'); // Add leading zero if second is less than 10
    dataFile.print(now.second(), DEC);
    dataFile.print(' ');
    dataFile.print(now.day(), DEC);
    dataFile.print(':');
    dataFile.print(now.month(), DEC);
    dataFile.print(':');
    dataFile.print(now.year(), DEC);
    dataFile.print(", ");

    if (mode == MODE_SPO2) {
      dataFile.print("red=");
      dataFile.print(redValue);
      dataFile.print(", ir=");
      dataFile.print(irValueSPO2);
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
