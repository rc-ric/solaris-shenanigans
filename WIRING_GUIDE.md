# RIC Solaris - Complete Pin Configuration & Wiring Guide

## 📍 Arduino Mega Pin Layout & Assignment

### Overview
- **Total Pins Used**: 22 GPIO pins + 1 Interrupt pin
- **Microcontroller**: Arduino Mega 2560 (70 GPIO pins available)
- **Available Pins for Extension**: 48 additional pins
- **Power Pins**: 5V, GND (multiple available)

---

## 🎛️ Pin Assignment Summary Table

| Function | Pin | Type | Voltage | Purpose |
|----------|-----|------|---------|---------|
| **PPM Input** | 2 | INPUT + INT | 5V | Receiver signal (interrupt) |
| **Motor EN** | 3 | PWM OUTPUT | 5V | BTS7960 speed control (0-255) |
| **Wheel Servo 1** | 22 | PWM OUTPUT | 5V | Left-Front wheel steering |
| **Wheel Servo 2** | 23 | PWM OUTPUT | 5V | Right-Front wheel steering |
| **Wheel Servo 3** | 24 | PWM OUTPUT | 5V | Left-Rear wheel steering |
| **Wheel Servo 4** | 25 | PWM OUTPUT | 5V | Right-Rear wheel steering |
| **Motor IN1** | 26 | DIGITAL OUT | 5V | BTS7960 forward direction |
| **Motor IN2** | 27 | DIGITAL OUT | 5V | BTS7960 reverse direction |
| **Arm Angle** | 32 | PWM OUTPUT | 5V | Base joint (0-180°) |
| **Arm Boom** | 33 | PWM OUTPUT | 5V | Boom extension (0-180°) |
| **Arm Rotate** | 34 | PWM OUTPUT | 5V | Base rotation (0-180°) |
| **Claw Wrist** | 35 | PWM OUTPUT | 5V | Wrist angle (0-180°) |
| **Camera Pan** | 30 | PWM OUTPUT | 5V | Camera horizontal pan |
| **Camera Tilt** | 31 | PWM OUTPUT | 5V | Camera vertical tilt |
| **Claw Gripper** | 40 | PWM OUTPUT | 5V | Gripper open/close (0-180°) |

---

## 🔌 Detailed Pin Breakdown

### INPUT PINS

#### Pin 2 - PPM Signal Input (INTERRUPT)
```
Function: Receive 8-channel RC signal from FlySky FS-i10A receiver
Type: INPUT with RISING edge interrupt (INT0)
Voltage: 5V logic level
Protocol: PPM (Pulse Position Modulation)
  - Frame Rate: 50Hz (20ms per frame)
  - Pulse Width: 1000-2000μs per channel
  - Sync Pulse: >3000μs (frame start)
Connection:
  Receiver PPM OUT → Arduino Pin 2
  Receiver GND → Arduino GND
  Receiver VCC → 5V (from servo power supply)
Notes:
  - Must use interrupt-capable pin (Mega: 2, 3, 18, 19, 20, 21)
  - Pin 2 chosen for optimal ISR performance
  - Voltage divider NOT needed (standard PPM is 5V)
```

---

### OUTPUT PINS - MOTOR CONTROL

#### Pin 3 - BTS7960 PWM (ENABLE)
```
Function: Motor speed control via PWM
Type: PWM OUTPUT (0-255)
Frequency: ~980Hz (Arduino default)
Current Capability: Can source 40mA (sufficient for TTL logic)
Connection:
  Arduino Pin 3 → BTS7960 ENABLE pin
  
Usage:
  analogWrite(3, speed);  // 0-255, where 255 = full power
  
Control Logic:
  - 0 = Motors off (freewheeling)
  - 128 = 50% power
  - 255 = 100% full power
```

#### Pin 26 - BTS7960 Direction Control (IN1)
```
Function: Motor forward/reverse direction selection
Type: DIGITAL OUTPUT (HIGH/LOW)
Voltage: 5V logic
Connection:
  Arduino Pin 26 → BTS7960 IN1 pin
  
H-Bridge Truth Table:
  IN1 | IN2 | Result
  HIGH| LOW | Forward (all 6 motors forward)
  LOW | HIGH| Reverse (all 6 motors backward)
  LOW | LOW | Stop (freewheeling)
  HIGH| HIGH| Brake (hard stop - not used)
  
Usage:
  digitalWrite(26, HIGH);   // Set forward direction
  digitalWrite(26, LOW);    // Disable forward
```

#### Pin 27 - BTS7960 Direction Control (IN2)
```
Function: Motor reverse direction selection
Type: DIGITAL OUTPUT (HIGH/LOW)
Voltage: 5V logic
Connection:
  Arduino Pin 27 → BTS7960 IN2 pin
  
H-Bridge Truth Table:
  IN1 | IN2 | Result
  HIGH| LOW | Forward
  LOW | HIGH| Reverse (all 6 motors backward)
  LOW | LOW | Stop
  HIGH| HIGH| Brake
  
Usage:
  digitalWrite(27, HIGH);   // Set reverse direction
  digitalWrite(27, LOW);    // Disable reverse
```

---

### OUTPUT PINS - WHEEL SERVOS (360° Continuous Rotation)

#### Pins 22-25 - Wheel Steering Servos
```
Hardware: 4x 360° Continuous Rotation Servos
Type: PWM OUTPUT (servo pulses, 1000-2000μs)
Voltage: 5V (powered from servo supply, NOT Arduino 5V)
Frequency: 50Hz servo frequency (controlled by Servo library)

Pin 22 - servo1 (Left-Front Wheel)
Pin 23 - servo2 (Right-Front Wheel)
Pin 24 - servo3 (Left-Rear Wheel)
Pin 25 - servo4 (Right-Rear Wheel)

Connection (per servo):
  Signal (white) → Arduino Pin (22-25)
  Power (red) → +5V (servo power supply)
  Ground (black) → GND (common with Arduino GND)

Control Values (PPM/Microseconds):
  1000μs → Maximum reverse rotation (CCW)
  1500μs → Stop/Neutral (no rotation)
  2000μs → Maximum forward rotation (CW)

Usage in Code:
  servo1.write(90);              // Neutral (90° maps to 1500μs)
  servo1.writeMicroseconds(1500); // Direct PPM value
  
Steering Mapping (Normal Drive Mode):
  - Roll < 1450μs → Turn Left (servo angle 90→0°)
  - Roll 1450-1550μs → Straight (servo angle 90°)
  - Roll > 1550μs → Turn Right (servo angle 90→180°)

Spin Mode Angles:
  servo1 = 135° (Left-Front)
  servo2 = 35°  (Right-Front)
  servo3 = 45°  (Left-Rear)
  servo4 = 135° (Right-Rear)
```

---

### OUTPUT PINS - ARM SERVOS (MG996R 180°)

#### Pin 32 - Arm Base Joint
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Base arm joint - controls arm height/elevation
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Arm minimum position
  - 90° = Neutral/Center
  - 180° = Arm maximum position

Connection:
  Signal (white) → Arduino Pin 32
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  armAngle.attach(32, 500, 2500);
  // 500μs = 0°, 2500μs = 180°

Control (in ARM MODE):
  armAngle.write(map(ppmChannels[CH_ROLL], 1000, 2000, 0, 180));
  // PPM 1000→0°, PPM 2000→180°

Torque: ~15kg/cm (sufficient for arm load)
Speed: ~0.2s/60° (relatively fast movement)
```

#### Pin 33 - Arm Boom Extension
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Boom joint - controls arm reach (in/out)
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Boom retracted (minimum reach)
  - 90° = Boom neutral
  - 180° = Boom extended (maximum reach)

Connection:
  Signal (white) → Arduino Pin 33
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  armBoom.attach(33, 500, 2500);

Control (in ARM MODE):
  armBoom.write(map(ppmChannels[CH_PITCH], 1000, 2000, 0, 180));

Notes:
  - Controls horizontal extension of arm
  - Requires opposite motion for counterbalance in some designs
  - Maximum load when fully extended
```

#### Pin 34 - Arm Rotation (Base Sweep)
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Base rotation - sweeps arm left/right
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Rotated fully left
  - 90° = Center position
  - 180° = Rotated fully right

Connection:
  Signal (white) → Arduino Pin 34
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  armRotate.attach(34, 500, 2500);

Control:
  IN DRIVE MODE: armRotate.write(map(ppmChannels[CH_THROTTLE], 1000, 2000, 0, 180));
  IN ARM MODE: armRotate.write(map(ppmChannels[CH_THROTTLE], 1000, 2000, 0, 180));

Application:
  - Rotates entire arm assembly around vertical axis
  - 180° total rotation = 90° left to 90° right
  - Useful for scanning/positioning arm
```

#### Pin 40 - Gripper/Claw
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Gripper mechanism - open/close claw
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Fully open (maximum grip opening)
  - 90° = Mid-position
  - 180° = Fully closed (maximum grip force)

Connection:
  Signal (white) → Arduino Pin 40
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  claw.attach(40, 500, 2500);

Control:
  IN DRIVE MODE: claw.write(map(ppmChannels[CH_RUDDER], 1000, 2000, 0, 180));
  IN ARM MODE: claw.write(map(ppmChannels[CH_RUDDER], 1000, 2000, 0, 180));

Mechanical Notes:
  - MG996R provides 15kg/cm torque (strong grip)
  - Requires linkage mechanism to convert rotation to linear motion
  - Typical construction: servo pulley + cable + gripper fingers
```

#### Pin 35 - Claw Wrist Angle
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Wrist/claw rotation - fine positioning
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Wrist fully rotated CCW
  - 90° = Neutral position
  - 180° = Wrist fully rotated CW

Connection:
  Signal (white) → Arduino Pin 35
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  clawAngle.attach(35, 500, 2500);

Control:
  IN DRIVE MODE: clawAngle.write(map(ppmChannels[CH_CAM_PAN], 1000, 2000, 0, 180));
  IN ARM MODE: clawAngle.write(map(ppmChannels[CH_CAM_PAN], 1000, 2000, 0, 180));

Application:
  - Rotates gripper/claw for object orientation
  - Allows picking up objects at different angles
  - Essential for precise object placement
```

---

### OUTPUT PINS - CAMERA SERVOS (MG996R 180°)

#### Pin 30 - Camera Pan (Horizontal)
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Camera horizontal pan - left/right movement
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Camera looking fully left
  - 90° = Camera looking straight ahead
  - 180° = Camera looking fully right

Connection:
  Signal (white) → Arduino Pin 30
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  pan.attach(30, 500, 2500);

Control:
  pan.write(map(ppmChannels[CH_CAM_PAN], 1000, 2000, 0, 180));

Usage:
  - Works in both DRIVE and ARM modes
  - Allows operator to scout left/right
  - 180° total sweep (90° left to 90° right from center)
```

#### Pin 31 - Camera Tilt (Vertical)
```
Hardware: 1x MG996R High-Torque Servo (180°)
Function: Camera vertical tilt - up/down movement
Type: PWM OUTPUT (servo control, 500-2500μs)
Voltage: 5V (from servo power supply)
Range: 0-180°
  - 0° = Camera pointing downward
  - 90° = Camera level/horizontal
  - 180° = Camera pointing upward

Connection:
  Signal (white) → Arduino Pin 31
  Power (red) → +5V (servo supply)
  Ground (black) → GND

Attachment Code:
  tilt.attach(31, 500, 2500);

Control:
  tilt.write(map(ppmChannels[CH_CAM_TILT], 1000, 2000, 0, 180));

Usage:
  - Works in both DRIVE and ARM modes
  - Allows operator to look up/down
  - Useful for obstacle detection and navigation
```

---

## 🔋 Power Distribution

### Power Pins (Arduino Mega)

#### 5V Power Pins (Multiple Available)
```
Location: Edge connector (multiple 5V pins)
Max Current: Limited by onboard regulator (~500mA total)
Usage:
  - PPM Receiver VCC (small current ~50mA)
  - Arduino logic reference
  
⚠️ WARNING: DO NOT power servos from Arduino 5V!
Servos draw too much current and will cause brownout/reset
```

#### GND (Ground Pins - Multiple Available)
```
Location: Edge connector (multiple GND pins)
Critical: ALL components must share common ground reference
Required connections:
  1. Arduino GND → Servo Power Supply GND
  2. Arduino GND → BTS7960 GND
  3. Arduino GND → PPM Receiver GND
  4. Arduino GND → Motor GND
Common ground is CRITICAL for proper signal levels
```

---

## 🔌 Power Supply Architecture

### Main Battery (12V LiPo)
```
12V LiPo Battery
    ↓
BTS7960 Motor Driver
    ↓
All 6 DC Motors (in parallel)

Max Current: ~15-20A under full throttle
```

### Servo Power Supply (7.4V LiPo with XL4016 Buck Converter)
```
2S 7.4V LiPo Battery
    ↓
XL4016 Buck Converter (step down)
    ↓
5V Output (regulated)
    ↓
All 11 Servos (in parallel)
    ├─ Wheel Servos (4x)
    ├─ Arm Servos (5x)
    └─ Camera Servos (2x)

Max Current: ~2-3A (all servos under load)
Capacitor: 100μF tantalum on output (recommended)
```

---

## 📋 Complete Wiring Checklist

### Step 1: Power & Ground
- [ ] 12V LiPo → BTS7960 positive input
- [ ] 2S 7.4V LiPo → XL4016 input
- [ ] XL4016 output (5V) → Servo power rail
- [ ] All GNDs connected together (common ground)
- [ ] Servo power GND → Arduino GND (critical!)

### Step 2: Motor Control
- [ ] BTS7960 ENABLE → Arduino Pin 3
- [ ] BTS7960 IN1 → Arduino Pin 26
- [ ] BTS7960 IN2 → Arduino Pin 27
- [ ] BTS7960 motor output → DC motors (parallel)
- [ ] All motor grounds → common GND

### Step 3: PPM Receiver
- [ ] Receiver PPM signal → Arduino Pin 2
- [ ] Receiver GND → Arduino GND
- [ ] Receiver VCC → 5V (from servo supply or Arduino 5V)

### Step 4: Wheel Servos
- [ ] Servo 1 signal → Arduino Pin 22
- [ ] Servo 2 signal → Arduino Pin 23
- [ ] Servo 3 signal → Arduino Pin 24
- [ ] Servo 4 signal → Arduino Pin 25
- [ ] All servo power → 5V rail
- [ ] All servo GND → common GND

### Step 5: Arm Servos
- [ ] Arm Angle signal → Arduino Pin 32
- [ ] Arm Boom signal → Arduino Pin 33
- [ ] Arm Rotate signal → Arduino Pin 34
- [ ] Gripper signal → Arduino Pin 40
- [ ] Wrist signal → Arduino Pin 35
- [ ] All servo power → 5V rail
- [ ] All servo GND → common GND

### Step 6: Camera Servos
- [ ] Camera Pan signal → Arduino Pin 30
- [ ] Camera Tilt signal → Arduino Pin 31
- [ ] All servo power → 5V rail
- [ ] All servo GND → common GND

### Step 7: Testing
- [ ] All servos center on startup
- [ ] PPM signal received (no timeout)
- [ ] Motors respond to throttle
- [ ] No brownout/reset issues
- [ ] All channels functional

---

## 🆘 Troubleshooting Pin Issues

### Arduino Not Detecting Receiver Signal
- [ ] Check Pin 2 connection (must be interrupt pin)
- [ ] Verify PPM signal voltage (should be 5V logic)
- [ ] Test with multimeter: should see 0-5V pulses
- [ ] Check Servo library isn't using Pin 2
- [ ] Restart Arduino after connection changes

### Servos Not Responding
- [ ] Verify 5V power on servo rail (should be 4.8-5.2V)
- [ ] Check signal pin connections (color-code: white=signal)
- [ ] Confirm servo power supply current capacity
- [ ] Test servo with separate power (verify servo works)
- [ ] Check for voltage sag when servos move

### Motors Won't Spin
- [ ] Test BTS7960 with multimeter (check logic levels)
- [ ] Verify 12V battery voltage (>11V)
- [ ] Check motor connections (not reversed)
- [ ] Test motor with direct battery connection
- [ ] Inspect pin soldering (cold joints possible)

### Random Resets/Brownouts
- [ ] Check 5V supply voltage (should be stable 5V)
- [ ] Verify all servos powered from separate supply
- [ ] Check for ground loop (confirm single GND reference)
- [ ] Reduce servo count when testing
- [ ] Add capacitors (100μF) on servo power rail

---

## 📊 Arduino Mega Pin Reference

```
MEGA 2560 Digital I/O Pin Map:

PORT B: Pins 10-13   (PWM capable: 11-13)
PORT C: Pins 30-37   (All I/O capable)
PORT D: Pins 38-45   (All I/O capable)
PORT E: Pins 2-5,6   (PWM capable: 2,3,5)
PORT F: Pins 54-61   (Analog inputs only on Mega)
PORT G: Pins 39,40,41  (All I/O capable)
PORT H: Pins 6-9,16-17 (PWM capable: 6-9)
PORT K: Pins 62-69   (Analog inputs only on Mega)
PORT L: Pins 42-49   (All I/O capable)

Interrupt-Capable Pins (Mega):
INT0 = Pin 2 ✓ (USED for PPM)
INT1 = Pin 3 (uses PWM)
INT2 = Pin 21
INT3 = Pin 20
INT4 = Pin 19
INT5 = Pin 18

PWM Pins Available on Mega: 2-13, 44-46
```

---

## 🎯 Arduino Mega Extension Pins (Available for Future Use)

### Unused Digital Pins (48 available)
```
Available Pins: 4-9, 10-21, 28-29, 36-39, 41-51, 52-53
Recommended for expansion:
  - Pins 4-9: General purpose I/O
  - Pins 52-53: SPI interface available
  - Pins 18-21: Future I2C/interrupt use
```

### Analog Pins (Available for Sensors)
```
Pins A0-A15 (Mega has 16 analog inputs)
Currently unused: All 16 available
Possible uses:
  - Battery voltage monitoring (A0)
  - IMU/accelerometer (A4-A5: SDA/SCL)
  - Temperature sensor (A6)
  - Light sensor (A7)
```

### Future Expansion Recommendations
1. **Sensor Pod** (Pins 4-7): Temperature, pressure, humidity
2. **IMU Module** (Pins 18-21): I2C IMU for tilt detection
3. **Battery Monitor** (A0): Voltage/current monitoring
4. **Status LEDs** (Pins 8-9): Visual feedback system
5. **Autonomous Mode** (Pins 10-13): Additional control system

---

## ⚡ Power Budget Summary

```
Total System Power Consumption:

Idle State:
  - Arduino: ~50mA
  - Servos holding position: ~500mA
  - Receiver: ~50mA
  - Total: ~600mA

Full Throttle (All Motors + Servos Moving):
  - Motors: 15-20A (12V system)
  - Servos: 2-3A (5V system)
  - Arduino & Control: ~200mA
  - Total: ~17-23A

Battery Life (Typical):
  - 12V 5000mAh LiPo @ 15A average: ~20 minutes
  - 7.4V 2000mAh LiPo @ 2A average: ~60 minutes
  - Servo battery lasts longer (lower current draw)
```

---

**Last Updated**: June 2026
**Status**: Complete Pin Reference
**Compatibility**: Arduino Mega 2560 & Compatible Boards
