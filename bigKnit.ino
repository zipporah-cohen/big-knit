#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

/*
 * Initialize Variables
 */

//constants
const int STEPPER_PORT = 2; //M3 and M4
const int STEPS_PER_REV = 200; //smaller stepper
const int LIMIT_PIN1 = A0;
const int LIMIT_PIN2 = A1;

//non-constant
bool stepDir;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myStepper = AFMS.getStepper(STEPS_PER_REV, STEPPER_PORT);

void forwardStep() {
  myStepper->onestep(FORWARD, SINGLE);
}

void backwardStep() {
  myStepper->onestep(BACKWARD, SINGLE);
}

AccelStepper stepper(forwardStep, backwardStep); // use functions to step

void setup() {
  Serial.begin(9600);
  
  stepDir = 1;
  stepper.setSpeed(50);
}

void loop() {
  
  if(Serial.available() > 0){
    String str = Serial.readString();
    str.trim();
    if (str == "home"){
      Serial.println("homing");
      //homing();
    } 
    else if(str == "knit"){
      knitSingleRow();
    }
    else if(str == "stop"){
      Serial.println("stopped");
    }
  }
}

void homing(){}

void knitSingleRow(){
  Serial.println("knitting");

  //while not at a limit switch{}
  stepper.runSpeed();

  //when at limit switch{switch dir}
   
//  if(stepDir == FORWARD){ 
//    stepDir = BACKWARD; 
//  }
//  else stepDir = FORWARD;
  
 }

 void stop(){
  stepper.stop();
 }
