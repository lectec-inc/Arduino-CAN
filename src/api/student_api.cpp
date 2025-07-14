#include "student_api.h"
#include "../vesc/commands.h"
#include <Arduino.h>

LiveData vescData;
IMUData lastImuData;
bool imuDataValid = false;
unsigned long lastImuUpdate = 0;

void onIMUResponse(uint8_t* data, uint8_t len) {
  VESCProtocol::parseIMUData(data, lastImuData);
  imuDataValid = true;
  lastImuUpdate = millis();
}

float getRPM() {
  return vescData.rpm;
}

float getDuty() {
  return vescData.duty;
}

float getMotorCurrent() {
  return vescData.current;
}

float getVoltage() {
  return vescData.voltage_in;
}

float getMotorTemp() {
  return vescData.temp_motor;
}

float getFETTemp() {
  return vescData.temp_fet;
}

float getAmpHours() {
  return vescData.amp_hours;
}

float getAmpHoursCharged() {
  return vescData.amp_hours_charged;
}

float getWattHours() {
  return vescData.watt_hours;
}

float getWattHoursCharged() {
  return vescData.watt_hours_charged;
}

float getCurrentIn() {
  return vescData.current_in;
}

float getPIDPosition() {
  return vescData.pid_pos_now;
}

float getTachoValue() {
  return vescData.tacho_value;
}

float getADC1() {
  return vescData.adc1;
}

float getADC2() {
  return vescData.adc2;
}

float getADC3() {
  return vescData.adc3;
}

float getPPM() {
  return vescData.ppm;
}

void setDutyCycle(float duty) {
  if (duty < -1.0f) duty = -1.0f;
  if (duty > 1.0f) duty = 1.0f;
  VESCCommands::sendSetDutyCycle(duty);
}

void setCurrent(float amps) {
  VESCCommands::sendSetCurrent(amps);
}

void setCurrentBrake(float amps) {
  VESCCommands::sendSetCurrentBrake(amps);
}

IMUData getImuData() {
  if (!imuDataValid || (millis() - lastImuUpdate > 5000)) {
    VESCCommands::sendGetIMU(onIMUResponse);
  }
  return lastImuData;
}

bool isDataFresh(unsigned long max_age_ms) {
  return (millis() - vescData.last_update) < max_age_ms;
}

void initVESC() {
  vescData.last_update = 0;
  imuDataValid = false;
  lastImuUpdate = 0;
}

void updateVESC() {
  VESCCommands::processTimeouts();
}