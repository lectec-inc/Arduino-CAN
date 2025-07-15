// Simple VESC Command Test
// Tests setDutyCycle() command with minimal output

#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("VESC Command Test Starting");
  
  if (!vesc.init()) {
    Serial.println("ERROR: VESC init failed");
    while (1) delay(1000);
  }
  
  Serial.println("VESC initialized");
  
  // Wait for connection
  while (!vesc.isConnected()) {
    vesc.update();
    delay(100);
  }
  
  Serial.println("VESC connected");
}

void loop() {
  vesc.update();
  
  static unsigned long lastTest = 0;
  
  if (millis() - lastTest > 3000) {
    Serial.println("=== TEST START ===");
    
    // Read initial duty
    float initialDuty = vesc.getDuty();
    Serial.print("Before: ");
    Serial.print(initialDuty);
    Serial.println("%");
    
    // Send command
    Serial.println("Sending setDutyCycle(10.0)");
    vesc.setDutyCycle(10.0);
    
    // Wait and read response
    delay(200);
    vesc.update();
    
    float afterDuty = vesc.getDuty();
    Serial.print("After: ");
    Serial.print(afterDuty);
    Serial.println("%");
    
    // Test result
    if (abs(afterDuty - initialDuty) > 1.0) {
      Serial.println("SUCCESS: Duty cycle changed!");
    } else {
      Serial.println("FAILED: No duty cycle change");
    }
    
    // Stop motor
    vesc.setDutyCycle(0.0);
    
    Serial.println("=== TEST END ===");
    Serial.println();
    
    lastTest = millis();
  }
  
  delay(10);
}