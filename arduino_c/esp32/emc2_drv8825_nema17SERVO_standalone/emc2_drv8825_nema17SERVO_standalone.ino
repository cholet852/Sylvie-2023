//This is code for the EMC2 Robot servo using DRV8825 driver.
//You don't have to change anything with the wiring.
//..Just make sure you use this code on your ESP32!!
//
//It is recommended to use the StepStick Protector with DRV8825

#define EN_PIN 13
#define DIR_PIN 32 // 33 for 38p esp32
#define STEP_PIN 33 // 25 for 38p esp32

// Pins used for SPI are now used to set microsteps to 32 res
#define MODE0_PIN 23
#define MODE1_PIN 18
#define MODE2_PIN 5
#define RESET_PIN 19

//#define CS_PIN 5
//#define R_SENSE 0.11f // Match to your driver

#define SDA_PIN 27 // 15 for 38p esp32
#define SCL_PIN 26 // 16 for 38p esp32

#define SWITCH_ONE_PIN 14 // 17 for 38p esp32
#define ANALOG_PIN 34 // Can use pin 35 or 34, but let's keep it at 34

#define I2C_SLAVE_ADDR 0x61

//#include <TMCStepper.h>
#include <FastAccelStepper.h>

#include <Wire.h>
#include <PID_v1.h>

// Seeed Studio AS5600.h library used for Encoder safety and error-checking.
// Make sure your i2c master pins (22 SCL, 21 SDA) are connected to the AS5600, in addition to the analog pin.
#include <AS5600.h>

//TMC2130Stepper driver = TMC2130Stepper(CS_PIN, R_SENSE); // Hardware SPI

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperA = NULL;

AMS_5600 ams5600;

// Homing handlers
int limitSwitchOne_state = 0;

float homeOffset = 2.5;
bool switchPressed = false;
bool offsetTriggered = false;

bool isInitiatingHoming = false;

bool isContinuousRotation = true;

// Wire character read stuffs
float Raw_Input = 0;
float User_Input = 0; // This while convert input string into integer

boolean flagRx = false;
boolean containsComma = false;
char readStringC[10]; //This while store the user input data

// PID stuffs
double kp = 6, ki = 5, kd = 0.1;
float newkp = 6, newki = 5, newkd = 0.1;
double input = 0, output = 0, setpoint = 0;
PID myPID( & input, & output, & setpoint, kp, ki, kd, DIRECT);

// Motor and driver speed stuffs
long openSpeed = 40; // Less is more
long closedSpeed = openSpeed + 10;

long openAccel = 100000;
long homingSpeed = 25;
long homingAccel = 1000000;

int enableThreshold = 1;

bool openLoopRunning = false;
bool closedLoopRunning = true;
bool closedLoopStart = true;

// Encoder stuffs
int quadrantNumber, previousQuadrantNumber;
bool quadrantClockwise = true;
long revolutions = 0; // number of revolutions the encoder has made

double totalAngle, startAngle;
double previousTotalAngle;

// Stepper driver settings and gearboxes (if any)
float bevelGear = (49.00 / 12.00);
float gearReduction = 53.575757576 * bevelGear;

float stepsPerRev = 200.00;
int microstepping = 32;
float formula = ((stepsPerRev * microstepping) / 360.00);

//long driverCurrent = 1000;

void setup() {
  Serial.begin(115200);
  //SPI.begin();

  Wire.begin();
  Wire.setClock(800000);
  bool success = Wire1.begin(I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN, 400000);
  if (!success) {
    Serial.println("I2C slave init failed");
    while (1) delay(100);
  } else {
    Serial.println("I2C slave init success!");
  }
  Wire1.onReceive(receiveEvent);

  myPID.SetMode(AUTOMATIC); //set PID in Auto mode
  myPID.SetSampleTime(2); // refresh rate of PID controller
  myPID.SetOutputLimits(-500, 500); // this is the MAX PWM value to move motor, here change in value reflect change in speed of motor.

  engine.init();
  stepperA = engine.stepperConnectToPin(STEP_PIN);
  if (stepperA) {
    stepperA -> setDirectionPin(DIR_PIN);
    stepperA -> setEnablePin(EN_PIN);
    stepperA -> setAutoEnable(true);

    stepperA -> setSpeedInUs(openSpeed); // the parameter is us/step !!!
    stepperA -> setAcceleration(openAccel);
  }

  pinMode(MODE0_PIN, OUTPUT);
  pinMode(MODE1_PIN, OUTPUT);
  pinMode(MODE2_PIN, OUTPUT);
  pinMode(RESET_PIN, OUTPUT);

  digitalWrite(MODE0_PIN, HIGH);
  digitalWrite(MODE1_PIN, HIGH);
  digitalWrite(MODE2_PIN, HIGH);
  digitalWrite(RESET_PIN, HIGH);

  //driver.begin(); // Initiate pins and registeries
  //driver.rms_current(driverCurrent); // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
  //driver.microsteps(microstepping);
  //driver.shaft(false);

  startAngle = degAngle(); //update startAngle with degAngle - for taring
  int magnetStrength = ams5600.getMagnetStrength();

  if (magnetStrength == 0){
    Serial.println("Error: No magnet detected!");
  } else if (magnetStrength == 1){
    Serial.println("Warning: Magnet is too weak!"); 
  } else if (magnetStrength == 2){
    Serial.println("Success: Magnet is OK.");  
  } else if (magnetStrength == 3){
    Serial.println("Warning: Magnet is too strong!");  
  }
}

double degAngle() {
  double rawAngle = ams5600.getRawAngle(); //analogRead(ANALOG_PIN);
  return rawAngle * (360.00 / 4096.00);
}

double taredAngle() {
  double correctedAngle = degAngle() - startAngle; //this tares the position

  if (correctedAngle < 0) { //if the calculated angle is negative, we need to "normalize" it
    correctedAngle = correctedAngle + 360.00; //correction for negative numbers (i.e. -15 becomes +345)
  }

  return correctedAngle;
}

void getEncoderReading() {
  while (ams5600.getMagnetStrength() < 2){  // Stops stepper motor if encoder not detected for safety
    if (stepperA -> isRunning() == true){
      stepperA -> forceStop();  
    }
    Serial.println("Error: Magnet is weak!");
  }
  
  double correctAngle = taredAngle(); //tare the value

  if (correctAngle >= 0 && correctAngle <= 90) {
    quadrantNumber = 1;
  } else if (correctAngle > 90 && correctAngle <= 180) {
    quadrantNumber = 2;
  } else if (correctAngle > 180 && correctAngle <= 270) {
    quadrantNumber = 3;
  } else if (correctAngle > 270 && correctAngle < 360) {
    quadrantNumber = 4;
  }

  if (quadrantNumber != previousQuadrantNumber) { //if we changed quadrant
    if (quadrantNumber == 1 && previousQuadrantNumber == 4) {
      revolutions++; // 4 --> 1 transition: CW rotation
      quadrantClockwise = true;
    } else if (quadrantNumber == 4 && previousQuadrantNumber == 1) {
      revolutions--; // 1 --> 4 transition: CCW rotation
      quadrantClockwise = false;
    }
    previousQuadrantNumber = quadrantNumber; //update to the current quadrant
  }
  totalAngle = (revolutions * 360.00) + correctAngle;
  //Serial.println(totalAngle);

  // Check for missing rotations! And correct them accordingly...
  if (abs(totalAngle - previousTotalAngle) > 350.00) {
    Serial.println("Warning! Missed revolution! Details: ");
    Serial.println(previousTotalAngle);
    Serial.println(totalAngle);
    if (quadrantClockwise == true) {
      revolutions--;
    } else {
      revolutions++;
    }
    totalAngle = (revolutions * 360.00) + correctAngle;
  }
  previousTotalAngle = totalAngle;
}

void stepperClosedLoop(int out) {
  if (closedLoopStart == true) {
    Serial.println(User_Input);
    Serial.println(totalAngle);
    closedLoopStart = false;
  }

  if (abs(out) < enableThreshold) {
    stepperA -> stopMove();
    closedLoopRunning = false;
  } else {
    if (out > 0) {
      int newSpeed = map(abs(out), 0, 500, 1000, closedSpeed);
      stepperA -> setSpeedInUs(newSpeed);
      stepperA -> applySpeedAcceleration();
      stepperA -> runForward();
    } else if (out < 0) {
      int newSpeed = map(abs(out), 0, 500, 1000, closedSpeed);
      stepperA -> setSpeedInUs(newSpeed);
      stepperA -> applySpeedAcceleration();
      stepperA -> runBackward();
    }
  }
}

void stepperOpenLoop(float openSteps, float moveSpeed) {
  int actualSteps_relative = openSteps * formula;
  int actualAngle = totalAngle * formula;

  openLoopRunning = true;
  closedLoopRunning = true;
  closedLoopStart = true;

  stepperA -> setCurrentPosition(actualAngle);

  // Open loop runs faster than closed loop error correction
  stepperA -> setSpeedInUs(moveSpeed);
  stepperA -> setAcceleration(openAccel);
  stepperA -> applySpeedAcceleration();
  stepperA -> moveTo(actualSteps_relative);
}

void receiveEvent(int byteCount) {
  int i;
  containsComma = false;

  for (i = 0; i < byteCount; i++) {
    readStringC[i] = Wire1.read(); // receive byte as a character
    if (readStringC[i] == ','){
      containsComma = true;
    }
  }
  readStringC[i] = '\0'; // assume it is a string, add zero-terminator
  flagRx = true; // set flag, we received something.
}

void loop() {
  limitSwitchOne_state = digitalRead(SWITCH_ONE_PIN);

  if (isInitiatingHoming == true) {
    if (limitSwitchOne_state == HIGH){
      switchPressed = true;
    }

    if (switchPressed == true) {
      if (offsetTriggered == false){
        stepperA -> stopMove();
        stepperA -> setCurrentPosition(0);

        stepperA -> moveTo((-homeOffset * gearReduction) * formula);
        offsetTriggered = true;
      }else{
        if (stepperA -> isRunning() == false){
          ESP.restart(); //Don't bother with fancy code. Restart the damn thing!
        }
      }
    }else{
      stepperA -> setSpeedInUs(homingSpeed); // the parameter is us/step !!!
      stepperA -> setAcceleration(homingAccel);
      stepperA -> applySpeedAcceleration();
      stepperA -> runForward();
    }
  } else {
    getEncoderReading();

    if (flagRx == true) { //Verify that the variable contains information
      
      if (readStringC[0] == 'm') {      // Manual user data feed
        readStringC[0] = '0';
        bool isNegative = false;
        float moveAngle;
        float moveSpeed;

        if (readStringC[1] == '-') {
          readStringC[1] = '0';
          isNegative = true;
        }

        if (containsComma == true){
          char *moveStrings[6]; // an array of pointers to the pieces of the above array after strtok()
          char *ptr = NULL;
          int moveIndex = 0;

          ptr = strtok(readStringC, ",");  // delimiter
          while (ptr != NULL) {
            moveStrings[moveIndex] = ptr;
            moveIndex++;
            ptr = strtok(NULL, ",");
          }
          moveAngle = atof(moveStrings[0]);
          moveSpeed = atof(moveStrings[1]);
        }else{
          moveAngle = atof(readStringC);
          moveSpeed = openSpeed;
        }

        //Check for negative numbers
        if (isNegative == true){
          Raw_Input = moveAngle; // here input data is stored in float form
          User_Input = -Raw_Input;
        } else {
          User_Input = moveAngle; // here input data is stored in float form
        }

        if (isContinuousRotation == false) {
          float Abs_Input = abs(User_Input);
          User_Input = -Abs_Input;
        }
        //Apply Gear Reduction and move angle with speed
        User_Input = User_Input * gearReduction;
        stepperOpenLoop(User_Input, moveSpeed);

      } else if (readStringC[0] == 's') {
        readStringC[0] = '0';
        openSpeed = atoi(readStringC);
        
      //} else if (readStringC[0] == 'c') {
      //  readStringC[0] = '0';
      //  if (atoi(readStringC) <= 1200) {
      //    driverCurrent = atoi(readStringC);
      //    driver.rms_current(driverCurrent);
      //  }

      } else if (readStringC[0] == 'h') {
        isInitiatingHoming = true;
      }
      flagRx = false;
    }

    if (stepperA -> isRunning() == false) {
      openLoopRunning = false;
    }

    if (openLoopRunning == false) {
      if (closedLoopRunning == true) {
        setpoint = User_Input; //PID while work to achive this value consider as SET value
        input = totalAngle; // data from encoder consider as a Process value
        myPID.Compute(); // calculate new output
        stepperClosedLoop(output);
      }
    }
  }
}
