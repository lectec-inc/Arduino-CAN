#include "can/can_interface.h"
#include "protocol/vesc_protocol.h"
#include "vesc/commands.h"
#include "api/student_api.h"

void setup() {
  Serial.begin(115200);
  delay(300);
  
  Serial.println("Initializing VESC CAN System...");
  
  if (!canInterface.init()) {
    Serial.println("CAN initialization failed!");
    while (1) delay(1000);
  }
  
  initVESC();
  Serial.println("VESC system ready!");
}

void loop() {
  CANMessage msg;
  
  while (canInterface.hasMessage()) {
    if (canInterface.readMessage(msg)) {
      if (VESCProtocol::isStatusMessage(msg.id)) {
        VESCProtocol::parseStatusMessage(msg.id, msg.len, msg.data, vescData);
      } else if (VESCProtocol::isCommandResponse(msg.id)) {
        VESCCommands::handleResponse(msg.id, msg.data, msg.len);
      }
    }
  }
  
  updateVESC();
  
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("RPM: ");
    Serial.print(getRPM());
    Serial.print(" | Duty: ");
    Serial.print(getDuty());
    Serial.print(" | Voltage: ");
    Serial.print(getVoltage());
    Serial.print(" | Motor Temp: ");
    Serial.println(getMotorTemp());
    lastPrint = millis();
  }
  
  delay(1);
}