#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

#include "utils.h"

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

inline uint8_t mapIndex(RcMap * map, uint8_t x, uint8_t y) {
    return y * map->width + x;
}

inline uint8_t getMapCell(RcMap * map, uint8_t x, uint8_t y)
{
    return map->map[mapIndex(map, x, y)];
}

void setMapCell(RcMap * map, uint8_t x, uint8_t y, uint8_t tile)
{
    map->map[mapIndex(map, x, y)] = tile;
}

// Fill map with all of the given tile
void fillMap(RcMap * map, uint8_t tile)
{
    memset(map->map, tile, size_t(map->width * map->height));
}

// Draw the given maze starting at the given screen x + y
void drawMap(Arduboy2Base * arduboy, RcMap * map, uint8_t x, uint8_t y)
{
    //This is INCREDIBLY slow but should be fine
    for(uint8_t i = 0; i < map->height; ++i)
        for(uint8_t j = 0; j < map->width; ++j)
            arduboy->drawPixel(x + j, y + i, getMapCell(map, j, map->height - i - 1) ? WHITE : BLACK);
}

