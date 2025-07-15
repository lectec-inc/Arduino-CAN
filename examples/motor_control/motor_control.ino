// Basic VESC Motor Control Example
// Shows how to control VESC motor using simple commands

#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("VESC Motor Control Example Starting...");
  
  // Initialize VESC system
  if (!vesc.init()) {
    Serial.println("ERROR: VESC initialization failed!");
    while (1) delay(1000);
  }
  
  Serial.println("VESC ready!");
  Serial.println("Commands:");
  Serial.println("  w - Forward");
  Serial.println("  s - Backward");
  Serial.println("  a - Stop");
  Serial.println("  d - Brake");
  Serial.println("  i - Info");
}

void loop() {
  // Always update VESC system first
  vesc.update();
  
  // Handle serial commands
  if (Serial.available()) {
    char command = Serial.read();
    
    switch(command) {
      case 'w':
        Serial.println("Going forward (20% duty)");
        vesc.setDutyCycle(10.0);
        break;
        
      case 's':
        Serial.println("Going backward (20% duty)");
        vesc.setDutyCycle(-10.0);
        break;
        
      case 'a':
        Serial.println("Stopping");
        vesc.setDutyCycle(0.0);
        break;
        
      case 'd':
        Serial.println("Braking");
        vesc.setCurrentBrake(5.0);  // 5A brake current
        break;
        
      case 'i':
        if (vesc.isConnected()) {
          Serial.println("=== VESC Status ===");
          Serial.print("RPM: ");
          Serial.println(vesc.getRPM());
          Serial.print("Voltage: ");
          Serial.print(vesc.getVoltage());
          Serial.println("V");
          Serial.print("Current: ");
          Serial.print(vesc.getMotorCurrent());
          Serial.println("A");
          Serial.print("Duty: ");
          Serial.print(vesc.getDuty());
          Serial.println("%");
          Serial.println("==================");
        } else {
          Serial.println("VESC not connected");
        }
        break;
    }
  }
  
  delay(10);
}