#include <Wire.h>
#include <AccelStepper.h>
#include <Adafruit_PWMServoDriver.h>

//constants - motors
const int SERVO_MIN = 500; // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
const int SERVO_MAX = 2000; // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
const int SERVO_FREQ = 50; // Analog servos run at ~50 Hz updates
const int YARN_SERVO = 0; //pin on servo driver
const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //placeholder

//Stepper Driver
//placeholders for digital pins for step and dir connection
const int DIR = 3; 
const int STEP = 2;
const int INTERFACE = 1; //interface type as defined in accelStepper

//constants - misc
const int LIM_L = A0; //NC
const int LIM_R = A1; //NC
const int NEEDLE_DIST = 1; //distance between needles to determine width of rug
const int NUM_NEEDLES = 4;

bool manualStop;
bool dir; //True = right, False = left
int stepSpeed;
int stepsToTake;
int lastPwm;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
AccelStepper stepper = AccelStepper(INTERFACE, STEP, DIR);

void setup() {
  
  Serial.begin(9600);
  
  pinMode(LIM_L, INPUT);
  pinMode(LIM_R, INPUT);

  manualStop = false;
  dir = true; //start expecting to move right
  stepSpeed = 50;
  lastPwm = 0;

  int totalDist = NUM_NEEDLES + NEEDLE_DIST;
  stepsToTake = totalDist/double(INCH_PER_REV/STEPS_PER_REV);

  pwm.begin();
  pwm.setOscillatorFrequency(25000000);
  pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates

  stepper.setCurrentPosition(stepper.currentPosition()); //set current pos to be the new 0
  stepper.setSpeed(stepSpeed);

  delay(10);
}

void loop() {

  //infrastructure for commands
  if (Serial.available() > 0) {
    String str = Serial.readString();
    str.trim();
    if (str == "home") {
      Serial.println("homing");
    }
    else if (str == "knit") {
      Serial.println("knitting");
    }
    else if (str == "stop") {
      manualStop = true;
      stepper.stop();
    }
  }

  if (!manualStop) {
    knitRow(3);
  }

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

void changeYarn(){
  uint16_t microsec;
  if(lastPwm < SERVO_MAX){
    for (microsec = SERVO_MIN; microsec < SERVO_MAX; microsec++) {
      pwm.writeMicroseconds(YARN_SERVO, microsec);
    }
    lastPwm = microsec;
  }
  else{
    for (microsec = SERVO_MAX; microsec > SERVO_MIN; microsec--) {
      pwm.writeMicroseconds(YARN_SERVO, microsec);
    }
    lastPwm = microsec;
  }

}

void castOn() {
  //use servo min and max and oscillate between the two at a specific interval
  //only move horizontally across once
  //use pre-determined stepsToTake, calculated by number of needles and dist between them
  }

void homing() {}
