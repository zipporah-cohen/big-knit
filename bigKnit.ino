#include <Wire.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

/*
 * Next to fix: it wil switch which limit to hit multiple times instead of once. 
 * Try: putting it all in the main loop insted of getting called. Or, changing logic of if/elses to somehow account for the outlier.
 * 
 */

//constants
const int AF_STEPPER_PORT = 2; //M3 and M4 of MotorShield
const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //place holder val
const int LIM_L = A0; //NC
const int LIM_R = A1; //NC

const bool USE_CNC = false;

bool manualStop;
int distToMove;
int stepsToTake;
int stepSpeed;
int nextLim;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myStepper = AFMS.getStepper(STEPS_PER_REV, AF_STEPPER_PORT);

void forwardStep() {
  myStepper->onestep(FORWARD, SINGLE);
}
void backwardStep() {
  myStepper->onestep(BACKWARD, SINGLE);
}

AccelStepper stepper(forwardStep, backwardStep);

void setup() {
  if(!USE_CNC){
    AFMS.begin();
  }
  
  Serial.begin(9600);
  pinMode(LIM_L, INPUT);
  pinMode(LIM_R, INPUT);
  
  manualStop = false;
  distToMove = 1000;//use later with math
  stepsToTake = 100;
  stepSpeed = 50; 
  nextLim = LIM_R; //which limit switch will get hit next

  stepper.setCurrentPosition(stepper.currentPosition()); //set current pos to be the new 0
  stepper.setSpeed(stepSpeed);
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
  
  //if the limit to hit is unpressed and the stepper is moving, continue
  if(analogRead(nextLim) == LOW && stepper.run()){
    stepper.run();
  }
  //if the limit switch has been hit and the stepper is still running
  else if(analogRead(nextLim) == HIGH && stepper.run()){
    //stop the stepper
    stepper.stop();
    //wait for stepper to stop moving
    while(stepper.run()){}
  }
  //if the stepper has stopped regardless of limit switch
  //(analogRead(AF_LIMIT_PIN1) == HIGH || analogRead(AF_LIMIT_PIN1) == LOW)
  else{
    Serial.println("stepper no longer running");

    //switch limit to hit
    if(nextLim == LIM_L){
      Serial.println("Switching to hit limit R (A1)");
      nextLim = LIM_R;
    }
    else{
      Serial.println("Switching to hit limit L (A0)");
      nextLim = LIM_L;
    }
    
    stepsToTake = -stepsToTake;
    stepper.move(stepsToTake);
    stepper.run();
  }
}

void cncLoop()  {
  //insert cnc code here
}

void homing(){}

void knitSingleRow(){}
