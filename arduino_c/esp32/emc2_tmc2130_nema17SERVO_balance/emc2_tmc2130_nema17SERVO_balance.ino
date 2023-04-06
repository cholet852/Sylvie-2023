#define EN_PIN 13
#define DIR_PIN 32 // 32 for 30p esp32, 33 for 38p esp32
#define STEP_PIN 33 // 33 for 30p esp32, 25 for 38p esp32
#define CS_PIN 5 //chip select
#define R_SENSE 0.11f // Match to your driver

#define SDA_PIN 27 // 27 for 30p esp32, 15 for 38p esp32
#define SCL_PIN 26 // 26 for 30p esp32, 16 for 38p esp32

#define SWITCH_ONE_PIN 14 // 14 for 30p esp32, 17 for 38p esp32
#define ANALOG_PIN 34 // Can use pin 35 or 34, but let's keep it at 34

#define I2C_SLAVE_ADDR 0x60 // 0x62 for thighs

#include <TMCStepper.h>
#include <FastAccelStepper.h> // Make sure you have the latest FastAccelstepper.h

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#include <Wire.h> // Make sure you have the latest ESP32 dev package: https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
#include <PID_v1.h>

// Seeed Studio AS5600.h library used for Encoder safety and error-checking.
// Make sure your i2c master pins (22 SCL, 21 SDA) are connected to the AS5600, in addition to the analog pin.
#include <AS5600.h>

TMC2130Stepper driver = TMC2130Stepper(CS_PIN, R_SENSE); // Hardware SPI

FastAccelStepperEngine engine = FastAccelStepperEngine();
FastAccelStepper *stepperA = NULL;

AMS_5600 ams5600;
Adafruit_MPU6050 emc2gyro;

// Mpu6050
long lvalue;
long solid_statepoint;

// Homing handlers
int limitSwitchOne_state = 0;

float homeOffset = 2.5; // Perfect alignment located at angle 115 (i.e. 6.5 for thighs). 2.5 is standard
bool switchPressed = false;
bool offsetTriggered = false;

bool isInitiatingHoming = false;
bool isContinuousRotation = false;

// Balance-related stuffs
int Raw_B = 0;
int Input_B = 0;

bool balanceModeOn = false;
bool balanceRunning = false;
double gyroOutput;

// Wire character read stuffs
float Raw_Input = 0;
float User_Input = 0; // This while convert input string into integer

boolean flagRx = false;
boolean containsComma = false;
boolean containsAt = false;
char readStringC[10]; //This while store the user input data

// PID stuffs
double kp = 6, ki = 5, kd = 0.1;
double kp2 = 24, ki2 = 8, kd2 = 0.01;

float newkp = 24, newki = 8, newkd = 0.01;

double input = 0, output = 0, setpoint = 0;
double input2 = 0, output2 = 0, setpoint2 = 0;

PID myPID( & input, & output, & setpoint, kp, ki, kd, DIRECT);
PID myPID2( & input2, &output2, &setpoint2, kp2, ki2, kd2, DIRECT);

// Motor and driver speed stuffs
long openSpeed = 40; // Less is more
long closedSpeed = openSpeed + 10;

// WARNING: USE SLOW VALUES WHEN FIRST TESTING AND SETTING UP! Make sure you have an on/off switch handy for main 36v
long balanceSpeed = 15; // 35 is a good speed for first test, then 8
long minBalanceSpeed = 45;
long balanceAccel = 100000;
long balanceCurrent = 700;
long cutoffCurrent = 450;

long openAccel = 100000;
long homingSpeed = 25;
long homingAccel = 1000000;

int enableThreshold = 1;
int gyroThreshold = 80;
int gyroCutoff = 20;

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

long driverCurrent = 1000;

void setup() {
  Serial.begin(115200);
  SPI.begin();

  Wire.begin();
  Wire.setClock(800000);
  bool success = Wire1.begin(I2C_SLAVE_ADDR, SDA_PIN, SCL_PIN, 400000);
  if (!success) {
    Serial.println("Error: I2C slave init failed");
    while (1) delay(100);
  } else {
    Serial.println("Success: I2C slave init!");
  }
  Wire1.onReceive(receiveEvent);

  myPID.SetMode(AUTOMATIC); //set PID in Auto mode
  myPID.SetSampleTime(2); // refresh rate of PID controller
  myPID.SetOutputLimits(-500, 500); // this is the MAX PWM value to move motor, here change in value reflect change in speed of motor.

  myPID2.SetMode(AUTOMATIC); //set PID in Auto mode
  myPID2.SetSampleTime(2); // refresh rate of PID controller
  myPID2.SetOutputLimits(-500, 500); // this is the MAX PWM value to move motor, here change in value reflect change in speed of motor.

  emc2gyro.begin();
  emc2gyro.setAccelerometerRange(MPU6050_RANGE_2_G);//2_G,4_G,8_G,16_G
  emc2gyro.setGyroRange(MPU6050_RANGE_250_DEG);//250,500,1000,2000
  emc2gyro.setFilterBandwidth(MPU6050_BAND_5_HZ);

  engine.init();
  stepperA = engine.stepperConnectToPin(STEP_PIN);
  if (stepperA) {
    stepperA -> setDirectionPin(DIR_PIN);
    stepperA -> setEnablePin(EN_PIN);
    stepperA -> setAutoEnable(true);

    stepperA -> setSpeedInUs(openSpeed); // the parameter is us/step !!!
    stepperA -> setAcceleration(openAccel);
  }

  pinMode(CS_PIN, OUTPUT);
  digitalWrite(CS_PIN, HIGH);
  driver.begin(); // Initiate pins and registeries
  driver.rms_current(driverCurrent); // Set stepper current to 600mA. The command is the same as command TMC2130.setCurrent(600, 0.11, 0.5);
  driver.microsteps(microstepping);
  driver.shaft(true); // Set this to True to change shaft direction without flipping cable. To change AS5600 encoder direction, short DIR to VCC & GPO

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
    Serial.println("Warning: Missed revolution. Details: ");
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
    //Serial.println(User_Input);
    //Serial.println(totalAngle);
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

void stepperBalance(int gyroOut, int bSpeed, int mbSpeed) {
  openLoopRunning = true;
  closedLoopRunning = false;
  closedLoopStart = false;

  if (abs(gyroOut) > gyroCutoff) {
    balanceRunning = true;
    if (gyroOut > 0) {
      int newSpeed = map(abs(gyroOut), 0, 500, mbSpeed, bSpeed);
      stepperA -> setSpeedInUs(newSpeed);
      stepperA -> setAcceleration(balanceAccel);
      stepperA -> applySpeedAcceleration();
      stepperA -> runBackward();
    } else if (gyroOut < 0) {
      int newSpeed = map(abs(gyroOut), 0, 500, mbSpeed, bSpeed);
      stepperA -> setSpeedInUs(newSpeed);
      stepperA -> setAcceleration(balanceAccel);
      stepperA -> applySpeedAcceleration();
      stepperA -> runForward();
    }
  }else{
    if (balanceRunning == true){
      stepperA -> stopMove();
      balanceRunning = false;
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
  containsAt = false;

  for (i = 0; i < byteCount; i++) {
    readStringC[i] = Wire1.read(); // receive byte as a character
    if (readStringC[i] == ','){
      containsComma = true;
    }
    else if (readStringC[i] == '@'){
      containsAt = true;
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

    sensors_event_t a, g, temp;
    emc2gyro.getEvent(&a, &g, &temp);

    float fvalue = a.acceleration.y; //a.acceleration.x for thighs, a.acceleration.y for waist
    lvalue = (fvalue * 10);
    lvalue = map(lvalue,  -90, 90, 180, 0);

    //Serial.println(lvalue);

    if (flagRx == true) { //Verify that the variable contains information
      if (readStringC[0] == 'b') {
        balanceModeOn = true;
        solid_statepoint = lvalue;
        driver.rms_current(balanceCurrent);
      }

      else if (readStringC[0] == 'x') {
        balanceModeOn = false;
        stepperA -> stopMove();
        closedLoopRunning = true;
        closedLoopStart = true;
        driver.rms_current(driverCurrent);
        User_Input = totalAngle;
      }

      // Manual user data feed
      else if (readStringC[0] == 'm') {
        if (containsAt == true){
          // Handle master requests for current encoder position, etc.
          char reqMessage[64];
          float gearedAngle = totalAngle / gearReduction;
          snprintf(reqMessage, 64, "%f", gearedAngle);
          Wire1.slaveWrite((uint8_t *)reqMessage, strlen(reqMessage));
        }else{
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
        }
      } else if (readStringC[0] == 's') {
        readStringC[0] = '0';
        openSpeed = atoi(readStringC);
      } else if (readStringC[0] == 'l') {
        readStringC[0] = '0';
        balanceSpeed = atoi(readStringC);
      } else if (readStringC[0] == 'n') {
        readStringC[0] = '0';
        minBalanceSpeed = atoi(readStringC);
      } else if (readStringC[0] == 'c') {
        readStringC[0] = '0';
        if (atoi(readStringC) <= 1200) {
          driverCurrent = atoi(readStringC);
          driver.rms_current(driverCurrent);
        }

        // For PID tuning on the fly
      } else if (readStringC[0] == 'p') {
        readStringC[0] = '0';
        newkp = atof(readStringC);
        myPID2.SetTunings(newkp, newki, newkd);
      } else if (readStringC[0] == 'i') {
        readStringC[0] = '0';
        newki = atof(readStringC);
        myPID2.SetTunings(newkp, newki, newkd);
      } else if (readStringC[0] == 'd') {
        readStringC[0] = '0';
        newkd = atof(readStringC);
        myPID2.SetTunings(newkp, newki, newkd);
      } else if (readStringC[0] == 'h') {
        isInitiatingHoming = true;
      }
      flagRx = false;
    }

    if (stepperA -> isRunning() == false) {
      openLoopRunning = false;
    }

    if (balanceModeOn == true) {
      setpoint2 = solid_statepoint; //PID while work to achive this value consider as SET value
      input2 = lvalue; // data from encoder consider as a Process value
      myPID2.Compute(); // calculate new output

      int gyro2out = output2;
      if(abs(gyro2out) < gyroThreshold){
        int newBalanceSpeed = balanceSpeed * 10;
        balanceAccel = 50000;
        int newMinBalanceSpeed = minBalanceSpeed * 10;
        driver.rms_current(cutoffCurrent);
        stepperBalance(output2, newBalanceSpeed, newMinBalanceSpeed);
      }else{
        balanceAccel = 600000;
        driver.rms_current(balanceCurrent);
        stepperBalance(output2, balanceSpeed, minBalanceSpeed);
      }
    }

    if (openLoopRunning == false) {
      if (closedLoopRunning == true) {
        setpoint = User_Input; //PID while work to achive this value consider as SET value
        //Serial.println(User_Input);
        input = totalAngle; // data from encoder consider as a Process value
        myPID.Compute(); // calculate new output
        stepperClosedLoop(output);
      }
    }
  }
}
