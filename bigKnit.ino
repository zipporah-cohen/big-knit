#include <Wire.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

/*
 * In this implementation, AccelStepper and limit switches together is a bit redundant.
 * Using both here keeps our options open, but if we use AccelStepper, we could conceivably not need limit swtiches.
 * We only need to do the math to determine our desired position based on distance.
 * 
 */

//constants
const int STEPPER_PORT = 2; //M3 and M4 of MotorShield
const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //place holder val
const int LIMIT_PIN1 = 5; //NC pin
const int LIMIT_PIN2 = 6; //NC pin

int desiredPos = 1000;

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myStepper = AFMS.getStepper(STEPS_PER_REV, STEPPER_PORT);
ezButton limitSwitch1(LIMIT_PIN1); 
ezButton limitSwitch2(LIMIT_PIN2); 

void forwardStep() {
  myStepper->onestep(FORWARD, SINGLE);
}

void backwardStep() {
  myStepper->onestep(BACKWARD, SINGLE);
}

AccelStepper stepper(forwardStep, backwardStep);

void setup() {
  AFMS.begin();
  Serial.begin(9600);
  
  stepper.setSpeed(50);
  stepper.moveTo(desiredPos);

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
      Serial.println("stopped");
    }
  }

  limitSwitch1.loop();
  limitSwitch2.loop();

  //while neither is pressed, move the motor (could use isReleased() instead of LOW)
  while(limitSwitch1.getState() == LOW && limitSwitch2.getState() == LOW){
    stepper.run();
  }

  //when one is pressed, stop motor, change direction/desired position
  if(limitSwitch1.isPressed() ^ limitSwitch2.isPressed()){
    Serial.println("Swtich is pressed");
    desiredPos = desiredPos*(-1);
    stepper.moveTo(desiredPos);
  }

  //Add check to make sure the switch is released after change
  if(stepper.run() && (limitSwitch1.isPressed() || limitSwitch2.isPressed())){
    Serial.println("Stepper is running but switch is pressed");
  }
}

void homing(){}

void knitSingleRow(){}
