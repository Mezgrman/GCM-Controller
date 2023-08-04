/*
Copyright (C) 2023 Julian Metzler

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "serial.h"
#include "rgb.h"


extern uint32_t sectorColors[NUM_SECTORS];


void serial_init() {
  Serial.begin(SERIAL_BAUDRATE);
}

serial_status_t serial_readBytesOrTimeout(uint8_t* buffer, uint16_t length) {
  uint32_t startTime = millis();
  for (uint16_t n = 0; n < length; n++) {
    while (!Serial.available()) {
      uint32_t timeTaken = millis() - startTime;
      if (timeTaken >= SERIAL_TIMEOUT) {
        return TIMEOUT;
      }
    }
    buffer[n] = (uint8_t)Serial.read();
  }
  return SUCCESS;
}

void serial_sendResponse(uint8_t status) {
  Serial.write(status);
}

void serial_clearBuffer() {
  while (Serial.available() > 0) {
    Serial.read();
  }
}

serial_status_t serial_handleSetSectors(uint8_t* sectorData) {
  // The sectorData parameter must be a pointer to a sufficiently large array
  // where color data is stored for each sector;
  // the index being the sector number and the value being the color in 0x00RRGGBB format.
  // So the input data per sector should be 0x00 (dummy byte), 0xRR, 0xGG, 0xBB.
  // Returns: 1 if successful, 0 on error

  serial_status_t s = SUCCESS;

  // Receive number of bytes
  uint8_t numBytes;
  s = serial_readBytesOrTimeout(&numBytes, 1);
  if (s != SUCCESS) return s;

  // Receive sector data
  s = serial_readBytesOrTimeout(sectorData, numBytes);
  if (s != SUCCESS) return s;

  return SUCCESS;
}

void serial_handleCommunication() {
  /*
     SERIAL PROTOCOL

     Explanation:
       0x00> - Byte from PC to Arduino
      <0x00  - Byte from Arduino to PC

     Status codes:
       <0xFF  - Success
       <0xE0  - Timeout while receiving serial data
       <0xEE  - Generic Error

     0xFF> - Start Byte
     0xAn> - Action byte:
       0xA0> - Set sector colors:
         byte> - Number of bytes following this one
         data> - Color data

     <byte  - Status code
  */

  serial_status_t s = SUCCESS;

  if (!Serial.available()) return;

  // Check for start byte
  uint8_t startByte;
  if (serial_readBytesOrTimeout(&startByte, 1) != SUCCESS) return;
  if (startByte != 0xFF) return;

  // Check action byte
  uint8_t actionByte;
  s = serial_readBytesOrTimeout(&actionByte, 1);
  if (s != SUCCESS) {
    serial_clearBuffer();
    serial_sendResponse(s);
  }

  switch (actionByte) {
    // Set sector data
    case 0xA0:
      {
        s = serial_handleSetSectors((uint8_t*)sectorColors);
        break;
      }

    // Return error
    default:
      {
        s = UNKNOWN_CMD;
        break;
      }
  }

  serial_clearBuffer();
  serial_sendResponse(s);
}
