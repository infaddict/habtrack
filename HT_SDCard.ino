////////////////////////////////////////////////////////////////////////////////
// HT_SDCard.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    SD Card data logger
////////////////////////////////////////////////////////////////////////////////

#ifdef SDLOG    // Use the SDLOG definition to control whether we include SD header & variables
#include <SdFat.h>
#define SD_SS_PIN 10      // CS/SS pin
SdFat sd;
SdFile myFile;
char fileName[12+1]; 
#endif

////////////////////////////////////////////////////////////////////////////////
// setupSDCard()
// Setup the SD card and create a new log file using the next spare log file
// number.
// Returns true if all good or false if there is a problem.
////////////////////////////////////////////////////////////////////////////////
#ifdef SDLOG    // Use the SDLOG definition to control whether we include this routine
boolean setupSDCard()
{  
  // Prepare for SD card
  if (!sd.begin(SD_SS_PIN, SPI_HALF_SPEED)) 
  {
    Serial.println(F("SD Card begin failed!"));
    return false;
  }
  
  // Search for a log file that has not been created (e.g. "LOG00123.txt")
  for(int i=1; i<10000;i++)
  {
    sprintf(fileName,"LOG%04i.txt",i);
    if (!sd.exists(fileName))
    {
      Serial.print(F("Creating log file "));
      Serial.println(fileName);
      writeLog(F("** START OF LOG **"));
      
      return true;
    }
  }
  
}
#endif

////////////////////////////////////////////////////////////////////////////////
// writeLog()
// Write a string of data to the SD card and serial debug
////////////////////////////////////////////////////////////////////////////////
void writeLog(String data)
{
  #ifdef SDLOG    // Use the SDLOG definition to control whether we open & write to SD card
  
  // Open the file.  It should not exist first time thru, so create it.
  // Subsequent times thru, open it and move to end of file.
  if (!myFile.open(fileName, O_WRITE | O_CREAT | O_AT_END)) 
  {
    Serial.println(F("SD WRITELOG ERROR!"));
  }
  else
  {  
    // Write a line to the log including flight time and the data string
    myFile.println(data);
    myFile.close();
  }
  
  #endif
  
  // Write the same data to the serial port
  Serial.println(data); 
  
}

// Overload method to accept a char array
void writeLog(char data[])
{
  writeLog((String)data);
}

