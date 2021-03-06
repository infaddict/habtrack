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

////////////////////////////////////////////////////////////////////////////////
// setTXString()
// Prepares the message string to be sent via RTTY.  
////////////////////////////////////////////////////////////////////////////////
void setDataString()
{
  // Sentence format is comma separated:
  // $$payload    $$INFCU1
  // Sentence id  00000
  // No of sats   00
  // Time         00:00:00 (hours, minutes, seconds)
  // Latitude     -0000000 (degrees)
  // Longtitude   -0000000 (degrees)
  // Altitude     -000000 (metres)
  // Int Temp     -00.00 (degrees celsius)
  // Ext Temp     -00.00 (degrees celsius)
  // Voltage      -00.00 (volts)
  
  uint16_t checkSum;
  char checkSumStr[6+1];  // includes \n char on end
  char temp[TX_SIZE];
  char cintTemp[5+1];
  char cextTemp[5+1];
  char cvoltage[5+1];

  // Always format the temperatures and voltage
  dtostrf(intTemp, 2, 1, cintTemp);  
  dtostrf(extTemp, 2, 1, cextTemp);    
  dtostrf(voltage, 2, 1, cvoltage);

  if (gpsLock)
  {
    // We have a good GPS lock so use GPS information to send via radio
    snprintf(temp,80,"$$INFCU1,%i,%u,%02u:%02u:%02u,%li,%li,%li,%s,%s,%s", sentenceId,  gpsInfo.numSV, gpsInfo.Hour, gpsInfo.Min, gpsInfo.Sec, gpsInfo.Lat, gpsInfo.Long, gpsInfo.HeightMSL, cintTemp, cextTemp, cvoltage);
  }
  else
  {
    // We have no GPS lock, either because we've never had one or we've lost it, so use last known good info
    snprintf(temp,80,"$$INFCU1,%i,%u,%02u:%02u:%02u,%li,%li,%li,%s,%s,%s", sentenceId,  gpsInfo.numSV, gpsInfo.Hour, gpsInfo.Min, gpsInfo.Sec, lastGoodFix.Lat, lastGoodFix.Long, lastGoodFix.HeightMSL, cintTemp, cextTemp, cvoltage);
  }
   
  // Create a checksum and add to end of the sentence
  checkSum = gpsCRC16Checksum(temp);   
  sprintf(checkSumStr, "*%04X\n", checkSum);  // Format the checksum correctly
  strcat(temp,checkSumStr);
  strcpy(dataString,temp);  

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

