


#include <SD.h>

const int cs = 53;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");
  if (!SD.begin(cs)) {
    Serial.println("Initialization failed!");
    while (1);
  }
  Serial.println("Initialization done.");
}

void loop() {
  // Create a file on the SD card
  File dataFile = SD.open("data.txt", FILE_WRITE);
  if (dataFile) {
    // Write some data to the file
    dataFile.println("Hello, microSD card!");
    dataFile.close();
    Serial.println("Data written to the file.");
  } else {
    Serial.println("Error opening data.txt");
  }

  // Read data from the file
  dataFile = SD.open("data.txt");
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  } else {
    Serial.println("Error opening data.txt for reading");
  }

  delay(1000);
}
