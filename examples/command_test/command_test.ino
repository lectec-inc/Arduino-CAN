// VESC Command Test Script
// Tests if setDutyCycle() command actually works by monitoring the response

#include "VESC_API.h"

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== VESC Command Test Starting ===");
  
  // Initialize VESC system
  if (!vesc.init()) {
    Serial.println("ERROR: VESC initialization failed!");
    while (1) {
      delay(1000);
      Serial.println("VESC FAILED - Check connections");
    }
  }
  
  Serial.println("VESC initialized successfully!");
  Serial.println("Waiting for VESC connection...");
  
  // Wait for VESC to connect
  while (!vesc.isConnected()) {
    vesc.update();
    delay(100);
  }
  
  Serial.println("VESC connected! Starting command test...");
  Serial.println();
}

void loop() {
  // Always update VESC system first
  vesc.update();
  
  static unsigned long lastTest = 0;
  static bool testInProgress = false;
  static unsigned long testStartTime = 0;
  
  // Run test every 5 seconds
  if (millis() - lastTest > 5000 && !testInProgress) {
    Serial.println(">>> STARTING COMMAND TEST <<<");
    
    // Read current duty cycle
    float initialDuty = vesc.getDuty();
    Serial.print("Initial duty cycle: ");
    Serial.print(initialDuty);
    Serial.println("%");
    
    // Send command to set 15% duty cycle
    Serial.println("Sending command: setDutyCycle(15.0)");
    vesc.setDutyCycle(15.0);
    
    testInProgress = true;
    testStartTime = millis();
    lastTest = millis();
  }
  
  // Monitor duty cycle for 2 seconds after command
  if (testInProgress) {
    unsigned long elapsed = millis() - testStartTime;
    
    if (elapsed < 2000) {  // Monitor for 2 seconds
      static unsigned long lastPrint = 0;
      if (millis() - lastPrint > 100) {  // Print every 100ms
        float currentDuty = vesc.getDuty();
        Serial.print("Time: ");
        Serial.print(elapsed);
        Serial.print("ms | Duty: ");
        Serial.print(currentDuty);
        Serial.print("% | RPM: ");
        Serial.print(vesc.getRPM());
        Serial.print(" | Connected: ");
        Serial.println(vesc.isConnected() ? "YES" : "NO");
        lastPrint = millis();
      }
    } else {
      // Test complete
      Serial.println(">>> COMMAND TEST COMPLETE <<<");
      
      // Send stop command
      Serial.println("Sending stop command: setDutyCycle(0.0)");
      vesc.setDutyCycle(0.0);
      
      float finalDuty = vesc.getDuty();
      Serial.print("Final duty cycle: ");
      Serial.print(finalDuty);
      Serial.println("%");
      
      Serial.println("Waiting 3 seconds before next test...");
      Serial.println();
      
      testInProgress = false;
    }
  }
  
  // Show connection status if disconnected
  if (!vesc.isConnected()) {
    static unsigned long lastDisconnectMsg = 0;
    if (millis() - lastDisconnectMsg > 2000) {
      Serial.println("WARNING: VESC disconnected!");
      lastDisconnectMsg = millis();
    }
  }
  
  delay(10);
}