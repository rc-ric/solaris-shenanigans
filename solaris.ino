/* RIC Solaris - 6-Wheel Hexapod with Robotic Arm
   Hardware: 6x DC Motors + 1x BTS7960 + 4x 360° Servos + 6x MG996R Arm + 2x MG996R Camera
   Power: 12V LiPo + 2S 7.4V LiPo + XL4016 Buck Converter
   Control: FlySky FS-i10A Receiver (8-channel PPM)
*/

#include <Servo.h>

// ================== CONFIGURATION ==================
#define PPM_PIN 2
#define PPM_TIMEOUT 500000  // 500ms - loss of signal timeout
#define DEADZONE 50

// Channel mapping (FlySky FS-i10A)
#define CH_ROLL 0          // Steering
#define CH_PITCH 1         // Forward/Backward
#define CH_THROTTLE 2      // Arm control / Spin speed
#define CH_RUDDER 3        // Claw control
#define CH_CAM_PAN 4       // Camera pan
#define CH_CAM_TILT 5      // Camera tilt
#define CH_MODE 6          // Drive/Arm mode switch
#define CH_TURN 7          // Turn-in-place mode

// Mode thresholds
#define MODE_DRIVE_THRESHOLD 1300
#define MODE_ARM_THRESHOLD 1700

// ================== SERVOS ==================
// 360° Continuous Rotation Servos (Wheel steering)
Servo wheelServo[4];  // servo1, servo2, servo3, servo4 (pins 22-25)

// MG996R Servos (180°) - Arm
Servo armAngle, armBoom, armRotate, armClaw, clawAngle;
Servo cameraPan, cameraTilt;

// ================== PPM INPUT ==================
volatile uint16_t ppmChannels[9];
volatile uint8_t ppmIndex = 0;
unsigned long lastPPMTime = 0;
unsigned long lastTime = 0;

// ================== MOTOR CONTROL (BTS7960) ==================
// BTS7960 requires PWM + IN pins for H-bridge control
#define BTS7960_ENABLE 3      // PWM pin for speed control
#define BTS7960_IN1 26        // Direction control (HIGH = forward)
#define BTS7960_IN2 27        // Direction control (LOW = backward)

// ================== HELPER FUNCTIONS ==================

int applyDeadzone(int value, int center = 1500, int deadzone = DEADZONE) {
  if (abs(value - center) < deadzone) return center;
  return value;
}

int constrainPPM(int value) {
  return constrain(value, 1000, 2000);
}

void stopMotors() {
  digitalWrite(BTS7960_IN1, LOW);
  digitalWrite(BTS7960_IN2, LOW);
  analogWrite(BTS7960_ENABLE, 0);
}

void motorForward(int speed) {
  speed = constrain(speed, 0, 255);
  digitalWrite(BTS7960_IN1, HIGH);
  digitalWrite(BTS7960_IN2, LOW);
  analogWrite(BTS7960_ENABLE, speed);
}

void motorBackward(int speed) {
  speed = constrain(speed, 0, 255);
  digitalWrite(BTS7960_IN1, LOW);
  digitalWrite(BTS7960_IN2, HIGH);
  analogWrite(BTS7960_ENABLE, speed);
}

// ================== PPM ISR ==================
void ppmISR() {
  unsigned long now = micros();
  unsigned long duration = now - lastTime;
  lastTime = now;

  if (duration > 3000) {
    ppmIndex = 0;
    lastPPMTime = now;
  }
  else if (ppmIndex < 8) {
    ppmChannels[ppmIndex] = constrainPPM(duration);
    ppmIndex++;
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  delay(100);
  
  Serial.println("\n=== RIC Solaris (6-Wheel Hexapod) ===");
  Serial.println("Hardware: 6x DC Motors + BTS7960 + Arm + Camera");

  // PPM Input
  pinMode(PPM_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PPM_PIN), ppmISR, RISING);

  // BTS7960 Motor Driver Pins
  pinMode(BTS7960_IN1, OUTPUT);
  pinMode(BTS7960_IN2, OUTPUT);
  pinMode(BTS7960_ENABLE, OUTPUT);
  stopMotors();

  // 360° Continuous Rotation Servos (Wheels)
  wheelServo[0].attach(22);  // servo1
  wheelServo[1].attach(23);  // servo2
  wheelServo[2].attach(24);  // servo3
  wheelServo[3].attach(25);  // servo4
  for (int i = 0; i < 4; i++) {
    wheelServo[i].writeMicroseconds(1500);  // Center/stop
  }

  // MG996R Arm Servos (180°)
  armAngle.attach(32, 500, 2500);
  armBoom.attach(33, 500, 2500);
  armRotate.attach(34, 500, 2500);
  armClaw.attach(40, 500, 2500);
  clawAngle.attach(35, 500, 2500);

  // Camera Servos (MG996R)
  cameraPan.attach(30, 500, 2500);    // pan
  cameraTilt.attach(31, 500, 2500);   // tilt

  delay(1000);

  // Initialize all servos to neutral
  armAngle.write(90);
  armBoom.write(90);
  armRotate.write(90);
  armClaw.write(90);
  clawAngle.write(90);
  cameraPan.write(90);
  cameraTilt.write(90);

  delay(1000);
  Serial.println("✓ Ready");
  Serial.println("=====================================\n");
}

// ================== LOOP ==================
void loop() {
  // Safety: Check PPM signal timeout
  if (micros() - lastPPMTime > PPM_TIMEOUT) {
    stopMotors();
    for (int i = 0; i < 4; i++) wheelServo[i].writeMicroseconds(1500);
    Serial.println("⚠ PPM Signal Lost!");
    delay(100);
    return;
  }

  int mode = applyDeadzone(ppmChannels[CH_MODE], 1500, 100);
  int turnMode = ppmChannels[CH_TURN];

  if (mode < MODE_DRIVE_THRESHOLD) {
    driveModeControl(turnMode);
  }
  else if (mode > MODE_ARM_THRESHOLD) {
    armModeControl();
  }
  else {
    stopMotors();
    for (int i = 0; i < 4; i++) wheelServo[i].writeMicroseconds(1500);
  }
}

// ================== DRIVE MODE ==================
void driveModeControl(int turnMode) {
  int pitch = applyDeadzone(ppmChannels[CH_PITCH]);
  int roll = applyDeadzone(ppmChannels[CH_ROLL]);
  int cameraPan_val = ppmChannels[CH_CAM_PAN];
  int cameraTilt_val = ppmChannels[CH_CAM_TILT];

  // Camera control
  cameraPan.write(map(cameraPan_val, 1000, 2000, 0, 180));
  cameraTilt.write(map(cameraTilt_val, 1000, 2000, 0, 180));

  if (turnMode < 1300) {
    // -------- NORMAL DRIVE MODE --------
    // Forward/Backward
    if (pitch > 1600) {
      int spd = map(pitch, 1600, 2000, 0, 255);
      motorForward(spd);
    }
    else if (pitch < 1400) {
      int spd = map(pitch, 1400, 1000, 0, 255);
      motorBackward(spd);
    }
    else {
      stopMotors();
    }

    // Wheel steering (all 4 wheels synchronized)
    for (int i = 0; i < 4; i++) {
      wheelServo[i].writeMicroseconds(roll);  // Direct PPM pass-through
    }
  }
  else {
    // -------- TURN-IN-PLACE MODE --------
    // Set wheels to maximum angle for tight turns
    wheelServo[0].writeMicroseconds(2000);  // Full right
    wheelServo[1].writeMicroseconds(2000);
    wheelServo[2].writeMicroseconds(1000);  // Full left
    wheelServo[3].writeMicroseconds(1000);

    // Spin based on roll input
    int spinSpeed = roll;
    if (spinSpeed < 1350) {
      int spd = map(spinSpeed, 1350, 1000, 0, 255);
      motorBackward(spd);  // Spin one direction
    }
    else if (spinSpeed > 1650) {
      int spd = map(spinSpeed, 1650, 2000, 0, 255);
      motorForward(spd);   // Spin opposite direction
    }
    else {
      stopMotors();
    }
  }
}

// ================== ARM MODE ==================
void armModeControl() {
  stopMotors();
  for (int i = 0; i < 4; i++) wheelServo[i].writeMicroseconds(1500);  // Center wheels

  // Arm joint control (map PPM to servo angles)
  int angle_input = ppmChannels[CH_ROLL];
  int boom_input = ppmChannels[CH_PITCH];
  int rotate_input = ppmChannels[CH_THROTTLE];
  int claw_input = ppmChannels[CH_RUDDER];
  int clawang_input = ppmChannels[CH_CAM_PAN];

  armAngle.write(map(angle_input, 1000, 2000, 0, 180));
  armBoom.write(map(boom_input, 1000, 2000, 0, 180));
  armRotate.write(map(rotate_input, 1000, 2000, 0, 180));
  armClaw.write(map(claw_input, 1000, 2000, 0, 180));
  clawAngle.write(map(clawang_input, 1000, 2000, 0, 180));
}
