
/*  This sketch is created for the ES&P 400 air quality and new construction groups. It 
includes a temp/humidity sensor, realtime clock, NO2 sensor, CO sensor, sound sensor and
logs data to an SD card */

// YOU MUST USE ARDUINO IDE 1.5+ for this to work

#include "AirQuality.h"
#include <Wire.h>
#include "DS1307.h" /// RTC
#include "DHT.h"  // temperature and humidity
#include <SPI.h>
#include <SD.h>
#include"Arduino.h"

// SET this to the sampling interval
// now set to read every minute
unsigned long sampletime_ms = 30000; //sample delay in ms

// SET THIS TO TRUE IF YOU WANT TO SET THE CLOCK
// you will need to change the values in the changing part...
boolean SET_CLOCK_FLAG=false;
static unsigned int current_year = 2014;  //(year)
static unsigned int current_mon = 4;  // 1 - 12
static unsigned int current_day = 4;  // 1-31
static unsigned int current_hour = 14; // 0-23
static unsigned int current_min = 7; // 0-59
static unsigned int current_sec = 0; // 0-59
#define CURRENT_DAY FRI   // MON, TUE, WED, THU, FRI, SAT, SUN

// TEMP AND HUMIDITY
#define DHTPIN A2 // temp humidity pin we're connected to
#define DHTTYPE DHT22 // DHT 22 (AM2302)
DHT dht(DHTPIN, DHTTYPE);    /// TEMP and HUmidity

// Clock - date and time object
DS1307 clock;//define a object of DS1307 class

/*macro definitions of the sound sensor and the LED*/
#define SOUND_SENSOR A1
unsigned long soundValue;
unsigned int maxSound = 0;

// for dust sensor
int pin = 8;
unsigned long duration;
unsigned long starttime;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
unsigned long triggerOnP1;
unsigned long triggerOffP1;
unsigned long pulseLengthP1;
unsigned long durationP1;
boolean valP1 = HIGH;
boolean triggerP1 = false;
unsigned long triggerOnP2;
unsigned long triggerOffP2;
unsigned long pulseLengthP2;
unsigned long durationP2;
boolean valP2 = HIGH;
boolean triggerP2 = false;
float ratioP1 = 0;
float ratioP2 = 0;
float countP1;
float countP2;

// pin definitions for CO2 and NO2 (MIC-5525 and MIC-2714)
#define CO_INPUT_PIN A4 // On ADC board  note these need the ADC board!
#define NO2_INPUT_PIN A5 // On ACD board note these need the ADC board!


// Air quality Sensor
AirQuality airqualitysensor;
int current_quality =-1;
#define AIR_QUALITY A0

//This will keep track of the last time we sent sensor values and updated the display
unsigned long lastUpdate = 0;

//SD CARD set up.
const int chipSelect = 10;

void setup() 
{
    Serial.begin(9600);
    pins_init();
    clock.begin();
    File dataFile;
    // Only need to run this once with the battery on the clock. otherwise comment it out..
    // be sure to check that the little battery is installed in the RTC
    // change values to the current ones of time/date
    if (SET_CLOCK_FLAG == true)
    {
      clock.fillByYMD(current_year,current_mon,current_day); //Jan 19,2013
      clock.fillByHMS(current_hour,current_min,current_sec); //15:28 30"
      clock.fillDayOfWeek(CURRENT_DAY); // e.g., FRI
      clock.setTime();//write time to the RTC chip
    }
    

    
    dht.begin();  // temp and humidity
    pinMode(8,INPUT);
    starttime = millis();//get the current time;
    // make sure that the default chip select pin is set to
    // output, even if you don't use it:
    pinMode(SS, OUTPUT);
    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
       Serial.println("Card failed, or not present");
      // don't do anything more:
       while (1) ;
     }
     // make sure that the default chip select pin is set to
     // output, even if you don't use it:
     pinMode(10, OUTPUT);
     Serial.println("card initialized.");
    // Open up the file we're going to log to!
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (! dataFile) {
        Serial.println(F("error opening datalog.txt"));
        // Wait forever since we cant write data
          while (1) ;
    } 
    dataFile.println(F("Date,Day,Time,Temp,Humidity,Polution_Value,Sound,Max_Sound,PM10Count,PM20Count,Concentration_Large,Concentration_Small,CO_ppm,CO_Volts,NO2_ppm,NO2_Volts"));
    Serial.println(F("Date,Day,Time,Temp,Humidity,Polution_Value,Sound,Max_Sound,PM10Count,PM20Count,Concentration_Large,Concentration_Small,CO_ppm,CO_Volts,NO2_ppm,NO2_Volts"));
    dataFile.close();
    // setup air quality sensor
    //airqualitysensor.init(A0);
}

String printAirQuality()
{
        delay(500);
        int first_vol=analogRead(AIR_QUALITY);

        String pollution;
        Serial.print(first_vol);
        Serial.print(F(","));
        String str = String(first_vol)+",";
        return str;
}
String printSound()
{
        delay(500);
        Serial.print(soundValue);
        Serial.print(F(","));
        Serial.print(maxSound);
        Serial.print(F(","));

        String str1 = String(soundValue) + "," + String(maxSound) + ",";
        maxSound=0;
        soundValue = 0; //reset sensor value
        return str1;
}

void pins_init()
{
	pinMode(SOUND_SENSOR, INPUT); 
        pinMode(AIR_QUALITY,INPUT);
        pinMode(DHTPIN,INPUT);
}

//Function: Display time on the serial monitor 
String printTime()
{	
  clock.getTime();
        
	Serial.print(clock.month, DEC);
	Serial.print(F("/"));

	Serial.print(clock.dayOfMonth, DEC);
	Serial.print(F("/"));

	Serial.print(clock.year+2000, DEC);
	Serial.print(F(","));
       
        switch (clock.dayOfWeek)// Friendly printout the weekday
	{
		case MON:
		  Serial.print(F("MON"));
     
		  break;
		case TUE:
		  Serial.print(F("TUE"));
 
		  break;
		case WED:
		  Serial.print(F("WED"));
    
		  break;
		case THU:
		  Serial.print(F("THU"));
     
		  break;
		case FRI:
		  Serial.print(F("FRI"));
       
		  break;
		case SAT:
		  Serial.print(F("SAT"));
             
		  break;
		case SUN:
		  Serial.print(F("SUN"));

		  break;
	}
        
        Serial.print(F(","));
	Serial.print(clock.hour, DEC);
	Serial.print(F(":"));
	Serial.print(clock.minute, DEC);

	Serial.print(F(":"));
	Serial.print(clock.second, DEC);
        Serial.print(F(","));

        String str = String(clock.month)+"/"+String(clock.dayOfMonth)
            +"/"+String(clock.year)+","+String(clock.dayOfWeek)+","+String(clock.hour)+":"
            +String(clock.minute)+":"+String(clock.second)+",";
        return str;
}

String printTempHumidity()
{
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to A0 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    // check if returns are valid, if they are NaN (not a number) then something went wrong!
    if (isnan(t) || isnan(h))
    {
        Serial.println(F("Failed to read from DHT"));
        return "error";
    }
    else
    {
      String str =  String(t) + "," + String(h) + ",";       
      //Serial.print("Temperature: ");
        Serial.print(t);
        Serial.print(F(","));
        //Serial.println(" *C");
       // Serial.print("Humidity: ");
        Serial.print(h);
        //Serial.print(" %\t");
        Serial.print(",");
  return str;  
  }
}

/* print out the current dust value
String printDust-old()
{

    ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62; // using spec sheet curve
    //Serial.print("DUST: ");
    String str = String(lowpulseoccupancy) + "," + String(ratio) + "," + String(concentration) + ",";
    Serial.print(lowpulseoccupancy);
    Serial.print(F(","));
    Serial.print(ratio);
    Serial.print(F(","));
    Serial.print(concentration);
    Serial.print(F(","));
    lowpulseoccupancy = 0; // this resets the counter

  return str;

}
*/

String printDust()
{
      ratioP1 = durationP1/(sampletime_ms*10.0); // Integer percentage 0=>100
      ratioP2 = durationP2/(sampletime_ms*10.0);
      countP1 = 1.1*pow(ratioP1,3)-3.8*pow(ratioP1,2)+520*ratioP1+0.62;
      countP2 = 1.1*pow(ratioP2,3)-3.8*pow(ratioP2,2)+520*ratioP2+0.62;
      float PM10count = countP2;
      float PM25count = countP1 - countP2;
      
      // first, PM10 count to mass concentration conversion
      double r10 = 2.6*pow(10,-6);
      double pi = 3.14159;
      double vol10 = (4/3)*pi*pow(r10,3);
      double density = 1.65*pow(10,12);
      double mass10 = density*vol10;
      double K = 3531.5;
      float concLarge = (PM10count)*K*mass10;
      
      // next, PM2.5 count to mass concentration conversion
      double r25 = 0.44*pow(10,-6);
      double vol25 = (4/3)*pi*pow(r25,3);
      double mass25 = density*vol25;
      float concSmall = (PM25count)*K*mass25;

      String str = String(PM10count) + "," + String(PM25count) + "," + String(concLarge) + "," + String(concSmall) + ",";
      return str;
}
 
/// print out ppm for NO2
String print_NO2()
{
  
    int PullUp = 10000;
    int Resistance = 90000;
    float hi;
    float mid;
    float sf;
    float ppm;
    //conversions = [((0,100),(0,0.25)),((100,133),(0.25,0.325)),((133,167),(0.325,0.475)),((167,200),(0.475,0.575)),((200,233),(0.575,0.665)),
    //((233,267),(0.666,0.75))]
     int rs = analogRead(NO2_INPUT_PIN);
     
     //Serial.print("NO2: ");
     //Serial.println(rs);
     float vin = (1024 / 3.3) * 5;
     //Serial.print("NO2: ");
     //Serial.println(rs);     
     float rs_value = (vin/rs - 1)*PullUp;
     float rsper = 100.00*(float(rs_value)/float(Resistance));
     //Serial.println(rsper);
     if (rsper>= 0 && rsper<100)
     {
       mid = float(rsper);
       hi= 100.0;
       sf = float(mid)/hi;
       ppm = sf * 0.25;
     } else if ( rsper>= 100 && rsper <133) 
     {
       mid=float(rsper)-100.0;
       hi=33.00;
       sf = float(mid)/hi;
       ppm = sf * (0.325-0.25) + 0.25;
     } else if ( rsper >= 133 && rsper <167)
     {  
       mid=float(rsper)-133.0;
       hi=33.0;
       sf = float(mid)/hi;
       ppm = sf * (0.475-0.325) + 0.325;
     } else if (rsper >= 167 && rsper < 200)
     {
       mid=float(rsper)-167.0;
       hi=33.0;
       sf = float(mid)/hi;
       ppm = sf * (0.575-0.475) + 0.475;
    } else if (rsper >= 200 && rsper < 233)
    {
      mid=float(rsper)-200.0;
      hi = 33.0;
      sf = float(mid)/hi;
      ppm = sf * (0.665-0.575) + 0.575;
    } else if (rsper >=233 && rsper <267)
    {
      mid=float(rsper)-233.0;
      hi = 33.0;
      sf= float(mid)/hi;
      ppm = sf * (0.75-0.666) + 0.666;
    } else {
      ppm = rsper * 0.002808988764;
    } 
    
    String str = String(ppm) + "," + String(rs);
    Serial.print(ppm);
    Serial.print(F(","));
    Serial.print(rs);

    return str;
 } 

String print_CO()
{
    int PullUp = 100000;
    int Resistance = 190000;
    float hi;
    float mid;
    float sf;
    float ppm;
    //conversions = [((110,90),(0,1.5)),((90,85),(1.5,5)),((85,80),(5,6)),((80,75),(6,7)),
    //((75,70),(7,8)),((70,65),(8,12)),((65,60),(12,14)),((60,55),(14,18)),((55,50),(18,22))]
     int rs = analogRead(CO_INPUT_PIN);
    // Serial.print("CO: ");
    // Serial.println(rs);
     
     float vin = (1024 / 3.3) * 5;
     float rs_value = (vin/rs - 1)*PullUp;
     float rsper = 100.00*(float(rs_value)/float(Resistance));   
    //Serial.println("");
    //Serial.println(rsper);  
     if (rsper> 110) 
     {
       ppm = 0.0;
     } else if (rsper> 90 && rsper <=110)
     {
       mid = float(rsper)-90.0;
       hi= -5.0;
       sf = float(mid)/hi;
       ppm = sf * (1.5);
     } else if (rsper> 85 && rsper <=90)
     {
       mid=float(rsper)-85.0;
       hi=-5.0;
       sf = float(mid)/hi;
       ppm = sf * (5-1.5)+1.5;
     } else if (rsper> 80 && rsper <=85)
     {  
       mid=float(rsper)-80.0;
       hi=-5.0;
       sf = float(mid)/hi;
       ppm = sf * (6-5)+5;
     } else if (rsper> 75 && rsper <=80)
     {
       mid=float(rsper)-75.0;
       hi=-5.0 ;
       sf = float(mid)/hi;
       ppm = sf * (7-6)+6;
    } else if (rsper> 70 && rsper <=75)
    {
      mid=float(rsper)-70.0;
      hi = -5.0;
      sf = float(mid)/hi;
      ppm = sf * (8-7) + 7;
    } else if (rsper> 65 && rsper <=70)
    {
      mid=float(rsper)-65.0;
      hi = -5.0;
      sf= float(mid)/hi;
      ppm = sf * (12-8) +8;
     } else if (rsper> 60 && rsper <=65)
     {  
       mid=float(rsper)-60.0;
       hi=-5.0;
       sf = float(mid)/hi;
       ppm = sf * (14-12)+12;
     } else if (rsper> 55 && rsper <= 60)
     {
      mid=float(rsper)-55.0;
       hi=-5.0;
       sf = float(mid)/hi;
       ppm = sf * (18-14)+14;
     } else if (rsper> 50 && rsper <= 55)
     {
       mid=float(rsper)-50.0;
       hi=-5.0;
       sf = float(mid)/hi;
       ppm = sf * (22-18)+18;
    } else {
      ppm = (1/float(rsper))*1100;
    } 
    String str = String(ppm) + "," + String(rs) + ",";
    Serial.print(ppm);
    Serial.print(F(","));
    Serial.print(rs);
    Serial.print(F(","));

return str;
 } 
 
 
void loop() 
{
  
     //current_quality=airqualitysensor.slope();
     //Serial.print("Air Quality: ");
     //Serial.println( current_quality);
      // make the dust measurements over time
      duration = pulseIn(pin, LOW);
      lowpulseoccupancy = lowpulseoccupancy+duration;
      // accumulate the sound value
      int instantSound = analogRead(SOUND_SENSOR);
      soundValue = soundValue + instantSound;
      if (instantSound > maxSound)
      {
        maxSound=instantSound;
      }
      valP1 = digitalRead(8);
      valP2 = digitalRead(9);
      //for the dust
        if(valP1 == LOW && triggerP1 == false){
            triggerP1 = true;
            triggerOnP1 = micros();
        }
  
        if (valP1 == HIGH && triggerP1 == true){
            triggerOffP1 = micros();
            pulseLengthP1 = triggerOffP1 - triggerOnP1;
            durationP1 = durationP1 + pulseLengthP1;
            triggerP1 = false;
        }
        
          if(valP2 == LOW && triggerP2 == false){
          triggerP2 = true;
          triggerOnP2 = micros();
        }
        
          if (valP2 == HIGH && triggerP2 == true){
            triggerOffP2 = micros();
            pulseLengthP2 = triggerOffP2 - triggerOnP2;
            durationP2 = durationP2 + pulseLengthP2;
            triggerP2 = false;
        }

      delay(100);
      if ((millis()-starttime) > sampletime_ms)//if the sample time == 30s
      {
        // open the file. note that only one file can be open at a time,
        // so you have to close this one before opening another.
        File dataFile = SD.open("datalog.txt", FILE_WRITE);
        String s1 = printTime();
        String s2 = printTempHumidity();
        //String s3;
        String s3 = printAirQuality();
        String s4 = printSound();
        String s5 = printDust();
        String s6 = print_CO();
        String s7 = print_NO2();
        starttime = millis();
        Serial.println(F(""));
        String output= "";
        output = s1+s2+s3+s4+s5+s6+s7;
        dataFile.println(output);
        Serial.print(F("Output: "));
        Serial.println(output);
        maxSound=0;
        dataFile.close();
      }
}
 

