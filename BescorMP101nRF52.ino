/**
 * Besor MP 101 controller Arduino code for nRF52 based dev board
 * Developed by Inside Out Electronics
 * https://www.youtube.com/c/InsideOutElectronics/
*/
#include <bluefruit.h>
#include "cameraslider.h"
#include <Adafruit_DotStar.h>

//Use these pin definitions for the ItsyBitsy M4 for LED
#define DATAPIN    8
#define CLOCKPIN   6
#define NUMPIXELS  1

Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

const int upPin =  10;
const int downPin =  11;
const int leftPin =  13;
const int rightPin =  12;

static const int NOT_CONNECTED_COLOR = 0xFF0000;
static const int CONNECTED_COLOR = 0x0000FF;
static const int MOVING_COLOR = 0x00FF00;

// stealthMode = true fo no LED indication
static const boolean stealthMode = false;

enum MoveValues {
  TILT_UP,
  TILT_DOWN,
  PAN_LEFT,
  PAN_RIGHT,
  STOP,
  TIGGER_ON,
  TIGGER_OFF
};

enum HeadStatus {
  NOT_CONNECTED,
  CONNECTED,      // Or Idle, waiting for commands
  MOVING
};

int currentHeadStatus = NOT_CONNECTED;

// OTA DFU service
BLEDfu bledfu;
// Uart over BLE service
BLEUart bleuart;
#define BLE_READPACKET_TIMEOUT 2
// Packet buffer
extern uint8_t packetbuffer[];

/**
 *  Sets the apropriate i/o to move head
 */
void moveHead(int direction) {  
  switch(direction) {
    case TILT_UP:        setHeadStatus(MOVING); digitalWrite(leftPin,LOW); digitalWrite(rightPin,LOW); digitalWrite(upPin,HIGH); digitalWrite(downPin,LOW); break;
    case TILT_DOWN:      setHeadStatus(MOVING); digitalWrite(leftPin,LOW); digitalWrite(rightPin,LOW); digitalWrite(upPin,LOW); digitalWrite(downPin,HIGH); break;
    case PAN_LEFT:       setHeadStatus(MOVING); digitalWrite(leftPin,HIGH); digitalWrite(rightPin,LOW); digitalWrite(upPin,LOW); digitalWrite(downPin,LOW); break;
    case PAN_RIGHT:      setHeadStatus(MOVING); digitalWrite(leftPin,LOW); digitalWrite(rightPin,HIGH); digitalWrite(upPin,LOW); digitalWrite(downPin,LOW); break;
    case STOP:           setHeadStatus(CONNECTED); digitalWrite(leftPin,LOW); digitalWrite(rightPin,LOW); digitalWrite(upPin,LOW); digitalWrite(downPin,LOW); break;
  }
}

/**
 * Callback invoked when an connection is established
 * @param conn_handle
 */
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");
  setHeadStatus(CONNECTED);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;
  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  moveHead(STOP);
  setHeadStatus(NOT_CONNECTED);
}

/**
 * Intialise and started advertising
 */
void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();
  
  // Include the BLE UART (AKA 'NUS') 128-bit UUID
  Bluefruit.Advertising.addService(bleuart);

  // Secondary Scan Response packet (optional)
  // Since there is no room for 'Name' in Advertising packet
  Bluefruit.ScanResponse.addName();
  
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void setHeadStatus(HeadStatus headStatus) {

  currentHeadStatus = headStatus;
  
  // Provide indication of the status
  if(!stealthMode){
    Serial.print ("Head Status: "); Serial.println(currentHeadStatus, HEX);
    switch(currentHeadStatus) {
      case NOT_CONNECTED: strip.setPixelColor(0, NOT_CONNECTED_COLOR); strip.show(); break;
      case CONNECTED: strip.setPixelColor(0, CONNECTED_COLOR); strip.show(); break;
      case MOVING: strip.setPixelColor(0, MOVING_COLOR); strip.show(); break;
    }
  }
}

void setup() {

  Serial.begin(115200); // Opening the serial port

  // Setup BLE
  Bluefruit.begin();
  Bluefruit.setTxPower(4);    // Check bluefruit.h for supported values
  Bluefruit.setName("Bescor MP101");
  // To be consistent OTA DFU should be added first if it exists
  bledfu.begin();
  // Configure and start the BLE Uart service
  bleuart.begin();
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);
  // Set up and start advertising
  startAdv();  

  // set the digital pin as output:
  pinMode(upPin, OUTPUT);
  pinMode(downPin, OUTPUT);
  pinMode(leftPin, OUTPUT);
  pinMode(rightPin, OUTPUT);

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP
  setHeadStatus(NOT_CONNECTED);
  strip.setPixelColor(0, NOT_CONNECTED_COLOR); strip.show();
}

void loop() {

  // Get new data if there is something
  if(bleuart.available()) {
    readPacketNew(&bleuart, BLE_READPACKET_TIMEOUT);
    char command = packetbuffer[0];
    Serial.print ("Command "); Serial.println(command, HEX);
    moveHead(command);
  } 
}
