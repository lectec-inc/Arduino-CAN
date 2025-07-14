#pragma once

#include <mcp_can.h>
#include <SPI.h>

constexpr uint8_t PIN_SCK  = 6;
constexpr uint8_t PIN_MISO = 2;
constexpr uint8_t PIN_MOSI = 7;
constexpr uint8_t PIN_CS   = 10;
constexpr uint8_t PIN_INT  = 4;

constexpr uint32_t VESC_ID = 74;
constexpr uint32_t ESP32_ID = 2;

struct CANMessage {
  uint32_t id;
  uint8_t len;
  uint8_t data[8];
};

class CANInterface {
private:
  MCP_CAN can;
  
public:
  CANInterface();
  bool init();
  bool hasMessage();
  bool readMessage(CANMessage& msg);
  bool sendMessage(uint32_t id, uint8_t len, uint8_t* data);
  void setMode(uint8_t mode);
};

extern CANInterface canInterface;