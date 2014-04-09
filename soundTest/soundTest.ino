#include <Wire.h>
//#include "DS1307.h" /// RTC
#include "DS1307.h"
#include "DHT.h"  // temperature and humidity
#include <SD.h>
#include <Adafruit_ADS1015.h>  // for NO2 and CO sensors

/*macro definitions of the sound sensor and the LED*/
#define SOUND_SENSOR A1
unsigned long soundValue;

int sampletime_ms =1000;
int starttime=0;
float sensorValue=0.0;

void setup() 
{
    Serial.begin(115200);
    starttime = millis();
    pins_init();
}
void loop()
{
    sensorValue = sensorValue + analogRead(SOUND_SENSOR);
   if ((millis()-starttime) > sampletime_ms)//if the sample time == 30s
      {
  	// sensorValue = analogRead(SOUND_SENSOR);//use A0 to read the electrical signal
        Serial.println(soundValue);
        starttime = millis();
      }
}
 void pins_init()
 { 
   pinMode(SOUND_SENSOR, INPUT);
 }

