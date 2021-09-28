#include <Arduino.h>
#include <bluefruit.h>
#include "cameraslider.h"

//    READ_BUFSIZE            Size of the read buffer for incoming packets
#define READ_BUFSIZE                    (20)

/* Buffer to hold incoming characters */
uint8_t packetbuffer[READ_BUFSIZE];

uint8_t readPacketNew(BLEUart *ble_uart, uint16_t timeout) 
{
  uint16_t origtimeout = timeout, replyidx = 0;

  // Cleanup buffer
  memset(packetbuffer, 0, READ_BUFSIZE);

  while (timeout--) {
    
    if (replyidx >= READ_BUFSIZE) break;
    
    while (ble_uart->available()) {
      char c =  ble_uart->read();
      packetbuffer[replyidx] = c;
      replyidx++;
      timeout = origtimeout;
    }
  
    if (timeout == 0) break;
    delay(1);
  }

  if (!replyidx)  // no data or timeout 
    return 0;

  return replyidx;
}

uint8_t writePacket (BLEUart *ble_uart, char* writeBuff, int sizeOfBuff) {
  ble_uart->write(writeBuff, sizeOfBuff);
}

bool parseStartPayload (startPacketPayload *payload) {

  if(packetbuffer[0] == STATUS_COMMAND) {
    payload->startPosition = int(packetbuffer[1] << 24 | packetbuffer[2] << 16 | packetbuffer[3] << 8 | packetbuffer[4]);
    payload->stopPosition = int(packetbuffer[5] << 24 | packetbuffer[6] << 16 | packetbuffer[7] << 8 | packetbuffer[8]);
    payload->timeToRun = int(packetbuffer[9] << 24 | packetbuffer[10] << 16 | packetbuffer[11] << 8 | packetbuffer[12]);
    return true;
  } else {
    return false;
  }
}
