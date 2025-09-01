#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

// Hardware Configuration
constexpr uint8_t PIN_SCK  = 6;
constexpr uint8_t PIN_MISO = 2;
constexpr uint8_t PIN_MOSI = 7;
constexpr uint8_t PIN_CS   = 10;
constexpr uint8_t PIN_INT  = 4;

// VESC Configuration
constexpr uint8_t VESC_ID = 74;  // Default VESC ID (0x4A)

// VESC CAN Message IDs
enum VESCStatusMessage {
  STATUS_1 = 0x8000094A,  // RPM, Current, Duty
  STATUS_2 = 0x80000E4A,  // Amp Hours
  STATUS_3 = 0x80000F4A,  // Watt Hours  
  STATUS_4 = 0x8000104A,  // Temperatures, Current In
  STATUS_5 = 0x80001B4A,  // Tacho, Voltage
  STATUS_6 = 0x80001C4A   // ADC values
};

// VESC Command IDs (as per VESC protocol)
enum VESCCommandID {
  CMD_SET_DUTY = 0,        // Set duty cycle
  CMD_SET_CURRENT = 1,     // Set motor current
  CMD_SET_CURRENT_BRAKE = 2, // Set brake current
  CMD_SET_RPM = 3,         // Set RPM
  CMD_SET_POS = 4          // Set position
};

// VESC Data Structure
struct VESCData {
  // Motor data
  float rpm;
  float duty_cycle;
  float motor_current;
  float input_current;
  
  // Power data
  float input_voltage;
  float amp_hours;
  float amp_hours_charged;
  float watt_hours;
  float watt_hours_charged;
  
  // Temperature data
  float fet_temp;
  float motor_temp;
  
  // Position data
  float pid_position;
  float tacho_value;
  
  // ADC inputs
  float adc1;
  float adc2;
  float adc3;
  float ppm;
  
  // System info
  unsigned long last_update;
  unsigned long message_count;
  bool data_valid;
};

// VESC API Class
class VESC_API {
public:
  // Constructor
  VESC_API();
  
  // Initialization
  bool init();
  
  // Data Reading Functions (for students)
  float getRPM();
  float getDuty();            // Returns duty cycle as percentage (0-100)
  float getMotorCurrent();    // Returns motor current in Amps
  float getBatteryCurrent();  // Returns battery current in Amps
  float getVoltage();         // Returns input voltage in Volts
  float getFETTemp();         // Returns FET temperature in Celsius
  float getMotorTemp();       // Returns motor temperature in Celsius
  float getAmpHours();        // Returns consumed amp hours
  float getWattHours();       // Returns consumed watt hours
  
  // Command Functions (for students)
  void setDutyCycle(float duty);    // Set duty cycle (-100 to 100)
  void setCurrent(float current);   // Set motor current in Amps
  void setCurrentBrake(float current); // Set brake current in Amps
  void setBrake(float brake);       // Set brake (0-100)
  void setRPM(float rpm);           // Set RPM
  
  // System Functions
  void update();              // Call this in loop() to process CAN messages
  bool isConnected();         // Returns true if VESC is responding
  unsigned long getLastUpdate(); // Returns time of last VESC message
  
  // Debug Functions
  void printStatus();         // Print all telemetry data
  void printDebug();          // Print debug information
  
private:
  MCP_CAN can;
  VESCData data;
  
  // Internal helper functions
  bool parseVESCMessage(uint32_t id, uint8_t len, uint8_t* msg_data);
  void parseStatus1(uint8_t* msg_data);
  void parseStatus2(uint8_t* msg_data);
  void parseStatus3(uint8_t* msg_data);
  void parseStatus4(uint8_t* msg_data);
  void parseStatus5(uint8_t* msg_data);
  void parseStatus6(uint8_t* msg_data);
  bool isStatusMessage(uint32_t id);
  
  // Utility functions
  int32_t buffer_get_int32(const uint8_t* buffer, int32_t* index);
  int16_t buffer_get_int16(const uint8_t* buffer, int32_t* index);
  
  // Command sending
  void sendCommand(uint32_t id, uint8_t* data, uint8_t len);
  uint32_t getCommandID(VESCCommandID cmd_id);
};

// Global VESC instance for easy access
extern VESC_API vesc;