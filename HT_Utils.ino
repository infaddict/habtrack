////////////////////////////////////////////////////////////////////////////////
// HT_Utils.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    General purpose routines
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// join4Bytes()
// Joins 4 bytes/unsigned chars into a long and returns it
////////////////////////////////////////////////////////////////////////////////
long join4Bytes(unsigned char data[])
{
  union long_union 
  {
    int32_t dword;
    uint8_t  byte[4];
  } longUnion;

  longUnion.byte[0] = *data;
  longUnion.byte[1] = *(data+1);
  longUnion.byte[2] = *(data+2);
  longUnion.byte[3] = *(data+3);
  return(longUnion.dword);
}

////////////////////////////////////////////////////////////////////////////////
// join2Bytes()
// Joins 2 bytes/unsigned chars into a short and returns it
////////////////////////////////////////////////////////////////////////////////
short join2Bytes(unsigned char data[])
{
  union long_union 
  {
    int16_t dword;
    uint8_t  byte[2];
  } longUnion;

  longUnion.byte[0] = *data;
  longUnion.byte[1] = *(data+1);
  return(longUnion.dword);
}

////////////////////////////////////////////////////////////////////////////////
// freeRam()
// Returns the amount of freeRam in bytes
////////////////////////////////////////////////////////////////////////////////
// Return the free RAM
int freeRam(void)
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void setTXString(char *msg)
{
  // Sentence format is comma separated:
  // $$payload    $$INFCU1
  // Flight Time  000000 (seconds)
  // Date         00000000 (YYYYMMDD)
  // Time         00:00:00 (hours, minutes, seconds)
  // Latitude     +-000.00000 (degrees)
  // Longtitude   +-000.00000 (degrees)
  // Altitude     +-000000 (metres)
  // Int Temp     +-00.00 (degrees celsius)
  // Ext Temp     +-00.00 (degrees celsius)
  // Voltage      +-00.00 (volts)
  
  unsigned int checkSum;
  char checkSumStr[5+1];
  char cdate[8+1];
  char ctime[8+1];
  long left;
  long right;
  char clat[10+1];
  char clon[10+1];
  char calt[7+1];
  char cintTemp[6+1];
  char cextTemp[6+1];
  char cvoltage[5+1];


  if (isBitSet(navPVT.Valid, 2))  // checks for fully valid UTC date/time (3rd bit)
  {
    sprintf(cdate, "%04u%02u%02u", navPVT.Year, navPVT.Month, navPVT.Day);
    sprintf(ctime, "%02u:%02u:%02u", navPVT.Hour, navPVT.Min, navPVT.Sec);
  }
  else
  {
    strcpy(cdate,"00000000");
    strcpy(ctime,"00:00:00");
  }

  if (gpsLock && navPVT.Lat != 0.0 && navPVT.Long != 0.0)
  {
   left = navPVT.Lat / LatLongFactor;
   right = abs((navPVT.Lat - (left * LatLongFactor)) / 100);
   sprintf(clat, "%03li.%05lu", left, right);
   
   left = navPVT.Long / LatLongFactor;
   right = abs((navPVT.Long - (left * LatLongFactor)) / 100);
   sprintf(clon, "%03li.%05lu", left, right);  
  
   sprintf(calt, "%06li", navPVT.HeightMSL); 
  }
  else
  {
    strcpy(clat,"+000.00000");
    strcpy(clon,"+000.00000");
    strcpy(calt,"+000000");
  }

  dtostrf(intTemp, 2, 2, cintTemp);  
  dtostrf(extTemp, 2, 2, cextTemp);    
  dtostrf(voltage, 2, 1, cvoltage);

  sprintf(msg,"$$INFCU1,%06li,%s,%s,%s,%s,%s,%s,%s,%s",flightTime,cdate,ctime,clat,clon,calt,cintTemp,cextTemp,cvoltage);  
  checkSum = gpsCRC16Checksum(msg);  // Calculates the checksum for this datastring
  sprintf(checkSumStr, "*%04X\n", checkSum);  // Format the checksum correctly
  strcat(msg,checkSumStr);  // Add the calculated checksum to end of string

}


bool isBitSet(unsigned char data, unsigned int bitindex)
{
    return (data & (1 << bitindex)) != 0;
}

////////////////////////////////////////////////////////////////////////////////
// gpsCRC16Checksum()
// Returns a CRC16 checksum for the provided string pointer
////////////////////////////////////////////////////////////////////////////////
uint16_t gpsCRC16Checksum (char *string)
{
  // Calculate a CRC16 checksum for the string parameter
  size_t i;
  uint16_t crc;
  uint8_t c;
 
  crc = 0xFFFF;
 
  // Calculate checksum ignoring the first two $s
  for (i = 2; i < strlen(string); i++)
  {
    c = string[i];
    crc = _crc_xmodem_update (crc, c);
  }
 
  return crc;
} 
