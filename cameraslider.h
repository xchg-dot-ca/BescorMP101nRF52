static const char UP_COMMAND = 0x0;
static const char DOWN_COMMAND = 0x01;
static const char LEFT_COMMAND = 0x02;
static const char RIGHT_COMMAND = 0x03;
static const char STOP_COMMAND = 0x09;

static const char STATUS_PAYLOAD_IDLE = 0x10;
static const char STATUS_PAYLOAD_BUSY = 0x11;
static const char STATUS_PAYLOAD_TIME_REMAINING = 0x12;
static const char STATUS_COMMAND = 0x13;
static const char STATUS_PAYLOAD_ERROR = 0xff;

static const char STATE_RUNNING = 1;
static const char STATE_IDLE = 2;
static const char STATE_ERROR = 0;

static const int DEFAULT_LENGTH = 500;                    // Length in mm
static const int DEFAULT_START_POSITION = 0;
static const int DEFAULT_MAX_POSITION = DEFAULT_LENGTH;
static const int DEFAULT_RUN_TIME = 5;                    // 5 minutes
static const double DEFAULT_STEPS_PER_MM = 100.0;            // 100 steps per mm, Resolution: 10 micron -> Teeth: 16  Rotation per step: 1.8°  Stepping: 1/16th  Pitch: 2mm

struct startPacketPayload {
  int startPosition = DEFAULT_START_POSITION;
  int stopPosition = DEFAULT_MAX_POSITION;
  int timeToRun = DEFAULT_RUN_TIME;
};

// Function prototypes for packetparser.cpp
uint8_t readPacketNew (BLEUart *ble_uart, uint16_t timeout);
uint8_t writePacket (BLEUart *ble_uart, char* writeBuff, int sizeOfBuff);
bool parseStartPayload (startPacketPayload *payload);
