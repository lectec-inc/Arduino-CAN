#include "commands.h"
#include "../can/can_interface.h"
#include <Arduino.h>

CommandRegistry commandRegistry;
CommandRegistry VESCCommands::registry;

CommandRegistry::CommandRegistry() : next_command_id(1) {
  for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
    pending_commands[i].active = false;
  }
}

uint32_t CommandRegistry::sendCommand(VESCCommand cmd, uint8_t* data, uint8_t len, 
                                    std::function<void(uint8_t*, uint8_t)> callback,
                                    uint32_t timeout_ms) {
  int slot = findFreeSlot();
  if (slot == -1) {
    return 0;
  }
  
  uint32_t cmd_id = next_command_id++;
  uint32_t can_id = VESCProtocol::getCommandID(cmd) | (cmd_id & 0xFF);
  
  pending_commands[slot].id = cmd_id;
  pending_commands[slot].sent_time = millis();
  pending_commands[slot].timeout_ms = timeout_ms;
  pending_commands[slot].command_type = cmd;
  pending_commands[slot].callback = callback;
  pending_commands[slot].active = true;
  pending_commands[slot].retry_attempted = false;
  
  if (!canInterface.sendMessage(can_id, len, data)) {
    pending_commands[slot].active = false;
    return 0;
  }
  
  return cmd_id;
}

bool CommandRegistry::handleResponse(uint32_t id, uint8_t* data, uint8_t len) {
  uint32_t cmd_id = id & 0xFF;
  int slot = findPendingCommand(cmd_id);
  
  if (slot == -1) {
    return false;
  }
  
  if (pending_commands[slot].callback) {
    pending_commands[slot].callback(data, len);
  }
  
  removeCommand(slot);
  return true;
}

void CommandRegistry::processTimeouts() {
  unsigned long current_time = millis();
  
  for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
    if (!pending_commands[i].active) continue;
    
    if (current_time - pending_commands[i].sent_time > pending_commands[i].timeout_ms) {
      if (!pending_commands[i].retry_attempted) {
        pending_commands[i].retry_attempted = true;
        pending_commands[i].sent_time = current_time;
      } else {
        removeCommand(i);
      }
    }
  }
}

void CommandRegistry::cleanup() {
  for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
    pending_commands[i].active = false;
  }
}

int CommandRegistry::findFreeSlot() {
  for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
    if (!pending_commands[i].active) {
      return i;
    }
  }
  return -1;
}

int CommandRegistry::findPendingCommand(uint32_t id) {
  for (int i = 0; i < MAX_PENDING_COMMANDS; i++) {
    if (pending_commands[i].active && pending_commands[i].id == id) {
      return i;
    }
  }
  return -1;
}

void CommandRegistry::removeCommand(int index) {
  if (index >= 0 && index < MAX_PENDING_COMMANDS) {
    pending_commands[index].active = false;
  }
}

bool VESCCommands::sendSetDutyCycle(float duty, std::function<void(uint8_t*, uint8_t)> callback) {
  uint8_t data[8];
  VESCProtocol::encodeSetDuty(duty, data);
  return registry.sendCommand(CMD_SET_DUTY, data, 4, callback) != 0;
}

bool VESCCommands::sendSetCurrent(float current, std::function<void(uint8_t*, uint8_t)> callback) {
  uint8_t data[8];
  VESCProtocol::encodeSetCurrent(current, data);
  return registry.sendCommand(CMD_SET_CURRENT, data, 4, callback) != 0;
}

bool VESCCommands::sendSetCurrentBrake(float current, std::function<void(uint8_t*, uint8_t)> callback) {
  uint8_t data[8];
  VESCProtocol::encodeSetCurrentBrake(current, data);
  return registry.sendCommand(CMD_SET_CURRENT_BRAKE, data, 4, callback) != 0;
}

bool VESCCommands::sendGetIMU(std::function<void(uint8_t*, uint8_t)> callback) {
  uint8_t data[8];
  VESCProtocol::encodeGetIMU(data);
  return registry.sendCommand(CMD_GET_IMU, data, 0, callback) != 0;
}

bool VESCCommands::handleResponse(uint32_t id, uint8_t* data, uint8_t len) {
  return registry.handleResponse(id, data, len);
}

void VESCCommands::processTimeouts() {
  registry.processTimeouts();
}

void VESCCommands::cleanup() {
  registry.cleanup();
}