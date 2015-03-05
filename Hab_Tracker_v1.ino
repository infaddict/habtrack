////////////////////////////////////////////////////////////////////////////////
// Hab_Tracker.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    Main module for HAB Tracker
////////////////////////////////////////////////////////////////////////////////
#include <Wire.h>
#include "HT_GPS.h"
const byte RED_LED_PIN = 8;      // for fatal errors
const byte GREEN_LED_PIN = 7;    // for GPS lock
unsigned long timeOfLock = 0;
GPS_INFO gpsInfo;
GPS_INFO lastGoodFix;
char dataString[85];    // must be big enough for largest RTTY sentence
char txString[85];      // must be big enough for largest RTTY sentence
boolean gpsLock=false;
float intTemp;
float extTemp;
float voltage;
long timeForGPS;
long timeForSensors;
boolean expectingGPSData=false;

////////////////////////////////////////////////////////////////////////////////
// setup()
// Setup everything we need before we enter main loop
// Called once, automatically, when Arduino starts up
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  
  Serial.println();
  Serial.println(F("Start of setup."));
  Serial.print(F("Free memory = "));
  Serial.println(freeRam());
  delay(1000);
  
  // Setup LED's and turn them off
  Serial.println(F("Preparing LED's..."));
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
    
  Serial.println(F("Preparing SD Card..."));
  if (!setupSDCard())
  {
    Serial.println(F("ERROR IN SDCARD INIT!!"));
    digitalWrite(RED_LED_PIN, HIGH);
  }
  
  // Prepare wire for master mode where we control I/O to GPS
  Wire.begin();    
   
  // Set the protocol to send and receive UBX only (disable NMEA)
  writeLog(F("Configuring TX and RX protocols..."));
  if (!sendUBX(portUBX_UBX, sizeof(portUBX_UBX)))
  {
    writeLog(F("ERROR SENDING PRT UBX-UBX COMMAND!!"));
    digitalWrite(RED_LED_PIN, HIGH);
  }
  if (!checkAck(portUBX_UBX))
  {
    writeLog(F("ACK NOT REVCV FOR PRT UBX-UBX!!"));
    digitalWrite(RED_LED_PIN, HIGH);
  }
  
  // Set the GPS to "Airborne <1g" mode suitable for high altitude
  writeLog(F("Setting airborne mode..."));
  if (!sendUBX(airborne1g, sizeof(airborne1g)))
  {
    writeLog(F("ERROR SENDING FLIGHT MODE COMMAND!!"));
    digitalWrite(RED_LED_PIN, HIGH);
  }
  if (!checkAck(airborne1g))
  {
    writeLog(F("ACK NOT RECV FOR FLIGHT MODE!!"));
    digitalWrite(RED_LED_PIN, HIGH);
  }
  
  // Setup the Radio and interrupts for RTTY transmission
  writeLog(F("Preparing Radio..."));
  setupRadio();
  setupInterrupt();
  
  // Setup the internal and external temperature sensors.
  writeLog(F("Preparing Temperature Sensors..."));
  setupTemperature();
  
  // Finished all setup tasks so show the green LED for 5 seconds to signal this
  writeLog(F("End of setup."));
  digitalWrite(GREEN_LED_PIN, HIGH);    
  delay(5000);    
  digitalWrite(GREEN_LED_PIN, LOW);
}

////////////////////////////////////////////////////////////////////////////////
// loop()
// Main processing loop for Arduino.
// This loops infinitely whist Arduino is powered.
// The loop is kept mostly free and we control what we want to do when with the
// use of millis().  Requests for data will hold his loop as follows:
//    - GPS data = 800ms to 1000ms
//    - Temperature = <150ms
//    - Voltage = < 50ms
////////////////////////////////////////////////////////////////////////////////
void loop()
{  
  // If we've previosuly requested GPS data then go check for it arriving.
  // This will timeout in 3 seconds if data not received.
  // Data usually arrives in < 1 second.
  if (expectingGPSData)
  {
    if (!checkForGPSData())
    {
      writeLog(F("ERROR READING UBX DATA!!"));
      digitalWrite(RED_LED_PIN, HIGH);
    }
    
    // Check for GPS lock.  We consider anything more than 4 satellites as good
    checkGPSLock();
  }
      
  // Set the TX string to contain current known variables
  setTXString(dataString);
  
  // If it's time to ask for GPS data then do it
  if (millis() > timeForGPS)
  {
    expectingGPSData = true;
    
    // Send request for a NAV-PVT message (position, velocity, time)
    if (!sendUBX(reqNAV_PVT, sizeof(reqNAV_PVT)))
    {
      writeLog(F("ERROR SENDING NAV PVT COMMAND!!"));
      digitalWrite(RED_LED_PIN, HIGH);
    }
   
    // set up when we next want to do this
    timeForGPS = millis() + 3000; 
  }
  
  // If it's time to ask for sensor data then do it
  if (millis() > timeForSensors)
  {
    // go and get temperature data
    checkTemperature();

    // go and get voltage data
    voltage = getVoltage();
    
    // set up when we next want to do this
    timeForSensors = millis() + 10000; 
  }
  
  
}
