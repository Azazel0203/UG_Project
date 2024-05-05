#include <SD.h>

const int cs = 10;

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
  // Open WHAT.TXT file
  File file = SD.open("/WHAT.TXT");
  
  // Ensure file is opened
  if (file) {
    Serial.println("Printing contents of WHAT.TXT:");
    // Read and print contents of the file
    while (file.available()) {
      Serial.write(file.read());
    }
    Serial.println(); // Newline after printing file contents
    file.close(); // Close the file
  } else {
    Serial.println("Error opening WHAT.TXT");
  }
  
  delay(10000); // Delay for 10 seconds before repeating
}
