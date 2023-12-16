#pragma once

#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

constexpr uint8_t BAYERGRADIENTS = 16;

// Bayer gradients, from full filled to zero filled. Represents 4x4; technically
// you only need two bytes for this but for speed, they're really 8x4
constexpr uint8_t b_shading[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, 
    0xEE, 0xFF, 0xFF, 0xFF, 
    0xEE, 0xFF, 0xBB, 0xFF,
    0xEE, 0xFF, 0xAA, 0xFF, 
    0xAA, 0xFF, 0xAA, 0xFF, 
    0xAA, 0xDD, 0xAA, 0xFF, 
    0xAA, 0xDD, 0xAA, 0x77,
    0xAA, 0xDD, 0xAA, 0x55, 
    0xAA, 0x55, 0xAA, 0x55,
    0xAA, 0x44, 0xAA, 0x55, 
    0xAA, 0x44, 0xAA, 0x11, 
    0xAA, 0x44, 0xAA, 0x00, 
    0xAA, 0x00, 0xAA, 0x00, 
    0x44, 0x00, 0xAA, 0x00, 
    0x44, 0x00, 0x22, 0x00,
    0x44, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
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

// Apply shading to the region of screen as though it were raycast walls (uses the same algorithm)
// X2 and Y2 are exclusive
template <uint8_t blackOrWhite>
void shadeScreen(Arduboy2Base * arduboy, uint8_t bayer, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2)
{
    uint8_t yEnd = (y2 >> 3) + (y2 & 7 ? 1 : 0);

    for(uint8_t j = x; j < x2; j++)
    {
        //Calculate shading here
        uint8_t shading = pgm_read_byte(b_shading + (bayer * 4) + (j & 3));

        for(uint8_t i = y >> 3; i < yEnd; ++i)
        {
            //Zero cost abstraction... other than doubling code size if you need both. 0 for black, 1 for white
            if constexpr (blackOrWhite == BLACK)
                arduboy->sBuffer[j + (i * WIDTH)] &= shading;
            else if constexpr(blackOrWhite == WHITE)
                arduboy->sBuffer[j + (i * WIDTH)] |= ~shading;
        }
    }
}

template <uint8_t blackOrWhite>
void shadeScreen(Arduboy2Base * arduboy, float amount, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2)
{
    shadeScreen<blackOrWhite>(arduboy, uint8_t(min(BAYERGRADIENTS, round(BAYERGRADIENTS * abs(amount)))), x, y, x2, y2);
}