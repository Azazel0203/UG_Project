#include "Wire.h"
//#include <U8glib.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include "heartRate.h"
#include "DFRobot_BloodOxygen_S.h"

#define MUX_Address 0x70 // TCA9548A Encoders address
long lastRead_time_df = millis();
long last_reading_df = 0;


char tmp_string[8];  // Temp string to convert numeric values to string before print to OLED display
//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_FAST);  // Fast I2C / TWI
MAX30105 particleSensor;
DFRobot_BloodOxygen_S_I2C MAX30102;


// Initialize I2C buses using TCA9548A I2C Multiplexer
void tcaselect(uint8_t i2c_bus) {
    if (i2c_bus > 7) return;
    Wire.beginTransmission(MUX_Address);
    Wire.write(1 << i2c_bus);
    Wire.endTransmission(); 
}

void setup(){
    Serial.begin(115200);

//    tcaselect(1);
//    u8g.firstPage();
//    do {
//        u8g.begin();
//      } while( u8g.nextPage() );
//    

    tcaselect(0);
    Serial.println("SpO2 Mode Setup");
    if (!particleSensor.begin()) {
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
    tcaselect(2);
    while (false == MAX30102.begin())
    {
      Serial.println("init fail!");
    }
    Serial.println("init success!");
    Serial.println("start measuring...");
    MAX30102.sensorStartCollect();

}
//
//// Initialize the displays 
//void DisplayInit(){
//    // for (int i = 0; i < 7; i++) {
//      tcaselect(1);   // Loop through each connected displays on the I2C buses  
//      u8g.firstPage();
//      do {
//        u8g.begin();  // Initialize display
//       } while( u8g.nextPage() );
//    // }
//}


void loop(){
//    DisplayInit();
//    tcaselect(1);
//    u8g.firstPage();
//      do {
//        /******** Display Something *********/
//        u8g.setFont(u8g_font_ncenB10);
//        u8g.drawStr(0, 13, "PPG -> ");
//        u8g.drawStr(0, 50, "PPG -> ");
//
//        // itoa(i, tmp_string, 10);  
//        u8g.setFont(u8g_font_ncenB18);
//        u8g.drawStr(25, 50, tmp_string);
//        /************************************/
//      } while( u8g.nextPage() );

    
    tcaselect(0);
    Serial.print("From PPG -> ");
    Serial.println(particleSensor.getIR());
    // if (lastRead_time_df - millis()>4000){
      tcaselect(2);
      // lastRead_time_df = millis();
      MAX30102.getHeartbeatSPO2();
      Serial.print("From DFrobot -> ");
      last_reading_df = MAX30102._sHeartbeatSPO2.Heartbeat;
      Serial.println(last_reading_df);
      delay(4000);
    // }
}
