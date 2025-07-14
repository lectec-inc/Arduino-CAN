#include "can_interface.h"

CANInterface canInterface;

CANInterface::CANInterface() : can(PIN_CS) {
}

bool CANInterface::init() {
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  
  if (can.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) != CAN_OK) {
    return false;
  }
  
  can.setMode(MCP_NORMAL);
  pinMode(PIN_INT, INPUT_PULLUP);
  
  return true;
}

bool CANInterface::hasMessage() {
  return !digitalRead(PIN_INT);
}

bool CANInterface::readMessage(CANMessage& msg) {
  if (!hasMessage()) {
    return false;
  }
  
  return can.readMsgBuf(&msg.id, &msg.len, msg.data) == CAN_OK;
}

bool CANInterface::sendMessage(uint32_t id, uint8_t len, uint8_t* data) {
  return can.sendMsgBuf(id, 0, len, data) == CAN_OK;
}

void CANInterface::setMode(uint8_t mode) {
  can.setMode(mode);
}