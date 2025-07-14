#include <SPI.h>
#include <mcp_can.h>

// ═══════════════════════════════════════════════════════════════════════════════
// 🔧 CONFIGURATION
// ═══════════════════════════════════════════════════════════════════════════════
const float PRINT_RATE_HZ = 5.0;  // 🔄 ADJUST THIS: 1.0 to 20.0 Hz
const unsigned long PRINT_INTERVAL_MS = (unsigned long)(1000.0 / PRINT_RATE_HZ);

// Hardware pins
constexpr uint8_t PIN_SCK  = 6;
constexpr uint8_t PIN_MISO = 2;
constexpr uint8_t PIN_MOSI = 7;
constexpr uint8_t PIN_CS   = 10;
constexpr uint8_t PIN_INT  = 4;

// VESC message IDs (from actual testing)
enum VESCStatusMessage {
  STATUS_1 = 0x8000094A,  // RPM, Current, Duty
  STATUS_2 = 0x80000E4A,  // Amp Hours
  STATUS_3 = 0x80000F4A,  // Watt Hours  
  STATUS_4 = 0x8000104A,  // Temperatures, Current In
  STATUS_5 = 0x80001B4A,  // Tacho, Voltage
  STATUS_6 = 0x80001C4A   // ADC values
};

// ═══════════════════════════════════════════════════════════════════════════════
// 📊 DATA STRUCTURES
// ═══════════════════════════════════════════════════════════════════════════════
struct VESCTelemetry {
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

struct CANMessage {
  uint32_t id;
  uint8_t len;
  uint8_t data[8];
};

// ═══════════════════════════════════════════════════════════════════════════════
// 🌐 GLOBAL VARIABLES
// ═══════════════════════════════════════════════════════════════════════════════
MCP_CAN CAN(PIN_CS);
VESCTelemetry vesc;
unsigned long total_messages = 0;

// ═══════════════════════════════════════════════════════════════════════════════
// 🔧 UTILITY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
int32_t buffer_get_int32(const uint8_t* buffer, int32_t* index) {
  int32_t res = ((uint32_t)buffer[*index]) << 24 |
                ((uint32_t)buffer[*index + 1]) << 16 |
                ((uint32_t)buffer[*index + 2]) << 8 |
                ((uint32_t)buffer[*index + 3]);
  *index += 4;
  return res;
}

int16_t buffer_get_int16(const uint8_t* buffer, int32_t* index) {
  int16_t res = ((uint16_t)buffer[*index]) << 8 |
                ((uint16_t)buffer[*index + 1]);
  *index += 2;
  return res;
}

bool isStatusMessage(uint32_t id) {
  return (id == STATUS_1 || id == STATUS_2 || id == STATUS_3 || 
          id == STATUS_4 || id == STATUS_5 || id == STATUS_6);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 📨 MESSAGE PARSING
// ═══════════════════════════════════════════════════════════════════════════════
void parseStatus1(uint8_t* data) {
  int32_t index = 0;
  vesc.rpm = buffer_get_int32(data, &index);
  vesc.motor_current = buffer_get_int16(data, &index) / 10.0f;
  vesc.duty_cycle = buffer_get_int16(data, &index) / 1000.0f;
}

void parseStatus2(uint8_t* data) {
  int32_t index = 0;
  vesc.amp_hours = buffer_get_int32(data, &index) / 10000.0f;
  vesc.amp_hours_charged = buffer_get_int32(data, &index) / 10000.0f;
}

void parseStatus3(uint8_t* data) {
  int32_t index = 0;
  vesc.watt_hours = buffer_get_int32(data, &index) / 10000.0f;
  vesc.watt_hours_charged = buffer_get_int32(data, &index) / 10000.0f;
}

void parseStatus4(uint8_t* data) {
  int32_t index = 0;
  vesc.fet_temp = buffer_get_int16(data, &index) / 10.0f;
  vesc.motor_temp = buffer_get_int16(data, &index) / 10.0f;
  vesc.input_current = buffer_get_int16(data, &index) / 10.0f;
  vesc.pid_position = buffer_get_int16(data, &index) / 50.0f;
}

void parseStatus5(uint8_t* data) {
  int32_t index = 0;
  vesc.tacho_value = buffer_get_int32(data, &index);
  vesc.input_voltage = buffer_get_int16(data, &index) / 10.0f;
}

void parseStatus6(uint8_t* data) {
  int32_t index = 0;
  vesc.adc1 = buffer_get_int16(data, &index) / 1000.0f;
  vesc.adc2 = buffer_get_int16(data, &index) / 1000.0f;
  vesc.adc3 = buffer_get_int16(data, &index) / 1000.0f;
  vesc.ppm = buffer_get_int16(data, &index) / 1000.0f;
}

bool parseVESCMessage(uint32_t id, uint8_t len, uint8_t* data) {
  if (!isStatusMessage(id)) {
    return false;
  }
  
  switch (id) {
    case STATUS_1: parseStatus1(data); break;
    case STATUS_2: parseStatus2(data); break;
    case STATUS_3: parseStatus3(data); break;
    case STATUS_4: parseStatus4(data); break;
    case STATUS_5: parseStatus5(data); break;
    case STATUS_6: parseStatus6(data); break;
    default: return false;
  }
  
  vesc.last_update = millis();
  vesc.data_valid = true;
  vesc.message_count++;
  return true;
}

// ═══════════════════════════════════════════════════════════════════════════════
// 📺 DISPLAY FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
void printHeader() {
  Serial.println();
  Serial.println("╔══════════════════════════════════════════════════════════════════════════════╗");
  Serial.println("║                            🚗 VESC TELEMETRY MONITOR                         ║");
  Serial.println("╚══════════════════════════════════════════════════════════════════════════════╝");
  Serial.print("📡 Print Rate: ");
  Serial.print(PRINT_RATE_HZ, 1);
  Serial.println(" Hz");
  Serial.println();
}

void printSeparator() {
  Serial.println("├─────────────────────────────────────────────────────────────────────────────┤");
}

void printValue(const char* label, float value, const char* unit, int width = 8, int decimals = 2) {
  Serial.print("│ ");
  Serial.print(label);
  Serial.print(": ");
  
  // Pad the value field
  String valueStr = String(value, decimals) + unit;
  while (valueStr.length() < width) {
    valueStr = " " + valueStr;
  }
  Serial.print(valueStr);
}

void clearScreen() {
  // Clear screen and move cursor to top
  Serial.print("\033[2J");      // Clear entire screen
  Serial.print("\033[H");       // Move cursor to home position (top-left)
}

void printStatus() {
  unsigned long data_age = millis() - vesc.last_update;
  
  // Clear screen and redraw (comment out the clearScreen() line for scrolling mode)
  clearScreen();
  
  Serial.println("╔══════════════════════════════════════════════════════════════════════════════╗");
  Serial.println("║                         🚗 VESC TELEMETRY MONITOR                          ║");
  Serial.println("╚══════════════════════════════════════════════════════════════════════════════╝");
  
  Serial.println("┌─────────────────────────────────────────────────────────────────────────────┐");
  Serial.print("│ ⏰ Time: ");
  Serial.print(millis() / 1000.0, 1);
  Serial.print("s");
  
  Serial.print("    📊 Messages: ");
  Serial.print(total_messages);
  
  Serial.print("    🔄 VESC: ");
  Serial.print(vesc.message_count);
  
  Serial.print("    📡 Age: ");
  Serial.print(data_age);
  Serial.println("ms │");
  
  printSeparator();
  
  // Motor Performance
  printValue("⚡ RPM", vesc.rpm, " rpm", 12, 0);
  printValue("🔋 Duty", vesc.duty_cycle * 100, "%", 8, 1);
  Serial.println(" │");
  
  printValue("🔌 Motor I", vesc.motor_current, "A", 8, 2);
  printValue("📥 Input I", vesc.input_current, "A", 8, 2);
  Serial.println(" │");
  
  printSeparator();
  
  // Power & Energy
  printValue("⚡ Voltage", vesc.input_voltage, "V", 8, 2);
  printValue("🔋 Ah Used", vesc.amp_hours, "Ah", 8, 3);
  Serial.println(" │");
  
  printValue("⚡ Wh Used", vesc.watt_hours, "Wh", 8, 2);
  printValue("🔄 Ah Regen", vesc.amp_hours_charged, "Ah", 8, 3);
  Serial.println(" │");
  
  printSeparator();
  
  // Temperatures
  printValue("🌡️ FET Temp", vesc.fet_temp, "°C", 8, 1);
  printValue("🔥 Motor T", vesc.motor_temp, "°C", 8, 1);
  Serial.println(" │");
  
  printSeparator();
  
  // Position & Control
  printValue("📍 PID Pos", vesc.pid_position, "°", 8, 1);
  printValue("🔢 Tacho", vesc.tacho_value, "", 12, 0);
  Serial.println(" │");
  
  printSeparator();
  
  // ADC Inputs
  printValue("📊 ADC1", vesc.adc1, "V", 8, 3);
  printValue("📊 ADC2", vesc.adc2, "V", 8, 3);
  Serial.println(" │");
  
  printValue("📊 ADC3", vesc.adc3, "V", 8, 3);
  printValue("📡 PPM", vesc.ppm, "V", 8, 3);
  Serial.println(" │");
  
  Serial.println("└─────────────────────────────────────────────────────────────────────────────┘");
  
  // Data status
  if (!vesc.data_valid) {
    Serial.println("⚠️  WARNING: No VESC data received yet");
  } else if (data_age > 1000) {
    Serial.println("⚠️  WARNING: VESC data is stale (>1s old)");
  } else if (data_age > 100) {
    Serial.println("⚠️  CAUTION: VESC data aging");
  }
  
  Serial.println();
  Serial.print("💡 Tip: To switch to scrolling mode, comment out clearScreen() call");
}

// ═══════════════════════════════════════════════════════════════════════════════
// 🚀 MAIN FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  printHeader();
  
  // Initialize VESC data
  memset(&vesc, 0, sizeof(vesc));
  vesc.data_valid = false;
  
  Serial.println("🔧 Initializing CAN interface...");
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  
  if (CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) != CAN_OK) {
    Serial.println("❌ ERROR: CAN initialization failed!");
    Serial.println("   Check MCP2515 wiring and connections");
    while (1) {
      delay(1000);
      Serial.println("💥 CAN FAILED - System halted");
    }
  }
  
  CAN.setMode(MCP_NORMAL);
  pinMode(PIN_INT, INPUT_PULLUP);
  
  Serial.println("✅ CAN interface ready");
  Serial.println("🔍 Listening for VESC messages...");
  Serial.println();
  
  delay(1000);
}

void loop() {
  CANMessage msg;
  
  // Process all available CAN messages
  while (!digitalRead(PIN_INT)) {
    if (CAN.readMsgBuf(&msg.id, &msg.len, msg.data) == CAN_OK) {
      total_messages++;
      
      if (parseVESCMessage(msg.id, msg.len, msg.data)) {
        // VESC message processed
      }
    }
  }
  
  // Print status at configured rate
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= PRINT_INTERVAL_MS) {
    printStatus();
    lastPrint = millis();
  }
  
  delay(1);  // Small delay for stability
}