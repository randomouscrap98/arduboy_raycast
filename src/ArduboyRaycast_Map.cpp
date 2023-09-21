#include "ArduboyRaycast_Map.h"

void RcMap::setCell(uint8_t x, uint8_t y, uint8_t tile)
{
    this->map[this->getIndex(x, y)] = tile;
}

// Fill map with all of the given tile
void RcMap::fillMap(uint8_t tile)
{
    memset(this->map, tile, size_t(this->width * this->height));
}

// Draw the given maze starting at the given screen x + y
void RcMap::drawMap(Arduboy2Base * arduboy, uint8_t x, uint8_t y)
{
    //This is INCREDIBLY slow but should be fine
    for(uint8_t i = 0; i < this->height; ++i)
        for(uint8_t j = 0; j < this->width; ++j)
            arduboy->drawPixel(x + j, y + i, this->getCell(j, this->height - i - 1) ? WHITE : BLACK);
}

void RcPlayer::initPlayerDirection(float angle, float fov)
{
    this->dirX = fov * cos(angle);
    this->dirY = fov * sin(angle);
}
