// Example code showing how students can use the VESC API
// This is a complete example they can start with

#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Student VESC Example Starting...");
  
  // Initialize VESC system
  if (!vesc.init()) {
    Serial.println("ERROR: VESC initialization failed!");
    while (1) delay(1000);
  }
  
  Serial.println("VESC ready! Try these commands:");
  Serial.println("- Type 'f' to go forward");
  Serial.println("- Type 'b' to go backward");
  Serial.println("- Type 's' to stop");
  Serial.println("- Type 'i' to see info");
}

void loop() {
  // Always update VESC system first
  vesc.update();
  
  // Handle serial commands
  if (Serial.available()) {
    char command = Serial.read();
    
    if (command == 'f') {
      Serial.println("Going forward...");
      vesc.setDutyCycle(10.0);  // 10% forward
    }
    else if (command == 'b') {
      Serial.println("Going backward...");
      vesc.setDutyCycle(-10.0);  // 10% backward
    }
    else if (command == 's') {
      Serial.println("Stopping...");
      vesc.setDutyCycle(0.0);  // Stop
    }
    else if (command == 'i') {
      Serial.println("=== VESC Info ===");
      Serial.print("Connected: ");
      Serial.println(vesc.isConnected() ? "YES" : "NO");
      Serial.print("RPM: ");
      Serial.println(vesc.getRPM());
      Serial.print("Voltage: ");
      Serial.print(vesc.getVoltage());
      Serial.println("V");
      Serial.print("Motor Current: ");
      Serial.print(vesc.getMotorCurrent());
      Serial.println("A");
      Serial.print("FET Temperature: ");
      Serial.print(vesc.getFETTemp());
      Serial.println("°C");
      Serial.println("================");
    }
  }
  
  // Optional: Print status every few seconds
  static unsigned long lastStatus = 0;
  if (millis() - lastStatus > 3000) {
    if (vesc.isConnected()) {
      Serial.print("Status: ");
      Serial.print(vesc.getRPM());
      Serial.print(" RPM, ");
      Serial.print(vesc.getVoltage());
      Serial.print("V, ");
      Serial.print(vesc.getDuty());
      Serial.println("% duty");
    } else {
      Serial.println("Status: VESC not connected");
    }
    lastStatus = millis();
  }
  
  delay(10);
}