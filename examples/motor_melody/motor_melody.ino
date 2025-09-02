// === Melody From Motor Spin (ESP32-C3 + VESC_API) ===
// Use remote to control your vehicle
// CONNECTION DIAGRAM:
// Buzzer GND  -> PCB GND
// Buzzer VCC  -> PCB 3V3
// Buzzer I/O  -> PCB IO19  

#include "VESC_API.h"
#include <math.h>

const int BUZZER_PIN = 19;

// Different tones for each 10% increase/decrease in duty
const int BAND_TONE[] = { 262, 330, 392, 523, 659 }; // C4, E4, G4, C5, E5

int lastBand = -2;   // remember last band (-2 = nothing yet)

int dutyToBand(float dutyPct) {
  if (dutyPct < 1.0)   return -1;     // idle (silent)
  if (dutyPct <= 10.0) return 0;
  if (dutyPct <= 20.0) return 1;
  if (dutyPct <= 30.0) return 2;
  if (dutyPct <= 40.0) return 3;
  if (dutyPct <= 50.0) return 4;
  return 4; // anything above 50% duty is the last tone
}

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  noTone(BUZZER_PIN);
  vesc.init();   
}

void loop() {
  vesc.update();
  if (!vesc.isConnected()) {
    noTone(BUZZER_PIN);
    delay(80);
    return;
  }

  float dutyPct = fabs(vesc.getDuty()); // read duty percent
  if (dutyPct > 50.0) dutyPct = 50.0;   // cap at 50%

  Serial.print("Current Duty: ");
  Serial.println(dutyPct, 2); // show 2 decimals

  int band = dutyToBand(dutyPct);

  // If we entered a NEW band, update buzzer pitch
  if (band != lastBand) {
    lastBand = band;

    if (band >= 0) {
      tone(BUZZER_PIN, BAND_TONE[band]);  // continuous tone for that band
    } else {
      noTone(BUZZER_PIN);                 // idle: silent
    }
  }

  delay(40); // small pause to keep things smooth
}
