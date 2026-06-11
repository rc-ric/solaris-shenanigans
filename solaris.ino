/* Coded with ai by tahsin.---
   RIC Solaris
   FlySky PPM Mode Switching Fixed
*/

#include <Servo.h>

// ---------------- SERVOS ----------------
Servo servo1, servo2, servo3, servo4, pan, tilt, angle, boom, rota, claw, clawang;

// ---------------- PPM ----------------
volatile uint16_t ppmChannels[9];
volatile uint8_t ppmIndex = 0;
unsigned long lastTime = 0;

int turn;
// ---------------- CHANNELS ----------------
int throttle, rudder, roll, pitch;
int cameraPan, cameraTilt;
int mode;

// ---------------- MOTOR PINS ----------------
int LEN1 = 26;
int REN1 = 27;
int LEN2 = 28;
int REN2 = 29;

int PWML1 = 3;
int PWMR1 = 4;
int PWML2 = 5;
int PWMR2 = 6;

// ---------------- PPM ISR ----------------
void ppmISR() {
  unsigned long now = micros();
  unsigned long duration = now - lastTime;
  lastTime = now;

  if (duration > 3000) {
    ppmIndex = 0;
  }
  else if (ppmIndex < 8) {
    ppmChannels[ppmIndex] = duration;
    ppmIndex++;
  }
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);

  pinMode(2, INPUT);
  attachInterrupt(digitalPinToInterrupt(2), ppmISR, RISING);

  servo1.attach(22, 500, 2500);
  servo2.attach(23, 500, 2500);
  servo3.attach(24, 500, 2500);
  servo4.attach(25, 500, 2500);

  pan.attach(30);
  tilt.attach(31);
  angle.attach(32, 500, 2500);
  boom.attach(33, 500, 2500);
  rota.attach(34);
  clawang.attach(35);
  claw.attach(40);

  //Wheels Connections
  pinMode(LEN1, OUTPUT);
  pinMode(REN1, OUTPUT);
  pinMode(LEN2, OUTPUT);
  pinMode(REN2, OUTPUT);

  pinMode(PWML1, OUTPUT);
  pinMode(PWMR1, OUTPUT);
  pinMode(PWML2, OUTPUT);
  pinMode(PWMR2, OUTPUT);

  delay(1000);

  //Starting Angles
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  pan.write(90);
  tilt.write(90);

  delay(1000);
  Serial.println("PPM Decoder with MODE Switching Started");
}

// ---------------- LOOP ----------------
void loop() {

  mode = ppmChannels[6];   // CH7 (FlySky switch)
  turn = ppmChannels[7];
  // ================= DRIVE MODE =================
  if (mode < 1300) {

    roll       = ppmChannels[0];
    pitch      = ppmChannels[1];
    throttle   = ppmChannels[2];
    rudder     = ppmChannels[3];
    cameraPan  = ppmChannels[4];
    cameraTilt = ppmChannels[5];

    digitalWrite(LEN1, HIGH);
    digitalWrite(REN1, HIGH);
    digitalWrite(LEN2, HIGH);
    digitalWrite(REN2, HIGH);

    pan.write(map(cameraPan, 1000, 2000, 0, 180));
    tilt.write(map(cameraTilt, 1000, 2000, 0, 180));
    rota.write(map(ppmChannels[2], 1000, 2000, 0, 180));
    claw.write(map(ppmChannels[3], 1000, 2000, 0, 180));
        if(turn < 1300){
    // -------- FORWARD / BACKWARD --------
    if (pitch > 1600) {
      int spd = map(pitch, 1600, 2000, 0, 255);
      analogWrite(PWML1, spd);
      analogWrite(PWML2, spd);
      analogWrite(PWMR1, 0);
      analogWrite(PWMR2, 0);
    }
    else if (pitch < 1400) {
      int spd = map(pitch, 1400, 1000, 0, 255);
      analogWrite(PWMR1, spd);
      analogWrite(PWMR2, spd);
      analogWrite(PWML1, 0);
      analogWrite(PWML2, 0);
    }
    else {
      analogWrite(PWML1, 0);
      analogWrite(PWMR1, 0);
      analogWrite(PWML2, 0);
      analogWrite(PWMR2, 0);
    }

    // -------- STEERING --------
    if (roll > 1550) {
      servo1.write(map(roll, 1550, 2000, 90, 180));
      servo2.write(map(roll, 1550, 2000, 90, 180));
      servo3.write(map(roll, 1550, 2000, 90, 0));
      servo4.write(map(roll, 1550, 2000, 90, 0));
    }
    else if (roll < 1450) {
      servo1.write(map(roll, 1450, 1000, 90, 0));
      servo2.write(map(roll, 1450, 1000, 90, 0));
      servo3.write(map(roll, 1450, 1000, 90, 180));
      servo4.write(map(roll, 1450, 1000, 90, 180));
    }
    else {
      servo1.write(90);
      servo2.write(90);
      servo3.write(90);
      servo4.write(90);
    }
  }
  else{
      servo1.write(135);
      servo2.write(35);
      servo3.write(45);
      servo4.write(135);
      int spd = roll;
      if(spd < 1350){
        int spdb = map(spd, 1350, 0, 0, 255);

      analogWrite(PWML1, 0);
      analogWrite(PWML2, spdb);
      analogWrite(PWMR1, spdb);
      analogWrite(PWMR2, 0);
      }
      else if(spd > 1650){
        int spdb = map(spd, 1650, 2000, 0, 255);
      analogWrite(PWML1, spdb);
      analogWrite(PWML2, 0);
      analogWrite(PWMR1, 0);
      analogWrite(PWMR2, spdb);
      }
      else{
      analogWrite(PWML1, 0);
      analogWrite(PWML2, 0);
      analogWrite(PWMR1, 0);
      analogWrite(PWMR2, 0);
      }
  }
  }

  // ================= ARM MODE =================
  else if (mode > 1700) {

    angle.write(map(ppmChannels[0], 1000, 2000, 0, 180));
    boom.write(map(ppmChannels[1], 1000, 2000, 0, 180));
    rota.write(map(ppmChannels[2], 1000, 2000, 0, 180));
    claw.write(map(ppmChannels[3], 1000, 2000, 0, 180));
    clawang.write(map(ppmChannels[4], 1000, 2000, 0, 180));
  }
}
