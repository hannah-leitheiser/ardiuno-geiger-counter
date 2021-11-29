/* Name of File  : geiger.ino
 * Type of File  : Arduino Studio Source File
 * Author        : Hannah Leitheiser
 * Date          : 2021NOV
 * Description   : Count Geiger pulses and record the count
 *               : in a text file on an SD card along with 
 *               : a timestamp in UTC.  Peridically reset the
 *               : RTC to GPS time.
 * Wiring        : An Ardiuno Uno connected to an older
 *               : Adafruit Datalogger Sheild with RTC and
 *               : SD card reader.  A NEO-6M GPS chip is 
 *               : connected to digital
 *               : pins 8 and 9, and Geekcreit
 *               : Geiger Counter send pulses on geigerPin.
 */

#include <TinyGPSPlus.h>
#include <AltSoftSerial.h>
#include <SPI.h>
#include <SD.h>
#include <RTClib.h>

const uint32_t serialBaud = 9600;

// Geiger Counter
const byte geigerPin = 3;
const unsigned int updateIntervalMS = 5000;
unsigned int pulseCount = 0;

// GPS, serial pins 8, 9
const uint32_t gpsBaud = 9600;
AltSoftSerial gpsSerial;
TinyGPSPlus gps;
const int minimumYear = 2021;
const uint32_t updateRTCMS = 3600000; // 1 hr

// RTC: pins A4, A5
const int chipSelect = 10;
RTC_DS1307 rtc;

uint32_t lastSaveMS = 0;
uint32_t clockSetMS = 0;

void addPulse() {
  pulseCount++;
}

void setup() {

  pinMode(geigerPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(geigerPin), addPulse, FALLING);
  
  gpsSerial.begin(gpsBaud);
  Serial.begin(serialBaud);

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    exit(0);
  }
  
  //rtc.adjust(DateTime(2021, 11, 28, 10, 11, 0));
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
  Serial.print("Initializing SD c1ard...");

  // keep trying
  while (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    SD.end();
    }
  
  Serial.println("card initialized.");
}

void loop() {

   while( gpsSerial.available() > 0) { 
      gps.encode(gpsSerial.read()); 
      }

   if (gps.date.isValid() && gps.date.year() >= minimumYear && gps.time.isValid() && millis() - clockSetMS > updateRTCMS ) 
      {
      clockSetMS = millis();
      rtc.adjust(DateTime(gps.date.year(), gps.date.month(), gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second()));
      Serial.print(F("GPS Clock Set: "));
      Serial.print(gps.time.hour());
      Serial.print(F(":"));
      Serial.print(gps.time.minute());
      Serial.print(F(":"));
      Serial.print(gps.time.second());
      Serial.println();
      }   


      if ( millis() > lastSaveMS + updateIntervalMS) {
         lastSaveMS=updateIntervalMS*(millis()/updateIntervalMS);
      
         File dataFile = SD.open("geiger.txt", FILE_WRITE);
      
         // if the file is available, write to it:
         if (dataFile) {
            dataFile.print(F("Geiger Count:"));
            Serial.print(F("Geiger Count:"));
            dataFile.print(rtc.now().timestamp(DateTime::TIMESTAMP_FULL));
            Serial.print(rtc.now().timestamp(DateTime::TIMESTAMP_FULL));
            dataFile.print(F(":"));
            Serial.print(F(":"));
            dataFile.print( pulseCount);
            Serial.print( pulseCount);
            dataFile.println();
            Serial.println();
            dataFile.close();
            }
         // if the file isn't open, pop up an error:
         else {
            Serial.println("error opening datalog.txt");
         }
      }
   }
