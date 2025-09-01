// Simple RPM Alarm - sounds buzzer when RPM > 10

// CONNECTION DIAGRAM!
// BUZZER GND -> PCB GND
// BUZZER I/O -> PCB IO19 (ESP32 Side)
// BUZZER VCC -> PCB 3v3

#include "VESC_API.h"

const int BUZZER_PIN = 19; 

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  vesc.init(); // initialize the VESC Communication
}

void loop() {
  vesc.update(); // update the realtime value readings
  
  // Check if RPM is above 10
  if (vesc.getRPM() > 10.0) {
    tone(BUZZER_PIN, 2000); // Half volume alarm
  } else {
    noTone(BUZZER_PIN);     // Turn off buzzer
  }
  
  delay(100); //length of the alarm or no alarm pause
}