#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

#define SERVOMIN  125 //  minimum pulse length count (out of 4096)
#define SERVOMAX  600 // maximum pulse length count (out of 4096)

uint8_t sg90_1 = 0; // EyeR Y
uint8_t sg90_2 = 1; // EyeL Y
uint8_t sg90_3 = 2; // EyeR X
uint8_t sg90_4 = 3; // EyeL X

uint8_t sg90_1b = 4;
uint8_t sg90_2b = 5;
uint8_t sg90_3b = 6;
uint8_t sg90_4b = 7;

uint8_t sg90_brow1 = 8;
uint8_t sg90_brow2 = 9;

uint8_t sg90_lip = 10;
uint8_t sg90_corner1 = 11;
uint8_t sg90_corner2 = 15;

uint8_t sg90_jaw = 13;
uint8_t sg90_jaw2 = 14;

int offset_lid_top_right = -7;
int offset_lid_top_left = 29;

int offset_lid_bottom_right = -10;
int offset_lid_bottom_left = 18;

int offset_right_y = 0;
int offset_right_x = 20;

int offset_left_y = 5;
int offset_left_x = -30;

int offset_brow1 = -25;
int offset_brow2 = 15;

int offset_lip = -22;
int offset_corner1 = -14;
int offset_corner2 = 14;

int offset_jaw = 10;
int offset_jaw2 = 10;

void setup() {
  pwm.begin();
  pwm.setPWMFreq(60);  // Analog servos run at ~60 Hz updates

  pwm.setPWM(sg90_1, 0, angleToPulse(90 + offset_right_y));
  pwm.setPWM(sg90_2, 0, angleToPulse(90 + offset_left_y));

  pwm.setPWM(sg90_3, 0, angleToPulse(90 + offset_right_x));
  pwm.setPWM(sg90_4, 0, angleToPulse(90 + offset_left_x));

  pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
  pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));

  pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
  pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));

  pwm.setPWM(sg90_brow1, 0, angleToPulse(90 + offset_brow1));
  pwm.setPWM(sg90_brow2, 0, angleToPulse(90 + offset_brow2));

  pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));
  pwm.setPWM(sg90_corner1, 0, angleToPulse(90 + offset_corner1));
  pwm.setPWM(sg90_corner2, 0, angleToPulse(90 + offset_corner2));

  pwm.setPWM(sg90_jaw, 0, angleToPulse(90 + offset_jaw));
  pwm.setPWM(sg90_jaw2, 0, angleToPulse(90 + offset_jaw2));

  Serial.begin(9600);
  Serial.setTimeout(10);

  // Start the I2C Bus as Master
  Wire.begin();
}

int angleToPulse(int ang) {
  int pulse = map(ang, 0, 180, SERVOMIN, SERVOMAX); // map angle of 0 to 180 to Servo min and Servo max
  return pulse;
}

long servoState = 0;
int  mouthState = 0;

void loop() {
  while (Serial.available() == 0) {}
  servoState = Serial.parseInt();

  Serial.print("Received command: ");
  Serial.println(servoState);
  
  if (servoState > 0) {

    if (servoState == 2) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(60 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(120 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(100 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(80 + offset_lid_bottom_left));
    }
    else if (servoState == 3) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));
    }
    else if (servoState == 4) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(80 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(100 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(100 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(80 + offset_lid_bottom_left));
    }
    else if (servoState == 5) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));
    }
    else if (servoState == 6) {
      pwm.setPWM(sg90_1, 0, angleToPulse(100 + offset_right_y));
      pwm.setPWM(sg90_2, 0, angleToPulse(80 + offset_left_y));

      pwm.setPWM(sg90_1b, 0, angleToPulse(95 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(85 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(100 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(80 + offset_lid_bottom_left));
    }
    else if (servoState == 7) {
      pwm.setPWM(sg90_1, 0, angleToPulse(75 + offset_right_y));
      pwm.setPWM(sg90_2, 0, angleToPulse(100 + offset_left_y));

      pwm.setPWM(sg90_1b, 0, angleToPulse(80 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(100 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(75 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(105 + offset_lid_bottom_left));
    }
    else if (servoState == 8) {
      pwm.setPWM(sg90_3, 0, angleToPulse(75 + offset_right_x));
      pwm.setPWM(sg90_4, 0, angleToPulse(110 + offset_left_x));
    }
    else if (servoState == 9) {
      pwm.setPWM(sg90_3, 0, angleToPulse(110 + offset_right_x));
      pwm.setPWM(sg90_4, 0, angleToPulse(75 + offset_left_x));
    }
    else if (servoState == 11) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(60 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(100 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));
    }
    else if (servoState == 12) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(120 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(80 + offset_lid_bottom_left));
    }
    else if (servoState == 13) {
      Wire.beginTransmission(0x10);
      Wire.write(54);
      Wire.endTransmission();
    }
    else if (servoState == 14) {
      Wire.beginTransmission(0x10);
      Wire.write(55);
      Wire.endTransmission();
    }
    else if (servoState == 15) {
      Wire.beginTransmission(0x20);
      Wire.write(50);
      Wire.endTransmission();
    }
    else if (servoState == 16) {
      Wire.beginTransmission(0x20);
      Wire.write(51);
      Wire.endTransmission();
    }
    else if (servoState == 17) {
      pwm.setPWM(sg90_brow1, 0, angleToPulse(75 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(105 + offset_brow2));
    }
    else if (servoState == 18) {
      pwm.setPWM(sg90_brow1, 0, angleToPulse(105 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(75 + offset_brow2));
    }
    else if (servoState == 19) {
      pwm.setPWM(sg90_lip, 0, angleToPulse(110 + offset_lip));
    }
    else if (servoState == 20) {
      pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));
    }
    else if (servoState == 21) {
      pwm.setPWM(sg90_corner1, 0, angleToPulse(60 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(120 + offset_corner2));
      pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));

      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));  

      pwm.setPWM(sg90_brow1, 0, angleToPulse(80 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(100 + offset_brow2));  
                           
      pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));

      pwm.setPWM(sg90_jaw, 0, angleToPulse(95 + offset_jaw));
      pwm.setPWM(sg90_jaw2, 0, angleToPulse(85 + offset_jaw2));      
    }
    else if (servoState == 22) {
      pwm.setPWM(sg90_corner1, 0, angleToPulse(110 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(70 + offset_corner2)); 
      delay(30);
      pwm.setPWM(sg90_corner1, 0, angleToPulse(130 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(50 + offset_corner2)); 
      delay(30);
      pwm.setPWM(sg90_corner1, 0, angleToPulse(150 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(30 + offset_corner2)); 

      pwm.setPWM(sg90_1b, 0, angleToPulse(85 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(95 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(105 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(75 + offset_lid_bottom_left));      

      pwm.setPWM(sg90_brow1, 0, angleToPulse(100 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(80 + offset_brow2));  

      pwm.setPWM(sg90_lip, 0, angleToPulse(100 + offset_lip)); 

      pwm.setPWM(sg90_jaw, 0, angleToPulse(97 + offset_jaw));
      pwm.setPWM(sg90_jaw2, 0, angleToPulse(82 + offset_jaw2));                        
    }
    else if (servoState == 23) {
      pwm.setPWM(sg90_jaw, 0, angleToPulse(100 + offset_jaw));
      pwm.setPWM(sg90_jaw2, 0, angleToPulse(80 + offset_jaw2));
      delay(30);
      pwm.setPWM(sg90_jaw, 0, angleToPulse(105 + offset_jaw));
      pwm.setPWM(sg90_jaw2, 0, angleToPulse(75 + offset_jaw2));    
      delay(30);
      pwm.setPWM(sg90_jaw, 0, angleToPulse(110 + offset_jaw));
      pwm.setPWM(sg90_jaw2, 0, angleToPulse(70 + offset_jaw2));      
      delay(50);
      pwm.setPWM(sg90_corner1, 0, angleToPulse(80 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(100 + offset_corner2)); 
      delay(50);
      pwm.setPWM(sg90_brow1, 0, angleToPulse(100 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(80 + offset_brow2));
      delay(50);
      pwm.setPWM(sg90_lip, 0, angleToPulse(95 + offset_lip));

      mouthState = 2;
    }
    else if (servoState == 24) {
      if (mouthState == 2) {
        pwm.setPWM(sg90_jaw, 0, angleToPulse(108 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(72 + offset_jaw2));      
        delay(30);
        pwm.setPWM(sg90_jaw, 0, angleToPulse(105 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(75 + offset_jaw2));      
        delay(30);        
        pwm.setPWM(sg90_jaw, 0, angleToPulse(100 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(80 + offset_jaw2));             
      }
      else {
        pwm.setPWM(sg90_jaw, 0, angleToPulse(95 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(85 + offset_jaw2)); 
        delay(30);        
        pwm.setPWM(sg90_jaw, 0, angleToPulse(100 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(80 + offset_jaw2));     
      }
      
      pwm.setPWM(sg90_corner1, 0, angleToPulse(90 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(90 + offset_corner2)); 
      
      pwm.setPWM(sg90_brow1, 0, angleToPulse(90 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(90 + offset_brow2));

      pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));     
      
      mouthState = 1;            
    }
    else if (servoState == 25) {
      delay(600);
      for (int i=0; i<3; i++) {
        for (int j=0; j<4; j++) {
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(100 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(80 + offset_jaw2));
    		  delay(30);
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(105 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(75 + offset_jaw2));    
    		  delay(30);
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(110 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(70 + offset_jaw2));      
    		  delay(50);
    		  pwm.setPWM(sg90_corner1, 0, angleToPulse(80 + offset_corner1));
    		  pwm.setPWM(sg90_corner2, 0, angleToPulse(100 + offset_corner2)); 
    		  delay(50);
    		  pwm.setPWM(sg90_brow1, 0, angleToPulse(100 + offset_brow1));
    		  pwm.setPWM(sg90_brow2, 0, angleToPulse(80 + offset_brow2));
    		  delay(50);
          pwm.setPWM(sg90_lip, 0, angleToPulse(95 + offset_lip));  
          delay(20);          
    		  pwm.setPWM(sg90_lip, 0, angleToPulse(105 + offset_lip));	

          delay(100);
          
    		  pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
    		  pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
    		  pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
    		  pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));
    
    		  pwm.setPWM(sg90_1, 0, angleToPulse(90 + offset_right_y));
    		  pwm.setPWM(sg90_2, 0, angleToPulse(90 + offset_left_y));
    		  pwm.setPWM(sg90_3, 0, angleToPulse(90 + offset_right_x));
    		  pwm.setPWM(sg90_4, 0, angleToPulse(90 + offset_left_x));
    
    		  pwm.setPWM(sg90_brow1, 0, angleToPulse(90 + offset_brow1));
    		  pwm.setPWM(sg90_brow2, 0, angleToPulse(90 + offset_brow2));
    
    		  pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));
    
    		  pwm.setPWM(sg90_corner1, 0, angleToPulse(90 + offset_corner1));
    		  pwm.setPWM(sg90_corner2, 0, angleToPulse(90 + offset_corner2));
    
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(100 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(80 + offset_jaw2));  
    		  delay(30);
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(95 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(85 + offset_jaw2));   
    		  delay(30);
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(92 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(88 + offset_jaw2));   
    		  delay(30);        
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(88 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(92 + offset_jaw2));   
    		  delay(30);          
    
    		  pwm.setPWM(sg90_jaw, 0, angleToPulse(90 + offset_jaw));
    		  pwm.setPWM(sg90_jaw2, 0, angleToPulse(90 + offset_jaw2));
  		  } 
        delay(350);
      }
      pwm.setPWM(sg90_1b, 0, angleToPulse(60 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(120 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(100 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(80 + offset_lid_bottom_left));      
      delay(100);
      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));             	
	  }
    else if (servoState == 26) {
      pwm.setPWM(sg90_1b, 0, angleToPulse(60 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(120 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(100 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(80 + offset_lid_bottom_left));      
      delay(100);
      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));    
    }
    else if (servoState == 10) {

      pwm.setPWM(sg90_1b, 0, angleToPulse(90 + offset_lid_top_right));
      pwm.setPWM(sg90_2b, 0, angleToPulse(90 + offset_lid_top_left));
      pwm.setPWM(sg90_3b, 0, angleToPulse(90 + offset_lid_bottom_right));
      pwm.setPWM(sg90_4b, 0, angleToPulse(90 + offset_lid_bottom_left));

      pwm.setPWM(sg90_1, 0, angleToPulse(90 + offset_right_y));
      pwm.setPWM(sg90_2, 0, angleToPulse(90 + offset_left_y));
      pwm.setPWM(sg90_3, 0, angleToPulse(90 + offset_right_x));
      pwm.setPWM(sg90_4, 0, angleToPulse(90 + offset_left_x));

      pwm.setPWM(sg90_brow1, 0, angleToPulse(90 + offset_brow1));
      pwm.setPWM(sg90_brow2, 0, angleToPulse(90 + offset_brow2));

      pwm.setPWM(sg90_lip, 0, angleToPulse(90 + offset_lip));

      pwm.setPWM(sg90_corner1, 0, angleToPulse(90 + offset_corner1));
      pwm.setPWM(sg90_corner2, 0, angleToPulse(90 + offset_corner2));

      if (mouthState == 1 || mouthState == 2) {
        pwm.setPWM(sg90_jaw, 0, angleToPulse(100 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(80 + offset_jaw2));  
        delay(30);
        pwm.setPWM(sg90_jaw, 0, angleToPulse(95 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(85 + offset_jaw2));   
        delay(30);
        pwm.setPWM(sg90_jaw, 0, angleToPulse(92 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(88 + offset_jaw2));   
        delay(30);        
        pwm.setPWM(sg90_jaw, 0, angleToPulse(88 + offset_jaw));
        pwm.setPWM(sg90_jaw2, 0, angleToPulse(92 + offset_jaw2));   
        delay(30);          
      }
      pwm.setPWM(sg90_jaw, 0, angleToPulse(90 + offset_jaw));
      pwm.setPWM(sg90_jaw2, 0, angleToPulse(90 + offset_jaw2));

      mouthState = 0;
    }
    Serial.begin(9600);
    Serial.setTimeout(10);
  }
}
