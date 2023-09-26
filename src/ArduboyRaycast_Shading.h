#pragma once

#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

constexpr uint8_t BAYERGRADIENTS = 16;

// Bayer gradients, not including the 0 fill (useless?).
// Takes up 64 precious bytes of RAM
constexpr uint8_t b_shading[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, // Bayer 16
    0xEE, 0xFF, 0xFF, 0xFF, // 0
    0xEE, 0xFF, 0xBB, 0xFF,
    0xEE, 0xFF, 0xAA, 0xFF, // 2 
    0xAA, 0xFF, 0xAA, 0xFF, 
    0xAA, 0xDD, 0xAA, 0xFF, // 4
    0xAA, 0xDD, 0xAA, 0x77,
    0xAA, 0xDD, 0xAA, 0x55, // 6
    0xAA, 0x55, 0xAA, 0x55,
    0xAA, 0x44, 0xAA, 0x55, // 8
    0xAA, 0x44, 0xAA, 0x11, 
    0xAA, 0x44, 0xAA, 0x00, // 10
    0xAA, 0x00, 0xAA, 0x00, 
    0x44, 0x00, 0xAA, 0x00, // 12
    0x44, 0x00, 0x22, 0x00,
    0x44, 0x00, 0x00, 0x00, // 14
};

// It's ridiculous that it's faster to read this from memory than to simply left/right shift...
constexpr uint8_t shade_mask[] PROGMEM = {
    0b11111111, 0b11111110, 0b11111100, 0b11111000, 0b11110000, 0b11100000, 0b11000000, 0b10000000,
    //0b11111111, 0b00000001, 0b00000011, 0b00000111, 0b00001111, 0b00011111, 0b00111111, 0b01111111
};

// Compute the shading byte for the given distance and x value
inline uint8_t calcShading(uflot perpWallDist, uint8_t x, const uflot DARKNESS)
{
    uint8_t dither = (perpWallDist * DARKNESS * perpWallDist).getInteger();
    asm volatile( // it refuses to do 8 bit left shift, why??
        "lsl %0     \n"
        "lsl %0     \n"
        : "+r" (dither)
    );
    return (dither >= BAYERGRADIENTS << 2) ? 0 : pgm_read_byte(b_shading + dither + (x & 3));
}
