
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>

#define ONE_WIRE_BUS 0

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
float calibrationA = -5.7

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
  int16_t sensorValue = ads.readADC_SingleEnded(0);
  float voltage = ads.computeVolts(sensorValue);
  Serial.print("Ph sensor value ");
  Serial.println(sensorValue);
  Serial.print("Voltage ");
  Serial.println(voltage);
  float phValue = calibrationA * voltage + calibrationB;
  Serial.print("Calculated ph value:  ");
  Serial.println(phValue);
  Serial.println();
}
