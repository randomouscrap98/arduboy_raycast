#pragma once

#include <Arduboy2.h>
#include "ArduboyRaycast_Draw.h"


template <uint8_t SpriteCount, uint8_t InternalStateBytes, uint8_t ScreenWidth, uint8_t ScreenHeight>
class RcContainer
{
public:
    RcSprite<InternalStateBytes> spritesBuffer[SpriteCount];
    SSprite<InternalStateBytes> sortedBuffer[SpriteCount];
    RcBounds boundsBuffer[SpriteCount];
    RcSpriteGroup<InternalStateBytes> sprites;

    uint8_t mapBuffer[RCMAXMAPDIMENSION * RCMAXMAPDIMENSION];
    RcPlayer player;
    RcMap worldMap;

    RcInstance<ScreenWidth, ScreenHeight> instance;

    RcContainer(const uint8_t * tilesheet, const uint8_t * spritesheet, const uint8_t * spritesheet_mask) 
    {
        sprites.sprites = this->spritesBuffer;
        sprites.sortedSprites = this->sortedBuffer;
        sprites.bounds = this->boundsBuffer;
        sprites.numbounds = SpriteCount;
        sprites.numsprites = SpriteCount;

        worldMap.map = this->mapBuffer;
        worldMap.width = RCMAXMAPDIMENSION;
        worldMap.height = RCMAXMAPDIMENSION;

        instance.tilesheet = tilesheet;
        instance.spritesheet = spritesheet;
        instance.spritesheet_mask = spritesheet_mask;
    }

    void runIteration(Arduboy2Base * arduboy)
    {
        this->instance.raycastWalls(&this->player, &this->worldMap, &arduboy);
        if(this->instance->tilesheet)
        {
            this->sprites.runSprites(&arduboy);
            this->instance.drawSprites(&this->player, &this->sprites, &arduboy);
        }
    }
};