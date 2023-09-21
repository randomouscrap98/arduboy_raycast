#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

constexpr uint8_t RCMAXMAPDIMENSION = 16;

// A single raycast map
struct RcMap {
    uint8_t * map;
    const uint8_t width;
    const uint8_t height;
};

// Representation of the player in an rcmap
struct RcPlayer {
    uflot posX;
    uflot posY;
    float dirX; //These HAVE TO be float, or something with a lot more precision
    float dirY; 
};

// Set the player direction, though you should probably use 1.0 as fov
void initPlayerDirection(RcPlayer * player, float angle, float fov);

inline uint8_t mapIndex(RcMap * map, uint8_t x, uint8_t y)
{
    return y * map->width + x;
}


inline uint8_t getMapCell(RcMap * map, uint8_t x, uint8_t y)
{
    return map->map[mapIndex(map, x, y)];
}

void setMapCell(RcMap * map, uint8_t x, uint8_t y, uint8_t tile);

// Fill map with all of the given tile
void fillMap(RcMap * map, uint8_t tile);

// Draw the given maze starting at the given screen x + y
void drawMap(Arduboy2Base * arduboy, RcMap * map, uint8_t x, uint8_t y);
