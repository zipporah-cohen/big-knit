#include <AccelStepper.h>
#include <Adafruit_MotorShield.h>

Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_StepperMotor *myStepper1 = AFMS.getStepper(200, 2);

void forwardstep1() {
  myStepper1->onestep(FORWARD, SINGLE);
}

void backwardstep1() {
  myStepper1->onestep(BACKWARD, SINGLE);
}

AccelStepper stepper(forwardstep1, backwardstep1); // use functions to step

void setup()
{
   Serial.begin(9600);           // set up Serial library at 9600 bps
   Serial.println("Stepper test!");

  if (!AFMS.begin()) {         // create with the default frequency 1.6KHz
  // if (!AFMS.begin(1000)) {  // OR with a different frequency, say 1KHz
    Serial.println("Could not find Motor Shield. Check wiring.");
    while (1);
  }
  Serial.println("Motor Shield found.");

  stepper.setSpeed(50);
}

void loop()
{
   stepper.runSpeed();
}
