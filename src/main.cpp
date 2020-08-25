#include <Arduino.h>
#include <M5Stack.h>
#include <max6675.h>
#include <Wire.h>
#include "WiFi.h"
#include <PubSubClient.h>


const char* ssid = "GP-ONEPLUS";
const char* password =  "bejczyLab";


// initializing MAX6675
const int thermoDO = 2;
const int thermoCS = 5;
const int thermoCLK = 17;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

// Stores current temperature data
float temp = 0;


// MQTT stuff
const char* mqtt_server = "test.mosquitto.org";

WiFiClient espClient;
PubSubClient client(espClient);

// make a cute degree symbol
uint8_t degree[8]  = {140,146,146,140,128,128,128,128};


void setupWifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("M5Stack-BBQMonitor")) {
      Serial.println("connected");
      // Subscribe
      // client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  M5.Power.begin();

  setupWifi();
  client.setServer(mqtt_server, 1883);

  // Setting up display
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Lcd.setTextSize(3);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("BBQ Monitor");

  // wait for MAX chip to stabilize
  delay(500);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  temp = thermocouple.readCelsius();
  Serial.println(temp);
  
  M5.Lcd.setCursor(0,50);
  M5.Lcd.print(thermocouple.readCelsius());
  M5.Lcd.println("C");

  // Publish the temerature value
  char msg[8];
  dtostrf(temp, 1, 2, msg);
  client.publish("BBQmonitor/gp/0", msg);
  
  delay(200);
}