#include <Arduino.h>
#include <Wire.h>

#define I2C_SLAVE_ADDR_0 0x60
#define I2C_SLAVE_ADDR_1 0x61
#define I2C_SLAVE_ADDR_2 0x62
#define I2C_SLAVE_ADDR_3 0x61
#define I2C_SLAVE_ADDR_4 0x62
#define I2C_SLAVE_ADDR_5 0x61
#define I2C_SLAVE_ADDR_6 0x62
#define I2C_SLAVE_ADDR_7 0x61
#define I2C_SLAVE_ADDR_8 0x62
#define I2C_SLAVE_ADDR_9 0x61
#define I2C_SLAVE_ADDR_10 0x62

//const int sclPin = D1;
//const int sdaPin = D2;

const unsigned int MAX_MESSAGE_LENGTH = 125;

int currentSpeed = 10;
int currentAmps = 1000;
float currentAngle = 0;

float current_kp = 6.00;
float current_ki = 5.00;
float current_kd = 0.01;

bool containsSemicolon = false;
bool requestFlagOn = false;
byte requestedAddress;

char readWire[8];

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(10);

  Wire.setClock(400000);
  Wire.begin();
  //Wire.begin(sdaPin, sclPin);
}

void sendMessage(char message[], int message_pos) {
  //Add null character to string
  char *messageTrimmed = &message[1];
  bool useLocal = false;
  int msgStart = 0;

  if (message[0] == '0') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_0);
    useLocal = true;
    requestedAddress = I2C_SLAVE_ADDR_0;
  } else if (message[0] == '1') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_1);
    requestedAddress = I2C_SLAVE_ADDR_1;
  } else if (message[0] == '2') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_2);
    requestedAddress = I2C_SLAVE_ADDR_2;    
  } else if (message[0] == '3') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_3);
    requestedAddress = I2C_SLAVE_ADDR_3;    
  } else if (message[0] == '4') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_4);
    requestedAddress = I2C_SLAVE_ADDR_4;    
  } else if (message[0] == '5') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_5);
    requestedAddress = I2C_SLAVE_ADDR_5;    
  } else if (message[0] == '6') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_6);
    requestedAddress = I2C_SLAVE_ADDR_6;    
  } else if (message[0] == '7') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_7);
    requestedAddress = I2C_SLAVE_ADDR_7;    
  } else if (message[0] == '8') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_8);
    requestedAddress = I2C_SLAVE_ADDR_8;    
  } else if (message[0] == '9') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_9);
    requestedAddress = I2C_SLAVE_ADDR_9;    
  } else if (message[0] == 'a') {
    Wire.beginTransmission(I2C_SLAVE_ADDR_10);
    requestedAddress = I2C_SLAVE_ADDR_10;    
  }

  if (useLocal == true){
    msgStart = 1;
  }

  for (msgStart; msgStart < message_pos; msgStart++){
    Wire.write(message[msgStart]);
    if (message[msgStart] == '@') { requestFlagOn = true; }
  }
  
  Wire.endTransmission();

  //Print the message (or do other things)
  if (messageTrimmed[0] == 'm') {
    messageTrimmed[0] = '0';

    if (messageTrimmed[1] == '-') {
      messageTrimmed[1] = '0';
      float newAngle = atof(messageTrimmed);
      currentAngle = -newAngle;
    } else {
      currentAngle = atof(messageTrimmed);
    }
    Serial.print("New angle: ");
    Serial.println(currentAngle);
  }
  else if (messageTrimmed[0] == 's') {
    messageTrimmed[0] = '0';
    int newSpeed = atoi(messageTrimmed);
    currentSpeed = newSpeed;
    Serial.print("New speed: ");
    Serial.println(currentSpeed);
  }
  else if (messageTrimmed[0] == 'c') {
    messageTrimmed[0] = '0';
    if (atoi(messageTrimmed) <= 1200) {
      int newAmps = atoi(messageTrimmed);
      currentAmps = newAmps;
      Serial.print("New current: ");
      Serial.println(currentAmps);
    } else {
      Serial.println("Current too high!");
    }
  }

  // PID tuning on the fly
  else if (messageTrimmed[0] == 'p') {
    messageTrimmed[0] = '0';
    float new_kp = atof(messageTrimmed);
    current_kp = new_kp;
    Serial.print("New kP: ");
    Serial.println(current_kp);
  }
  else if (messageTrimmed[0] == 'i') {
    messageTrimmed[0] = '0';
    float new_ki = atof(messageTrimmed);
    current_ki = new_ki;
    Serial.print("New kI: ");
    Serial.println(current_ki);
  }
  else if (messageTrimmed[0] == 'd') {
    messageTrimmed[0] = '0';
    float new_kd = atof(messageTrimmed);
    current_kd = new_kd;
    Serial.print("New kD: ");
    Serial.println(current_kd);
  }
}

void loop() {
  if (requestFlagOn == true){
    delay(30);
    Wire.requestFrom(requestedAddress, 6, true);    // request 8 bytes from selected slave device

    int wireIndex = 0;
    while(Wire.available())    // slave may send less than requested
    {
      readWire[wireIndex] = Wire.read(); // receive a byte as character
      wireIndex++;
    }

    readWire[wireIndex] = '\0'; // assume it is a string, add zero-terminator
    Serial.print("Requested value: ");
    Serial.println(readWire);
    requestFlagOn = false;
  } else {
    //Check to see if anything is available in the serial receive buffer
    while (Serial.available() > 0)
    {
      //Create a place to hold the incoming message
      static char message[MAX_MESSAGE_LENGTH];
      static unsigned int message_pos = 0;

      //Read the next available byte in the serial receive buffer
      char inByte = Serial.read();

      //Message coming in (check not terminating character) and guard for over message size
      if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) )
      {
        //Add the incoming byte to our message
        message[message_pos] = inByte;
        if (message[message_pos] == ';'){
          containsSemicolon = true;
        }
        message_pos++;
      }
      //Full message received...
      else
      {
        message[message_pos] = '\0';
        if (containsSemicolon == true){
          char *moveStrings[6]; // an array of pointers to the pieces of the above array after strtok()
          char *ptr = NULL;
          int moveIndex = 0;

          ptr = strtok(message, ";");  // delimiter
          while (ptr != NULL) {
            moveStrings[moveIndex] = ptr;
            moveIndex++;
            ptr = strtok(NULL, ";");
          }
          int i;
          for (i = 0; i < moveIndex; i++) {
            int thisLength = strlen(moveStrings[i]) + 1;
            sendMessage(moveStrings[i], thisLength);
          }
        }else{
          sendMessage(message, message_pos);
        }
        //Reset for the next message
        message_pos = 0;
        containsSemicolon = false;
      }
    }
  }
}
