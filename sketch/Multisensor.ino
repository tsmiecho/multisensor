
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_ADS1X15.h>
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

#define TEMP_INPUT_PIN 12
#define PH_SENSOR_ADS_1115_ANALOG_PIN 2

OneWire oneWire(TEMP_INPUT_PIN);
DallasTemperature sensors(&oneWire);
Adafruit_ADS1115 ads;
WiFiClient espClient;
PubSubClient client(espClient);

void setup(void) {
  Serial.begin(115200);
  sensors.begin();
  ads.begin();
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
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

  delay(10000);

  Serial.println("Requesting ph...");
  float phValue = phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  delay(10000);
  phValue = phValue + phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  delay(10000);
  phValue = phValue + phCalibrationA * ads.computeVolts(ads.readADC_SingleEnded(PH_SENSOR_ADS_1115_ANALOG_PIN)) + phCalibrationB;
  phValue = phValue / 3; //average
  Serial.print("Calculated ph value:  ");
  Serial.println(phValue);

  delay(60000);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Serial.println("Publish message ");
  client.publish(mqtt_topic_temperature_out, String(tempC).c_str());
  client.publish(mqtt_topic_ph_out, String(phValue).c_str());

  Serial.println();
  delay(86400000); //24 hours
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
