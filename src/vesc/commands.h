#pragma once

#include <stdint.h>
#include <functional>
#include "../protocol/vesc_protocol.h"

struct PendingCommand {
  uint32_t id;
  unsigned long sent_time;
  unsigned long timeout_ms;
  VESCCommand command_type;
  std::function<void(uint8_t*, uint8_t)> callback;
  bool active;
  bool retry_attempted;
};

class CommandRegistry {
private:
  static constexpr uint8_t MAX_PENDING_COMMANDS = 10;
  static constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;
  
  PendingCommand pending_commands[MAX_PENDING_COMMANDS];
  uint32_t next_command_id;
  
public:
  CommandRegistry();
  
  uint32_t sendCommand(VESCCommand cmd, uint8_t* data, uint8_t len, 
                      std::function<void(uint8_t*, uint8_t)> callback = nullptr,
                      uint32_t timeout_ms = DEFAULT_TIMEOUT_MS);
  
  bool handleResponse(uint32_t id, uint8_t* data, uint8_t len);
  void processTimeouts();
  void cleanup();
  
private:
  int findFreeSlot();
  int findPendingCommand(uint32_t id);
  void removeCommand(int index);
};

class VESCCommands {
private:
  static CommandRegistry registry;
  
public:
  static bool sendSetDutyCycle(float duty, std::function<void(uint8_t*, uint8_t)> callback = nullptr);
  static bool sendSetCurrent(float current, std::function<void(uint8_t*, uint8_t)> callback = nullptr);
  static bool sendSetCurrentBrake(float current, std::function<void(uint8_t*, uint8_t)> callback = nullptr);
  static bool sendGetIMU(std::function<void(uint8_t*, uint8_t)> callback = nullptr);
  
  static bool handleResponse(uint32_t id, uint8_t* data, uint8_t len);
  static void processTimeouts();
  static void cleanup();
};

extern CommandRegistry commandRegistry;