// === Controlling Motor with Button Board (ESP32-C3 + VESC_API) ===
// CONNECTION DIAGRAM!
//Button Board GND-> PCB GND 
//Button Board K1-> PCB IO1 
//Button Board K2-> PCB IO3  

#include <Arduino.h>
#include "VESC_API.h"

const int PIN_S1 = 1;                 // forward
const int PIN_S2 = 3;                 // brake
const unsigned long COMMAND_PERIOD_MS = 50;

unsigned long LAST_COMMAND = 0;
int lastAction = -1;  // -1 unknown, 0 stop, 1 forward, 2 brake

void setup() {
  Serial.begin(115200);
  pinMode(PIN_S1, INPUT_PULLUP);      // pressed = LOW
  pinMode(PIN_S2, INPUT_PULLUP);

  if (!vesc.init()) {
    Serial.println("ERROR: VESC init failed!");
    while (true) { delay(1000); }
  }
  Serial.println("VESC ready. Hold S1=forward, S2=brake ");
}

void loop() {
  vesc.update();

  // Read buttons (active-low)
  bool s1 = (digitalRead(PIN_S1) == LOW);
  bool s2 = (digitalRead(PIN_S2) == LOW);

  // Refresh command periodically to avoid VESC timeout
  unsigned long now = millis();
  if (now - LAST_COMMAND >= COMMAND_PERIOD_MS) {
    LAST_COMMAND = now;

    int action;
    if (s2) {        // brake overrides forward
      vesc.setCurrentBrake(5.0);  //applies 5Amps break current
      action = 2;
    } else if (s1) {
      vesc.setRPM(10000);   //MAX RPM = 17500
      action = 1;
    } else {
      vesc.setDutyCycle(0.0);
      action = 0;
    }

    if (action != lastAction) {
      lastAction = action;
      if (action == 2)      Serial.println("Action: BRAKE");
      else if (action == 1) Serial.println("Action: FORWARD");
      else                  Serial.println("No Action");
    }
  }
}
