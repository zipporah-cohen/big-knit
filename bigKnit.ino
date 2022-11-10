#include <Wire.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

//constants
const int AF_STEPPER_PORT = 2; //M3 and M4 of MotorShield
const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //place holder val
const int LIM_L = A0; //NC
const int LIM_R = A1; //NC
const int NEEDLE_DIST = 1; //distance between needles to determine width of rug

const bool USE_CNC = false;

bool yarnColor; //True = yarn1, False = yarn2
bool manualStop;
bool dir; //True = right, False = left
int distToMove;
int stepSpeed;
int stepsToTake;

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
  dir = true; //start expecting to move right
  distToMove = 1000;//use later with math
  stepsToTake = 100;
  stepSpeed = 50;

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
  
  //if carriage going right
  if(dir){
    //if carriage not at limit switch and not in pos
    if(analogRead(LIM_R) == LOW && stepper.distanceToGo() != 0){
      stepper.run();
      Serial.println("Going to the right");
    }
    //either at limit switch or hit target pos
    else{
      Serial.println("Reason to stop at right");
      //if at switch, stop moving
      if(stepper.distanceToGo() != 0){
        stepper.stop();
        //wait for stepper to stop moving
        while(stepper.run()){}
      }
      Serial.println("Changing to left");
      dir = !dir;
      stepsToTake = -stepsToTake;
      stepper.move(stepsToTake);
      stepper.run();
    }
  }
  
  //if carriage going left
  if(!dir){
    //if carriage not at limit switch and not in pos
    if(analogRead(LIM_L) == LOW && stepper.distanceToGo() != 0){
      Serial.println("Going to the left");
      stepper.run();
    }
    //either at limit switch or hit target pos
    else{
      Serial.println("Reason to stop at left");
      //if at switch, stop moving
      if(stepper.distanceToGo() != 0){
        stepper.stop();
        //wait for stepper to stop moving
        while(stepper.run()){}
      }
      Serial.println("Changing to right");
      dir = !dir;
      stepsToTake = -stepsToTake;
      stepper.move(stepsToTake);
      stepper.run();
    }
  }
}

void cncLoop()  {
  //insert cnc code here
}

void homing(){}

void knitSingleRow(){}
