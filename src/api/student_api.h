#pragma once

#include "../protocol/vesc_protocol.h"

extern LiveData vescData;

float getRPM();
float getDuty();
float getMotorCurrent();
float getVoltage();
float getMotorTemp();
float getFETTemp();
float getAmpHours();
float getAmpHoursCharged();
float getWattHours();
float getWattHoursCharged();
float getCurrentIn();
float getPIDPosition();
float getTachoValue();
float getADC1();
float getADC2();
float getADC3();
float getPPM();

void setDutyCycle(float duty);
void setCurrent(float amps);
void setCurrentBrake(float amps);
IMUData getImuData();

bool isDataFresh(unsigned long max_age_ms = 100);
void initVESC();
void updateVESC();