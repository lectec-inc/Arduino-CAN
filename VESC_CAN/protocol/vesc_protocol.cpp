#include "vesc_protocol.h"
#include <Arduino.h>

bool VESCProtocol::parseStatusMessage(uint32_t id, uint8_t len, uint8_t* data, LiveData& liveData) {
  if (!isStatusMessage(id)) {
    return false;
  }
  
  switch (id) {
    case STATUS_1:
      parseStatus1(data, liveData);
      break;
    case STATUS_2:
      parseStatus2(data, liveData);
      break;
    case STATUS_3:
      parseStatus3(data, liveData);
      break;
    case STATUS_4:
      parseStatus4(data, liveData);
      break;
    case STATUS_5:
      parseStatus5(data, liveData);
      break;
    case STATUS_6:
      parseStatus6(data, liveData);
      break;
    default:
      return false;
  }
  
  liveData.last_update = millis();
  return true;
}

void VESCProtocol::parseStatus1(uint8_t* data, LiveData& liveData) {
  int32_t index = 0;
  liveData.rpm = buffer_get_int32(data, &index);
  liveData.current = buffer_get_int16(data, &index) / 10.0f;
  liveData.duty = buffer_get_int16(data, &index) / 1000.0f;
}

void VESCProtocol::parseStatus2(uint8_t* data, LiveData& liveData) {
  int32_t index = 0;
  liveData.amp_hours = buffer_get_int32(data, &index) / 10000.0f;
  liveData.amp_hours_charged = buffer_get_int32(data, &index) / 10000.0f;
}

void VESCProtocol::parseStatus3(uint8_t* data, LiveData& liveData) {
  int32_t index = 0;
  liveData.watt_hours = buffer_get_int32(data, &index) / 10000.0f;
  liveData.watt_hours_charged = buffer_get_int32(data, &index) / 10000.0f;
}

void VESCProtocol::parseStatus4(uint8_t* data, LiveData& liveData) {
  int32_t index = 0;
  liveData.temp_fet = buffer_get_int16(data, &index) / 10.0f;
  liveData.temp_motor = buffer_get_int16(data, &index) / 10.0f;
  liveData.current_in = buffer_get_int16(data, &index) / 10.0f;
  liveData.pid_pos_now = buffer_get_int16(data, &index) / 50.0f;
}

void VESCProtocol::parseStatus5(uint8_t* data, LiveData& liveData) {
  int32_t index = 0;
  liveData.tacho_value = buffer_get_int32(data, &index);
  liveData.voltage_in = buffer_get_int16(data, &index) / 10.0f;
}

void VESCProtocol::parseStatus6(uint8_t* data, LiveData& liveData) {
  int32_t index = 0;
  liveData.adc1 = buffer_get_int16(data, &index) / 1000.0f;
  liveData.adc2 = buffer_get_int16(data, &index) / 1000.0f;
  liveData.adc3 = buffer_get_int16(data, &index) / 1000.0f;
  liveData.ppm = buffer_get_int16(data, &index) / 1000.0f;
}

bool VESCProtocol::parseIMUData(uint8_t* data, IMUData& imuData) {
  int32_t index = 0;
  imuData.acc_x = buffer_get_int16(data, &index) / 1000.0f;
  imuData.acc_y = buffer_get_int16(data, &index) / 1000.0f;
  imuData.acc_z = buffer_get_int16(data, &index) / 1000.0f;
  imuData.gyro_x = buffer_get_int16(data, &index) / 1000.0f;
  imuData.gyro_y = buffer_get_int16(data, &index) / 1000.0f;
  imuData.gyro_z = buffer_get_int16(data, &index) / 1000.0f;
  return true;
}

void VESCProtocol::encodeSetDuty(float duty, uint8_t* data) {
  int32_t index = 0;
  buffer_append_int32(data, (int32_t)(duty * 100000.0f), &index);
}

void VESCProtocol::encodeSetCurrent(float current, uint8_t* data) {
  int32_t index = 0;
  buffer_append_int32(data, (int32_t)(current * 1000.0f), &index);
}

void VESCProtocol::encodeSetCurrentBrake(float current, uint8_t* data) {
  int32_t index = 0;
  buffer_append_int32(data, (int32_t)(current * 1000.0f), &index);
}

void VESCProtocol::encodeGetIMU(uint8_t* data) {
  for (int i = 0; i < 8; i++) {
    data[i] = 0;
  }
}

uint32_t VESCProtocol::getCommandID(VESCCommand cmd) {
  return cmd | (74 << 8);
}

bool VESCProtocol::isStatusMessage(uint32_t id) {
  return (id == STATUS_1 || id == STATUS_2 || id == STATUS_3 || 
          id == STATUS_4 || id == STATUS_5 || id == STATUS_6);
}

bool VESCProtocol::isCommandResponse(uint32_t id) {
  return ((id & 0xFF00) == (74 << 8));
}

int32_t VESCProtocol::buffer_get_int32(const uint8_t* buffer, int32_t* index) {
  int32_t res = ((uint32_t)buffer[*index]) << 24 |
                ((uint32_t)buffer[*index + 1]) << 16 |
                ((uint32_t)buffer[*index + 2]) << 8 |
                ((uint32_t)buffer[*index + 3]);
  *index += 4;
  return res;
}

int16_t VESCProtocol::buffer_get_int16(const uint8_t* buffer, int32_t* index) {
  int16_t res = ((uint16_t)buffer[*index]) << 8 |
                ((uint16_t)buffer[*index + 1]);
  *index += 2;
  return res;
}

void VESCProtocol::buffer_append_int32(uint8_t* buffer, int32_t number, int32_t* index) {
  buffer[(*index)++] = number >> 24;
  buffer[(*index)++] = number >> 16;
  buffer[(*index)++] = number >> 8;
  buffer[(*index)++] = number;
}

void VESCProtocol::buffer_append_int16(uint8_t* buffer, int16_t number, int32_t* index) {
  buffer[(*index)++] = number >> 8;
  buffer[(*index)++] = number;
}