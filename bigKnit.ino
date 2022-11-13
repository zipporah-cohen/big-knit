#include <Wire.h>
#include <ezButton.h>
#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>
//#include <Adafruit_PWMServoDriver.h>

//constants - motors
const int SERVO_DOWN = 600; // This is the rounded 'minimum' microsecond length based on the minimum pulse of 150
const int SERVO_UP = 2400; // This is the rounded 'maximum' microsecond length based on the maximum pulse of 600
const int SERVO_FREQ = 50; // Analog servos run at ~50 Hz updates
const int YARN_SERVO = 0;
const int AF_STEPPER_PORT = 2; //M3 and M4 of MotorShield
const int STEPS_PER_REV = 200; //smaller stepper
const int INCH_PER_REV = 1; //place holder val

//constants - misc
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

// called this way, it uses the default address 0x40
//Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

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
  if (!USE_CNC) {
    AFMS.begin();
  }
  
  Serial.begin(9600);

  /*Adjust these. Using example values*/
//  pwm.begin();
//  pwm.setOscillatorFrequency(27000000);
//  pwm.setPWMFreq(50);  // Analog servos run at ~50 Hz updates

  delay(10);

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
    if (USE_CNC) {
      cncLoop();
    }
    else {
      afmsKnitRow(3, true);
    }
  }

}

void afmsKnitRow(int numRows, bool yarnColor) {

  //REFACTOR: set boolean to check changeYarnColor in which case it will only move if it needs to. Read current position and change accordingly
  //set yarn color
//  if(yarnColor){
//    for (uint16_t pulselen = SERVO_DOWN; pulselen < SERVO_UP; pulselen++) {
//    pwm.setPWM(YARN_SERVO, 0, pulselen);
//    }
//  }
//  else{
//    for (uint16_t pulselen = SERVO_UP; pulselen < SERVO_DOWN; pulselen++) {
//    pwm.setPWM(YARN_SERVO, 0, pulselen);
//    }
//  }
  
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


void cncLoop()  {
  //insert cnc code here
}

void homing() {}

void knitSingleRow() {}
