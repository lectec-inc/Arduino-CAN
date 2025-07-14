#pragma once

#include <stdint.h>

enum VESCStatusMessage {
  STATUS_1 = 0x00000900,
  STATUS_2 = 0x00000E00,
  STATUS_3 = 0x00000F00,
  STATUS_4 = 0x00001000,
  STATUS_5 = 0x00001B00,
  STATUS_6 = 0x00001C00
};

enum VESCCommand {
  CMD_SET_DUTY = 0x00000000,
  CMD_SET_CURRENT = 0x00000100,
  CMD_SET_CURRENT_BRAKE = 0x00000200,
  CMD_GET_IMU = 0x00001F00
};

struct VESCStatus1 {
  int32_t rpm;
  int16_t current;
  int16_t duty;
};

struct VESCStatus2 {
  int32_t amp_hours;
  int32_t amp_hours_charged;
};

struct VESCStatus3 {
  int32_t watt_hours;
  int32_t watt_hours_charged;
};

struct VESCStatus4 {
  int16_t temp_fet;
  int16_t temp_motor;
  int16_t current_in;
  int16_t pid_pos_now;
};

struct VESCStatus5 {
  int32_t tacho_value;
  int16_t voltage_in;
};

struct VESCStatus6 {
  int16_t adc1;
  int16_t adc2;
  int16_t adc3;
  int16_t ppm;
};

struct IMUData {
  float acc_x;
  float acc_y;
  float acc_z;
  float gyro_x;
  float gyro_y;
  float gyro_z;
};

struct LiveData {
  float rpm;
  float current;
  float duty;
  float amp_hours;
  float amp_hours_charged;
  float watt_hours;
  float watt_hours_charged;
  float temp_fet;
  float temp_motor;
  float current_in;
  float pid_pos_now;
  float tacho_value;
  float voltage_in;
  float adc1;
  float adc2;
  float adc3;
  float ppm;
  unsigned long last_update;
};

class VESCProtocol {
public:
  static bool parseStatusMessage(uint32_t id, uint8_t len, uint8_t* data, LiveData& liveData);
  static bool parseIMUData(uint8_t* data, IMUData& imuData);
  static void encodeSetDuty(float duty, uint8_t* data);
  static void encodeSetCurrent(float current, uint8_t* data);
  static void encodeSetCurrentBrake(float current, uint8_t* data);
  static void encodeGetIMU(uint8_t* data);
  static uint32_t getCommandID(VESCCommand cmd);
  static bool isStatusMessage(uint32_t id);
  static bool isCommandResponse(uint32_t id);
  
private:
  static void parseStatus1(uint8_t* data, LiveData& liveData);
  static void parseStatus2(uint8_t* data, LiveData& liveData);
  static void parseStatus3(uint8_t* data, LiveData& liveData);
  static void parseStatus4(uint8_t* data, LiveData& liveData);
  static void parseStatus5(uint8_t* data, LiveData& liveData);
  static void parseStatus6(uint8_t* data, LiveData& liveData);
  
  static int32_t buffer_get_int32(const uint8_t* buffer, int32_t* index);
  static int16_t buffer_get_int16(const uint8_t* buffer, int32_t* index);
  static void buffer_append_int32(uint8_t* buffer, int32_t number, int32_t* index);
  static void buffer_append_int16(uint8_t* buffer, int16_t number, int32_t* index);
};