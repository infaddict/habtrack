////////////////////////////////////////////////////////////////////////////////
// HT_GPS.h
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    Variables and constants for GPS
////////////////////////////////////////////////////////////////////////////////
#define GPS_ADDR   0x42    // Default address for uBlox M8 GPS
const unsigned char UBX_MAX_PAYLOAD = 100;  // set to maximum message you want to receive
const unsigned char UBX_SYNC_CHAR1 = 0xB5;
const unsigned char UBX_SYNC_CHAR2 = 0x62;

unsigned char airborne1g[]  = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};
unsigned char portUBX_UBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x96}; 
unsigned char reqNAV_PVT[] =  {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};
//unsigned char checkNAV5[]   = {0xB5, 0x62, 0x06, 0x24, 0x00, 0x00, 0x2A, 0x84};
//unsigned char disableGGA[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x23};
//unsigned char disableGLL[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2A};
//unsigned char disableGSA[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x31};
//unsigned char disableGSV[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x38};
//unsigned char disableRMC[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x3F};
//unsigned char disableVTG[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x46};
//unsigned char disableZDA[]  = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x5B};
//unsigned char portUBX_UBXNMEA[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA2, 0xA2}; 
// To disable NMEA (if used) see below (and repeat for each NMEA message as required):
//   Serial.println("Disabling GSA messages");
//   sendUBX(disableGSA, sizeof(disableGSA));
//   checkAck(disableGSA);

// Structure to hold a NAV-PVT UBX message (position, time & velocity)
struct NAV_PVT
{
//  unsigned long Time;
  unsigned short Year;
  unsigned char Month;
  unsigned char Day;
  unsigned char Hour;
  unsigned char Min;
  unsigned char Sec;
  unsigned char Valid;
//  unsigned long TimeAcc;
//  long Nano;
  unsigned char FixType;
//  unsigned char Flags;
//  unsigned char Reserved1;
  unsigned char numSV;
  long Long;
  long Lat;
//  long HeightEll;
  long HeightMSL;
//  unsigned long HAcc;
//  unsigned long VAcc;
};
