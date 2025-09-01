// === Multi-Button Metrics Display (ESP32-C3 + VESC_API + SSD1306) ===
// OLED: GND->GND, VCC->3V3, SCL->IO9, SDA->IO8
// Buttons: K1->IO1, K2->IO3, K3->IO19, K4->IO18  (all to GND as common ground)

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "VESC_API.h"

#define OLED_SDA 8
#define OLED_SCL 9
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// ---- Buttons (active-LOW) ----
const int PIN_K1 = 1;   // Battery page
const int PIN_K2 = 3;   // Currents page
const int PIN_K3 = 19;  // Motor Temp page
const int PIN_K4 = 18;  // Energy page

// ---- Battery calibration ----
const float V_FULL  = 25.2f;   // volts at 100%
const float V_EMPTY = 18.6f;   // volts at 0%

// Smoothing to reduce flicker
float vin_filtered = 0.0f;
const float ALPHA = 0.25f; // 0..1 (higher = snappier)

int page = 0; // Page index: 0=Battery, 1=Currents, 2=Motor Temp, 3=Energy

void showTitle(const char* t) {
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(t);
}

void drawBattery() {
  showTitle("Battery");

  float vin = vesc.getVoltage();

  // Smooth voltage (first-run init)
  if (vin_filtered == 0.0f) vin_filtered = vin;
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
}

void drawCurrents() {
  showTitle("Currents");

  float iMotor = vesc.getMotorCurrent();
  float iBatt  = vesc.getBatteryCurrent();

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Motor: ");
  display.print(iMotor, 2);
  display.println(" A");

  display.setCursor(0, 36);
  display.print("Battery: ");
  display.print(iBatt, 2);
  display.println(" A");
}

void drawTemp() {
  showTitle("Temperature");

  float tMotor = vesc.getMotorTemp();
  float tFET   = vesc.getFETTemp();  

  // Big motor temp
  display.setTextSize(1);
  display.setCursor(0, 25);
  display.print("Motor Temp: ");
  display.print(tMotor, 2);
  display.println(" C");

  // Small FET temp line (fits above your footer)
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.print("FET Temp: ");
  display.print(tFET, 2);
  display.println(" C");
}

void drawEnergy() {
  showTitle("Energy");

  float ah = vesc.getAmpHours();   // consumed
  float wh = vesc.getWattHours();  // consumed

  display.setTextSize(1);
  display.setCursor(0, 20);
  display.print("Amp-hours:  ");
  display.print(ah, 2);

  display.setCursor(0, 36);
  display.print("Watt-hours: ");
  display.print(wh, 2);
}

void setup() {
  Serial.begin(115200);

  // Display init
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    while (1) {} // OLED not found
  }
  display.setTextColor(SSD1306_WHITE);

  // Buttons
  pinMode(PIN_K1, INPUT_PULLUP);
  pinMode(PIN_K2, INPUT_PULLUP);
  pinMode(PIN_K3, INPUT_PULLUP);
  pinMode(PIN_K4, INPUT_PULLUP);

  // VESC
  if (!vesc.init()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("VESC init failed");
    display.display();
    while (1) {}
  }
}

void loop() {
  vesc.update();

  // Select page by button press (active-LOW)
  if (digitalRead(PIN_K1) == LOW) page = 0; // Battery
  if (digitalRead(PIN_K2) == LOW) page = 1; // Currents
  if (digitalRead(PIN_K3) == LOW) page = 2; // Motor Temp
  if (digitalRead(PIN_K4) == LOW) page = 3; // Energy

  display.clearDisplay();

  // If VESC not connected, show hint
  if (!vesc.isConnected()) {
    display.setCursor(0, 24);
    display.println("VESC is off");
    display.display();
    delay(150);
    return;
  } 

  // Render selected page with simple if/else chain
  if (page == 0) {
    drawBattery();
  } else if (page == 1) {
    drawCurrents();
  } else if (page == 2) {
    drawTemp();
  } else if (page == 3) {
    drawEnergy();
  }

  Serial.println("K1:Bat  K2:Curr  K3:Temp  K4:Energy");
  display.display();
  delay(200); // gentle refresh + basic debounce feel
}
