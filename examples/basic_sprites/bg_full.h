#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

constexpr uint8_t bg_fullWidth = 128;
constexpr uint8_t bg_fullHeight = 64;

constexpr uint8_t bg_full[] PROGMEM
{
  bg_fullWidth, bg_fullHeight,

  //Frame 0
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 
  0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x50, 0xA0, 0x40, 0xA0, 0x50, 
  0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 
  0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x44, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 
  0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55, 0xAA, 0x54, 0xAA, 0x55, 0xAA, 0x45, 0xAA, 0x55
};
