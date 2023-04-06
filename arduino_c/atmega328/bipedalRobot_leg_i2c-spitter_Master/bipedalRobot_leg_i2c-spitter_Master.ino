// Arduino Serial Example #2 Remote Control Blink - Master
#include <Wire.h>

#define i2cAddress 0x64 // 0x61 for Right leg, 0x62 for Left leg, 0x64 for both knees
bool flagRx = false;

const unsigned int MAX_MESSAGE_LENGTH = 125;
static char message[MAX_MESSAGE_LENGTH];
bool requestCommand = false;
bool messageReceived = false;

void setup()
{
  Wire.begin(i2cAddress);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  Serial.begin(115200);
  // wait for the serial port to connect. Required for Leonardo/Micro native USB port only
  while (!Serial) {  ;  }
}

void receiveEvent(int byteCount) {
  int i;
  requestCommand = false;
  for (i = 0; i < byteCount; i++) {
    char c = Wire.read();
    if (c == '@'){ requestCommand = true; }
    Serial.print(c);
  }
  Serial.println("\0");
}

void requestEvent() {
  if (messageReceived == true){
    Wire.write(message);
    messageReceived = false;
  }
}

void loop() {
  if (requestCommand == true){
    while (Serial.available() > 0)
    {
      //static char message[MAX_MESSAGE_LENGTH];
      static unsigned int message_pos = 0;

      char inByte = Serial.read();
      if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) )
      {
        message[message_pos] = inByte;
        message_pos++;
      }
      //Full message received...
      else
      {
        message[message_pos] = '\0';
        message_pos = 0;
        messageReceived = true;
        requestCommand = false;
      }
    }    
  }
}
