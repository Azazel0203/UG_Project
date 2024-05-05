#include <SD.h>
#include <SPI.h>

const int cs = 53;
const int interval = 5000; // Interval in milliseconds
unsigned long previousMillis = 0;
int dataArray[] = {10, 20, 30, 40, 50}; // Sample numerical array

void setup() {
  Serial.begin(9600);

  // Initialize SD card
  Serial.print("Initializing SD card...");
  if (!SD.begin(cs)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");

  // Create or append to the file
  File dataFile = SD.open("DATA.TXT", FILE_WRITE);
  if (dataFile) {
    dataFile.println("Timestamp, Data");
    dataFile.close();
    Serial.println("File 'DATA.TXT' created.");
  } else {
    Serial.println("Error opening 'DATA.TXT'");
  }
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Open the file
    File dataFile = SD.open("DATA.TXT", FILE_WRITE);
    if (dataFile) {
      // Get current time
      unsigned long currentTime = millis();
      String timeStamp = String(currentTime);

      // Write timestamp and data array to the file
      dataFile.print(timeStamp);
      dataFile.print(", ");
      for (int i = 0; i < sizeof(dataArray) / sizeof(dataArray[0]); i++) {
        dataFile.print(dataArray[i]);
        if (i < sizeof(dataArray) / sizeof(dataArray[0]) - 1) {
          dataFile.print(", ");
        }
      }
      dataFile.println();
      dataFile.close();
      Serial.println("Data written to 'DATA.TXT'");
    } else {
      Serial.println("Error opening 'DATA.TXT'");
    }
  }
}
