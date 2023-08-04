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

#pragma once

#include <Arduino.h>


#define SERIAL_BAUDRATE 115200
#define SERIAL_TIMEOUT 1000


typedef enum serial_status {
  SUCCESS = 0xFF,
  TIMEOUT = 0xE0,
  UNKNOWN_CMD = 0xE1,
  ERROR = 0xEE,
} serial_status_t;

void serial_init();
serial_status_t serial_readBytesOrTimeout(uint8_t* buffer, uint16_t length);
void serial_sendResponse(uint8_t status);
void serial_clearBuffer();
serial_status_t serial_handleSetSectors(uint8_t* sectorData);
void serial_handleCommunication();