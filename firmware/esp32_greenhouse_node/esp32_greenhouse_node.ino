/*
  ESP32 智能大棚节点示例固件

  功能：
  - 采集温湿度、土壤湿度、光照强度
  - 通过 MQTT 上报节点数据
  - 接收云端/上位机控制命令
  - 自动控制风机、水泵、补光灯和通风窗

  需要安装的 Arduino 库：
  - DHT sensor library
  - BH1750
  - PubSubClient
  - ArduinoJson
*/

#include <ArduinoJson.h>
#include <BH1750.h>
#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>

#define NODE_ID "GH-01"
#define DHT_PIN 4
#define DHT_TYPE DHT22
#define SOIL_PIN 34
#define FAN_PIN 18
#define PUMP_PIN 19
#define LIGHT_PIN 23
#define SERVO_PIN 13

const char* WIFI_SSID = "your-wifi-ssid";
const char* WIFI_PASS = "your-wifi-password";
const char* MQTT_HOST = "192.168.1.10";
const int MQTT_PORT = 1883;

DHT dht(DHT_PIN, DHT_TYPE);
BH1750 lightMeter;
WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

struct SensorData {
  float temperature;
  float humidity;
  int soilMoisture;
  int lightLux;
};

bool fanOn = false;
bool pumpOn = false;
bool lightOn = false;
int ventPercent = 20;
String mode = "auto";
unsigned long lastPublish = 0;

void setup() {
  Serial.begin(115200);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LIGHT_PIN, OUTPUT);
  Wire.begin();
  dht.begin();
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);

  connectWiFi();
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);
}

void loop() {
  if (!mqtt.connected()) {
    reconnectMqtt();
  }
  mqtt.loop();

  SensorData data = readSensors();
  if (mode == "auto") {
    autoControl(data);
  }
  applyActuators();

  if (millis() - lastPublish > 5000) {
    publishTelemetry(data);
    lastPublish = millis();
  }
}

SensorData readSensors() {
  SensorData data;
  data.temperature = dht.readTemperature();
  data.humidity = dht.readHumidity();
  int rawSoil = analogRead(SOIL_PIN);
  data.soilMoisture = map(rawSoil, 4095, 1100, 0, 100);
  data.lightLux = (int)lightMeter.readLightLevel();

  if (isnan(data.temperature)) data.temperature = 25.0;
  if (isnan(data.humidity)) data.humidity = 65.0;
  data.soilMoisture = constrain(data.soilMoisture, 0, 100);
  return data;
}

void autoControl(SensorData data) {
  fanOn = data.temperature > 28.0 || data.humidity > 82.0;
  pumpOn = data.soilMoisture < 45;
  lightOn = data.lightLux < 12000;

  if (data.temperature > 30.0 || data.humidity > 82.0) {
    ventPercent = 85;
  } else if (data.temperature > 27.0) {
    ventPercent = 55;
  } else {
    ventPercent = 25;
  }
}

void applyActuators() {
  digitalWrite(FAN_PIN, fanOn ? HIGH : LOW);
  digitalWrite(PUMP_PIN, pumpOn ? HIGH : LOW);
  digitalWrite(LIGHT_PIN, lightOn ? HIGH : LOW);
}

void publishTelemetry(SensorData data) {
  StaticJsonDocument<384> doc;
  doc["node_id"] = NODE_ID;
  doc["temperature"] = data.temperature;
  doc["humidity"] = data.humidity;
  doc["soil_moisture"] = data.soilMoisture;
  doc["light_lux"] = data.lightLux;
  doc["mode"] = mode;
  doc["fan"] = fanOn;
  doc["pump"] = pumpOn;
  doc["light"] = lightOn;
  doc["vent"] = ventPercent;

  char payload[384];
  serializeJson(doc, payload);
  mqtt.publish("greenhouse/nodes/GH-01/telemetry", payload);
}

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, payload, length);
  if (err) return;

  if (doc.containsKey("mode")) {
    mode = String((const char*)doc["mode"]);
  }
  if (doc.containsKey("fan")) fanOn = doc["fan"];
  if (doc.containsKey("pump")) pumpOn = doc["pump"];
  if (doc.containsKey("light")) lightOn = doc["light"];
  if (doc.containsKey("vent")) ventPercent = constrain((int)doc["vent"], 0, 100);
}

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void reconnectMqtt() {
  while (!mqtt.connected()) {
    if (mqtt.connect(NODE_ID)) {
      mqtt.subscribe("greenhouse/nodes/GH-01/command");
    } else {
      delay(2000);
    }
  }
}
