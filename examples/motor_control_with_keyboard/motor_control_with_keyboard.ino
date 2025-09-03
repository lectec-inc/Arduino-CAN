// VESC Motor Control + RPM Buzzer Alarm
// w = forward, s = backward, a = stop, d = brake, i = info
// Buzzer sounds whenever motor RPM > threshold

// CONNECTION DIAGRAM!
// BUZZER GND -> PCB GND
// BUZZER I/O -> PCB IO19 (ESP32 Side)
// BUZZER VCC -> PCB 3v3

#include "VESC_API.h"

const int BUZZER_PIN = 19;      // ESP32 pin for buzzer I/O
const float RPM_LIMIT = 10.0;    // buzz when RPM above this

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN); 

  Serial.println("VESC Motor Control Example Starting...");

  if (!vesc.init()) {
    Serial.println("ERROR: VESC initialization failed!");
    while (true) { delay(1000); }
  }

  Serial.println("VESC ready!");
  Serial.println("Commands:");
  Serial.println("  w - Forward");
  Serial.println("  s - Backward");
  Serial.println("  d - Brake");
}

void loop() {
  // Update telemetry each loop
  vesc.update();

  // Handle keyboard commands
  if (Serial.available()) {
    char command = Serial.read();

    switch (command) {
      case 'w':
        Serial.println("Going forward (10% duty)");
        vesc.setDutyCycle(10.0);
        break;

      case 's':
        Serial.println("Going backward (-10% duty)");
        vesc.setDutyCycle(-10.0);
        break;

      case 'd':
        Serial.println("Braking (5A)");
        vesc.setCurrentBrake(5.0);
        break;

      default:
        // ignore other keys
        break;
    }
  }

  // ---- RPM -> Buzzer logic (always-on check) ----
  float rpm = vesc.getRPM();
  if (rpm > RPM_LIMIT) {
    tone(BUZZER_PIN, 1000);
  } else {
    noTone(BUZZER_PIN); // keeps the buzzer off if motor isn't spinning
  }

  delay(100); // loop cadence
}