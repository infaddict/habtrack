////////////////////////////////////////////////////////////////////////////////
// HT_GPS.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>  // required for I2C/DDC communication

unsigned int  UBXstate = 0;
unsigned char Checksum_A = 0;
unsigned char Checksum_B = 0;
unsigned char UBXclass;
unsigned char UBXid;
unsigned char UBXlengthLSB;
unsigned char UBXlengthMSB;
unsigned int  UBXlength;
unsigned char UBXpayloadIdx;
unsigned char UBXbuffer[UBX_MAX_PAYLOAD];
unsigned char UBXckA;
unsigned char UBXckB;

////////////////////////////////////////////////////////////////////////////////
// readUBX()
// Requests the maximum buffer length from the GPS and reads the data.
// Uses a state machine to process UBX bytes, including checks for payload 
// length and checksum.
// Resultant UBX message (if any) is placed into UBXbuffer[]
// Returns true is all ok
// Returns false if bad payload or checksum detected.
////////////////////////////////////////////////////////////////////////////////
boolean readUBX()
{
  unsigned char uChar;
  
  setGPSforRead();    // Request max buffer length from GPS
  
  while ( GPSdataAvailable() )
  {
    uChar = readGPSchar();  // Read 1 char (byte) from GPS
    
    //Serial.print(uChar,HEX);
    //Serial.print("-");
      
    // Start the state machine to process the message bytes
    switch(UBXstate)
    {
      case 0:    // Awaiting Sync Char 1
        if (uChar == UBX_SYNC_CHAR1)
        {
          UBXstate++;
        }
        break;
      case 1:    // Awauting Sync Char 2
        if (uChar == UBX_SYNC_CHAR2)
        {
          UBXstate++;
        }
        else
        {
          UBXstate = 0;    // Wrong sequence so start again
        }
        break;
      case 2:    // Awaiting Class
        UBXclass = uChar;
        UBXchecksum(UBXclass);
        UBXstate++;
        break;
      case 3:    // Awaiting Id
        UBXid = uChar;
        UBXchecksum(UBXid);
        UBXstate++;
        break;
      case 4:    // Awaiting Length LSB (little endian so LSB is first)
        UBXlengthLSB = uChar;
        UBXchecksum(UBXlengthLSB);
        UBXstate++;       
        break;
      case 5:    // Awaiting Length MSB
        UBXlengthMSB = uChar;
        UBXchecksum(UBXlengthMSB);
        UBXstate++;
        UBXpayloadIdx = 0;
        UBXlength = (unsigned int)(UBXlengthMSB << 8) | UBXlengthLSB;  // convert little endian MSB & LSB into integer
        if (UBXlength >= UBX_MAX_PAYLOAD)
        {
          Serial.println("UBX PAYLOAD BAD LENGTH!!");
          UBXstate=0;    // Bad data received so reset to start again
          Checksum_A=0;
          Checksum_B=0;
          return false;
        }
        break;
      case 6:    // Awaiting Payload
        if (UBXpayloadIdx < UBXlength)
        {
          UBXbuffer[UBXpayloadIdx] = uChar;
          UBXchecksum(uChar);
          UBXpayloadIdx++;
          if (UBXpayloadIdx == UBXlength)
          {
            UBXstate++;  // Just processed last byte of payload, so move o
          }
        }         
        break;
      case 7:    // Awaiting Checksum 1
        UBXckA = uChar;
        UBXstate++;
        break;
      case 8:    // Awaiting Checksum 2
        UBXckB = uChar;
        if ((Checksum_A == UBXckA) && (Checksum_B == UBXckB))    // Check the calculated checksums match actual checksums
        {
          parseUBX();
        }
        else
        {
          Serial.println("UBX PAYLOAD BAD CHECKSUM!!");
          return false;
        }
        
        UBXstate=0;    // Start again at 0
        Checksum_A=0;
        Checksum_B=0;
        break;      
    }
 
  }
  
  return true;  // everything ok
    
}

////////////////////////////////////////////////////////////////////////////////
// parseUBX()
// Parse a UBX message according to its class and id.
// Data is populated into structures according to message class/type.
// Currently supports the following UBX messages:
//     NAV-PVT     Data into navPVT
////////////////////////////////////////////////////////////////////////////////
void parseUBX()
{
  /*Serial.println("Parsing: ");
  for (int i=0;i<=UBXpayloadIdx;i++)
  {
   Serial.print(UBXbuffer[i],HEX); 
  }
  Serial.println();*/
  
  int i;
  
  if (UBXclass == 0x01)  // Class 1 is NAV messages
  {
    if (UBXid == 0x07)  // Id 7 is the PVT (Position Velocity Time) main NAV message
    {
      i=0;
      //navPVT.Time = join4Bytes(&UBXbuffer[i]);
      i+=4;
      navPVT.Year = join2Bytes(&UBXbuffer[i]);
      i+=2;
      navPVT.Month = UBXbuffer[i];
      i+=1;
      navPVT.Day = UBXbuffer[i];
      i+=1;
      navPVT.Hour = UBXbuffer[i];
      i+=1;
      navPVT.Min = UBXbuffer[i];
      i+=1;
      navPVT.Sec = UBXbuffer[i];
      i+=1;
      navPVT.Valid = UBXbuffer[i];
      i+=1;
      //navPVT.TimeAcc = join4Bytes(&UBXbuffer[i]);
      i+=4;
      //navPVT.Nano = join4Bytes(&UBXbuffer[i]);
      i+=4;
      navPVT.FixType = UBXbuffer[i];
      i+=1;
      //navPVT.Flags = UBXbuffer[i];
      i+=1;
      //navPVT.Reserved1 = UBXbuffer[i];
      i+=1;
      navPVT.numSV = UBXbuffer[i];
      i+=1;
      navPVT.Long = join4Bytes(&UBXbuffer[i]);
      i+=4;
      navPVT.Lat = join4Bytes(&UBXbuffer[i]);
      i+=4;
      //navPVT.HeightEll = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
      navPVT.HeightMSL = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
      //navPVT.HAcc = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
      //navPVT.VAcc = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
   /*
     Serial.println();
     Serial.print("Flight Time (s): ");
     Serial.print(flightTime);
     //Serial.print("  Time : ");     Serial.print(navPVT.Time);  //U4 unsigned long
     Serial.print("  Year : ");     Serial.print(navPVT.Year);  //U2 unsigned short
     Serial.print("  Month : ");     Serial.print(navPVT.Month);  //U1 unsigned char
     Serial.print("  Day : ");     Serial.print(navPVT.Day);
     Serial.print("  Hour : ");     Serial.print(navPVT.Hour);
     Serial.print("  Min : ");     Serial.print(navPVT.Min);
     Serial.print("  Sec : ");     Serial.println(navPVT.Sec);
     Serial.print("Valid : "); //    Serial.print(navPVT.Valid);  //X1 bitfield
     //Serial.print(" - H: ");
     //Serial.print(navPVT.Valid,HEX);  //X1 bitfield
     //Serial.print(" - D: ");
     //Serial.print(navPVT.Valid,DEC);  //X1 bitfield
     //Serial.print(" - B: ");
     Serial.print(navPVT.Valid,BIN);  //X1 bitfield
     //Serial.print("  TimeAcc : ");     Serial.print(navPVT.TimeAcc);  //U4 unsigned long
     //Serial.print("  Nano : ");     Serial.print(navPVT.Nano);  //I4 signed long (2's compliment)
     Serial.print("  FixType : ");     Serial.print(navPVT.FixType);  //U1 unsigned char
     //Serial.print("  Flags : ");     Serial.print(navPVT.Flags);  
     //Serial.print(" - H: ");
     //Serial.print(navPVT.Flags,HEX);  
     //Serial.print(" - D: ");
     //Serial.print(navPVT.Flags,DEC);  
     //Serial.print(" - B: ");
     //Serial.print(navPVT.Flags,BIN); 
     //Serial.print("  Reserved1 : ");      Serial.print(navPVT.Reserved1);
      Serial.print("  numSV : ");      Serial.println(navPVT.numSV);
      Serial.print("Long : ");      Serial.print(navPVT.Long);
      Serial.print("  Lat : ");      Serial.print(navPVT.Lat);
      //Serial.print("  HeightEll : ");      Serial.print(navPVT.HeightEll);
      Serial.print("  HeightMSL : ");      Serial.println(navPVT.HeightMSL);
      //Serial.print("  HAcc : ");      Serial.print(navPVT.HAcc);
      //Serial.print("  Vacc : ");      Serial.println(navPVT.VAcc);
      Serial.print("Internal Temp: ");
  Serial.print(intTemp);
  Serial.print("       External Temp: ");
  Serial.println(extTemp);
  
      */
    }
  }
  
}

////////////////////////////////////////////////////////////////////////////////
// UBXchecksum()
// Adds the data to the current checksum A and B variables
////////////////////////////////////////////////////////////////////////////////
void UBXchecksum(unsigned char data)
{
  Checksum_A += data;
  Checksum_B += Checksum_A;
}

////////////////////////////////////////////////////////////////////////////////
// checkAck()
// Returns true if the correct ack packet for the Message parameter is read from 
// GPS.
// Returns false after 3 seconds if correct ack packet not read
//////////////////////////////////////////////////////////////////////////////// 
boolean checkAck(unsigned char *Message)
{
  unsigned char Character;
  unsigned char ackPacket[10];
  unsigned int ackIndex = 0;
  unsigned long startTime = millis();    // log the start time of ack check
  
  // Build up the expected ack packet
  ackPacket[0] = 0xB5;	// header
  ackPacket[1] = 0x62;	// header
  ackPacket[2] = 0x05;	// class
  ackPacket[3] = 0x01;	// id
  ackPacket[4] = 0x02;	// length
  ackPacket[5] = 0x00;
  ackPacket[6] = Message[2];	// ACK class
  ackPacket[7] = Message[3];	// ACK id
  ackPacket[8] = 0;		// CK_A
  ackPacket[9] = 0;		// CK_B
  
  // Calculate the expected checksum in pos 8 and 9
  for (unsigned int i=2; i<8; i++) 
  {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
  
  // Infinite loop to read bytes from the GPS looking for ack packet
  while(true)
  {
    // Check if 3 seconds have elapsed without success
    if (millis() - startTime > 3000)
    {
      Serial.println("FAILED!");
      return false;
    }
    
    // Read from the GPS
    setGPSforRead();     
    while ( GPSdataAvailable() )
    {      
      Character = readGPSchar();
      
      if (Character != 0xFF)    // 0xFF means no data available
      {
        
        // Check if the char matches the expected ack packet char
        if (Character == ackPacket[ackIndex])
        {
          //Serial.print(" Match ");
          // We have a match so move onto next ack packet char
          ackIndex++;
          
          // If we've matched all ack packet chars then we are good
          if (ackIndex > 9)
          {
            Serial.println("Success");
            return true;
          }
        }
        else
        {
          // Unexpected char so start again
          ackIndex = 0;
        }
        
      }
      
    }
    
  }
}

////////////////////////////////////////////////////////////////////////////////
// setGPSforRead()
// Prepare the GPS by making data request
////////////////////////////////////////////////////////////////////////////////
// Prepare the GPS by making data request
void setGPSforRead()
{
  Wire.requestFrom(GPS_ADDR, BUFFER_LENGTH);    // Request 32 bytes of data from the GPS
}

////////////////////////////////////////////////////////////////////////////////
// setGPSforRead()
// Read a single char from the GPS
////////////////////////////////////////////////////////////////////////////////
char readGPSchar()
{
  return Wire.read();
}

////////////////////////////////////////////////////////////////////////////////
// setGPSforRead()
// Returns how many chars of GPS data are waiting to be read
// 0 means no data ready
////////////////////////////////////////////////////////////////////////////////
int GPSdataAvailable()
{
  return Wire.available();
}

////////////////////////////////////////////////////////////////////////////////
// setGPSforRead()
// Send a UBX message to the GPS
// Due to almost invisible buffer length limit in Wire, we have to split up the 
// message if required
////////////////////////////////////////////////////////////////////////////////
boolean sendUBX(unsigned char *Message, int Length)
{
  boolean result = false;
  Wire.beginTransmission(GPS_ADDR);    // Begin transmission to the GPS
  
  // Write 1 byte of the message at a time to the GPS
  for (int i=0; i<Length; i++)
  {   
    if (i > 0 && i % BUFFER_LENGTH == 0)  // Detect a full buffer and end Transmission if needed (this actually sends data)
    {
      if (Wire.endTransmission() != 0)
      {
        Serial.println("ERROR IN WIRE END TRANSMISSION!!!!");
        return result;
      };
      
      Wire.beginTransmission(GPS_ADDR);  // more data to write, so begin another transmission
      
    }
    
    if (Wire.write(Message[i]) != 1)  // send the byte to the buffer
    {
      Serial.println("ERROR IN WIRE WRITE!!!!");
      return result;
    }
   
  }
  
  if (Wire.endTransmission() != 0)
  {
    Serial.println("ERROR IN WIRE END TRANSMISSION!!!!");
    return result;
  };
  
  return true;    // everything ok
}


