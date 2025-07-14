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

void printStatus() {
  unsigned long data_age = millis() - vesc.last_update;
  
  Serial.print("Time: ");
  Serial.print(millis() / 1000.0, 1);
  Serial.print("s | ");
  
  // Check if data is fresh (within last 1 second)
  if (!vesc.data_valid || data_age > 1000) {
    Serial.println("Status: NO DATA - VESC disconnected or not responding");
    return;
  }
  
  // Check if data is getting stale (within last 500ms but show warning)
  if (data_age > 500) {
    Serial.print("Status: STALE DATA (");
    Serial.print(data_age);
    Serial.print("ms old) | ");
  } else {
    Serial.print("Status: CONNECTED | ");
  }
  
  Serial.print("Voltage: ");
  Serial.print(vesc.input_voltage, 2);
  Serial.print("V | RPM: ");
  Serial.print((int)vesc.rpm);
  Serial.print(" | Duty: ");
  Serial.print(vesc.duty_cycle * 100, 1);
  Serial.print("% | Motor Current: ");
  Serial.print(vesc.motor_current, 2);
  Serial.print("A | Battery Current: ");
  Serial.print(vesc.input_current, 2);
  Serial.print("A | FET Temp: ");
  Serial.print(vesc.fet_temp, 1);
  Serial.print("C | Amp Hours: ");
  Serial.print(vesc.amp_hours, 3);
  Serial.println("Ah");
}

// ═══════════════════════════════════════════════════════════════════════════════
// 🚀 MAIN FUNCTIONS
// ═══════════════════════════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("VESC CAN Monitor Starting...");
  Serial.print("Print Rate: ");
  Serial.print(PRINT_RATE_HZ, 1);
  Serial.println(" Hz");
  
  // Initialize VESC data
  memset(&vesc, 0, sizeof(vesc));
  vesc.data_valid = false;
  
  Serial.println("Initializing CAN interface...");
  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  
  if (CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) != CAN_OK) {
    Serial.println("ERROR: CAN initialization failed!");
    Serial.println("Check MCP2515 wiring and connections");
    while (1) {
      delay(1000);
      Serial.println("CAN FAILED - System halted");
    }
  }
  
  CAN.setMode(MCP_NORMAL);
  pinMode(PIN_INT, INPUT_PULLUP);
  
  Serial.println("CAN interface ready");
  Serial.println("Listening for VESC messages...");
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