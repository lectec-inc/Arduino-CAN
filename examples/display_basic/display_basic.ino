// === OLED Battery % + Voltage Display (ESP32-C3 + VESC_API) ===
// CONNECTION DIAGRAM!
// LCD SCREEN GND -> PCB GND
// LCD SCREEN VCC -> PCB 3V3
// LCD SCREEN SCL -> PCB IO9
// LCD SCREEN SDA -> PCB IO8

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "VESC_API.h"

#define OLED_SDA 8
#define OLED_SCL 9
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ---- Battery calibration ----
const float V_FULL  = 25.2f;   // volts at 100%
const float V_EMPTY = 18.6f;   // volts at 0%

// smoothing to reduce flicker
float vin_filtered = 0.0f;
const float ALPHA = 0.25f; // 0..1 (higher = snappier)

void setup() {
  Serial.begin(115200);
  Wire.begin(OLED_SDA, OLED_SCL);
  
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1);
  }
  display.setTextColor(SSD1306_WHITE);
  if (!vesc.init()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("VESC init failed");
    display.display();
    while (1);
  }
}

void loop() {
  vesc.update();
  display.clearDisplay();

  // Title
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Lectec Battery");

  if (!vesc.isConnected()) {
    display.setTextSize(1);
    display.setCursor(0, 24);
    display.println("VESC is off");
    display.display();
    delay(200);
    return;
  }

  float vin = vesc.getVoltage();

  // Smooth voltage
  if (vin_filtered == 0.0f) {
    vin_filtered = vin;
  }
  vin_filtered = ALPHA * vin + (1.0f - ALPHA) * vin_filtered;

  float vin_display = roundf(vin_filtered * 10.0f) / 10.0f;
  float frac = (vin_display - V_EMPTY) / (V_FULL - V_EMPTY);
  int soc = (int)roundf(constrain(frac, 0.0f, 1.0f) * 100.0f);

  // Show %
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.print(soc);
  display.println("%");

  // Show voltage
  display.setTextSize(1);
  display.setCursor(0, 45);
  display.print("Voltage: ");
  display.print(vin_display, 1);
  display.println(" V");

  display.display();
  delay(200);
}
