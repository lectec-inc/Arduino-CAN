#include "VESC_API.h"

// Global VESC instance
VESC_API vesc;

// Constructor
VESC_API::VESC_API() : can(PIN_CS) {
  memset(&data, 0, sizeof(data));
  data.data_valid = false;
}

// Initialize VESC CAN system
bool VESC_API::init() {
  Serial.println("Initializing VESC CAN system...");
  
  // Initialize SPI
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  
  // Initialize CAN
  if (can.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) != CAN_OK) {
    Serial.println("ERROR: CAN initialization failed!");
    return false;
  }
  
  can.setMode(MCP_NORMAL);
  pinMode(PIN_INT, INPUT_PULLUP);
  
  Serial.println("VESC CAN system ready!");
  return true;
}

// Update function - call this in loop()
void VESC_API::update() {
  // Process all available CAN messages
  while (!digitalRead(PIN_INT)) {
    uint32_t id;
    uint8_t len;
    uint8_t msg_data[8];
    
    if (can.readMsgBuf(&id, &len, msg_data) == CAN_OK) {
      parseVESCMessage(id, len, msg_data);
    }
  }
}

// Student-friendly data reading functions
float VESC_API::getRPM() {
  return data.rpm;
}

float VESC_API::getDuty() {
  return data.duty_cycle * 100.0f; // Convert to percentage
}

float VESC_API::getMotorCurrent() {
  return data.motor_current;
}

float VESC_API::getBatteryCurrent() {
  return data.input_current;
}

float VESC_API::getVoltage() {
  return data.input_voltage;
}

float VESC_API::getFETTemp() {
  return data.fet_temp;
}

float VESC_API::getMotorTemp() {
  return data.motor_temp;
}

float VESC_API::getAmpHours() {
  return data.amp_hours;
}

float VESC_API::getWattHours() {
  return data.watt_hours;
}

// System status functions
bool VESC_API::isConnected() {
  return data.data_valid && (millis() - data.last_update) < 1000;
}

unsigned long VESC_API::getLastUpdate() {
  return data.last_update;
}

// Command functions
void VESC_API::setDutyCycle(float duty) {
  // Clamp duty cycle to valid range (-100% to 100%)
  duty = constrain(duty, -100.0f, 100.0f);
  
  // Convert to VESC format: duty cycle * 100000 (e.g., 10% = 10000)
  int32_t duty_vesc = (int32_t)(duty * 100000.0f);
  
  uint8_t cmd_data[4];
  cmd_data[0] = (duty_vesc >> 24) & 0xFF;
  cmd_data[1] = (duty_vesc >> 16) & 0xFF;
  cmd_data[2] = (duty_vesc >> 8) & 0xFF;
  cmd_data[3] = duty_vesc & 0xFF;
  
  sendCommand(getCommandID(CMD_SET_DUTY), cmd_data, 4);
}

void VESC_API::setCurrent(float current) {
  // Convert to VESC format (multiply by 1000)
  int32_t current_vesc = (int32_t)(current * 1000.0f);
  
  uint8_t cmd_data[4];
  cmd_data[0] = (current_vesc >> 24) & 0xFF;
  cmd_data[1] = (current_vesc >> 16) & 0xFF;
  cmd_data[2] = (current_vesc >> 8) & 0xFF;
  cmd_data[3] = current_vesc & 0xFF;
  
  sendCommand(getCommandID(CMD_SET_CURRENT), cmd_data, 4);
}

void VESC_API::setCurrentBrake(float current) {
  // Convert to VESC format (multiply by 1000)
  int32_t current_vesc = (int32_t)(current * 1000.0f);
  
  uint8_t cmd_data[4];
  cmd_data[0] = (current_vesc >> 24) & 0xFF;
  cmd_data[1] = (current_vesc >> 16) & 0xFF;
  cmd_data[2] = (current_vesc >> 8) & 0xFF;
  cmd_data[3] = current_vesc & 0xFF;
  
  sendCommand(getCommandID(CMD_SET_CURRENT_BRAKE), cmd_data, 4);
}

void VESC_API::setBrake(float brake) {
  // Convert percentage to current and call setCurrentBrake
  setCurrentBrake(brake * 0.5f); // Simple conversion - adjust as needed
}

void VESC_API::setRPM(float rpm) {
  int32_t rpm_vesc = (int32_t)rpm;
  
  uint8_t cmd_data[4];
  cmd_data[0] = (rpm_vesc >> 24) & 0xFF;
  cmd_data[1] = (rpm_vesc >> 16) & 0xFF;
  cmd_data[2] = (rpm_vesc >> 8) & 0xFF;
  cmd_data[3] = rpm_vesc & 0xFF;
  
  sendCommand(getCommandID(CMD_SET_RPM), cmd_data, 4);
}

// Display functions
void VESC_API::printStatus() {
  unsigned long data_age = millis() - data.last_update;
  
  Serial.print(millis() / 1000.0, 1);
  Serial.print("s ");
  
  // Check if data is fresh (within last 1 second)
  if (!data.data_valid || data_age > 1000) {
    Serial.println("Status: NO DATA - VESC disconnected or not responding");
    return;
  }
  
  // Check if data is getting stale (within last 500ms but show warning)
  if (data_age > 500) {
    Serial.print("Status: STALE DATA (");
    Serial.print(data_age);
    Serial.print("ms old) | ");
  } else {
    Serial.print("✅ ");
  }
  
  Serial.print("Voltage: ");
  Serial.print(data.input_voltage, 2);
  Serial.print("V | RPM: ");
  Serial.print((int)data.rpm);
  Serial.print(" | Duty: ");
  Serial.print(data.duty_cycle * 100, 1);
  Serial.print("% | Motor Current: ");
  Serial.print(data.motor_current, 2);
  Serial.print("A | Battery Current: ");
  Serial.print(data.input_current, 2);
  Serial.print("A | FET Temp: ");
  Serial.print(data.fet_temp, 1);
  Serial.print("C | Amp Hours: ");
  Serial.print(data.amp_hours, 3);
  Serial.println("Ah");
}

void VESC_API::printDebug() {
  Serial.println("=== VESC Debug Info ===");
  Serial.print("Connected: ");
  Serial.println(isConnected() ? "YES" : "NO");
  Serial.print("Last Update: ");
  Serial.print(data.last_update);
  Serial.print(" (");
  Serial.print(millis() - data.last_update);
  Serial.println("ms ago)");
  Serial.print("Message Count: ");
  Serial.println(data.message_count);
  Serial.print("Data Valid: ");
  Serial.println(data.data_valid ? "YES" : "NO");
  Serial.println("========================");
}

// Internal helper functions
bool VESC_API::parseVESCMessage(uint32_t id, uint8_t len, uint8_t* msg_data) {
  if (!isStatusMessage(id)) {
    return false;
  }
  
  switch (id) {
    case STATUS_1: parseStatus1(msg_data); break;
    case STATUS_2: parseStatus2(msg_data); break;
    case STATUS_3: parseStatus3(msg_data); break;
    case STATUS_4: parseStatus4(msg_data); break;
    case STATUS_5: parseStatus5(msg_data); break;
    case STATUS_6: parseStatus6(msg_data); break;
    default: return false;
  }
  
  data.last_update = millis();
  data.data_valid = true;
  data.message_count++;
  return true;
}

void VESC_API::parseStatus1(uint8_t* msg_data) {
  int32_t index = 0;
  data.rpm = buffer_get_int32(msg_data, &index);
  data.motor_current = buffer_get_int16(msg_data, &index) / 10.0f;
  data.duty_cycle = buffer_get_int16(msg_data, &index) / 1000.0f;
}

void VESC_API::parseStatus2(uint8_t* msg_data) {
  int32_t index = 0;
  data.amp_hours = buffer_get_int32(msg_data, &index) / 10000.0f;
  data.amp_hours_charged = buffer_get_int32(msg_data, &index) / 10000.0f;
}

void VESC_API::parseStatus3(uint8_t* msg_data) {
  int32_t index = 0;
  data.watt_hours = buffer_get_int32(msg_data, &index) / 10000.0f;
  data.watt_hours_charged = buffer_get_int32(msg_data, &index) / 10000.0f;
}

void VESC_API::parseStatus4(uint8_t* msg_data) {
  int32_t index = 0;
  data.fet_temp = buffer_get_int16(msg_data, &index) / 10.0f;
  data.motor_temp = buffer_get_int16(msg_data, &index) / 10.0f;
  data.input_current = buffer_get_int16(msg_data, &index) / 10.0f;
  data.pid_position = buffer_get_int16(msg_data, &index) / 50.0f;
}

void VESC_API::parseStatus5(uint8_t* msg_data) {
  int32_t index = 0;
  data.tacho_value = buffer_get_int32(msg_data, &index);
  data.input_voltage = buffer_get_int16(msg_data, &index) / 10.0f;
}

void VESC_API::parseStatus6(uint8_t* msg_data) {
  int32_t index = 0;
  data.adc1 = buffer_get_int16(msg_data, &index) / 1000.0f;
  data.adc2 = buffer_get_int16(msg_data, &index) / 1000.0f;
  data.adc3 = buffer_get_int16(msg_data, &index) / 1000.0f;
  data.ppm = buffer_get_int16(msg_data, &index) / 1000.0f;
}

bool VESC_API::isStatusMessage(uint32_t id) {
  return (id == STATUS_1 || id == STATUS_2 || id == STATUS_3 || 
          id == STATUS_4 || id == STATUS_5 || id == STATUS_6);
}

int32_t VESC_API::buffer_get_int32(const uint8_t* buffer, int32_t* index) {
  int32_t res = ((uint32_t)buffer[*index]) << 24 |
                ((uint32_t)buffer[*index + 1]) << 16 |
                ((uint32_t)buffer[*index + 2]) << 8 |
                ((uint32_t)buffer[*index + 3]);
  *index += 4;
  return res;
}

int16_t VESC_API::buffer_get_int16(const uint8_t* buffer, int32_t* index) {
  int16_t res = ((uint16_t)buffer[*index]) << 8 |
                ((uint16_t)buffer[*index + 1]);
  *index += 2;
  return res;
}

void VESC_API::sendCommand(uint32_t id, uint8_t* cmd_data, uint8_t len) {
  can.sendMsgBuf(id, 1, len, cmd_data); // 1 = extended frame
}

uint32_t VESC_API::getCommandID(VESCCommandID cmd_id) {
  // VESC command format: (command_id << 8) | vesc_id
  return ((uint32_t)cmd_id << 8) | VESC_ID;
}