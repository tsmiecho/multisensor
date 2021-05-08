
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>

#define TEMP_SENSOR_BUS 12
#define PH_SENSOR_ADS_1115_ANALOG_PIN 2
#define EC_SENSOR_ADS_1115_ANALOG_PIN 3

OneWire oneWire(TEMP_SENSOR_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1115 ads;

void setup(void) {
  Serial.begin(115200);
  sensors.begin();
  ads.begin();
}

//interpolation as a linear y=ax+b, a and b are calculated base on liquids 4 and 7, a=(y2-y1)/(x2-x1)
// more info https://bestengineeringprojects.com/arduino-ph-meter-using-ph-sensor/
float phCalibrationB = 21.34;
float phCalibrationA = -5.7;

//more info https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
float k = 0.71; //static value based on electrode
float ecCalibrationA = 133.42;
float ecCalibrationB = 255.86;
float ecCalibrationC = 857.39;

void loop(void) {
  Serial.println("Requesting temperature...");
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  if(tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Error: Could not read temperature data");
  }
  Serial.print("Temperature: ");
  Serial.println(tempC);

  Serial.println("Requesting ph...");
  float phValue = phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  Serial.print("Calculated ph value:  ");
  Serial.println(phValue);



  Serial.println("Requesting EC...");

  //more info https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
  float compensationCoefficient=1.0+0.02*(tempC-25.0);
  float compensationVoltage=(ads.computeVolts(ads.readADC_SingleEnded(EC_SENSOR_ADS_1115_ANALOG_PIN)))/compensationCoefficient;
  // EC= (av^3 - bv^2 + cv) * k
  float ecValue = (ecCalibrationA * compensationVoltage*compensationVoltage*compensationVoltage - ecCalibrationB * compensationVoltage*compensationVoltage + ecCalibrationC * compensationVoltage) * k;
  Serial.print("Calculated ec value:  ");
  Serial.println(ecValue);

  Serial.println();
  delay(5000);
}
