# RIC Solaris - 6-Wheel Hexapod Robot with Robotic Arm

## 📋 Project Overview

**RIC Solaris** is a sophisticated 6-wheel hexapod rover equipped with a 5-DOF robotic arm and dual-camera mount, controlled via FlySky FS-i10A radio receiver. The system features dual operating modes (Drive & Arm) with advanced motor control, servo management, and safety features.

---

## 🎯 Key Features

### Hardware Components
- **Drive System**: 6 × DC Motors controlled via single BTS7960 H-bridge (intelligent high-side driver)
- **Steering**: 4 × 360° Continuous Rotation Servos (synchronized wheel control)
- **Arm Mechanism**: 5 × MG996R Servos (180°) for multi-joint articulation
- **Camera System**: 2 × MG996R Servos (180°) for pan/tilt control
- **Power Management**: 
  - 12V LiPo Battery (main drive power)
  - 2S 7.4V LiPo Battery (servo power)
  - XL4016 Buck Converter (voltage regulation)
- **Control**: FlySky FS-i10A Receiver (8-channel PPM input)
- **Processor**: Arduino Mega (or compatible microcontroller)

### Operating Modes
1. **DRIVE MODE** (Mode Switch < 1300μs)
   - Forward/Backward movement via pitch stick
   - Synchronized 4-wheel steering
   - Independent camera pan/tilt control
   - Arm rotation & claw auxiliary control during drive
   
2. **ARM MODE** (Mode Switch > 1700μs)
   - All drive motors automatically stop
   - Wheels center to safe position
   - Full robotic arm control (5 joints)
   - Independent gripper manipulation

3. **TURN-IN-PLACE MODE** (Turn Switch > 1300μs during drive mode)
   - Wheels at maximum angle for tight turning radius
   - 360° rotation around hexapod center
   - Roll stick controls rotation direction & speed

### Safety Features
- **PPM Signal Watchdog**: 500ms timeout → automatic emergency stop
- **Deadzone Filtering**: Prevents stick drift (±50μs neutral zone)
- **Mode Isolation**: Arm mode disables drive motors automatically
- **Servo Centering**: All servos return to neutral on mode switch or signal loss
- **Input Validation**: All PPM values constrained to valid range (1000-2000μs)

---

## 🎮 Control Scheme

### Channel Mapping (FlySky FS-i10A)

| Channel | Function | Range | Default | Notes |
|---------|----------|-------|---------|-------|
| CH0 (Roll) | Steering / Arm Angle | 1000-2000μs | 1500 | Left/Right control |
| CH1 (Pitch) | Forward/Backward / Arm Boom | 1000-2000μs | 1500 | Up/Down control |
| CH2 (Throttle) | Spin Speed / Arm Rotation | 1000-2000μs | 1500 | Rotation control |
| CH3 (Rudder) | Claw / Claw Open-Close | 1000-2000μs | 1500 | Fine control |
| CH4 (Aux1) | Camera Pan / Claw Wrist | 1000-2000μs | 1500 | Horizontal sweep |
| CH5 (Aux2) | Camera Tilt | 1000-2000μs | 1500 | Vertical sweep |
| CH6 (SWA) | **MODE SWITCH** | 1000-2000μs | 1500 | Drive/Arm selector |
| CH7 (SWB) | **TURN MODE** | 1000-2000μs | 1500 | Normal/Spin selector |

### Mode Selection Logic
```
Mode Switch Value:
├─ < 1300μs  → DRIVE MODE
├─ 1300-1700μs → NEUTRAL (All Stop)
└─ > 1700μs  → ARM MODE
```

### Drive Mode Controls
#### Normal Drive (Turn < 1300μs)
- **Pitch (CH1)**:
  - `> 1600μs`: Forward (mapped to 0-255 speed)
  - `1400-1600μs`: Stop/Neutral
  - `< 1400μs`: Backward (mapped to 0-255 speed)

- **Roll (CH0)**: Wheel Steering
  - `> 1550μs`: Turn Right (servo angle 90→180°)
  - `1450-1550μs`: Straight (servo angle 90°)
  - `< 1450μs`: Turn Left (servo angle 90→0°)

#### Turn-In-Place Mode (Turn > 1300μs)
- Wheels set to maximum angle (servo1=135°, servo2=35°, servo3=45°, servo4=135°)
- **Roll (CH0)**:
  - `< 1350μs`: Spin Counterclockwise
  - `1350-1650μs`: Stop spinning
  - `> 1650μs`: Spin Clockwise

### Arm Mode Controls
Wheels automatically center, all drive motors stop.

| Joint | Channel | Control | Range |
|-------|---------|---------|-------|
| Base Angle | CH0 (Roll) | Arm swing up/down | 0-180° |
| Boom Extension | CH1 (Pitch) | Reach in/out | 0-180° |
| Base Rotation | CH2 (Throttle) | Sweep left/right | 0-180° |
| Gripper Claw | CH3 (Rudder) | Open/close | 0-180° |
| Wrist Angle | CH4 (Pan) | Fine positioning | 0-180° |

---

## 📌 Pin Configuration

### PPM Receiver
- **Pin 2**: PPM Signal Input (Interrupt)

### Motor Control (BTS7960 H-Bridge)
- **Pin 26**: IN1 (Direction Control - Forward)
- **Pin 27**: IN2 (Direction Control - Reverse)
- **Pin 3**: ENABLE (PWM Speed Control, 0-255)

### Wheel Steering Servos (360° Continuous)
- **Pin 22**: Servo1 (Left-Front Wheel)
- **Pin 23**: Servo2 (Right-Front Wheel)
- **Pin 24**: Servo3 (Left-Rear Wheel)
- **Pin 25**: Servo4 (Right-Rear Wheel)

### Arm Joint Servos (MG996R 180°)
- **Pin 32**: Arm Angle (Base Joint)
- **Pin 33**: Arm Boom (Reach Extension)
- **Pin 34**: Arm Rotate (Base Rotation)
- **Pin 40**: Claw (Gripper)
- **Pin 35**: Claw Angle (Wrist)

### Camera Mount Servos (MG996R 180°)
- **Pin 30**: Camera Pan (Horizontal)
- **Pin 31**: Camera Tilt (Vertical)

---

## ⚡ Motor Control Details

### BTS7960 H-Bridge Operation
The BTS7960 is an intelligent high-side driver that controls motor direction and speed:

```
IN1  | IN2  | ENABLE | Result
-----|------|--------|---------------------------
HIGH | LOW  | 0-255  | Forward (speed proportional to PWM)
LOW  | HIGH | 0-255  | Reverse (speed proportional to PWM)
LOW  | LOW  | x      | Stop (Freewheeling/Coasting)
HIGH | HIGH | x      | Brake (Hard stop)
```

**Current Implementation**: Uses Forward, Backward, and Stop modes.

### Speed Mapping
- **Throttle Input**: 1000-2000μs (PPM)
- **Speed Output**: 0-255 (PWM to ENABLE pin)
- **Mapping Formula**: `speed = map(ppmValue, minPPM, maxPPM, 0, 255)`

Example (Forward):
```cpp
if (pitch > 1600) {
  int spd = map(pitch, 1600, 2000, 0, 255);  // 1600→0 speed, 2000→255 speed
  motorForward(spd);
}
```

---

## 🔧 Code Architecture

### Main Sections

#### 1. **Configuration & Declarations**
- Pin definitions and thresholds
- Servo objects (11 total)
- PPM buffer variables with volatile flags
- Motor driver pins

#### 2. **Helper Functions**
- `applyDeadzone()`: Filter stick jitter (±50μs)
- `constrainPPM()`: Validate PPM range (1000-2000)
- `stopMotors()`: Emergency motor stop
- `motorForward()`: Drive forward at speed
- `motorBackward()`: Drive backward at speed
- `centerServos()`: Reset all servos to neutral

#### 3. **PPM ISR (Interrupt Handler)**
- Decodes 8-channel PPM signal at 50Hz frame rate
- Detects frame sync (>3000μs pulse)
- Validates received values

#### 4. **Setup()**
- Initializes serial communication (115200 baud)
- Configures PPM interrupt on pin 2
- Attaches servo objects to GPIO pins
- Initializes motor driver pins
- Centers all servos

#### 5. **Main Loop()**
- Checks PPM signal timeout (500ms watchdog)
- Reads mode switch value
- Routes control to appropriate function (Drive/Arm)
- Executes mode-specific control logic

#### 6. **Control Functions**
- `driveModeControl()`: Handles 4-wheel steering, forward/backward, camera control
- `armModeControl()`: Manages arm joints, disables drive motors

---

## 🛡️ Safety Mechanisms

### 1. PPM Signal Timeout
```cpp
if (micros() - lastPPMTime > PPM_TIMEOUT) {  // 500ms
  stopMotors();      // Emergency stop
  centerServos();    // Safe position
  return;            // Skip rest of loop
}
```
**Purpose**: Prevents runaway if receiver loses signal or crashes.

### 2. Deadzone Filtering
```cpp
int applyDeadzone(int value, int center = 1500, int deadzone = 50) {
  if (abs(value - center) < deadzone) return center;
  return value;
}
```
**Purpose**: Eliminates drift from stick centering error (~50μs tolerance).

### 3. Mode Isolation
- Arm mode automatically stops all drive motors
- Wheels center when entering arm mode
- No simultaneous drive + arm operation possible

### 4. Input Validation
```cpp
int constrainPPM(int value) {
  return constrain(value, 1000, 2000);  // Valid PPM range
}
```
**Purpose**: Reject corrupted signals outside valid range.

---

## 🚀 Operation Guide

### Startup Procedure
1. Power on receiver first (avoid binding issues)
2. Power on hexapod main battery
3. Observe servo centering during setup (verify servo health)
4. Wait for "✓ Ready" message on serial monitor
5. Check mode switch position before operation

### Flight Safety Checklist
- [ ] Mode switch in NEUTRAL position (between Drive/Arm)
- [ ] All servos centered visually
- [ ] Arm properly stowed (not extended dangerously)
- [ ] Wheels centered (straight position)
- [ ] Battery voltage adequate (avoid over-discharge)
- [ ] Receiver LED blinking (signal acquired)

### Typical Mission
1. Set mode switch to DRIVE (< 1300μs)
2. Use pitch stick for forward/backward movement
3. Use roll stick for steering (or turn-in-place if engaged)
4. Control camera with CH4-CH5 (pan/tilt)
5. Switch to ARM mode (> 1700μs) for object manipulation
6. Switch back to DRIVE for movement
7. Return to NEUTRAL on completion

---

## 📊 Technical Specifications

### PPM Protocol
- **Frame Rate**: 50 Hz (20ms per frame)
- **Channel Count**: 8 channels
- **Pulse Width**: 1000-2000μs per channel
- **Sync Pulse**: >3000μs between frames
- **Baudrate**: 115200 (serial debugging)

### Servo Specifications
- **360° Continuous**: Standard servo library, 1500μs = stop
- **MG996R (180°)**: Pulse range 500-2500μs, controlled via angle (0-180°)

### Motor Specifications
- **DC Motors**: 6× (typical hobby-grade 3-6V, run on 12V via PWM)
- **BTS7960 Max Current**: 43A per channel (sufficient for 6 motors)
- **PWM Frequency**: ~980 Hz (Arduino default)

### Power Consumption (Estimated)
- **Idle**: ~500mA (servos holding position)
- **Full Throttle**: ~15-20A (all motors + servos)
- **Battery Life**: 20-30 minutes (depends on duty cycle)

---

## 🔌 Wiring Diagram Summary

```
FlySky FS-i10A Receiver
├─ PPM Signal → Arduino Pin 2 (Interrupt)
├─ GND → Arduino GND
└─ VCC → 5V (servo power supply)

BTS7960 H-Bridge
├─ IN1 → Arduino Pin 26
├─ IN2 → Arduino Pin 27
├─ ENABLE → Arduino Pin 3 (PWM)
├─ GND → Battery GND
├─ +12V → Battery (+)
└─ OUT1, OUT2 → Motor Terminals

Servos (All powered from servo power supply)
├─ Wheels (4×) → Pins 22-25
├─ Arm (5×) → Pins 32-35, 40
└─ Camera (2×) → Pins 30-31

Batteries
├─ 12V LiPo → BTS7960 Motor Power
├─ 7.4V LiPo → Servo Power Supply
└─ XL4016 Buck Converter → Voltage Regulation
```

---

## 🐛 Troubleshooting

### Motors Don't Spin
- [ ] Check BTS7960 connections (IN1/IN2/ENABLE)
- [ ] Verify battery voltage (>11V for 12V LiPo)
- [ ] Test motor direction pins (should toggle HIGH/LOW)
- [ ] Check PWM signal on oscilloscope (should vary 0-255)

### Servos Jitter/Don't Respond
- [ ] Verify servo power supply voltage (5V nominal)
- [ ] Check servo pin connections (30-35, 40)
- [ ] Look for PPM sync issues (check serial output)
- [ ] Test servo manually via Arduino IDE (servo.write())

### Signal Loss/Timeout
- [ ] Verify receiver antenna orientation
- [ ] Check PPM wire for damage/loose connections
- [ ] Confirm transmitter is powered on
- [ ] Look for "PPM Signal Lost!" message on serial

### Unstable Control
- [ ] Increase deadzone value (higher = less sensitive)
- [ ] Check for electromagnetic interference (motors near wires)
- [ ] Verify servo centering (90° = true neutral)
- [ ] Reduce PWM frequency if needed (increase motor responsiveness)

---

## 📝 Code Modifications Guide

### Adjusting Deadzone
```cpp
#define DEADZONE 50  // Change to 30-80 depending on stick quality
```

### Adjusting Motor Speeds
Modify the `map()` ranges in `driveModeControl()`:
```cpp
int spd = map(pitch, 1600, 2000, 0, 255);  // Change 0-255 range
```

### Adjusting Steering Sensitivity
Modify the servo angle ranges:
```cpp
servo1.write(map(roll, 1550, 2000, 90, 160));  // More gradual steering
```

### Adding Speed Ramping
Implement acceleration limiting in `motorForward()`:
```cpp
static int currentSpeed = 0;
int targetSpeed = constrain(speed, 0, 255);
currentSpeed += min(10, abs(targetSpeed - currentSpeed));
analogWrite(BTS7960_ENABLE, currentSpeed);
```

---

## 📚 References

- **Arduino Servo Library**: https://www.arduino.cc/reference/en/libraries/servo/
- **PPM Protocol**: https://en.wikipedia.org/wiki/Pulse_position_modulation
- **BTS7960 Datasheet**: Intelligent High-Side Driver
- **FlySky FS-i10A Manual**: 8-Channel RC Transmitter
- **MG996R Specs**: High-Torque Metal Gear Servo (180°)

---

## 📄 Version History

- **v1.0** (Current): Complete refactor with comprehensive documentation, safety features, and optimized control logic
- **v0.1** (Original): Initial implementation with basic PPM decoding

---

## 👥 Support & Contributions

For bugs, feature requests, or improvements:
1. Check the troubleshooting section
2. Review code comments for implementation details
3. Test changes in controlled environment
4. Document modifications clearly

---

**Last Updated**: June 2026  
**Status**: Production Ready  
**License**: Open Source (RIC Robotics)
