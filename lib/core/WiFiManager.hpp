#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiMulti.h>

const char* ssid = "realme 6";
const char* password = "micromouse";

extern WiFiMulti wifiMulti;

void wifimanager(){
    Serial.print("connecting to wifi...");
    wifiMulti.addAP(ssid, password);

    while(wifiMulti.run() != WL_CONNECTED){
        Serial.print("connecting......");
        delay(1000);
    }

    Serial.println();
    Serial.print("Phase 1 completed!");
    Serial.print("you are connected to : ");
    Serial.println(ssid);


    Serial.print("IP address");
    Serial.print(WiFi.localIP());

    long rssi=WiFi.RSSI();
    Serial.print("WIFI RSSI");
    Serial.print(rssi);
    Serial.println("dbm");

    WiFi.setSleep(false);
    WiFi.setTxPower(WIFI_POWER_19_5dBm);
}
