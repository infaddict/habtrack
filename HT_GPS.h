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
const long LatLongFactor = 10000000;    // Lat and Long are held with 1e-7 scaling

unsigned char airborne1g[]  = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};
unsigned char portUBX_UBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x96}; 
unsigned char reqNAV_PVT[] =  {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};

// Structure to hold a NAV-PVT UBX message fields
// Note: Variables not required by me have been removed for RAM reasons
struct GPS_INFO
{
  unsigned char Hour;
  unsigned char Min;
  unsigned char Sec;
  unsigned char FixType;
  unsigned char FixFlags;
  unsigned char numSV;
  long Long;
  long Lat;
  long HeightMSL;
};
