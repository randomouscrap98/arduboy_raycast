#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

constexpr uint8_t RCMAXMAPDIMENSION = 16;

// A single raycast map
class RcMap 
{
public:
    uint8_t * map;
    uint8_t width;
    uint8_t height;

    void setCell(uint8_t x, uint8_t y, uint8_t tile)
    {
        this->map[this->getIndex(x, y)] = tile;
    }

    // Fill map with all of the given tile
    void fillMap(uint8_t tile)
    {
        memset(this->map, tile, size_t(this->width * this->height));
    }

    // Draw the given maze starting at the given screen x + y
    void drawMap(Arduboy2Base * arduboy, uint8_t x, uint8_t y)
    {
        //This is INCREDIBLY slow but should be fine
        for(uint8_t i = 0; i < this->height; ++i)
            for(uint8_t j = 0; j < this->width; ++j)
                arduboy->drawPixel(x + j, y + i, this->getCell(j, this->height - i - 1) ? WHITE : BLACK);
    }

    inline uint8_t getIndex(uint8_t x, uint8_t y)
    {
        return y * this->width + x;
    }

    inline uint8_t getCell(uint8_t x, uint8_t y)
    {
        return this->map[this->getIndex(x, y)];
    }
};
