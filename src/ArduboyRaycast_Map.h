#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

constexpr uint8_t RCMAXMAPDIMENSION = 16;

// A single raycast map
class RcMap {
    public:
        uint8_t * map;
        uint8_t width;
        uint8_t height;

        inline uint8_t getIndex(uint8_t x, uint8_t y)
        {
            return y * this->width + x;
        }

        inline uint8_t getCell(uint8_t x, uint8_t y)
        {
            return this->map[this->getIndex(x, y)];
        }

        void setCell(uint8_t x, uint8_t y, uint8_t tile);

        // Fill map with all of the given tile
        void fillMap(uint8_t tile);

        // Draw the given maze starting at the given screen x + y
        void drawMap(Arduboy2Base * arduboy, uint8_t x, uint8_t y);
};

// Representation of the player in an rcmap
class RcPlayer {
    public:
        uflot posX;
        uflot posY;
        float dirX; //These HAVE TO be float, or something with a lot more precision
        float dirY; 

        void initPlayerDirection(float angle, float fov);
};

