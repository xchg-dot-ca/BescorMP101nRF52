/**
 * Besor MP 101 controller Arduino code
*/

#include <bluefruit.h>
#include "cameraslider.h"
#include <Adafruit_DotStar.h>

//Use these pin definitions for the ItsyBitsy M4
#define DATAPIN    8
#define CLOCKPIN   6
#define NUMPIXELS 1

Adafruit_DotStar strip(NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BGR);

const int upPin =  10;
const int downPin =  11;
const int leftPin =  13;
const int rightPin =  12;

int headDirection = 0;
int newCommand = -1;
int currentCommand = -1;

enum MoveValues {
  TILT_UP,
  TILT_DOWN,
  PAN_LEFT,
  PAN_RIGHT,
  STOP,
  TIGGER_ON,
  TIGGER_OFF
};

// OTA DFU service
BLEDfu bledfu;
// Uart over BLE service
BLEUart bleuart;
#define BLE_READPACKET_TIMEOUT 2
// Packet buffer
extern uint8_t packetbuffer[];

void moveHead(int direction) {  
  switch(direction) {
    case TILT_UP:        digitalWrite(leftPin,LOW); digitalWrite(rightPin,LOW); digitalWrite(upPin,HIGH); digitalWrite(downPin,LOW); break;
    case TILT_DOWN:      digitalWrite(leftPin,LOW); digitalWrite(rightPin,LOW); digitalWrite(upPin,LOW); digitalWrite(downPin,HIGH); break;
    case PAN_LEFT:      digitalWrite(leftPin,HIGH); digitalWrite(rightPin,LOW); digitalWrite(upPin,LOW); digitalWrite(downPin,LOW); break;
    case PAN_RIGHT:     digitalWrite(leftPin,LOW); digitalWrite(rightPin,HIGH); digitalWrite(upPin,LOW); digitalWrite(downPin,LOW); break;
    case STOP:      digitalWrite(leftPin,LOW); digitalWrite(rightPin,LOW); digitalWrite(upPin,LOW); digitalWrite(downPin,LOW); break;
  }
}

/**
 * Callback invoked when an connection is established
 * @param conn_handle
 */
void connect_callback(uint16_t conn_handle)
{
  Serial.println("Connected");
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
}

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

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */ 
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
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
  strip.setPixelColor(0, 255, 0, 0); strip.show(); // RED
}

void loop() {

  // Get new data if there is something
  if(bleuart.available()) {
    readPacketNew(&bleuart, BLE_READPACKET_TIMEOUT);
    char command = packetbuffer[0];
    Serial.print ("Command "); Serial.println(command, HEX);
    moveHead(command);
  }
  
  /*
  if(Serial.available()) {
    newCommand = -1;
    currentCommand = -1;
  }

  while(Serial.available()) {
      newCommand = Serial.read();
  }
  
  if(newCommand != currentCommand) {
    currentCommand = newCommand;
    moveHead(currentCommand);
  }
  */
      
  // Echo command back to Commander for DEBUGGING
  /*
  if (currentCommand != "") {
    commandLabel.concat(currentCommand);
    Serial.print(commandLabel);
  }*/     
}
