#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiManager.hpp>
#include <HttpsClient.hpp>
#include <SPIFFS.hpp>


WiFiMulti wifiMulti;

void setup() {
  Serial.begin(115200);
  delay(5000);

  Serial.println("System Booting...");
  if(!SPIFFS.begin(true)){
    Serial.println("spiffs mount failed");
    return;
  }

  Serial.println("spiffs mounted");


  wifimanager();
  httpsclient();
  //added
  readFile();


}

void loop() {


}

