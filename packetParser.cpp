#include <Arduino.h>
#include <bluefruit.h>
#include "cameraslider.h"

// Size of the read buffer for incoming packets
#define READ_BUFSIZE (20)

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
