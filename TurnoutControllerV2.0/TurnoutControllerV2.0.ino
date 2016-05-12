//  Servo Controller
//  Created by Karl-Fredrik Johansson
//  Kalf89@gmail.com

#include <SoftPWM.h>
#include <SoftPWM_timer.h>
#include <Servo.h>
#include <EEPROM.h>
#include <Wire.h>

//Pin Configutation
#define PinServo1Signal 2
#define PinServo2Signal 3
#define PinServo1Enable 4
#define PinServo2Enable 5
#define PinLedNormal1 6
#define PinLedNormal2 7
#define PinLedReverse 8
#define PinSwNormal 11  
#define PinSwReverse 12 
#define PinRelayNormal 9
#define PinRelayReverse 10
#define PinProgramLed A0
#define PinProgramServo1Pos A1
#define PinProgramServo2Pos A2
#define PinProgramServoSpeed A3

//Setup
const byte SlaveDeviceId = 0x37;
#define FLASHTIME 500
#define BUTTONPRESSTIME 60
#define SERVO2 

//Other definitions
#define NORMAL  1
#define REVERSE 2
#define MOVN2R  3
#define MOVR2N  4
#define CHECKSTATUS 5
#define HANDSHAKE 1
#define PROGRAMLED 7
#define PROGRAMPOS 8
#define PROGRAMSPEED 9
#define TRUE 1
#define FALSE 0

//Time Variables
unsigned long CurrentMillis = 0;
unsigned long TimerFlashNormal = 0;
unsigned long TimerFlashReverse = 0;
unsigned long TimerMotorSpeed = 0;
unsigned long TimerLedDim = 0;
unsigned long TimerProgramPos = 0;
unsigned long TimerServoSpeed = 0;

//Flags
bool FlagSwNormal = 0;
bool FlagSwReverse = 0;
bool FlagLedNormal = 0;
bool FlagLedReverse = 0;
bool FlagServoEnable = 0;
bool FlagRelayNormalEnable = 0;
bool FlagRelayReverseEnable = 0;
bool FlagProgramLed = 0;
byte FlagProgramServo1Pos = 0;
byte FlagProgramServo2Pos = 0;
bool FlagProgramServoSpeed = 0;
bool FlagFlashNormalLed = 0;
bool FlagFlashReverseLed = 0;

//Sequence Variables
byte SequenceN2R = 0;
byte SequenceR2N = 0;

//EEPROM addresses
byte eeAddressMotor1PosNormal = 0;
byte eeAddressMotor1PosReverse = 10;
byte eeAddressLedDim = 20;
byte eeAddressMotorSpeed = 30;  
byte eeAddressMotor2PosNormal = 40;
byte eeAddressMotor2PosReverse = 50;

//Other Variables
byte CurrentState = 0;
byte CurrentPos1 = 0;
byte CurrentPos2 = 0;
byte LastMasterCommand = 0;
byte Motor1PosNormal = 0;
byte Motor1PosReverse = 0;
byte LedDim = 0;
byte MotorSpeed = 0;  
byte Motor2PosNormal = 0;
byte Motor2PosReverse = 0;
byte TempState = 0;
byte Modifier = 0;
byte Direction = 0;

Servo Servo1;
SERVO2 Servo Servo2;

void setup() {
  //Initiate Serialport
  Serial.begin(115200); // opens Serial port, sets data rate to 115 200 bps
  while (!Serial) {
    ; // wait for Serial port to connect. Needed for native USB
  }
  Serial.println("//Serial....OK");

  //Initiate Software PWM
  SoftPWMBegin();
  
  //I/O Init
  pinMode(PinServo1Signal, OUTPUT);
  SERVO2 pinMode(PinServo2Signal, OUTPUT);
  pinMode(PinServo1Enable, OUTPUT);
  SERVO2 pinMode(PinServo2Enable, OUTPUT);
  pinMode(PinLedNormal1, OUTPUT);
  SERVO2 pinMode(PinLedNormal2, OUTPUT);
  pinMode(PinLedReverse, OUTPUT);
  pinMode(PinRelayNormal, OUTPUT);
  pinMode(PinRelayReverse, OUTPUT);
  pinMode(PinSwNormal, INPUT);
  pinMode(PinSwReverse, INPUT);
  pinMode(PinProgramLed, INPUT);
  pinMode(PinProgramServo1Pos, INPUT);
  SERVO2 pinMode(PinProgramServo2Pos, INPUT);
  pinMode(PinProgramServoSpeed, INPUT);

  //Initiate Servos
  Servo1.attach(PinServo1Signal);
  SERVO2 Servo2.attach(PinServo2Signal);

  //initiate EEPROM
  EEPROM.get(eeAddressMotor1PosNormal, Motor1PosNormal);
  Serial.print("eeAddressMotor1PosNormal: ");
  Serial.println(Motor1PosNormal);
  EEPROM.get(eeAddressMotor1PosReverse, Motor1PosReverse);
  Serial.print("eeAddressMotor1PosReverse: ");
  Serial.println(Motor1PosReverse);
  EEPROM.get(eeAddressMotor2PosNormal, Motor2PosNormal);
  Serial.print("eeAddressMotor2PosNormal: ");
  Serial.println(Motor2PosNormal);
  EEPROM.get(eeAddressMotor2PosReverse, Motor2PosReverse);
  Serial.print("eeAddressMotor2PosReverse: ");
  Serial.println(Motor2PosReverse);
  EEPROM.get(eeAddressLedDim, LedDim);
  Serial.print("eeAddressLedDim: ");
  Serial.println(LedDim);
  EEPROM.get(eeAddressMotorSpeed, MotorSpeed);
  Serial.print("eeAddressMotorSpeed: ");
  Serial.println(MotorSpeed);

  //Initiate I2C Bus
  Wire.begin(SlaveDeviceId);      // join i2c bus with Slave ID
  Wire.onReceive(receiveCommand); // register talk event
  Wire.onRequest(slavesRespond);  // register callback event

  //Go to Start position
  CurrentPos1 = Motor1PosNormal;
  SERVO2   CurrentPos2 = Motor2PosNormal;
  CurrentState = REVERSE;
  FlagSwNormal = 1;
}

void loop() {
  CurrentMillis = millis();

  //Check Buttons
  CheckInputs();

  //Update Outputs
  Leds();
  SendOutputs();

  //Move Normal to Reverse
  Normal2Reverse();
  
  //Move Revese to Normal
  Reverse2Normal();
  
  //Program parameters
  ProgramLed();
  ProgramServo1Pos();
  SERVO2  ProgramServo2Pos();
  ProgramServoSpeed();

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


void Leds(void) {
  //Blink Normal Led
  if((FlagFlashNormalLed == 1) && (CurrentMillis - TimerFlashNormal > FLASHTIME)) {
    FlagLedNormal = FlagLedNormal ^ 1;
    TimerFlashNormal = CurrentMillis;
  }
  
  //Blink Reverse Led
  if((FlagFlashReverseLed == 1) && (CurrentMillis - TimerFlashReverse > FLASHTIME)) {
    FlagLedReverse = FlagLedReverse ^ 1;
    TimerFlashReverse = CurrentMillis;
  }

  //Turn on ProgramLed
  if((CurrentState == PROGRAMLED) || (CurrentState == PROGRAMPOS) || (CurrentState == PROGRAMSPEED)) {
    digitalWrite(13, HIGH);
  } else {
    digitalWrite(13, LOW);
  }
}

void Normal2Reverse(void) {
  //Step 1: Change State to moving and zero the Switch flag
  if((FlagSwReverse == 1) && (CurrentState == NORMAL) && (SequenceN2R == 0)) {
    CurrentState = MOVN2R;
    FlagSwReverse = 0;
    FlagSwNormal = 0;
    SequenceN2R = 1;
    Serial.println("Normal2Reverse");
  }

  //Step 2: Turn of Normal LED
  if(SequenceN2R == 1) {
    FlagLedNormal = 0;
    SequenceN2R = 2;
  }
  
  //Step 3: Start Flashing Reverse LED
  if(SequenceN2R == 2) {
    FlagFlashReverseLed = 1;
    SequenceN2R = 3;
  }

  //Step 4: Turn of the Normal relay
  if(SequenceN2R == 3) {
      FlagRelayNormalEnable = 0;
      SequenceN2R = 4;
  }

  //Step 5: Enable Servos
  if(SequenceN2R == 4) {
      FlagServoEnable = 1;
      SequenceN2R = 5;
  }
  
  //Step 6: Start Moving Servos
  if((SequenceN2R == 5) && (CurrentMillis - TimerMotorSpeed > MotorSpeed)) {
    //Move Servos
    CurrentPos1 = StepServo(CurrentPos1, Motor1PosReverse);
    SERVO2 CurrentPos2 = StepServo(CurrentPos2, Motor2PosReverse);
    Servo1.write(CurrentPos1); //Move the servo one step
    SERVO2 Servo2.write(CurrentPos2); //Move the servo one step
    Serial.print("CurrentPos1: ");
    Serial.println(CurrentPos1);
    SERVO2 Serial.print("CurrentPos2: ");
    SERVO2 Serial.println(CurrentPos2);

    //Check if in position
    if((Motor1PosReverse == CurrentPos1) /*&& (Motor2PosReverse == CurrentPos2)*/) {
      SequenceN2R = 6;
    }
    TimerMotorSpeed = CurrentMillis;
  }

  //Step 7: Disable Servos
  if(SequenceN2R == 6) {
      FlagServoEnable = 0;
      SequenceN2R = 7;
  }
  
  //Step 8: Enable the Reverse Relay
  if(SequenceN2R == 7) {
      FlagRelayReverseEnable = 1;
      SequenceN2R = 8;
  }
  
  //Step 9: Disable flashing
  if(SequenceN2R == 8) {
    FlagFlashReverseLed = 0;
    SequenceN2R = 9;
  }
  
  //Step 10: Enable LED
  if(SequenceN2R == 9) {
    FlagLedReverse = 1;
    SequenceN2R = 0;
    CurrentState = REVERSE;
  }
}

void Reverse2Normal(void) {
  //Step 1: Change State to moving and zero the Switch flag
  if((FlagSwNormal == 1) && (CurrentState == REVERSE) && (SequenceR2N == 0)) {
    CurrentState = MOVR2N;
    FlagSwNormal = 0;
    FlagSwReverse = 0;
    SequenceR2N = 1;
    Serial.println("Reverse2Normal");
  }

  //Step 2: Turn of Reverse LED
  if(SequenceR2N == 1) {
    FlagLedReverse = 0;
    SequenceR2N = 2;
  }
  
  //Step 3: Start Flashing Normal LED
  if(SequenceR2N == 2) {
    FlagFlashNormalLed = 1;
    SequenceR2N = 3;
  }

  //Step 4: Turn of the Reverse relay
  if(SequenceR2N == 3) {
      FlagRelayReverseEnable = 0;
      SequenceR2N = 4;
  }
  
  //Step 5: Enable Servos
  if(SequenceR2N == 4) {
      FlagServoEnable = 1;
      SequenceR2N = 5;
  }
  
  //Step 6: Start Moving Servos
  if((SequenceR2N == 5) && (CurrentMillis - TimerMotorSpeed > MotorSpeed)) {
    //Move Servos
    CurrentPos1 = StepServo(CurrentPos1, Motor1PosNormal);
    SERVO2 CurrentPos2 = StepServo(CurrentPos2, Motor2PosNormal);
    Servo1.write(CurrentPos1); //Move the servo one step
    SERVO2 Servo2.write(CurrentPos2); //Move the servo one step
    Serial.print("CurrentPos1: ");
    Serial.println(CurrentPos1);
    SERVO2 Serial.print("CurrentPos2: ");
    SERVO2 Serial.println(CurrentPos2);

    //Check if in position
    if((Motor1PosNormal == CurrentPos1)/* && (Motor2PosNormal == CurrentPos2)*/) {
      SequenceR2N = 6;
    }
    TimerMotorSpeed = CurrentMillis;
  }

  //Step 7: Disable Servos
  if(SequenceR2N == 6) {
      FlagServoEnable = 0;
      SequenceR2N = 7;
  }
  
  //Step 8: Enable the Normal Relay
  if(SequenceR2N == 7) {
      FlagRelayNormalEnable = 1;
      SequenceR2N = 8;
  }
  
  //Step 9: Disable flashing
  if(SequenceR2N == 8) {
    FlagFlashNormalLed = 0;
    SequenceR2N = 9;
  }
  
  //Step 10: Enable LED
  if(SequenceR2N == 9) {
    FlagLedNormal = 1;
    SequenceR2N = 0;
    CurrentState = NORMAL;
  }
}

byte StepServo(byte CurrentPos, byte EndPos) {
  if(CurrentPos > EndPos) {
      CurrentPos--;
    } else if(CurrentPos < EndPos) {
      CurrentPos++;
    }
    
    return CurrentPos;
}

void CheckInputs(void) {
  //Read switch Normal
  if(digitalRead(PinSwNormal) == HIGH) {
    FlagSwNormal = 1;  
  }

  //Read switch Reverse
  if(digitalRead(PinSwReverse) == HIGH) {
    FlagSwReverse = 1;  
  }

  //Read Program pin Led Brightness
  if(digitalRead(PinProgramLed) == HIGH) {
    FlagProgramLed = 1;  
  } else if(digitalRead(PinProgramLed) == LOW) {
    FlagProgramLed = 0;
  }

  //Read Program pin Servo 1 Position
  if(digitalRead(PinProgramServo1Pos) == HIGH) {
    FlagProgramServo1Pos = 1;  
  } else if(digitalRead((PinProgramServo1Pos) == LOW) && (FlagProgramServo1Pos == 1)) {
    FlagProgramServo1Pos = 2;
  }
  
  //Read Program pin Servo 2 Position
  SERVO2 if(digitalRead(PinProgramServo2Pos) == HIGH) {
  SERVO2   FlagProgramServo2Pos = 1;  
  SERVO2 } else if((digitalRead(PinProgramServo2Pos) == LOW) && (FlagProgramServo2Pos == 1)) {
  SERVO2   FlagProgramServo2Pos = 2;
  SERVO2 }

  //Read Program pin Servo Speed
  if(digitalRead(PinProgramServoSpeed) == HIGH) {
    FlagProgramServoSpeed = 1;  
  } else if(digitalRead(PinProgramServoSpeed) == LOW) {
    FlagProgramServoSpeed = 0;
  }
}

void SendOutputs(void) {

  //Enable or disable servos
  if(FlagServoEnable == 1) {
    digitalWrite(PinServo1Enable, HIGH);
    SERVO2 digitalWrite(PinServo2Enable, HIGH);
  } else {
    digitalWrite(PinServo1Enable, LOW);
    SERVO2 digitalWrite(PinServo2Enable, LOW);
  }
  
  //Turn on/off Led Normal
  if(FlagLedNormal == 1) {
    SoftPWMSetPercent(PinLedNormal1, LedDim);
    SERVO2 SoftPWMSetPercent(PinLedNormal2, LedDim);
  } else {
    SoftPWMSetPercent(PinLedNormal1, 0);
    SERVO2 SoftPWMSetPercent(PinLedNormal2, 0);
  }

  //Turn on/off Led Reverse
  if(FlagLedReverse == 1) {
    SoftPWMSetPercent(PinLedReverse, LedDim);
  } else {
    SoftPWMSetPercent(PinLedReverse, 0);
  }

  //Turn on or off relay Normal
  if(FlagRelayNormalEnable == 1) {
    digitalWrite(PinRelayNormal, HIGH);
  } else {
    digitalWrite(PinRelayNormal, LOW);  
  }
  
  //Turn on or off relay Reverse
  if(FlagRelayReverseEnable == 1) {
    digitalWrite(PinRelayReverse, HIGH); 
  } else {
    digitalWrite(PinRelayReverse, LOW); 
  }
   
}

void ProgramLed(void) {
  if((FlagProgramLed == 1)) {
    if(CurrentState != PROGRAMLED) {
      TempState = CurrentState;
    }
    CurrentState = PROGRAMLED;
    
    //Increase Led brightness
    if((FlagSwNormal == 1) && (CurrentMillis - TimerLedDim > BUTTONPRESSTIME)) {
      FlagSwNormal = 0;
      TimerLedDim = CurrentMillis;
      LedDim = constrain((LedDim + 1), 0, 100);
      Serial.println(LedDim);
    }

    //Decrease Led brightness
    if((FlagSwReverse == 1) && (CurrentMillis - TimerLedDim > BUTTONPRESSTIME)) {
      FlagSwReverse = 0;
      TimerLedDim = CurrentMillis;
      LedDim = constrain((LedDim - 1), 0, 100);
      Serial.println(LedDim);
    }
  }

  if((FlagProgramLed == 0) && (CurrentState == PROGRAMLED) && (CurrentMillis - TimerLedDim > (BUTTONPRESSTIME * 100))) {
    CurrentState = TempState;
    TimerLedDim = CurrentMillis;
    EEPROM.put(eeAddressLedDim, LedDim);
    Serial.println("Brightness stored");
  }
}

void ProgramServo1Pos(void) {
  if((FlagProgramServo1Pos == 1) && ((CurrentState == NORMAL) || (CurrentState == REVERSE) || (CurrentState == PROGRAMPOS))) {
    if(CurrentState != PROGRAMPOS) {
      TempState = CurrentState;
      Serial.println("ProgramServoPos");
      FlagServoEnable = 1;
      FlagRelayNormalEnable = 0;
      FlagRelayReverseEnable = 0;
    }
    CurrentState = PROGRAMPOS;

    if((FlagSwNormal == 1) && (CurrentMillis - TimerProgramPos > BUTTONPRESSTIME)) {
      TimerProgramPos = CurrentMillis;
      FlagSwNormal = 0;
      Modifier = 1;
      Serial.println("Normal pressed");
    } else if((FlagSwReverse == 1) && (CurrentMillis - TimerProgramPos > BUTTONPRESSTIME)) {
      TimerProgramPos = CurrentMillis;
      FlagSwReverse = 0;
      Modifier = 2;
      Serial.println("Reverse pressed");
    } else {
      Modifier = 0;
    }

    if((TempState == NORMAL) && (Modifier != 0)) {
      if(Modifier == 1) {
        Motor1PosNormal = constrain((Motor1PosNormal + 1), 0, 120);
      } else if(Modifier == 2) {
        Motor1PosNormal = constrain((Motor1PosNormal - 1), 0, 120);
      }
      Servo1.write(Motor1PosNormal);
      Serial.print("Motor1PosNormal: ");
      Serial.println(Motor1PosNormal); 
    } else if((TempState == REVERSE) && (Modifier != 0)) {
      if(Modifier == 1) {
        Motor1PosReverse = constrain((Motor1PosReverse + 1), 0, 120);
      } else if(Modifier == 2) {
        Motor1PosReverse = constrain((Motor1PosReverse - 1), 0, 120);
      }
      Servo1.write(Motor1PosReverse);
      Serial.print("Motor1PosReverse: ");
      Serial.println(Motor1PosReverse); 
    }
  }

  if((FlagProgramServo1Pos == 2) && (CurrentState == PROGRAMPOS) && (CurrentMillis - TimerProgramPos > (BUTTONPRESSTIME * 100))) {
    CurrentState = TempState;
    TimerProgramPos = CurrentMillis;
    FlagProgramServo1Pos = 0;
    if(CurrentState == NORMAL) {
      EEPROM.put(eeAddressMotor1PosNormal, Motor1PosNormal);
      FlagRelayNormalEnable = 1;
    } else if(CurrentState == REVERSE) {
      EEPROM.put(eeAddressMotor1PosReverse, Motor1PosReverse);
      FlagRelayReverseEnable = 1;
    }
    Serial.println("Motor position 1 stored");
    FlagServoEnable = 0;
  }
}

void ProgramServo2Pos(void) {
  if((FlagProgramServo2Pos == 1) && ((CurrentState == NORMAL) || (CurrentState == REVERSE) || (CurrentState == PROGRAMPOS))) {
    if(CurrentState != PROGRAMPOS) {
      TempState = CurrentState;
      Serial.println("ProgramServo2Pos");
      FlagServoEnable = 1;
      FlagRelayNormalEnable = 0;
      FlagRelayReverseEnable = 0;
    }
    CurrentState = PROGRAMPOS;

    if((FlagSwNormal == 1) && (CurrentMillis - TimerProgramPos > BUTTONPRESSTIME)) {
      TimerProgramPos = CurrentMillis;
      FlagSwNormal = 0;
      Modifier = 1;
      Serial.println("Normal pressed");
    } else if((FlagSwReverse == 1) && (CurrentMillis - TimerProgramPos > BUTTONPRESSTIME)) {
      TimerProgramPos = CurrentMillis;
      FlagSwReverse = 0;
      Modifier = 2;
      Serial.println("Reverse pressed");
    } else {
      Modifier = 0;
    }

    if((TempState == NORMAL) && (Modifier != 0)) {
      if(Modifier == 1) {
        Motor2PosNormal = constrain((Motor2PosNormal + 1), 0, 120);
      } else if(Modifier == 2) {
        Motor2PosNormal = constrain((Motor2PosNormal - 1), 0, 120);
      }
      Servo2.write(Motor2PosNormal);
      Serial.print("Motor2PosNormal: ");
      Serial.println(Motor2PosNormal); 
    } else if((TempState == REVERSE) && (Modifier != 0)) {
      if(Modifier == 1) {
        Motor2PosReverse = constrain((Motor2PosReverse + 1), 0, 120);
      } else if(Modifier == 2) {
        Motor2PosReverse = constrain((Motor2PosReverse - 1), 0, 120);
      }
      Servo2.write(Motor2PosReverse);
      Serial.print("Motor2PosReverse: ");
      Serial.println(Motor2PosReverse); 
    }
  }

  if((FlagProgramServo2Pos == 2) && (CurrentState == PROGRAMPOS) && (CurrentMillis - TimerProgramPos > (BUTTONPRESSTIME * 100))) {
    CurrentState = TempState;
    TimerProgramPos = CurrentMillis;
    FlagProgramServo2Pos = 0;
    if(CurrentState == NORMAL) {
      EEPROM.put(eeAddressMotor2PosNormal, Motor2PosNormal);
      FlagRelayNormalEnable = 1;
    } else if(CurrentState == REVERSE) {
      EEPROM.put(eeAddressMotor2PosReverse, Motor2PosReverse);
      FlagRelayReverseEnable = 1;
    }
    Serial.println("Motor position 2 stored");
    FlagServoEnable = 0;
  }
}

void ProgramServoSpeed(void) {
  if((FlagProgramServoSpeed == 1)) {
    if(CurrentState != PROGRAMSPEED) {
      TempState = CurrentState;
      FlagServoEnable = 1;
      FlagRelayNormalEnable = 0;
      FlagRelayReverseEnable = 0;
      if(TempState == NORMAL) {
        Direction = REVERSE;
      } else if(TempState == REVERSE) {
        Direction = NORMAL;
      }
    }
    CurrentState = PROGRAMSPEED;
    
    //Increase Speed
    if((FlagSwNormal == 1) && (CurrentMillis - TimerServoSpeed > BUTTONPRESSTIME)) {
      FlagSwNormal = 0;
      TimerServoSpeed = CurrentMillis;
      MotorSpeed = constrain((MotorSpeed - 1), 0, 255);
      Serial.print("MotorSpeed: ");
      Serial.println(MotorSpeed);
    }

    //Decrease Speed
    if((FlagSwReverse == 1) && (CurrentMillis - TimerServoSpeed > BUTTONPRESSTIME)) {
      FlagSwReverse = 0;
      TimerServoSpeed = CurrentMillis;
      MotorSpeed = constrain((MotorSpeed + 1), 0, 255);
      Serial.print("MotorSpeed: ");
      Serial.println(MotorSpeed);
    }

    //Move Servos Normal to Reverse
    if((CurrentMillis - TimerMotorSpeed > MotorSpeed) && (Direction == NORMAL)) {
      //Move Servos
      CurrentPos1 = StepServo(CurrentPos1, Motor1PosNormal);
      CurrentPos2 = StepServo(CurrentPos2, Motor2PosNormal);
      Servo1.write(CurrentPos1); //Move the servo one step
      Servo2.write(CurrentPos2); //Move the servo one step
  
      //Check if in position
      if((Motor1PosNormal == CurrentPos1) && (Motor2PosNormal == CurrentPos2)) {
        Direction = REVERSE;
      }
      TimerMotorSpeed = CurrentMillis;
    }

    //Move Servos Reverse to Normal
    if((CurrentMillis - TimerMotorSpeed > MotorSpeed) && (Direction == REVERSE)) {
      //Move Servos
      CurrentPos1 = StepServo(CurrentPos1, Motor1PosReverse);
      CurrentPos2 = StepServo(CurrentPos2, Motor2PosReverse);
      Servo1.write(CurrentPos1); //Move the servo one step
      SERVO2 Servo2.write(CurrentPos2); //Move the servo one step
  
      //Check if in position
      if((Motor1PosReverse == CurrentPos1) && (Motor2PosReverse == CurrentPos2)) {
        Direction = NORMAL;
      }
      TimerMotorSpeed = CurrentMillis;
    }
  }

  if((FlagProgramServoSpeed == 0) && (CurrentState == PROGRAMSPEED) && (CurrentMillis - TimerServoSpeed > (BUTTONPRESSTIME * 100))) {
    CurrentState = TempState;
    TimerServoSpeed = CurrentMillis;
    if(CurrentState == NORMAL) {
      FlagRelayNormalEnable = 1;
    } else if(CurrentState == REVERSE) {
      FlagRelayReverseEnable = 1;
    }
    FlagServoEnable = 0;
    EEPROM.put(eeAddressMotorSpeed, MotorSpeed);
    Serial.println("Speed stored");
  }
}

void receiveCommand(int howMany){
  LastMasterCommand = Wire.read(); // 1 byte (maximum 256 commands)
  //Serial.println(LastMasterCommand);
}
 
void slavesRespond(){
 
  int returnValue = 0;
 
  switch(LastMasterCommand){
    case NORMAL:   //Replace with Check if Normal
    break;
    
    case REVERSE:   //Replace with Check if Reverse
    break;
 
    case MOVN2R:   //Throw switch N2R
        Wire.write(HANDSHAKE);  //Send handshake
        FlagSwReverse = 1;
        
    break;

    case MOVR2N:   //Throw switch R2N
        Wire.write(HANDSHAKE);  //Send handshake
        FlagSwNormal = 1;

    break;

    case CHECKSTATUS:   
      Wire.write(CurrentState);
    break;
    
  }
  LastMasterCommand = 0;          // null last Master's command
}
