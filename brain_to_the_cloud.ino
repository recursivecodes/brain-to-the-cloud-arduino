
// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include "creds.h"
#include <Brain.h>
#include "EspMQTTClient.h"
#include <ArduinoJson.h>
#include <TelnetStream.h>
#define SWITCH_PIN 4
#define LED_PIN 12

unsigned long lastBrainUpload = millis();
unsigned long lastDemoUpload = millis();

const long brainTimeout = 5000;
const long demoTimeout = 1000;

boolean mqttInit = false;

Brain brain(Serial);

EspMQTTClient mqttClient(
  ssid,
  password,
  mqtt_url,
  mqtt_username,
  mqtt_password,
  mqtt_client_id
);

void onConnectionEstablished() {
  /*
  client.subscribe("mytopic/test", [] (const String &payload)  {
    Serial.println(payload);
  });
  */
  mqttInit = true;
}


void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  
  Serial.begin(9600);
  TelnetStream.begin();
  
  Serial.print("Connecting to ");
  TelnetStream.print("Connecting to ");
  
  delay(5000);
  int switchState = digitalRead(SWITCH_PIN);
  
  if(switchState == HIGH) {
    Serial.println(ssid);
    TelnetStream.println(ssid);
    WiFi.begin(ssid, password);
  }
  else {
    Serial.println(hotspot_ssid);
    TelnetStream.println(hotspot_ssid);
    WiFi.begin(hotspot_ssid, hotspot_password);
  }
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.print("SSID: ");
  
  TelnetStream.println("");
  TelnetStream.println("WiFi connected.");
  TelnetStream.print("SSID: ");

  pinMode(LED_BUILTIN, OUTPUT);
  if( WiFi.status() == WL_CONNECTED ) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  else {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  if(switchState == HIGH) {
    Serial.println(ssid);
    TelnetStream.println(ssid);
  }
  else {
    Serial.println(hotspot_ssid);
    TelnetStream.println(hotspot_ssid);
  }
  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  TelnetStream.println("IP address: ");
  TelnetStream.println(WiFi.localIP());

  mqttClient.enableOTA();
}


void loop(){
  mqttClient.loop();
  
  if( brain.update() ) {
    Serial.println(brain.readCSV());
    TelnetStream.println(brain.readCSV());
    unsigned long now = millis();

    StaticJsonDocument<150> doc;
    char readingsJson[256];
    uint8_t signalQuality = brain.readSignalQuality();
    doc["signalStrength"] = signalQuality;
    doc["attention"] = brain.readAttention();
    doc["meditation"] = brain.readMeditation();
    doc["delta"] = brain.readDelta();
    doc["theta"] = brain.readTheta();
    doc["lowAlpha"] = brain.readLowAlpha();
    doc["highAlpha"] = brain.readHighAlpha();
    doc["lowBeta"] = brain.readLowBeta();
    doc["highBeta"] = brain.readHighBeta();

    if(signalQuality == 200) {
      digitalWrite(LED_PIN, HIGH);
    }
    else {
      digitalWrite(LED_PIN, LOW);
    }
    
    serializeJson(doc, readingsJson);
    
    if(now - lastBrainUpload > brainTimeout) {
      Serial.println("publishing brain...");
      TelnetStream.println("publishing brain...");
      mqttClient.publish(brain_topic, readingsJson);  
      lastBrainUpload = now;
    }
    
    if(now - lastDemoUpload > demoTimeout) {
      Serial.println("publishing demo...");
      TelnetStream.println("publishing demo...");
      mqttClient.publish(demo_topic, readingsJson);  
      lastDemoUpload = now;
    }
    
  }
  else {
    Serial.println("no data");
    TelnetStream.println("no data");
  }
  
  if(mqttInit) {  
    StaticJsonDocument<150> doc;
    char readingsJson[256];
    doc["test"] = String(millis());
    serializeJson(doc, readingsJson);
    mqttClient.publish("bttc/test", readingsJson);  
  }
  
  delay(1000);
  
}
