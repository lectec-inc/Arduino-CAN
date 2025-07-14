🧠 GOAL
Create a headless Arduino system on ESP32-C3 that:

Continuously listens to CAN messages from a VESC (broadcasting Status 1–6 at 50Hz).

Sends commands and handles asynchronous responses over CAN (non-blocking).

Exposes a student-facing API via Arduino functions like setDutyCycle(0.3) or getRPM() without requiring any protocol knowledge.

🧱 SYSTEM OVERVIEW
Runtime Components
main.ino
Main entry point that:

Initializes MCP2515-based CAN bus (SPI).

Sets up internal state and telemetry polling.

Runs a non-blocking loop() to:

Listen for new CAN messages.

Match incoming messages to:

Telemetry handler (Status 1–6).

Response callback for a pending command.

Clean up expired/failed commands.

VESC CAN Interface Layer (vesc_interface.h/.cpp)
Encapsulates:

Low-level CAN.send() and CAN.read() logic.

Command serialization + ID tagging.

Response dispatching to correct handler.

Timeout/retry management for sent commands.

Command Layer (commands.h/.cpp)
Supports sending only the following commands:

setDutyCycle(float duty)

setCurrent(float amps)

setCurrentBrake(float amps)

getImuData()

All packet formats must match those defined in VESC firmware:

Reference: comm_can.c and comm_can.h

Protocol Parsing Layer (protocol.h/.cpp)
Handles:

Status Message decoding (Status 1–6 only).

IMU data parsing.

Return value decoding from supported commands.

Byte-level parsing based on official VESC definitions.

Confirm structure using comm_can.h

Apply scaling factors and offsets exactly as specified.

Student-Facing API (student_api.h/.cpp)
Arduino functions callable like:

setDutyCycle(0.3);
float rpm = getRPM();
float temp = getMotorTemp();
Design goals:

Clean return types (float, int, bool).

Internally map to VESC protocol and CAN.

Non-blocking, if waiting for responses.

Graceful failure (timeouts, unavailable data).

🔄 MAIN LOOP DESIGN
Use a millis()-driven non-blocking loop.

Each iteration:

Check for new CAN messages:

Match to either:

Status Message (pass to telemetry parser).

Response for a pending command (invoke callback).

Update shared state struct (e.g., LiveData) with parsed telemetry.

Check for expired commands and clean up.

No command-response logic should block the main loop.

🚩 PENDING COMMAND REGISTRY
When a command is sent:

Store a struct with:

Command ID or UUID

Time sent (millis())

Timeout duration

Expected response ID

Callback pointer or result storage reference

On response match:

Call callback (or resolve result)

Remove from registry

On timeout:

Retry once (optional)

Log and discard

🧩 MESSAGE FILTERING
Only process:

Broadcast messages with VESC Status 1–6

Responses to the 4 supported commands

All others should be ignored or logged in debug mode. Use the correct VESC ID (default: 74) and device ID for ESP32 (default: 2).

🧑‍🎓 STUDENT-FACING API
Provide a simple header (student_api.h) exposing safe, clear Arduino functions:

Example Read Functions

float getRPM();
float getDuty();
float getMotorCurrent();
float getVoltage();
float getMotorTemp();
float getFETTemp();
Example Write Functions

void setDutyCycle(float duty);        // e.g., 0.3
void setCurrent(float amps);          // e.g., 5.0
void setCurrentBrake(float amps);     // e.g., -3.0
IMUData getImuData();                 // Struct: acc_x, acc_y, gyro_z, etc.
All functions abstract away:

CAN IDs

Protocol encoding

Bit shifting / scaling

Students will just call the function; any return values or errors should be self-contained.

📦 FILE STRUCTURE
Recommended folder layout:

bash
Copy
Edit
/src
  main.ino
  /can         → mcp2515 setup and helpers
  /vesc        → command+response protocol
  /protocol    → telemetry & message parsing
  /api         → student-facing header (Arduino functions)
🧪 TESTING
Create loopback / simulated VESC CAN broadcast mode (optional).

Validate:

50Hz Status message parsing under load.

Command timeouts and retry logic.

Concurrent command handling.

Include unit test code in a disabled-by-default #ifdef DEBUG block.

⚙️ HARDWARE / ENVIRONMENT
Board: ESP32-DEVKIT C3

CAN Transceiver: MCP2515 over SPI

VESC: Default ID 74

ESP32 CAN ID: 2

Baudrate: 500k

Wiring: 
- CAN_MIS0 = 2
- CAN_CS = 10
- CAN_INT = 4
- CAN_RST = 5
- CAN_SCK = 6
- CAN_MOSI = 7

Working Code that prints packets:

/*
  MCP2515 listen-only sanity check
  ESP32-C3-DevKitM-1  ·  MCP2515 @16 MHz  ·  500 kbit/s
*/

#include <SPI.h>
#include <mcp_can.h>

// ── Pin map ─────────────────────────────────────────────────────────
constexpr uint8_t PIN_SCK  = 6;    // GPIO6  → SCK
constexpr uint8_t PIN_MISO = 2;    // GPIO2  → SO
constexpr uint8_t PIN_MOSI = 7;    // GPIO7  → SI
constexpr uint8_t PIN_CS   = 10;   // GPIO10 → CS  (10 k pull-up to 3 V3)
constexpr uint8_t PIN_INT  = 4;    // GPIO4  → INT (100 Ω series OK)

MCP_CAN CAN(PIN_CS);

// ── Setup ───────────────────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  delay(300);

  SPI.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);

  if (CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_16MHZ) != CAN_OK) {
    Serial.println(F("CAN init failed – check wiring / crystal enum"));
    while (1) delay(1000);
  }

  CAN.setMode(MCP_LISTENONLY);      // **passive mode**
  pinMode(PIN_INT, INPUT_PULLUP);

  Serial.println(F("Listening only …"));
}

// ── Loop ────────────────────────────────────────────────────────────
void loop() {
  if (!digitalRead(PIN_INT)) {      // INT asserted ⇒ frame latched
    unsigned long can_id;           // 29-bit or 11-bit ID
    byte         len;
    byte         buf[8];

    CAN.readMsgBuf(&can_id, &len, buf);   // library’s 3-arg version

    Serial.print(F("ID 0x"));
    Serial.print(can_id, HEX);
    Serial.print(F("  DLC "));
    Serial.print(len);
    Serial.print(F("  DATA "));
    for (byte i = 0; i < len; ++i) {
      if (buf[i] < 0x10) Serial.print('0');
      Serial.print(buf[i], HEX);
      Serial.print(' ');
    }
    Serial.println();
  }
}

// end of code

You are currently connected to an Arduino via USB. You can utilize the Arduino CLI tools to automatically upload your code and test it with the serial port for feedback.


Each sketch should be in a folder with the same name as the .ino file

To compile, use this command: 

  arduino-cli compile --fqbn esp32:esp32:esp32c3 .

To upload, use this command: 

  arduino-cli upload -p /dev/cu.usbserial-0001 --fqbn esp32:esp32:esp32c3 .


You can monitor the Serial port for feedback with command 

  arduino-cli monitor -p /dev/cu.usbserial-0001