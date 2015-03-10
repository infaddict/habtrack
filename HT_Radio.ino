////////////////////////////////////////////////////////////////////////////////
// HT_Radio.ino
//
// Project: HAB_Tracker
// Author:  infaddict
// Date:    February 2015
// Desc:    Radiomatrix NXT2B-FA code to send data via RTTY
////////////////////////////////////////////////////////////////////////////////
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <SPI.h>
 
#define ASCII 7          // ASCII 7 or 8
#define STOPBITS 2       // Either 1 or 2
#define TXDELAY 0        // Delay between sentence TX's
#define RTTY_BAUD 50     // Baud rate
#define RADIO_PIN 3      // Pin must support PWM.  Pin 3 and 11 use TIMER2.

// Volatile variables are modified inside the ISR
volatile unsigned int sentenceId=1;
volatile byte txStatus=0;
volatile byte txStringLength=0;
volatile int  txDelayCount=0;
volatile byte txBitCount=0;
volatile byte txCharPos=0;
volatile char txChar;

////////////////////////////////////////////////////////////////////////////////
// setupRadio()
// Prepares the radio for transmission
////////////////////////////////////////////////////////////////////////////////
void setupRadio()
{
    pinMode(RADIO_PIN, OUTPUT);    
}

////////////////////////////////////////////////////////////////////////////////
// setupInterrupt()
// Prepares TIMER1 registers so interrupt fires at correct timing for chosen
// baud rate, to send bits via radio.
// Prepares TIMER2 registers for PWM.
////////////////////////////////////////////////////////////////////////////////
void setupInterrupt()
{
  cli();          // disable global interrupts whilst we set stuff up
 
  // Setup TIMER1 according to chosen baud rate
  // TIMER1 is a 16 bit timer with max counter of 65535
  // We use TIMER1 to fire our ISR at correct interval according to baud rate
  // The TIMER1 ISR will send 1 bit of data ever time it fires
  // TIMER1 is not used for PWM, purely for timing of radio TX
   
  TCCR1A = 0;     // set entire TCCR1A register to 0
  TCCR1B = 0;     // same for TCCR1B
  
  OCR1A = F_CPU / 1024 / RTTY_BAUD - 1;  // set TIMER1 compare match register to desired timer count:
  
  TCCR1B |= (1 << WGM12);   // turn on TIMER1 CTC (Clear Timer on Compare) mode:
  
  // Set TIMER1 CS10 and CS12 bits on for 1024 pre-scaler
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  
  // Setup TIMER2 for PWM
  // TIMER2 is a 8 bit timer with max counter of 255
  // We want TIMER2 to be as fast as possible (no pre-scale) to give best PWM
  // TIMER2 has no ISR and is used by pin 3 for radio TX
  TCCR2B = TCCR2B & 0b11111000 | 0x01;
  
  sei();          // enable global interrupts
}

////////////////////////////////////////////////////////////////////////////////
// rttyTxbit()
// Transmit a single bit via the radio
////////////////////////////////////////////////////////////////////////////////
void rttyTxbit (int bit)
{
  // Transmit a high or low value depending on the bit (1 or 0)
  if (bit)
  {
    //Serial.print(".");
    analogWrite(RADIO_PIN,115); // High
  }
  else
  {
    //Serial.print(" ");
    analogWrite(RADIO_PIN,100); // Low
  }
}

////////////////////////////////////////////////////////////////////////////////
// ISR()
// Interrupt Service Routine for TIMER1
// This gets called when TIMER1 elapses, which is very regularly to achieve
// desired baud rate.  E.g. 50 baud = 50 times per second = once very 0.02 second.
// This is a state machine to send the contents of dataString bit by bit
////////////////////////////////////////////////////////////////////////////////
ISR(TIMER1_COMPA_vect)
{
  switch(txStatus) 
  {
  case 0: // This is the optional delay between transmissions.
    txDelayCount++;
    if(txDelayCount>(TXDELAY*RTTY_BAUD)) 
    { 
      txDelayCount=0;
      txStatus=1;  // delay done, move onto initialisation state
    }
    break; // SHOULD THIS BE HERE???!!!
  case 1: // Initialise transmission, take a copy of the string so it doesn't change mid transmission. 
    strcpy((char*)txString,dataString);
    txStringLength=strlen((char*)txString);
    txStatus=2;
    txCharPos=0;
    break;
  case 2: // Grab a char and lets go transmit it. 
    if ( txCharPos < txStringLength)
    {
      txChar = txString[txCharPos];
      txCharPos++;
      txStatus=3;
      rttyTxbit (0); // Start Bit;
      txBitCount=0;
    }
    else
    {
      txStatus=0; // Should be finished
      txCharPos=0;
      sentenceId++;
    }
    break;
  case 3:
    if(txBitCount<ASCII)
    {
      txBitCount++;
      if (txChar & 1) 
      {
        rttyTxbit(1); 
      }
      else 
      {
        rttyTxbit(0); 
      }  
      txChar = txChar >> 1;
      break;
    }
    else
    {
      rttyTxbit (1); // Stop Bit
      txStatus=4;
      txBitCount=0;
      break;
    } 
  case 4:
    if(STOPBITS==2)
    {
      rttyTxbit (1); // 2nd Stop Bit
      txStatus=2;
      break;
    }
    else
    {
      txStatus=2;
      break;
    }
 
  }
  
  
}
