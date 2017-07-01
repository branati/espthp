#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "config.h"

#define STATUS_DISCONNECTED "disconnected"
#define STATUS_ONLINE "online"

const String chipId = String(ESP.getChipId());
const String baseTopic = "raw/" + chipId + "/";
const String tempTopic = baseTopic + "temperature";
const String humiTopic = baseTopic + "humidity";
const String presTopic = baseTopic + "pressure";
const String willTopic = baseTopic + "status";

WiFiClient WiFiClient;
PubSubClient client(WiFiClient);
Adafruit_BME280 bme; // I2C

void setup() {
  
  Serial.begin(115200);
  delay(10);

  Wire.begin(0, 2);
  Wire.setClock(100000);
  if (!bme.begin(0x76)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
  }

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  client.setServer(MQTT_IP, MQTT_PORT);
}

void loop() {
  yield();
  if (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    if (client.connect(chipId.c_str(), willTopic.c_str(), 1, true, STATUS_DISCONNECTED)) {
      Serial.println("MQTT client connected.");
      client.publish(willTopic.c_str(), STATUS_ONLINE, true);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" will try again in 5 seconds");      
      delay(5000);    
    }
  }

  if (client.connected()) {
    bmeReadSend();
    client.loop();
    delay(10000);
  }
}

void bmeReadSend() {  
  char temperature[6];
  char humidity[6];  
  char pressure[7];
  
  float t = bme.readTemperature();  
  float h = bme.readHumidity();
  float p = bme.readPressure()/100.0F;
  
  dtostrf(t, 5, 1, temperature);
  dtostrf(h, 5, 1, humidity);  
  dtostrf(p, 5, 1, pressure);

  Serial.print("t,h,p: ");
  Serial.print(temperature); Serial.print(", ");
  Serial.print(humidity);    Serial.print(", ");
  Serial.print(pressure);    Serial.println();
  client.publish(tempTopic.c_str(), temperature);
  client.publish(humiTopic.c_str(), humidity);
  client.publish(presTopic.c_str(), pressure);
}

