
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
#include <EEPROM.h>
#include "GravityTDS.h"

#define TdsSensorPin 0
#define TEMP_SENSOR_BUS 12
#define PH_SENSOR_ADS_1115_ANALOG_PIN 2
#define EC_SENSOR_ADS_1115_ANALOG_PIN 3

OneWire oneWire(TEMP_SENSOR_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1115 ads;
GravityTDS gravityTds;



void setup(void) {
  Serial.begin(115200);
  sensors.begin();
  ads.begin();
  gravityTds.setPin(TdsSensorPin);
  gravityTds.setAref(5.0);
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();
}

//interpolation as a linear y=ax+b, a and b are calculated base on liquids 4 and 7, a=(y2-y1)/(x2-x1)
// more info https://bestengineeringprojects.com/arduino-ph-meter-using-ph-sensor/
float phCalibrationB = 21.34;
float phCalibrationA = -5.7;

void loop(void) {
  Serial.println("Requesting temperature...");
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if(tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
  }
  Serial.print("Temperature: ");
  Serial.println(tempC);

  delay(1000);

  Serial.println("Requesting ph...");
  float phValue = phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  Serial.print("Calculated ph value:  ");
  Serial.println(phValue);

  delay(1000);

  //more info https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
  Serial.println("Requesting EC...");
  gravityTds.setTemperature(tempC);
  gravityTds.update();
  float ecValue = gravityTds.getEcValue();
  Serial.print("Calculated ec value:  ");
  Serial.println(ecValue);

  Serial.println();
  delay(5000);
}
