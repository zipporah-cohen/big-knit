#include <Wire.h>
#include <AccelStepper.h>

const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //placeholder

//placeholders for digital pins for step and dir connection
const int DIR = 2; 
const int STEP = 3;
const int INTERFACE = 1; //interface type as defined in accelStepper

//constants - misc
const int LIM_L = A0; //NC
const int LIM_R = A1; //NC

bool manualStop;
bool dir; //True = right, False = left
int distToMove;
int stepsToTake;
int stepSpeed;

AccelStepper stepper = AccelStepper(INTERFACE, STEP, DIR);

void setup() {
  
  Serial.begin(9600);
  
  pinMode(LIM_L, INPUT);
  pinMode(LIM_R, INPUT);

  manualStop = false;
  dir = true; //start expecting to move right
  stepsToTake = 100;
  stepSpeed = 50;

  stepper.setCurrentPosition(stepper.currentPosition()); //set current pos to be the new 0
  stepper.setSpeed(stepSpeed);

  delay(10);
}

void loop() {
  knitRow(3);
}

void knitRow(int numRows) {

  for (int i = 0; i < numRows; i++) {
    Serial.print("Row: ");
    Serial.println(i);

    //if carriage going right
    if (dir) {
      //if carriage not at limit switch and not in pos
      while (analogRead(LIM_R) == LOW && stepper.distanceToGo() != 0) {
        stepper.run();
      }
      //either at limit switch or hit target pos
      //if at switch, stop moving
      if (stepper.distanceToGo() != 0) {
        stepper.stop();
        //wait for stepper to stop moving
        while (stepper.run()) {}
      }
      dir = !dir;
      stepsToTake = -stepsToTake;
      stepper.move(stepsToTake);
    }
    else { //if carriage going left
      //if carriage not at limit switch and not in pos
      while (analogRead(LIM_L) == LOW && stepper.distanceToGo() != 0) {
        stepper.run();
      }
      //if at switch, stop moving
      if (stepper.distanceToGo() != 0) {
        stepper.stop();
        //wait for stepper to stop moving
        while (stepper.run()) {}
      }
      dir = !dir;
      stepsToTake = -stepsToTake;
      stepper.move(stepsToTake);
    }
  }
  Serial.print("Done knitting ");
  Serial.print(numRows);
  Serial.println("rows");
}
