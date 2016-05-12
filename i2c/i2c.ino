

#include "PCF8574.h"
#include <Wire.h>

//definitions
#define MaskOrSw11N 0b00110000  //PCF01
#define MaskOrSw11R 0b01000000  //PCF01
#define MaskOrSw21N 0b10000000  //PCF01
#define MaskOrSw21R 0b00010000  //PCF02
#define MaskOrSw22N 0b00100000  //PCF02
#define MaskOrSw22R 0b01000000  //PCF02
#define MaskOrSw23N 0b10000000  //PCF02
#define MaskOrSw23R 0b00010000  //PCF03
#define MaskOrSw24N 0b00100000  //PCF03
#define MaskOrSw24R 0b01000000  //PCF03
#define MaskOrSw31N 0b10000000  //PCF03
#define MaskOrSw31R 0b00010000  //PCF04
#define MaskOrSw32N 0b01100000  //PCF04
#define MaskOrSw32R 0b10000000  //PCF04

#define MaskAndSw11N 0b10111111  //PCF01
#define MaskAndSw11R 0b11001111  //PCF01
#define MaskAndSw21N 0b11101111  //PCF02
#define MaskAndSw21R 0b01111111  //PCF01
#define MaskAndSw22N 0b10111111  //PCF02
#define MaskAndSw22R 0b11011111  //PCF02
#define MaskAndSw23N 0b11101111  //PCF03
#define MaskAndSw23R 0b01111111  //PCF02
#define MaskAndSw24N 0b10111111  //PCF03
#define MaskAndSw24R 0b11011111  //PCF03
#define MaskAndSw31N 0b11101111  //PCF04
#define MaskAndSw31R 0b01111111  //PCF03
#define MaskAndSw32N 0b01111111  //PCF04
#define MaskAndSw32R 0b10011111  //PCF04

#define MaskXorSw11N 0b00110000  //PCF01
#define MaskXorSw11R 0b01000000  //PCF01
#define MaskXorSw21N 0b10000000  //PCF01
#define MaskXorSw21R 0b00010000  //PCF02
#define MaskXorSw22N 0b00100000  //PCF02
#define MaskXorSw22R 0b01000000  //PCF02
#define MaskXorSw23N 0b10000000  //PCF02
#define MaskXorSw23R 0b00010000  //PCF03
#define MaskXorSw24N 0b00100000  //PCF03
#define MaskXorSw24R 0b01000000  //PCF03
#define MaskXorSw31N 0b10000000  //PCF03
#define MaskXorSw31R 0b00010000  //PCF04
#define MaskXorSw32N 0b01100000  //PCF04
#define MaskXorSw32R 0b10000000  //PCF04

#define NORMAL  1
#define REVERSE 2
#define MOVN2R  3
#define MOVR2N  4
#define CHECKSTATUS 5
#define HANDSHAKE 1
#define FLASHTIME 500


//Global Variables
byte OutputPCF_01 = 0b00000000;
byte OutputPCF_02 = 0b00000000;
byte OutputPCF_03 = 0b00000000;
byte OutputPCF_04 = 0b00000000;

//Flags
byte FlagSW11 = 0;
byte FlagSW21 = 0;
byte FlagSW22 = 0;
byte FlagSW23 = 0;
byte FlagSW24 = 0;
byte FlagSW31 = 0;
byte FlagSW32 = 0;

//Time Variables
byte cnt = 0;
unsigned long CurrentMillis = 0;
unsigned long TimerBlinkSW11N = 0;
unsigned long TimerBlinkSW11R = 0;
unsigned long TimerBlinkSW21N = 0;
unsigned long TimerBlinkSW21R = 0;
unsigned long TimerBlinkSW22N = 0;
unsigned long TimerBlinkSW22R = 0;
unsigned long TimerBlinkSW23N = 0;
unsigned long TimerBlinkSW23R = 0;
unsigned long TimerBlinkSW24N = 0;
unsigned long TimerBlinkSW24R = 0;
unsigned long TimerBlinkSW31N = 0;
unsigned long TimerBlinkSW31R = 0;
unsigned long TimerBlinkSW32N = 0;
unsigned long TimerBlinkSW32R = 0;
unsigned long TimerSendCurrentStates = 0;

//I2C addresses
byte MasterAddress = 0x40;
byte SW11addr = 0x31;
byte SW21addr = 0x32;
byte SW22addr = 0x33;
byte SW23addr = 0x34;
byte SW24addr = 0x35;
byte SW31addr = 0x36;
byte SW32addr = 0x37;

// adjust addresses if needed
PCF8574 PCF_01(0x21);  
PCF8574 PCF_02(0x22);  
PCF8574 PCF_03(0x23);  
PCF8574 PCF_04(0x24);  

void setup()
{
  Serial.begin(115200);
  Serial.println("\nTEST PCF8574\n");

  Wire.begin(MasterAddress);   //Initiate I2C bus

  //Test to light all Leds

      LightSW11N();
      LightSW21N();
      LightSW22N();
      LightSW23N();
      LightSW24N();
      LightSW31N();
      LightSW32N();

  Serial.println("Delay finished!");
  
}

void loop()
{
  CurrentMillis = millis();
  
  
  SendOutputs();
  FlagSW11 = CheckSlaveState(SW11addr);
  FlagSW21 = CheckSlaveState(SW21addr);
  FlagSW22 = CheckSlaveState(SW22addr);
  FlagSW23 = CheckSlaveState(SW23addr);
  FlagSW24 = CheckSlaveState(SW24addr);
  FlagSW31 = CheckSlaveState(SW31addr);
  FlagSW32 = CheckSlaveState(SW32addr);
  PrintFlagStates();
  LightLEDs();

  

  //Read button for SW11
  if((PCF_01.read(2) == 1) && ((FlagSW11 == NORMAL) || (FlagSW11 == REVERSE))) {
      FlagSW11 = SendSlaveCommand(SW11addr, MOVN2R);
    }

  if((PCF_01.read(3) == 1) && ((FlagSW11 == NORMAL) || (FlagSW11 == REVERSE))) {
      FlagSW11 = SendSlaveCommand(SW11addr, MOVR2N);
    }

  //Read button for SW21
  if((PCF_01.read(0) == 1) && ((FlagSW21 == NORMAL) || (FlagSW21 == REVERSE))) {
      FlagSW21 = SendSlaveCommand(SW21addr, MOVN2R);
    }

  if((PCF_01.read(1) == 1) && ((FlagSW21 == NORMAL) || (FlagSW21 == REVERSE))) {
      FlagSW21 = SendSlaveCommand(SW21addr, MOVR2N);
    }

  //Read button for SW22
  if((PCF_02.read(2) == 1) && ((FlagSW22 == NORMAL) || (FlagSW22 == REVERSE))) {
      FlagSW22 = SendSlaveCommand(SW22addr, MOVN2R);
    }

  if((PCF_02.read(3) == 1) && ((FlagSW22 == NORMAL) || (FlagSW22 == REVERSE))) {
      FlagSW22 = SendSlaveCommand(SW22addr, MOVR2N);
    } 

  //Read button for SW23
  if((PCF_02.read(0) == 1) && ((FlagSW23 == NORMAL) || (FlagSW23 == REVERSE))) {
      FlagSW23 = SendSlaveCommand(SW23addr, MOVN2R);
    }

  if((PCF_02.read(1) == 1) && ((FlagSW23 == NORMAL) || (FlagSW23 == REVERSE))) {
      FlagSW23 = SendSlaveCommand(SW23addr, MOVR2N);
    }

  //Read button for SW24
  if((PCF_03.read(2) == 1) && ((FlagSW24 == NORMAL) || (FlagSW24 == REVERSE))) {
      FlagSW24 = SendSlaveCommand(SW24addr, MOVN2R);
    }

  if((PCF_03.read(3) == 1) && ((FlagSW24 == NORMAL) || (FlagSW24 == REVERSE))) {
      FlagSW24 = SendSlaveCommand(SW24addr, MOVR2N);
    }

  //Read button for SW31
  if((PCF_03.read(0) == 1) && ((FlagSW31 == NORMAL) || (FlagSW31 == REVERSE))) {
      FlagSW31 = SendSlaveCommand(SW31addr, MOVR2N);
    }

  if((PCF_03.read(1) == 1) && ((FlagSW31 == NORMAL) || (FlagSW31 == REVERSE))) {
      FlagSW31 = SendSlaveCommand(SW31addr, MOVN2R);
    } 
  //Read button for SW32
  if((PCF_04.read(3) == 1) && ((FlagSW32 == NORMAL) || (FlagSW32 == REVERSE))) {
      FlagSW32 = SendSlaveCommand(SW32addr, MOVR2N);
    }

  if((PCF_04.read(2) == 1) && ((FlagSW32 == NORMAL) || (FlagSW32 == REVERSE))) {
      FlagSW32 = SendSlaveCommand(SW32addr, MOVN2R);
    }
}

void LightLEDs(void) {
  switch(FlagSW11) {
    case NORMAL:
      LightSW11N();
      break;
    case REVERSE:
      LightSW11R();
      break;
    case MOVN2R:
      if(CurrentMillis - TimerBlinkSW11R > FLASHTIME)
      {
        FlashSW11R();
        TimerBlinkSW11R = CurrentMillis;
      }
      break;
    case MOVR2N:
      if(CurrentMillis - TimerBlinkSW11N > FLASHTIME)
      {
        FlashSW11N();
        TimerBlinkSW11N = CurrentMillis;
      }
      break;
  }
   switch(FlagSW21) {
    case NORMAL:
      LightSW21N();
      break;
    case REVERSE:
      LightSW21R();
      break;
    case MOVN2R:
    if(CurrentMillis - TimerBlinkSW21R > FLASHTIME)
      {
        FlashSW21R();
        TimerBlinkSW21R = CurrentMillis;
      }
      break;
    case MOVR2N:
    if(CurrentMillis - TimerBlinkSW21N > FLASHTIME)
      {
        FlashSW21N();
        TimerBlinkSW21N = CurrentMillis;
      }
      break;
  }
   switch(FlagSW22) {
    case NORMAL:
      LightSW22N();
      break;
    case REVERSE:
      LightSW22R();
      break;
    case MOVN2R:
    if(CurrentMillis - TimerBlinkSW22R > FLASHTIME)
      {
        FlashSW22R();
        TimerBlinkSW22R = CurrentMillis;
      }
      break;
    case MOVR2N:
    if(CurrentMillis - TimerBlinkSW22N > FLASHTIME)
      {
        FlashSW22N();
        TimerBlinkSW22N = CurrentMillis;
      }
      break;
  }
   switch(FlagSW23) {
    case NORMAL:
      LightSW23N();
      break;
    case REVERSE:
      LightSW23R();
      break;
    case MOVN2R:
      if(CurrentMillis - TimerBlinkSW23R > FLASHTIME)
      {
        FlashSW23R();
        TimerBlinkSW23R = CurrentMillis;
      }
      break;
    case MOVR2N:
      if(CurrentMillis - TimerBlinkSW23N > FLASHTIME)
      {
        FlashSW23N();
        TimerBlinkSW23N = CurrentMillis;
      }
      break;
  }
   switch(FlagSW24) {
    case NORMAL:
      LightSW24N();
      break;
    case REVERSE:
      LightSW24R();
      break;
    case MOVN2R:
      if(CurrentMillis - TimerBlinkSW24R > FLASHTIME)
      {
        FlashSW24R();
        TimerBlinkSW24R = CurrentMillis;
      }
      break;
    case MOVR2N:
      if(CurrentMillis - TimerBlinkSW24N > FLASHTIME)
      {
        FlashSW24N();
        TimerBlinkSW24N = CurrentMillis;
      }
      break;
  }
   switch(FlagSW31) {
    case NORMAL:
      LightSW31N();
      break;
    case REVERSE:
      LightSW31R();
      break;
    case MOVN2R:
      if(CurrentMillis - TimerBlinkSW31R > FLASHTIME)
      {
        FlashSW31R();
        TimerBlinkSW31R = CurrentMillis;
      }
      break;
    case MOVR2N:
      if(CurrentMillis - TimerBlinkSW31N > FLASHTIME)
      {
        FlashSW31N();
        TimerBlinkSW31N = CurrentMillis;
      }
      break;
  }
   switch(FlagSW32) {
    case NORMAL:
      LightSW32N();
      break;
    case REVERSE:
      LightSW32R();
      break;
    case MOVN2R:
      if(CurrentMillis - TimerBlinkSW32R > FLASHTIME)
      {
        FlashSW32R();
        TimerBlinkSW32R = CurrentMillis;
      }
      break;
    case MOVR2N:
      if(CurrentMillis - TimerBlinkSW32N > FLASHTIME)
      {
        FlashSW32N();
        TimerBlinkSW32N = CurrentMillis;
      }
      break;
  }
}

void SendOutputs(void){
  PCF_01.write8(OutputPCF_01);
  PCF_02.write8(OutputPCF_02);
  PCF_03.write8(OutputPCF_03);
  PCF_04.write8(OutputPCF_04);  
}

void PrintFlagStates(void) {
  if(cnt >= 100) {
    //Prints out the current states of the Flags
    Serial.println("----------------------------");
    Serial.print("FlagSW11: ");
    TypeStates(FlagSW11); 
    Serial.print("FlagSW21: ");
    TypeStates(FlagSW21); 
    Serial.print("FlagSW22: ");
    TypeStates(FlagSW22); 
    Serial.print("FlagSW23: ");
    TypeStates(FlagSW23); 
    Serial.print("FlagSW24: ");
    TypeStates(FlagSW24); 
    Serial.print("FlagSW31: ");
    TypeStates(FlagSW31); 
    Serial.print("FlagSW32: ");
    TypeStates(FlagSW32);
    Serial.println("----------------------------");
    Serial.print("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
    cnt = 0; 
  } else {
    cnt++;  
  }
}

void TypeStates(byte Flag) {
  switch (Flag) {
    case 0:
      Serial.println("Null");
      break;
    case NORMAL:
      Serial.println("Normal");
      break;
    case REVERSE:
      Serial.println("Reverse");
      break;
    case MOVN2R:
      Serial.println("Moving Normal to Reverse");  
      break;
    case MOVR2N:
      Serial.println("Moving Reverse to Normal");
      break;
  }
}

void LightSW11N(void) {
  //Mask bits
  OutputPCF_01 = OutputPCF_01 | MaskOrSw11N;
  OutputPCF_01 = OutputPCF_01 & MaskAndSw11N;
}

void LightSW11R(void) {
  //Mask bits
  OutputPCF_01 = OutputPCF_01 | MaskOrSw11R;
  OutputPCF_01 = OutputPCF_01 & MaskAndSw11R;
}

void LightSW21N(void) {
  //Mask bits
  OutputPCF_01 = OutputPCF_01 | MaskOrSw21N;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw21N;
}

void LightSW21R(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 | MaskOrSw21R;
  OutputPCF_01 = OutputPCF_01 & MaskAndSw21R;
}

void LightSW22N(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 | MaskOrSw22N;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw22N;
}

void LightSW22R(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 | MaskOrSw22R;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw22R;
}

void LightSW23N(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 | MaskOrSw23N;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw23N;
}

void LightSW23R(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 | MaskOrSw23R;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw23R;
}

void LightSW24N(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 | MaskOrSw24N;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw24N;
}

void LightSW24R(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 | MaskOrSw24R;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw24R;
}

void LightSW31N(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 | MaskOrSw31N;
  OutputPCF_04 = OutputPCF_04 & MaskAndSw31N;
}

void LightSW31R(void) {
  //Mask bits
  OutputPCF_04 = OutputPCF_04 | MaskOrSw31R;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw31R;
}

void LightSW32N(void) {
  //Mask bits
  OutputPCF_04 = OutputPCF_04 | MaskOrSw32N;
  OutputPCF_04 = OutputPCF_04 & MaskAndSw32N;
}

void LightSW32R(void) {
  //Mask bits
  OutputPCF_04 = OutputPCF_04 | MaskOrSw32R;
  OutputPCF_04 = OutputPCF_04 & MaskAndSw32R;
}

void FlashSW11N(void) {
  //Mask bits
  OutputPCF_01 = OutputPCF_01 ^ MaskOrSw11N;
  OutputPCF_01 = OutputPCF_01 & MaskAndSw11R;
}

void FlashSW11R(void) {
  //Mask bits
  OutputPCF_01 = OutputPCF_01 ^ MaskOrSw11R;
  OutputPCF_01 = OutputPCF_01 & MaskAndSw11N;
}

void FlashSW21N(void) {
  //Mask bits
  OutputPCF_01 = OutputPCF_01 ^ MaskOrSw21N;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw21N;
}

void FlashSW21R(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 ^ MaskOrSw21R;
  OutputPCF_01 = OutputPCF_01 & MaskAndSw21R;
}

void FlashSW22N(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 ^ MaskOrSw22N;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw22N;
}

void FlashSW22R(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 ^ MaskOrSw22R;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw22R;
}

void FlashSW23N(void) {
  //Mask bits
  OutputPCF_02 = OutputPCF_02 ^ MaskOrSw23N;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw23N;
}

void FlashSW23R(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 ^ MaskOrSw23R;
  OutputPCF_02 = OutputPCF_02 & MaskAndSw23R;
}

void FlashSW24N(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 ^ MaskOrSw24N;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw24N;
}

void FlashSW24R(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 ^ MaskOrSw24R;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw24R;
}

void FlashSW31N(void) {
  //Mask bits
  OutputPCF_03 = OutputPCF_03 ^ MaskOrSw31N;
  OutputPCF_04 = OutputPCF_04 & MaskAndSw31N;
}

void FlashSW31R(void) {
  //Mask bits
  OutputPCF_04 = OutputPCF_04 ^ MaskOrSw31R;
  OutputPCF_03 = OutputPCF_03 & MaskAndSw31R;
}

void FlashSW32N(void) {
  //Mask bits
  OutputPCF_04 = OutputPCF_04 ^ MaskOrSw32N;
  OutputPCF_04 = OutputPCF_04 & MaskAndSw32N;
}

void FlashSW32R(void) {
  //Mask bits
  OutputPCF_04 = OutputPCF_04 ^ MaskOrSw32R;
  OutputPCF_04 = OutputPCF_04 & MaskAndSw32R;
}

byte SendSlaveCommand(byte SlaveAddr, byte SlaveCommand) {
  
  //Send Data to SW21 Controller
  Wire.beginTransmission(SlaveAddr);   //Open transmission
  Wire.write(SlaveCommand);                 //Send Command to Slave device
  delay(10);                          //Wait 10 ms

  //Wait for Handshake
  byte receivedValue;
  byte available = Wire.requestFrom(SlaveAddr, (byte)1);
  if(available == HANDSHAKE)
  {
    receivedValue = Wire.read(); // get response
    /*if(receivedValue != 1) {
        Serial.println("ERROR: Unexpected response to handshake!");
        FlagSW11 = REVERSE;
    }*/
    Serial.print("Received value: ");
    Serial.println(receivedValue);
  }
  else
  {
    Serial.print("ERROR: Unexpected number of bytes received: ");
    Serial.println(available);
  }
  Wire.endTransmission();

  return SlaveCommand;  //Return Sent state
}

byte CheckSlaveState(byte SlaveAddr) {
  
  //Send Data to Slave
  Wire.beginTransmission(SlaveAddr);   //Open transmission
  Wire.write(CHECKSTATUS);                 //Send Command to Slave device
  delay(10);                          //Wait 10 ms

  //Wait for Handshake
  byte receivedValue;
  byte available = Wire.requestFrom(SlaveAddr, (byte)1);
  if(available == HANDSHAKE)
  {
    receivedValue = Wire.read(); // get response
    if(receivedValue == 0) {
        Serial.println("ERROR: Unexpected response!");
    }
    /*Serial.print("Current State of slave ");
    Serial.print(SlaveAddr);
    Serial.print(" is: ");
    Serial.println(receivedValue);*/
  }
  else
  {
    Serial.print("ERROR: Unexpected number of bytes received: ");
    Serial.println(available);
  }
  Wire.endTransmission();

  return receivedValue;
}
