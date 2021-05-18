
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
#include "GravityTDS.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "";
const char* mqtt_user = "";
const char* mqtt_password = "";
const char* mqtt_topic_temperature_out = "";
const char* mqtt_topic_ec_out = "";
const char* mqtt_topic_ph_out = "";


#define TDS_SENSOR_PIN 0
#define TEMP_INPUT_PIN 12
#define PH_SENSOR_ADS_1115_ANALOG_PIN 2
#define RELAY_PIN 15

OneWire oneWire(TEMP_INPUT_PIN);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1115 ads;
GravityTDS gravityTds;
WiFiClient espClient;
PubSubClient client(espClient);

void setup(void) {
  Serial.begin(115200);
  sensors.begin();
  ads.begin();
  gravityTds.setPin(TDS_SENSOR_PIN);
  gravityTds.setAref(5.0);
  gravityTds.setAdcRange(1024);  //1024 for 10bit ADC;4096 for 12bit ADC
  gravityTds.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);
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

  delay(100);

  Serial.println("Requesting ph...");
  float phValue = phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  delay(10000);
  phValue = phValue + phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  delay(10000);
  phValue = phValue + phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  delay(10000);
  phValue = phValue + phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  delay(10000);
  phValue = phValue + phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  phValue = phValue / 5; //average
  Serial.print("Calculated ph value:  ");
  Serial.println(phValue);

  delay(60000);

  //more info https://wiki.dfrobot.com/Gravity__Analog_TDS_Sensor___Meter_For_Arduino_SKU__SEN0244
  Serial.println("Requesting EC...");
  digitalWrite(RELAY_PIN, HIGH);
  delay(1000);
  gravityTds.setTemperature(tempC);
  gravityTds.update();
  float ecValue = gravityTds.getEcValue();
  delay(10000);
  gravityTds.update();
  ecValue = ecValue + gravityTds.getEcValue();
  delay(10000);
  gravityTds.update();
  ecValue = ecValue + gravityTds.getEcValue();
  delay(10000);
  gravityTds.update();
  ecValue = ecValue + gravityTds.getEcValue();
  delay(10000);
  gravityTds.update();
  ecValue = ecValue + gravityTds.getEcValue();
  ecValue = ecValue / 5; //average
  digitalWrite(RELAY_PIN, LOW);
  Serial.print("Calculated ec value:  ");
  Serial.println(ecValue);


  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println("Publish message ");
  client.publish(mqtt_topic_temperature_out, String(tempC).c_str());
  client.publish(mqtt_topic_ec_out, String(ecValue).c_str());
  client.publish(mqtt_topic_ph_out, String(phValue).c_str());

  Serial.println();
  delay(18000000); //30 minutes
}

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.println("Message arrived [");
  Serial.println(topic);
  Serial.println("] ");
  Serial.println();
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "test-clientId-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
