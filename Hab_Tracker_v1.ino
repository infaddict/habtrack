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
const int RED_LED_PIN = 8;
const int GREEN_LED_PIN = 7;
unsigned long flightTime;
unsigned long timeOfLock = 0;
NAV_PVT navPVT;
char dataString[95];
char txString[95];
boolean gpsLock=false;
float intTemp;
float extTemp;
float voltage=7.12;


////////////////////////////////////////////////////////////////////////////////
// setup()
// Setup everything we need before we enter main loop
// Called once, automatically, when Arduino starts up
////////////////////////////////////////////////////////////////////////////////
void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Start of setup...");
  Serial.print("Free memory = ");
  Serial.println(freeRam());
  
  Wire.begin();    // Prepare wire for master mode where we control I/O  
  
  // Setup LED's and turn them off
  Serial.println("Preparing LED's...");
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);
  
  // Set the protocol to send and receive UBX only (disable NMEA)
  Serial.println("Configuring TX and RX protocols");
  if (!sendUBX(portUBX_UBX, sizeof(portUBX_UBX)))
  {
    Serial.println("ERROR SENDING PRT UBX-UBX COMMAND!!");
    digitalWrite(RED_LED_PIN, HIGH);
  }
  if (!checkAck(portUBX_UBX))
  {
    Serial.println("ACK NOT REVCV FOR PRT UBX-UBX!!");
    digitalWrite(RED_LED_PIN, HIGH);
  }

  // Set the GPS to "Airborne <1g" mode suitable for high altitude
  Serial.println("Setting airborne mode");
  if (!sendUBX(airborne1g, sizeof(airborne1g)))
  {
    Serial.println("ERROR SENDING FLIGHT MODE COMMAND!!");
    digitalWrite(RED_LED_PIN, HIGH);
  }
  if (!checkAck(airborne1g))
  {
    Serial.println("ACK NOT RECV FOR FLIGHT MODE!!");
    digitalWrite(RED_LED_PIN, HIGH);
  }
  
  Serial.println("Preparing Radio...");
  setupRadio();
  setupInterrupt();
   
  Serial.println("End of setup.");
}


////////////////////////////////////////////////////////////////////////////////
// loop()
// Main processing loop for Arduino
// This loops infinitely whist Arduino is powered
////////////////////////////////////////////////////////////////////////////////
void loop()
{
  delay(500);
  
  intTemp = getInternalTemp();
  extTemp = getExternalTemp();
 
 //TEMP STUFF
/*
  navPVT.Valid = 255;
  navPVT.Year = 2015;
  navPVT.Month = 12;
  navPVT.Day = 31;
  navPVT.Hour = 23;
  navPVT.Min = 59;
  navPVT.Sec = 59;
  gpsLock = true;
  navPVT.Lat =  1799001234;
  navPVT.Long = -1799040556;
  navPVT.HeightMSL = 42492;
  intTemp = -20.99;
  extTemp = -46.05;

  voltage = 3.39;  
*/

  setTXString(dataString);
  Serial.println(dataString);
   
  // Convert time running in milliseconds to seconds
  flightTime = millis() * 0.001;
  
  // Send request for a NAV-PVT message (position, velocity, time)
  if (!sendUBX(reqNAV_PVT, sizeof(reqNAV_PVT)))
  {
    Serial.println("ERROR SENDING NAV PVT COMMAND!!");
    digitalWrite(RED_LED_PIN, HIGH);
  }
  
  // Read any data returned by the GPS
  if (!readUBX())
  {
    Serial.println("ERROR READING UBX DATA!!");
    digitalWrite(RED_LED_PIN, HIGH);
  }
  
  // Check for GPS lock.  We consider anything more than 4 satellites as good
  if ((int)navPVT.FixType >= 3 && (int)navPVT.numSV > 4)
  {
    gpsLock = true;
    if (timeOfLock == 0)
    {
      Serial.println("GPS LOCK !! GPS LOCK !! GPS LOCK !! GPS LOCK !!");
      digitalWrite(GREEN_LED_PIN, HIGH);
      timeOfLock = flightTime;
    }
    else  // already locked on
    {
      if (flightTime > timeOfLock + 60)  // only show the green LED for 60 seconds
      {
        digitalWrite(GREEN_LED_PIN, LOW);
      }
    }
  }
  else
  {
    gpsLock = false;
  }
  
}