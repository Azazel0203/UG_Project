#include <ArduinoBLE.h>
#include "DFRobot_BloodOxygen_S.h"

#define I2C_COMMUNICATION  //use I2C for communication, but use the serial port for communication if the line of codes were masked

#ifdef  I2C_COMMUNICATION
#define I2C_ADDRESS    0x57
DFRobot_BloodOxygen_S_I2C MAX30102(&Wire ,I2C_ADDRESS);
#else

#if defined(ARDUINO_AVR_UNO) || defined(ESP8266)
//SoftwareSerial mySerial(21, 22);//esp32
SoftwareSerial mySerial(8,9);//BLE
DFRobot_BloodOxygen_S_SoftWareUart MAX30102(&mySerial, 9600);
#else
DFRobot_BloodOxygen_S_HardWareUart MAX30102(&Serial1, 9600); 
#endif
#endif

// Initalizing global variables for sensor data to pass onto BLE
int Sp, HR;
//string Sp, HR;
float Temp;

// Bluetooth device name
const char* deviceName = "MyArduinoBLE";

BLEService bloodOxygenService("180C"); // Custom BLE service for Blood Oxygen
BLEUnsignedIntCharacteristic spo2Characteristic("2A50", BLERead | BLENotify); // SPO2 characteristic
BLEUnsignedIntCharacteristic heartRateCharacteristic("2A51", BLERead | BLENotify); // Heart Rate characteristic
BLEUnsignedIntCharacteristic temperatureCharacteristic("2A52", BLERead | BLENotify); // Temperature characteristic

//void readValues();

void setup() {
  Serial.begin(115200);
  while (false == MAX30102.begin()) {
    Serial.println("Sensor initialization failed!");
    delay(1000);
  }
  Serial.println("init success!");
  Serial.println("start measuring...");
  MAX30102.sensorStartCollect();

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }
  BLE.setLocalName(deviceName);
  //BLE.begin();

  BLE.setAdvertisedService(bloodOxygenService); // Add the Blood Oxygen service
  bloodOxygenService.addCharacteristic(spo2Characteristic); // Add the SPO2 characteristic
  bloodOxygenService.addCharacteristic(heartRateCharacteristic); // Add the Heart Rate characteristic
  bloodOxygenService.addCharacteristic(temperatureCharacteristic); // Add the Temperature characteristic
  BLE.addService(bloodOxygenService); // Publish the Blood Oxygen service

  BLE.advertise(); // Start advertising

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {

  //MAX30102.getHeartbeatSPO2();
  //BLEDevice central = BLE.central();
  //Serial.print("SPO2 is : ");
  //Serial.print(MAX30102._sHeartbeatSPO2.SPO2);
  //Serial.println("%");
  //Serial.print("heart rate is : ");
  //Serial.print(MAX30102._sHeartbeatSPO2.Heartbeat);
  //Serial.println("Times/min");
  //Serial.print("Temperature value of the board is : ");
  //Serial.print(MAX30102.getTemperature_C());
  //Serial.println(" ℃");
  BLEDevice central = BLE.central();
  
  if (central) {
    //MAX30102.getHeartbeatSPO2();
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    while (central.connected())
    //while (Serial.available()) 
    {
      MAX30102.getHeartbeatSPO2();
      //digitalWrite(LED_BUILTIN, HIGH);
      //delay(300);
      //digitalWrite(LED_BUILTIN, LOW);
      //delay(300);
      //MAX30102.getHeartbeatSPO2();
      //char data = Serial.read();
      // BLEUART.notify(&data, sizeof(data)); // Incorrect, not needed
      readValues();
  
  // Update the BLE characteristics with the sensor data
      spo2Characteristic.writeValue(Sp);//MAX30102._sHeartbeatSPO2.SPO2);
      heartRateCharacteristic.writeValue(HR); //MAX30102._sHeartbeatSPO2.Heartbeat);
      temperatureCharacteristic.writeValue(Temp); //MAX30102.getTemperature_C());

  // Display data on the serial monitor
      Serial.println("Reading Sensors");
      Serial.print("SPO2: ");
      Serial.print(Sp);//MAX30102._sHeartbeatSPO2.SPO2);
      Serial.println("%");

      Serial.print("Heart Rate: ");
      Serial.print(HR);//MAX30102._sHeartbeatSPO2.Heartbeat);
      Serial.println(" bpm");

      Serial.print("Temperature: ");
      Serial.print(Temp);//MAX30102.getTemperature_C());
      Serial.println(" °C");

      delay(300);
    }
  }
  Serial.print("Disconnected from central: ");
  Serial.println(central.address());
}
void readValues(){
   MAX30102.getHeartbeatSPO2();
   //float x, y, z;
   float SPO2 = MAX30102._sHeartbeatSPO2.SPO2;
   float HeartRate = MAX30102._sHeartbeatSPO2.Heartbeat;
   float Temperature = MAX30102.getTemperature_C();
   Sp = SPO2;
   HR = HeartRate;
   Temp = Temperature;
   //Sp=UnsignedInt(SPO2) + " % ";
   //HR=UnsignedInt(HeartRate) + " bpm ";
   //Temp=UnsignedInt(Temperature) + " C";

}