/* Coded by Tahsin Haque---
RIC Solaris
*/

// Arduino Mega using interrupt on pin 2 (INT4)
#include <Servo.h>
Servo servo1, servo2, servo3, servo4, pan, tilt;
volatile uint16_t ppmChannels[8];
volatile uint8_t ppmIndex = 0;
unsigned long lastTime = 0;

// Named channels
int throttle; 
int rudder;   
int roll;     
int pitch;    
int cameraPan;
int cameraTilt;


int LEN1 = 26;
int REN1 = 27;
int LEN2 = 28;
int REN2 = 29;


int PWML1 = 3;
int PWMR1 = 4;
int PWML2 = 5;
int PWMR2 = 6;



void ppmISR() {
  unsigned long now = micros();
  unsigned long duration = now - lastTime;
  lastTime = now;

  if (duration > 3000) {
    ppmIndex = 0; // Start of new frame
  } else if (ppmIndex < 8) {
    ppmChannels[ppmIndex] = duration;
    ppmIndex++;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(2, INPUT);             
  attachInterrupt(digitalPinToInterrupt(2), ppmISR, RISING);
  servo1.attach(22);
  servo2.attach(23);
  servo3.attach(24);
  servo4.attach(25);
  pan.attach(30);
  tilt.attach(31);


  pinMode(LEN1, OUTPUT);
  pinMode(REN1, OUTPUT);
  pinMode(LEN2, OUTPUT);
  pinMode(REN2, OUTPUT);

  pinMode(PWML1, OUTPUT);
  pinMode(PWMR1, OUTPUT);
  pinMode(PWML2, OUTPUT);
  pinMode(PWMR2, OUTPUT);



delay(1000);

  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  pan.write(90);
  tilt.write(90);

delay(1000); 
  Serial.println("PPM Decoder with Named Channels Started");
}

void loop() {

  // FS-i6X PPM channel order:
  // CH1 = Roll
  // CH2 = Pitch
  // CH3 = Throttle
  // CH4 = Rudder

  roll = ppmChannels[0];
  pitch = ppmChannels[1];
  throttle = ppmChannels[2];
  rudder = ppmChannels[3];
  cameraPan = ppmChannels[4];
  cameraTilt = ppmChannels[5];

  Serial.print("Roll: "); Serial.print(roll);
  Serial.print(" Pitch: "); Serial.print(pitch);
  Serial.print(" Throttle: "); Serial.print(throttle);
  Serial.println(" Rudder: "); Serial.println(rudder);


  digitalWrite(LEN1,1);
  digitalWrite(REN1,1);
  digitalWrite(LEN2,1);
  digitalWrite(REN2,1);

  pan.write(cameraPan);
  tilt.write(cameraTilt);

  if(pitch > 600){
    analogWrite(PWML1, map(pitch, 600, 995, 10, 255));
    analogWrite(PWMR1,0);
    analogWrite(PWML2, map(pitch, 600, 995, 10, 255));
    analogWrite(PWMR2,0);
  }
  else if(pitch < 400){
    analogWrite(PWML1,0);
    analogWrite(PWMR1,map(pitch, 400, 5, 10, 255));
    analogWrite(PWML2,0);
    analogWrite(PWMR2,map(pitch, 400, 5, 10, 255));
  }
  else{
    analogWrite(PWML1,0);
    analogWrite(PWMR1,0);
    analogWrite(PWML2,0);
    analogWrite(PWMR2,0);
  }
 

  if(roll > 550){
    servo1.write(map(roll, 550, 995, 90, 180));
    servo2.write(map(roll, 550, 995, 90, 180));
    servo3.write(map(roll, 550, 995, 90, 0));
    servo4.write(map(roll, 550, 995, 90, 0));
  }
  else if(roll < 450){
    servo1.write(map(roll, 450, 0, 90, 0));
    servo2.write(map(roll, 450, 0, 90, 0));
    servo3.write(map(roll, 450, 0, 90, 180));
    servo4.write(map(roll, 450, 0, 90, 180));
  }
  else{
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  }
  
}
