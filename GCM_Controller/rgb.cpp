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

#include <SPI.h>
#include <TimerOne.h>

#include "rgb.h"


// This array contains the sector colors as 24-bit hex values
// in order from top sector (0) to bottom sector (NUM_SECTORS - 1).
uint32_t sectorColors[NUM_SECTORS] = { 0x000000 };

// Default SPI settings
static SPISettings spiSettings(4000000UL, LSBFIRST, SPI_MODE0);

// Internal bit time counter, this is free to overflow
static uint8_t bitCounter = 0;


void rgb_init() {
  pinMode(PIN_LATCH, OUTPUT);
  pinMode(PIN_ENABLE, OUTPUT);

  digitalWrite(PIN_ENABLE, LOW);

  Timer1.initialize(40);  // 40 µs per bit for 25 kHz PWM and ~100 Hz update rate
  Timer1.attachInterrupt(rgb_timerInterrupt);
  SPI.begin();
}

void rgb_update(uint8_t blockMask) {
  /*
  blockMask represents the current timing block length,
  i.e. 128, 64, 32, 16, 8, 4, 2, or 1.
  It is used in binary code modulation to determine
  if a specific output should be on for this block or not.

  The output data format is as follows:
  B32 ... B25
  B24 ... B17
  B16 ... B9
  B8  ... B1
  G32 ... G25
  G24 ... G17
  G16 ... G9
  G8  ... G1
  R32 ... R25
  R24 ... R17
  R16 ... R9
  R8  ... R1
  */

  uint8_t data = 0x00;
  SPI.beginTransaction(spiSettings);

  // Loop over all three color channels (0 = blue, 1 = green, 2 = red)
  for (uint8_t color = 0; color < 3; color++) {
    // Loop over all sectors
    for (uint8_t i = 0; i < NUM_SECTORS; i++) {
      // Current sector index
      uint8_t index = NUM_SECTORS - 1 - i;

      // Color channel value or current sector
      uint8_t colorValue = (sectorColors[index] >> (color * 8)) & 0xFF;

      // Position of the current shift register bit in the current data byte
      uint8_t bitPos = (7 - (index % 8));

      // If color value matches current block, set shift register bit
      if (colorValue & blockMask) {
        data |= (1 << bitPos);
      }

      // Transfer current byte if it's complete or if this is the last loop cycle
      if (bitPos == 7 || index == 0) {
        SPI.transfer(data);
        data = 0x00;
      }
    }
  }

  SPI.endTransaction();

  // Pulse latch pin (will get set back to low by timer interrupt)
  digitalWrite(PIN_LATCH, HIGH);
}

void rgb_timerInterrupt() {
  bitCounter++;
  digitalWrite(PIN_LATCH, LOW);
  
  // If bitCounter is a power of 2
  if ((bitCounter & (bitCounter - 1)) == 0) {
    rgb_update(bitCounter);
  }
}