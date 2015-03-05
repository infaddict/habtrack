////////////////////////////////////////////////////////////////////////////////
// HT_GPS.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
////////////////////////////////////////////////////////////////////////////////

#include <Wire.h>  // required for I2C/DDC communication

byte UBXstate = 0;
unsigned char Checksum_A = 0;
unsigned char Checksum_B = 0;
unsigned char UBXclass;
unsigned char UBXid;
unsigned char UBXlengthLSB;
unsigned char UBXlengthMSB;
byte UBXlength;
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
boolean checkForGPSData()
{
  long gpsTimeout = millis() + 3000;  // 3 second timeout waiting for data
  boolean timeout = false;
  boolean endOfMessage = false;
  
  // Loop looking for data until end of message or we reach timeout period
  while(!endOfMessage && !timeout)
  {   
    if (millis() > gpsTimeout)
    {
      timeout = true;
    }
    
    // If there is no GPS data available, request another buffers worth of data
    if (!GPSdataAvailable())
    {
      setGPSforRead();
    }
    else
    {
      // There is GPS data available so read it until there is no more or end of message
      unsigned char uChar;
      while (GPSdataAvailable() && !endOfMessage)
      {
        uChar = readGPSchar();  // Read 1 char (byte) from GPS
        
        //Serial.print(uChar,HEX);
        //Serial.print(" ");
        
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
            UBXlength = (byte)(UBXlengthMSB << 8) | UBXlengthLSB;  // convert little endian MSB & LSB into integer
            if (UBXlength >= UBX_MAX_PAYLOAD)
            {
              writeLog(F("UBX PAYLOAD BAD LENGTH!!"));
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
                UBXstate++;  // Just processed last byte of payload, so move on
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
              // Checksum is good so parse the message
              parseUBX();
              endOfMessage = true;
            }
            else
            {
              writeLog(F("UBX PAYLOAD BAD CHECKSUM!!"));
              return false;
            }
            
            UBXstate=0;    // Start again at 0
            Checksum_A=0;
            Checksum_B=0;
            break;      
        }
              
      } 
    
    }
    
  } // while !eom or !timeout
  
  // Only when received all expected data with checksum do we stop
  expectingGPSData = false;

  return true;  // everything ok
    
}

////////////////////////////////////////////////////////////////////////////////
// parseUBX()
// Parse a UBX message according to its class and id.
// Data is populated into structures according to message class/type.
// Currently supports the following UBX messages:
//     NAV-PVT     Data into gpsInfo
////////////////////////////////////////////////////////////////////////////////
void parseUBX()
{
  /*writeLog("Parsing: ");
  for (int i=0;i<=UBXpayloadIdx;i++)
  {
   writeLog(UBXbuffer[i],HEX); 
  }
  writeLog();*/
  
  byte i;
  
  if (UBXclass == 0x01)  // Class 1 is NAV messages
  {
    if (UBXid == 0x07)  // Id 7 is the PVT (Position Velocity Time) main NAV message
    {
      i=0;
      //gpsInfo.Time = join4Bytes(&UBXbuffer[i]);
      i+=4;
      //gpsInfo.Year = join2Bytes(&UBXbuffer[i]);
      i+=2;
      //gpsInfo.Month = UBXbuffer[i];
      i+=1;
      //gpsInfo.Day = UBXbuffer[i];
      i+=1;
      gpsInfo.Hour = UBXbuffer[i];
      i+=1;
      gpsInfo.Min = UBXbuffer[i];
      i+=1;
      gpsInfo.Sec = UBXbuffer[i];
      i+=1;
      gpsInfo.Valid = UBXbuffer[i];
      i+=1;
      //gpsInfo.TimeAcc = join4Bytes(&UBXbuffer[i]);
      i+=4;
      //gpsInfo.Nano = join4Bytes(&UBXbuffer[i]);
      i+=4;
      gpsInfo.FixType = UBXbuffer[i];
      i+=1;
      //gpsInfo.Flags = UBXbuffer[i];
      i+=1;
      //gpsInfo.Reserved1 = UBXbuffer[i];
      i+=1;
      gpsInfo.numSV = UBXbuffer[i];
      i+=1;
      gpsInfo.Long = join4Bytes(&UBXbuffer[i]) / 100;
      i+=4;
      gpsInfo.Lat = join4Bytes(&UBXbuffer[i]) / 100;
      i+=4;
      //gpsInfo.HeightEll = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
      gpsInfo.HeightMSL = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
      //gpsInfo.HAcc = join4Bytes(&UBXbuffer[i]) * 0.001;
      i+=4;
      //gpsInfo.VAcc = join4Bytes(&UBXbuffer[i]) * 0.001;
      
        writeLog(dataString);
      
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
      writeLog(F("FAILED!"));
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
          //writeLog(" Match ");
          // We have a match so move onto next ack packet char
          ackIndex++;
          
          // If we've matched all ack packet chars then we are good
          if (ackIndex > 9)
          {
            writeLog(F("Success"));
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
        writeLog(F("ERROR IN WIRE END TRANSMISSION!!!!"));
        return result;
      };
      
      Wire.beginTransmission(GPS_ADDR);  // more data to write, so begin another transmission
      
    }
    
    if (Wire.write(Message[i]) != 1)  // send the byte to the buffer
    {
      writeLog(F("ERROR IN WIRE WRITE!!!!"));
      return result;
    }
   
  }
  
  if (Wire.endTransmission() != 0)
  {
    writeLog(F("ERROR IN WIRE END TRANSMISSION!!!!"));
    return result;
  };
  
  return true;    // everything ok
}

////////////////////////////////////////////////////////////////////////////////
// checkGPSLock()
// Determine if we have a good GPS lock by examining the fix type and number of
// satellites in view.
////////////////////////////////////////////////////////////////////////////////
void checkGPSLock()
{
  // Check for GPS lock.  We consider anything more than 4 satellites as good
  if ((int)gpsInfo.FixType >= 3 && (int)gpsInfo.numSV > 4)
  {
    if (!gpsLock) 
    {
      gpsLock = true;
      writeLog(F("GPS LOCK !! GPS LOCK !!"));
    }
    
    lastGoodFix = gpsInfo;    // Save this good fix away in case we lose signal
    
    if (timeOfLock == 0)
    {     
      digitalWrite(GREEN_LED_PIN, HIGH);
      timeOfLock = millis();
    }
    else  // already locked on
    {
      if (millis() > timeOfLock + 30000)  // only show the green LED for 30 seconds
      {
        digitalWrite(GREEN_LED_PIN, LOW);
      }
    }
  }
  else
  {
    if (gpsLock)    // we had a lock then lost it
    {
      writeLog(F("!! LOST LOCK !!"));
    }
    
    gpsLock = false;
    
  }
}


