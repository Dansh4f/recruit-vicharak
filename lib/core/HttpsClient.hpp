#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <SPIFFS.h>
#include <WiFiClientSecure.h>

const char* url= "https://100daysofcode.s3-eu-west-1.amazonaws.com/rfc2616.txt";

const char* filename= "/file.txt";

WiFiClientSecure client;
HTTPClient http;
extern WiFiMulti wifiMulti;

static uint8_t buff[8192];

void space(int fileSize){
size_t totalBytes = SPIFFS.totalBytes();
size_t usedBytes = SPIFFS.usedBytes();
size_t freeBytes = totalBytes - freeBytes;

Serial.printf("SPIFFS total  space: %d bytes \n used space: %d bytes \n free space: %d bytes\n",totalBytes, usedBytes,freeBytes);

if(fileSize > 0 && fileSize > freeBytes){
    Serial.println("not enough storage");
    return;
}
}

void httpsclient(){
client.setInsecure();
Serial.print("\n fetching url: ");
Serial.println(url);

if (wifiMulti.run() == WL_CONNECTED){
    Serial.print("HTTP begin..\n");
    if(!http.begin(client,url)){
        Serial.println("unable to connect to the network");
        return;
    }
    int httpCode = http.GET();
    if (httpCode > 0) {
        Serial.printf("HTTP GET code: %d\n",httpCode);

        if(httpCode == HTTP_CODE_OK){
            int len = http.getSize();
            Serial.printf("file size: %d byte\n",len);

            space(len);

            if(len > 0 && len >(SPIFFS.totalBytes() - SPIFFS.usedBytes())){
                Serial.println("Storage is fulll in SPIFFS");
                http.end();
                return;
            }

            //open the file for writing
            File file =SPIFFS.open(filename, FILE_WRITE);
            if(!file){
                Serial.println("sorry the can't be opened try next time");
                return;
            }
            WiFiClient *stream = http.getStreamPtr();

            Serial.println("writing to SPIFFS...");
            unsigned long StartDownload = micros();
            unsigned long writeTime=0;
            size_t totalRecived = 0;
            size_t totalWritten = 0;

            while(http.connected() && (len > 0 || len == -1)){
                size_t availByte = stream -> available();
                if(availByte == 0){
                    delayMicroseconds(100);
                    continue;
                }
                size_t toRead = availByte;
                if (toRead > sizeof(buff)) toRead = sizeof(buff);

                int c = stream -> readBytes(buff , toRead );
                if(c <= 0) continue;

                totalRecived += c;

                unsigned long startWrite= micros();
                file.write(buff,c);

                unsigned long endWrite = micros();
                writeTime += (endWrite - startWrite);
                totalWritten +=c;

                if(len > 0) len -= c;
                
            }

            unsigned long endDownload = micros();
            file.close();
            Serial.println("file written to spiffs");

            float durationDownlaodsec = (endDownload - StartDownload) / 1e6;
            float downnloadSpeedKBs = (totalRecived/1024.0f) /max(durationDownlaodsec, 1e-6f);
            float WriteDurationSec = writeTime /1e6;
            float WriteSpeedKBs = (totalWritten / 1024.0f)/max(WriteDurationSec, 1e-6f);

            Serial.printf("file size wrrtten: %d bytes\n",totalWritten);
            Serial.printf("average download speed: %.2f KB/s\n",downnloadSpeedKBs);
            Serial.printf("avg write speed: %.2f KB/s\n",WriteSpeedKBs);
            delay(5000);
        }
    }else{
        Serial.printf("http failed,error: %s \n",http.errorToString(httpCode).c_str());
    }
    http.end();
    
}
delay(1000);
}