
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>

const char* wifiSsid = "SSID_HERE";
const char* wifiPassword = "WIFI_PWD_HERE";
const char* mqttBroker = "MQTT_BROKER_ADDRESS_HERE";
const char* mqttUser = "MQTT_USERNAME_HERE";
const char* mqttPassword = "MQTT_PASSWORD_HERE";
const String sensorIdentifier = "IDENTIFIER_HERE";

SHT3X sht30(0x45);
WiFiClient espClient;
PubSubClient client(espClient);

int sleepTime = 600e6;

void setup() {
  Serial.begin(115200);
  
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, LOW);

  int shtStatus = sht30.get();

  connectToWifi();
  connectToMqttBroker();

  if (shtStatus == 0){
    String temp = String(sht30.cTemp);
    Serial.print("Temperature in Celsius : ");
    Serial.println(temp);

    String hum = String(sht30.humidity);
    Serial.print("Relative Humidity : ");
    Serial.println(hum);

    // TODO: use some nice json library for this
    String json = "{\"temperature\": " + temp + ", \"humidity\": " + hum + "}";
    Serial.println(json);
    
    client.publish(sensorIdentifier.c_str(), (json).c_str());
  } else {
    Serial.println("Error reading the sensor");
    client.publish((sensorIdentifier + "/status").c_str(), "error");
  }
  client.loop();
  digitalWrite(BUILTIN_LED, HIGH);

  ESP.deepSleep(sleepTime);
}

void loop() {
}

void connectToWifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to WiFi, SSID: ");
  Serial.println(wifiSsid);

  WiFi.begin(wifiSsid, wifiPassword);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToMqttBroker() {
  client.setServer(mqttBroker, 1883);
  
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(sensorIdentifier.c_str(), mqttUser, mqttPassword)) {
      Serial.println("Connected to MQTT server");
    } else {
      Serial.print("Connection failed, reason: ");
      Serial.println(client.state());
      Serial.println("Trying again in 5 seconds..");

      delay(5000);
    }
  }
}
