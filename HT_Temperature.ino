////////////////////////////////////////////////////////////////////////////////
// HT_Temperature.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    DS18B20 temperature sensor.  We have 2 sensors, internal and exteral
////////////////////////////////////////////////////////////////////////////////
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 9

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

// Array to hold up to 8 device addresses
DeviceAddress internalSensor, externalSensor;

////////////////////////////////////////////////////////////////////////////////
// setupTemperature()
// Prepare the temperature sensors by detecting them and setting resolution
////////////////////////////////////////////////////////////////////////////////
void setupTemperature()
{
  // Start up the library
  sensors.begin();

  // Locate devices on the bus
  writeLog(F("Locating devices...Found "));
  String num = (String) sensors.getDeviceCount();
  writeLog(num);
  writeLog(F("devices."));

  // Set the resolution of all sensors to 9 bit (0.5 degree precision and 78ms update time)
  sensors.setResolution(TEMPERATURE_PRECISION);

}

////////////////////////////////////////////////////////////////////////////////
// checkTemperature
// Request temperature reading from both internal and external sensors
////////////////////////////////////////////////////////////////////////////////
void checkTemperature()
{
  // Request the temperature from all sensors.  This won't complete until data is ready to be read.
  sensors.requestTemperatures(); 
  
  // At this point the data is ready so go and read it
  intTemp = sensors.getTempCByIndex(0);
  extTemp = sensors.getTempCByIndex(1);
}

