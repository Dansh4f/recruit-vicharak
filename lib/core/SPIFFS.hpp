
#include <Arduino.h>
#include <SPIFFS.h>

extern const char* filename;

void readFile(){
    File file =SPIFFS.open(filename,FILE_READ);
    if (!file){
        Serial.println("can't open the file better luck next time");
        return;
    }
    Serial.println("reading...");
    delay(5000);
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
    Serial.println("\n mission completed");
}
