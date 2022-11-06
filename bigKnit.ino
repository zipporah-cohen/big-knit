#include <Wire.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

/*
 * Next to fix: currently relies on the limit switch being quickly pressed and released. Start just reading the limit switch's analog directly.
 * With current logic, will run forever until limit switch is pressed. Change logic to take specified steps and stop at end or at limit switch. 
 * 
 */

//constants
const int AF_STEPPER_PORT = 2; //M3 and M4 of MotorShield
const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //place holder val
const int AF_LIMIT_PIN1 = A0; //NC pin
const int AF_LIMIT_PIN2 = A1; //NC pin

const bool USE_CNC = false;

bool manualStop;
int distToMove;
int stepsToTake;
int stepSpeed;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myStepper = AFMS.getStepper(STEPS_PER_REV, AF_STEPPER_PORT);

void forwardStep() {
  myStepper->onestep(FORWARD, SINGLE);
}
void backwardStep() {
  myStepper->onestep(BACKWARD, SINGLE);
}

AccelStepper stepper(forwardStep, backwardStep);

ezButton limitSwitch1(AF_LIMIT_PIN1); 
ezButton limitSwitch2(AF_LIMIT_PIN2); 

void setup() {
  if(!USE_CNC){
    AFMS.begin();
  }
  
  Serial.begin(9600);

  manualStop = false;
  distToMove = 1000;//use later with math
  stepsToTake = 50;
  stepSpeed = 50; 

  stepper.setCurrentPosition(stepper.currentPosition()); //set current pos to be the new 0
  stepper.setSpeed(stepSpeed);

  limitSwitch1.setDebounceTime(50);
  limitSwitch2.setDebounceTime(50);
}

void loop() {

  //infrastructure for commands
  if(Serial.available() > 0){
    String str = Serial.readString();
    str.trim();
    if (str == "home"){
      Serial.println("homing");
    } 
    else if(str == "knit"){
      Serial.println("knitting");
    }
    else if(str == "stop"){
      manualStop = true;
      stepper.stop();
    }
  }

  limitSwitch1.loop();
  limitSwitch2.loop();

  if(!manualStop){
    if(USE_CNC){
      cncLoop();
    }
    else{
      afmsLoop();
    }
  }

}

void afmsLoop() {
  
  //while neither is pressed, move the motor
  if(!(limitSwitch1.isPressed() || limitSwitch2.isPressed())){
    Serial.println("Neither is pressed");
    stepper.move(stepsToTake);
    stepper.run();
  }

  //when one is pressed, stop motor, change direction/desired position
  if(limitSwitch1.isPressed() ^ limitSwitch2.isPressed()){
    Serial.println("Swtich is pressed");
    stepper.stop();
    stepsToTake = -stepsToTake;
    //stepper.move(stepsToTake);
  }
}

void cncLoop()  {
  //insert cnc code here
}

void homing(){}

void knitSingleRow(){}
