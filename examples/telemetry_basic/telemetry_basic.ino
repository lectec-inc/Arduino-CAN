// Simple VESC Telemetry Monitor
// Shows how to read basic VESC data using the simple API

#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Simple VESC Telemetry Starting...");
  
  // Initialize VESC system
  if (!vesc.init()) {
    Serial.println("ERROR: VESC initialization failed!");
    while (1) delay(1000);
  }
  
  Serial.println("VESC ready!");
}

void loop() {
  // Always update VESC system first
  vesc.update();
  
  // Print telemetry every 2 seconds
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 2000) {
    
    if (vesc.isConnected()) {
      Serial.println("=== VESC Telemetry ===");
      Serial.print("RPM: ");
      Serial.println(vesc.getRPM());
      Serial.print("Voltage: ");
      Serial.print(vesc.getVoltage());
      Serial.println("V");
      Serial.print("Motor Current: ");
      Serial.print(vesc.getMotorCurrent());
      Serial.println("A");
      Serial.print("Battery Current: ");
      Serial.print(vesc.getBatteryCurrent());
      Serial.println("A");
      Serial.print("Duty Cycle: ");
      Serial.print(vesc.getDuty());
      Serial.println("%");
      Serial.print("FET Temperature: ");
      Serial.print(vesc.getFETTemp());
      Serial.println("°C");
      Serial.println("=====================");
    } else {
      Serial.println("VESC not connected");
    }
    
    lastPrint = millis();
  }
  
  delay(10);
}