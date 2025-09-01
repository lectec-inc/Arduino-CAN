// === Buttons + OLED RPM Display (ESP32-C3 + VESC_API + SSD1306) ===
// CONNECTIONS
// Button Board GND -> PCB GND
// Button Board K1  -> PCB IO1  (S1 = forward)
// Button Board K2  -> PCB IO3  (S2 = backward)
//
// OLED GND -> PCB GND
// OLED VCC -> PCB 3V3
// OLED SCL -> PCB IO9
// OLED SDA -> PCB IO8

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "VESC_API.h"

// ---- Pins ----
const int PIN_S1 = 1;   // forward
const int PIN_S2 = 3;   // backward
#define OLED_SDA 8
#define OLED_SCL 9

// ---- Display ----
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ---- Command cadence (avoid VESC timeout) ----
const unsigned long COMMAND_PERIOD_MS = 50;
unsigned long LAST_COMMAND = 0;
int lastAction = -1; // -1 unknown, 0 stop, 1 forward, 2 backward

void drawRPM(float rpm) { 
  display.clearDisplay();
  // Title
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Lectec: Live RPM");
  // Big RPM number
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print("RPM: ");
  display.println((int)roundf(rpm));
  // Hint line
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println("S1=Forward  S2=Back");

  display.display();
}

void setup() {
  Serial.begin(115200);

  // Buttons (active-low)
  pinMode(PIN_S1, INPUT_PULLUP);
  pinMode(PIN_S2, INPUT_PULLUP);

  // OLED
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    // If display init fails, stay here so it's obvious
    while (true) { delay(1000); }
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.println("Starting...");
  display.display();

  // VESC
  if (!vesc.init()) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println("VESC init");
    display.setCursor(0, 34);
    display.println("failed!");
    display.display();
    while (true) { delay(1000); }
  }
  Serial.println("VESC ready. S1=forward, S2=backward");
}

void loop() {
  vesc.update();
  
  // If VESC not connected, show message and do nothing
  if (!vesc.isConnected()) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 18);
    display.println("VESC off");
    display.display();
    delay(200);
    return;
  }

  // Read buttons (active-low)
  bool s1 = (digitalRead(PIN_S1) == LOW); // forward
  bool s2 = (digitalRead(PIN_S2) == LOW); // backward

  // Periodically refresh command so VESC doesn't timeout
  unsigned long now = millis();
  if (now - LAST_COMMAND >= COMMAND_PERIOD_MS) {
    LAST_COMMAND = now;

    int action;
    if (s1 && !s2) {
      vesc.setDutyCycle(0.5);   // forward
      action = 1;
    } else if (s2 && !s1) {
      vesc.setDutyCycle(-0.3);  // backward
      action = 2;
    } else {
      vesc.setDutyCycle(0.0);     // stop (or both pressed)
      action = 0;
    }

    if (action != lastAction) {
      lastAction = action;
      if (action == 1)      Serial.println("Action: FORWARD");
      else if (action == 2) Serial.println("Action: BACKWARD");
      else                  Serial.println("Action: STOP");
    }
  }

  // Read and show RPM
  float rpm = vesc.getRPM();
  drawRPM(rpm);

  delay(50);
}
