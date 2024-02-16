#include <Wire.h>
#include <MAX30105.h>
#include <NimBLEDevice.h>
#include <NimBLEServer.h>

MAX30105 particleSensor;
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;

#define debug Serial // Uncomment this line if you're using an Uno or ESP

struct SensorData {
  int red;
  int ir;
  int green;
};

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"        // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

void setup()
{
  debug.begin(9600);
  debug.println("MAX30105 Basic Readings Example");

  // Initialize sensor
  if (particleSensor.begin() == false)
  {
    debug.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  particleSensor.setup(); // Configure sensor. Use 6.4mA for LED drive

  // Start Bluetooth
  startBLE();

  // Print "waiting" until connected to Bluetooth
  while (!pServer->getConnectedCount())
  {
    debug.println("Waiting for Bluetooth connection...");
    delay(1000);
  }

  debug.println("Connected to Bluetooth!");
}

void loop()
{
  SensorData sensorData;

  // Read sensor data
  sensorData.red = particleSensor.getRed();
  sensorData.ir = particleSensor.getIR();
  sensorData.green = particleSensor.getGreen();

  // Print sensor data to Serial
  debug.print(" R[");
  debug.print(sensorData.red);
  debug.print("] IR[");
  debug.print(sensorData.ir);
  debug.print("] G[");
  debug.print(sensorData.green);
  debug.print("]");

  // Send sensor data via Bluetooth
  sendSensorData(sensorData);

  debug.println();
  delay(1000); // Add a delay to control the frequency of data transmission
}

void startBLE()
{
  BLEDevice::init("MAX30105_Sensor");
  pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, NIMBLE_PROPERTY::NOTIFY);
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->startAdvertising();
}

void sendSensorData(const SensorData &data)
{
  if (pServer->getConnectedCount() > 0)
  {
    // Only send the "ir" value via Bluetooth as a single byte
    uint8_t irValue = static_cast<uint8_t>(data.ir);

    // Send the sensor data via Bluetooth
    pTxCharacteristic->setValue(&irValue, 1);  // Sending a single byte
    pTxCharacteristic->notify();
  }
}

