////////////////////////////////////////////////////////////////////////////////
// HT_Temperature.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    DS18B20 temperature sensor.  We have 2 sensors, internal and exteral
////////////////////////////////////////////////////////////////////////////////
#include <OneWire.h>

const int TempInt_Pin = 2; // on pin 2 (with 4.7k resistor in series)
const int TempExt_Pin = 4; // on pin 4 (with 4.7k resistor in series)

////////////////////////////////////////////////////////////////////////////////
// getInternalTemp()
// Returns internal temperature in degrees celsius
// Returns -1000 if any error occurs
////////////////////////////////////////////////////////////////////////////////
float getInternalTemp()
{
  return getTemp(TempInt_Pin);
}

////////////////////////////////////////////////////////////////////////////////
// getExternalTemp()
// Returns external temperature in degrees celsius
// Returns -1000 if any error occurs
////////////////////////////////////////////////////////////////////////////////
float getExternalTemp()
{
  return getTemp(TempExt_Pin);
}

////////////////////////////////////////////////////////////////////////////////
// getTemp()
// Returns temperature in degrees celsius for the selected pin
// Returns -1000 if any error occurs
////////////////////////////////////////////////////////////////////////////////
float getTemp(unsigned int pin)
{
  OneWire ds(pin);
  byte data[12];
  byte addr[8];
  
  // Search for sensors on the bus
  if (!ds.search(addr)) 
  {
    // No more sensors found so reset the search
    Serial.println("No more sensors found!");
    ds.reset_search();
    return -1000;
  }
  
  // Check the checksum of the sensor ROM id
  if (OneWire::crc8( addr, 7) != addr[7]) 
  {
    Serial.println("CRC is not valid!");
    return -1000;
  }
  
  // Check what type of devie we have found
  // 0x10 = DS18S20
  // 0x28 = DS18B20
  // 0x22 = S1822
  if (addr[0] != 0x28) 
  {
    Serial.print("Device is not recognized");
    return -1000;
  }
  
  // What is this doing?
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // Start conversion, with parasite power on at the end
  
  // What is this doing?
  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad
  
  // Read 8 bytes of data
  for (int i = 0; i < 9; i++) 
  {
    data[i] = ds.read();
  }
  
  ds.reset_search();  // Is this required?
  
  byte MSB = data[1];
  byte LSB = data[0];
  
  float tempRead = ((MSB << 8) | LSB); // using two's compliment
  float TemperatureSum = tempRead / 16.0;
  
  return TemperatureSum;

}
