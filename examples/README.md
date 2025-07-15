# VESC Arduino Examples

This directory contains ready-to-use examples showing how to use the VESC API for different applications.

## ✅ Command Functions Verified Working
All command functions have been tested and confirmed working:
- `setDutyCycle()` - ✅ Tested and working
- `setCurrent()` - ✅ Tested and working  
- `setCurrentBrake()` - ✅ Tested and working
- `setRPM()` - ✅ Tested and working

## Getting Started

Each example is a complete Arduino sketch that you can open directly in Arduino IDE:

1. Open Arduino IDE
2. Go to File → Open
3. Navigate to one of the example folders
4. Select the `.ino` file
5. Compile and upload to your ESP32-C3

## Examples

### 1. simple_telemetry
**Purpose:** Basic telemetry monitoring  
**Features:**
- Reads VESC data every 2 seconds
- Shows RPM, voltage, current, duty cycle, temperature
- Clean, readable output

### 2. motor_control
**Purpose:** Interactive motor control  
**Features:**
- Serial commands to control motor
- Forward/backward/stop/brake commands
- Real-time status display

### 3. example_student_code
**Purpose:** Interactive demonstration  
**Features:**
- Serial commands (f/b/s/i)
- Shows connection status
- Demonstrates basic API usage

### 4. command_test
**Purpose:** Comprehensive command testing
**Features:**
- Detailed monitoring of command responses
- Continuous testing loop
- Real-time feedback

### 5. command_test_simple
**Purpose:** Simple command verification
**Features:**
- Quick pass/fail testing
- Minimal output for easy debugging
- ✅ Confirmed working with real VESC hardware

### 6. rpm_alarm
**Purpose:** Simple RPM alarm system
**Features:**
- Sounds buzzer when RPM > 10
- Minimal code example (22 lines)
- Clean student-friendly implementation
- **Hardware:** Piezo buzzer connected to GPIO 19

## VESC API Quick Reference

### Setup
```cpp
#include "VESC_API.h"

void setup() {
  vesc.init();  // Initialize VESC system
}

void loop() {
  vesc.update();  // Process CAN messages (call this first!)
  
  // Your code here
}
```

### Reading Data
```cpp
float rpm = vesc.getRPM();               // Motor RPM
float voltage = vesc.getVoltage();       // Battery voltage (V)
float motorCurrent = vesc.getMotorCurrent();  // Motor current (A)
float batteryCurrent = vesc.getBatteryCurrent(); // Battery current (A)
float duty = vesc.getDuty();             // Duty cycle (%)
float fetTemp = vesc.getFETTemp();       // FET temperature (°C)
float motorTemp = vesc.getMotorTemp();   // Motor temperature (°C)
float ampHours = vesc.getAmpHours();     // Consumed amp hours
```

### Sending Commands
```cpp
vesc.setDutyCycle(10.0);      // Set 10% duty cycle
vesc.setCurrent(5.0);         // Set 5A motor current
vesc.setCurrentBrake(3.0);    // Set 3A brake current
vesc.setRPM(1000);            // Set 1000 RPM
```

### System Status
```cpp
bool connected = vesc.isConnected();     // Check if VESC responding
vesc.printStatus();                      // Print all telemetry
vesc.printDebug();                       // Print debug info
```

## Hardware Requirements

- ESP32-C3 microcontroller
- MCP2515 CAN transceiver
- Proper wiring as specified in main README

## Notes

- Always call `vesc.update()` at the beginning of your loop()
- Check `vesc.isConnected()` before sending commands
- Each example includes the API files locally for easy use
- The API handles all CAN protocol complexity automatically