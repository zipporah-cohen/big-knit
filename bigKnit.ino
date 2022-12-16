#include <AccelStepper.h>
#include <ezButton.h>

#define directionPin 3  
#define stepPin 2
#define enablePin 4
#define motorInterfaceType 1

// Define a stepper and the pins it will use
AccelStepper stepper = AccelStepper(motorInterfaceType, stepPin, directionPin); // Defaults to AccelStepper::FULL4WIRE (4 pins) on 2, 3, 4, 5

const int leftLimitSwitchPin = 8;
const int rightLimitSwitchPin = 7;

//placeholders
const int needle_dist = 1;
const int stepper_diameter = 2;
const int steps_per_rev = 200;

ezButton leftLimitSwitch(leftLimitSwitchPin);
ezButton rightLimitSwitch(rightLimitSwitchPin);

struct machineState{
  bool crashed = false;
};
machineState machine;

struct rowState{
  int n = 0;
  bool start = false;
  int steps = 0;
};
rowState row;

struct limitState{
  bool left = false;
  bool right = false;
};

limitState limit;

long initial_homing = -1;
int BedLength = 22000;
int BedStart = 300;
int BedEnd = 22800;

int rugWidth;
int rugLength;

void setup() {
  Serial.begin(115200);
  
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW);
  
  leftLimitSwitch.setDebounceTime(50);
  rightLimitSwitch.setDebounceTime(50);

  stepper.setMaxSpeed(2000);
  stepper.setAcceleration(5000);
  
  fastHome(-1);

  stepper.setMaxSpeed(5000);
  stepper.setAcceleration(5000);

  row.steps = BedEnd;

  row.start = true;

  //getUserValues();
  //calculateValues(10);
}

void loop() {
  leftLimitSwitch.loop();
  rightLimitSwitch.loop();
  knitRowState(1000);
}

void knitRowState(int numRows) {
  limit.left = leftLimitSwitch.getState();
  limit.right = rightLimitSwitch.getState();

  if (row.start){
    Serial.print("starting row: ");
    Serial.println(row.n);
    stepper.moveTo(row.steps);
    row.start = false;
  }

  if(stepper.distanceToGo() == 0){
    if(row.n < numRows){
      row.n ++;
      if(row.n % 2 == 0){
        row.steps = BedStart;
        row.start = true;
        Serial.println(", knitting forwards");
      }else{
        row.steps = BedEnd;
        row.start = true;
        Serial.println(", knitting backwards");
      }
    }
  }

  if (limit.left || limit.right){
    if(!machine.crashed){
      Serial.println("crash!!");
      machine.crashed = true;
      digitalWrite(enablePin,HIGH);
    }
  }else {
    if(machine.crashed){
      machine.crashed = false;
      digitalWrite(enablePin,LOW);
    }
    stepper.run();
  }
}

void fastHome(int direction){

  stepper.moveTo(100000*direction);
  
  while(rightLimitSwitch.getStateRaw() == LOW) {
    stepper.run();
  }
  stepper.stop(); // Stop as fast as possible 
    Serial.println("motor at low end with limit switch touched");  // uncomment for debug 

  stepper.moveTo(100*-direction);

  while(rightLimitSwitch.getStateRaw() == LOW) {
    stepper.run();
  }
    stepper.stop(); // Stop as fast as possible 
    Serial.println("motor at low end with limit switch not touched"); // uncomment for debug 
    
  stepper.setCurrentPosition(0);
  stepper.setMaxSpeed(10000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepper.setAcceleration(10000.0);
  stepper.runToNewPosition(200);

  delay(1000);
}

void home() {

   //  Set Max Speed and Acceleration of each Steppers at startup for homing
  stepper.setMaxSpeed(10000.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(10000.0);  // Set Acceleration of Stepper
 

// Start Homing procedure of Stepper Motor at startup

  Serial.print("Stepper is Homing . . . . . . . . . . . ");

  while (digitalRead(leftLimitSwitchPin)) {  // Make the Stepper move CCW until the switch is activated   
    stepper.moveTo(initial_homing);  // Set the position to move to
    initial_homing ++ ;  // Decrease by 1 for next move if needed
    stepper.run();  // Start moving the stepper
    delay(5);
  }

  stepper.setCurrentPosition(0);  // Set the current position as zero for now
  stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
  initial_homing=1;

  while (!digitalRead(leftLimitSwitchPin)) { // Make the Stepper move CW until the switch is deactivated
    stepper.moveTo(initial_homing);  
    stepper.run();
    initial_homing++;
    // delay(5);
  }
  
  stepper.setCurrentPosition(0);
  Serial.println("Homing Completed");
  Serial.println("");
  stepper.setMaxSpeed(10000.0);      // Set Max Speed of Stepper (Faster for regular movements)
  stepper.setAcceleration(10000.0);
  stepper.runToNewPosition(-200);

  delay(1000);
}

void getUserValues(){
  Serial.println("Enter your desired rug width");
  while(Serial.available() == 0) {}
  rugWidth = readSerial();
  
  Serial.println("Width: ");
  Serial.println(rugWidth);   // Do something with the value
    
  Serial.println("Enter your desired rug length");
  while(Serial.available() == 0) {}
  rugLength = readSerial();
  
  Serial.println("Length: ");
  Serial.println(rugLength);   // Do something with the value

  row.steps = 100*rugLength;

  row.start = true;
}

int readSerial(){
  int val = 0;
  while(1) {        // force into a loop until new line received
    int incomingByte = Serial.read();
    if (incomingByte == '\n') break;   // exit the while(1), we're done receiving
    if (incomingByte == -1) continue;  // if no characters are in the buffer read() returns -1
    val *= 10;  // shift left 1 decimal place
    // convert ASCII to integer, add, and shift left 1 decimal place
    val = ((incomingByte - 48) + val);
  }
  return val;
}

void calculateValues(int rugWidth){
  //program will err on the shorter side of the desired width
  int numNeedles =  (int)(rugWidth/needle_dist)+1;
  double r = stepper_diameter/2.0;
  //can change the buffer distance
  double totalDist = (numNeedles + 1) * needle_dist;
  //double theta = 360.0/STEPS_PER_REV;
  //arc length = (theta/360)*r2pi
  double distPerStep = (1/steps_per_rev)*r*2*3.14159;
  row.steps = totalDist/distPerStep;
}
