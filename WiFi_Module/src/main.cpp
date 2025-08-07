#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "./arduino_secrets.h"

#define INTERVAL 10000 // 10 seconds

// WiFi credentials
const char *wifi_ssid = SECRET_SSID;
const char *wifi_password = SECRET_PASS;

// MQTT credentials
const char *mqtt_username = SECRET_MQTTUSER;
const char *mqtt_password = SECRET_MQTTPASS;
const char *mqtt_server = "mqtt.cetools.org";
const char *mqtt_topic = "student/ultrasonic_anemometer/data";
const int mqtt_port = 1884;



// Global Objects
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Function Declarations
void startWifi(void);
void checkWifi(void);
void checkMQTT(void);
void sendMQTT(char *msg);

void setup() {
  // Set software serial baud to 115200;
  Serial.begin(9600);
  delay(1000); // Wait for Serial to initialize

  Serial.println("ESP32 MQTT");
  delay(1000);

  // Start WiFi connection
  Serial.println("Connecting to WiFi...");
  startWifi();

  // Initialize MQTT client
  mqttClient.setServer(mqtt_server, mqtt_port);
}

void loop() {
  char mqtt_msg[100];
  
  if(Serial.available()) {
    
    String input = Serial.readStringUntil('\n');
    // Prepare MQTT message
    sprintf(mqtt_msg, input.c_str());
    sendMQTT(mqtt_msg);
  }

  // Ensure WiFi and MQTT connections are maintained
  checkWifi();
  checkMQTT();
}

void startWifi(void)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void checkWifi(void)
{
    // Reconnect to WiFi if disconnected
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= INTERVAL)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}

void checkMQTT(void)
{
  if(WiFi.status() != WL_CONNECTED) {
    checkWifi();
  }

  while(!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(mqttClient.connect("ESP32Client", mqtt_username, mqtt_password)) {
      Serial.println("connected");
      // Subscribe to a topic if needed
      // mqttClient.subscribe("your/topic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void sendMQTT(char *msg)
{
  char mqttTopic[35];
  sprintf(mqttTopic, "student/ultrasonic_anemometer");
  
  Serial.println("SENDING MSG...");
  Serial.print("Topic: ");
  Serial.println(mqttTopic);
  Serial.print("Message: ");
  Serial.println(msg);

  if (mqttClient.publish(mqttTopic, msg))
  {
    Serial.println("Message published");
  }
  else
  {
    Serial.println("Failed to publish message");
  }

  delay(1000); // Wait for a second before next operation
}
