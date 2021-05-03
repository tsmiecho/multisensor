
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>

#define ONE_WIRE_BUS 0
#define PH_SENSOR_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1115 ads;

void setup(void) {
  Serial.begin(115200);
  sensors.begin();
  ads.begin();
}

//interpolation as a linear y=ax+b, a and b are calculated base on liquids 4 and 7, a=(y2-y1)/(x2-x1)
// more info https://bestengineeringprojects.com/arduino-ph-meter-using-ph-sensor/
float calibrationB = 21.34;
float calibrationA = -5.7;

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
  float phValue = calibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_BUS)) + calibrationB;
  Serial.print("Calculated ph value:  ");
  Serial.println(phValue);
  Serial.println();
  delay(30000);
}
