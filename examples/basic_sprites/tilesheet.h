#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

constexpr uint8_t tilesheetWidth = 16;
constexpr uint8_t tilesheetHeight = 16;

// Generated by Arduboy Toolset, but remember to remove the width and height!
constexpr uint8_t tilesheet[] PROGMEM
{
  //Frame 0
  0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 
  0x5F, 0x4F, 0x4F, 0x5F, 0x5F, 0x4F, 0x4F, 0x5F, 0x5F, 0x4F, 0x4F, 0x5F, 0x5F, 0x4F, 0x4F, 0x5F, 

  //Frame 1
  0xFE, 0xFA, 0xFE, 0xFA, 0xFE, 0xFA, 0xFE, 0xFA, 0xFE, 0xFA, 0xFE, 0xFA, 0xFE, 0xFA, 0xFE, 0xFA, 
  0x5F, 0x4F, 0x4F, 0x5F, 0x5F, 0x4F, 0x4F, 0x5F, 0x5F, 0x4F, 0x4F, 0x5F, 0x5F, 0x4F, 0x4F, 0x5F, 

  //Frame 2
  0xFE, 0x02, 0xFE, 0xFE, 0x02, 0xFE, 0xFE, 0x02, 0xFE, 0xFE, 0xFE, 0x02, 0xFE, 0xFE, 0x02, 0xFE, 
  0x7F, 0x40, 0x7F, 0x7F, 0x40, 0x7F, 0x7F, 0x40, 0x7F, 0x7F, 0x7F, 0x40, 0x7F, 0x7F, 0x40, 0x7F, 

  //Frame 3
  0xFE, 0xFA, 0x06, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0x06, 0xFE, 0xFA, 
  0x5F, 0x7F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFB, 0xFF, 0x00, 0x7F, 0x5F
};