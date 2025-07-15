# VESC Arduino CAN API

A simple, student-friendly Arduino library for controlling VESC motor controllers via CAN bus.

## 🚀 Quick Start

### Hardware Requirements
- **ESP32-C3** microcontroller
- **MCP2515 CAN transceiver** module
- **VESC motor controller** (default ID: 74)

### Wiring
Connect the MCP2515 to your ESP32-C3:
```
MCP2515  →  ESP32-C3
VCC      →  3.3V
GND      →  GND
CS       →  GPIO 10
SO       →  GPIO 2
SI       →  GPIO 7
SCK      →  GPIO 6
INT      →  GPIO 4
```

### Installation
1. Copy the files from `Arduino_Library/` to your Arduino libraries folder
2. Or simply copy `VESC_API.h` and `VESC_API.cpp` to your sketch folder

## 📖 Basic Usage

### 1. Include and Initialize
```cpp
#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  
  // Initialize VESC system
  if (!vesc.init()) {
    Serial.println("VESC initialization failed!");
    while (1) delay(1000);
  }
  
  Serial.println("VESC ready!");
}

void loop() {
  // Always call this first in loop()
  vesc.update();
  
  // Your code here...
}
```

### 2. Reading Data
```cpp
// Get motor status
float rpm = vesc.getRPM();              // Motor RPM
float voltage = vesc.getVoltage();      // Battery voltage (V)
float current = vesc.getMotorCurrent(); // Motor current (A)
float duty = vesc.getDuty();            // Duty cycle (%)
float temp = vesc.getFETTemp();         // FET temperature (°C)

// Check connection
if (vesc.isConnected()) {
  Serial.println("VESC is responding!");
}
```

### 3. Controlling the Motor
```cpp
// Set duty cycle (percentage: -100 to 100)
vesc.setDutyCycle(15.0);    // 15% forward
vesc.setDutyCycle(-10.0);   // 10% reverse
vesc.setDutyCycle(0.0);     // Stop

// Set motor current (Amps)
vesc.setCurrent(5.0);       // 5A forward
vesc.setCurrent(-3.0);      // 3A reverse

// Set brake current (Amps)
vesc.setCurrentBrake(2.0);  // 2A brake

// Set RPM
vesc.setRPM(1000);          // 1000 RPM
vesc.setRPM(-500);          // 500 RPM reverse
```

## 🛠️ Complete API Reference

### Data Reading Functions
| Function | Returns | Description |
|----------|---------|-------------|
| `vesc.getRPM()` | float | Motor RPM |
| `vesc.getDuty()` | float | Duty cycle (%) |
| `vesc.getMotorCurrent()` | float | Motor current (A) |
| `vesc.getBatteryCurrent()` | float | Battery current (A) |
| `vesc.getVoltage()` | float | Input voltage (V) |
| `vesc.getFETTemp()` | float | FET temperature (°C) |
| `vesc.getMotorTemp()` | float | Motor temperature (°C) |
| `vesc.getAmpHours()` | float | Consumed amp hours |
| `vesc.getWattHours()` | float | Consumed watt hours |

### Control Functions
| Function | Parameter | Description |
|----------|-----------|-------------|
| `vesc.setDutyCycle(duty)` | float (-100 to 100) | Set duty cycle percentage |
| `vesc.setCurrent(current)` | float (Amps) | Set motor current |
| `vesc.setCurrentBrake(current)` | float (Amps) | Set brake current |
| `vesc.setRPM(rpm)` | float | Set motor RPM |
| `vesc.setBrake(brake)` | float (0-100) | Set brake percentage |

### System Functions
| Function | Returns | Description |
|----------|---------|-------------|
| `vesc.init()` | bool | Initialize VESC system |
| `vesc.update()` | void | Process CAN messages (call in loop!) |
| `vesc.isConnected()` | bool | Check if VESC is responding |
| `vesc.getLastUpdate()` | unsigned long | Time of last VESC message |
| `vesc.printStatus()` | void | Print all telemetry data |
| `vesc.printDebug()` | void | Print debug information |

## 📋 Example Projects

### Simple Motor Control
```cpp
#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  vesc.init();
}

void loop() {
  vesc.update();
  
  if (Serial.available()) {
    char cmd = Serial.read();
    
    switch(cmd) {
      case 'w': vesc.setDutyCycle(20.0); break;  // Forward
      case 's': vesc.setDutyCycle(-20.0); break; // Backward  
      case 'a': vesc.setDutyCycle(0.0); break;   // Stop
      case 'd': vesc.setCurrentBrake(5.0); break; // Brake
    }
  }
}
```

### Telemetry Monitor
```cpp
#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  vesc.init();
}

void loop() {
  vesc.update();
  
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    if (vesc.isConnected()) {
      Serial.print("RPM: ");
      Serial.print(vesc.getRPM());
      Serial.print(" | Voltage: ");
      Serial.print(vesc.getVoltage());
      Serial.print("V | Current: ");
      Serial.print(vesc.getMotorCurrent());
      Serial.println("A");
    }
    lastPrint = millis();
  }
}
```

## 🔧 Examples Directory

This repository includes several complete examples:

- **`simple_telemetry/`** - Basic telemetry reading
- **`motor_control/`** - Interactive motor control via serial
- **`command_test/`** - Comprehensive command testing
- **`command_test_simple/`** - Simple command verification
- **`example_student_code/`** - Advanced interactive demo

Each example is a complete Arduino sketch that you can open directly in Arduino IDE.

## ⚠️ Important Notes

### Always Call `vesc.update()`
```cpp
void loop() {
  vesc.update();  // MUST be first in loop()
  
  // Your code here...
}
```

### Check Connection Status
```cpp
if (vesc.isConnected()) {
  // Safe to send commands
  vesc.setDutyCycle(10.0);
} else {
  Serial.println("VESC not connected!");
}
```

### Safety Considerations
- Start with low duty cycles (5-10%)
- Always have an emergency stop method
- Monitor motor temperature
- Check battery voltage regularly

## 🐛 Troubleshooting

### VESC Not Connecting
1. Check wiring connections
2. Verify VESC is powered on
3. Confirm CAN bus termination
4. Check VESC ID (default: 74)

### Commands Not Working
1. Ensure `vesc.update()` is called in loop
2. Check `vesc.isConnected()` status
3. Verify CAN bus communication
4. Try simple telemetry first

### Compilation Errors
1. Ensure `mcp_can` library is installed
2. Check ESP32 board package version
3. Verify file paths are correct

## 📜 License

This project is provided as-is for educational purposes. Use at your own risk.

## 🤝 Contributing

This API was designed for simplicity and educational use. Feel free to extend it for your specific needs!

---

**Happy coding!** 🚀