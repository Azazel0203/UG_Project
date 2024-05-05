#include <Wire.h>
// #include "DFRobot_BloodOxygen_S.h"
#include "MAX30105.h"
#include "heartRate.h"


// #define I2C_ADDRESS    0x57
#define MUX_Address 0x70 

MAX30105 particleSensor;
// DFRobot_BloodOxygen_S_I2C df_sensor(&Wire, I2C_ADDRESS);



float df_spo2;
float df_heartrate;


// NECESSARY
const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;


//MY
long ppg_irvalue;
float ppg_beatsperminute;
int ppg_beatavg;


// Functions
void tcaselect(uint8_t i2c_bus) {
    if (i2c_bus > 7) return;
    Wire.beginTransmission(MUX_Address);
    Wire.write(1 << i2c_bus);
    Wire.endTransmission(); 
}

// void get_df_data(){
//   selectI2CChannels(1);
//   df_sensor.getHeartbeatSPO2();
//   df_spo2 = df_sensor._sHeartbeatSPO2.SPO2;
//   df_heartrate = df_sensor._sHeartbeatSPO2.Heartbeat;
//   delay(4000);
// }


void get_heartrate_ppg(){
  tcaselect(2);
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
  // selectI2CChannels(1);
  
  // //df_sensor
  // while (false == df_sensor.begin())
  // {
  //   Serial.println("init fail!");
  //   delay(1000);
  // }
  // df_sensor.sensorStartCollect();

  tcaselect(2);
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST))
  {
    Serial.println("MAX30105 was not found. Please check wiring/power. ");
    while (1);
  }

  tcaselect(2);
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
}

void loop() {
  
  //DF BLOCK
  // selectI2CChannels(1);
  // get_df_data();
  // Serial.print("DF_ROBOT -> ");
  // Serial.print(df_spo2);
  // Serial.print(" | ");
  // Serial.print(df_heartrate);
  //PPG BLOCK
  tcaselect(2);
  get_heartrate_ppg();
  Serial.print(" || PPG -> ");
  Serial.print(ppg_irvalue);
  Serial.print(" | ");
  Serial.print(ppg_beatsperminute);
  Serial.print(" | ");
  Serial.println(ppg_beatavg);
}


