#pragma once

#include "ArduboyRaycast_Sprite.h"

template<uint8_t InternalStateBytes>
class RcSpriteGroup
{
public:
    RcSprite<InternalStateBytes> * sprites;
    SSprite<InternalStateBytes> * sortedSprites;
    RcBounds * bounds;
    uint8_t numsprites;
    uint8_t numbounds;

    void resetSprites()
    {
        memset(this->sprites, 0, sizeof(RcSprite<InternalStateBytes>) * this->numsprites);
    }

    void resetBounds()
    {
        memset(this->bounds, 0, sizeof(RcBounds) * this->numbounds);
    }

    void resetAll()
    {
        this->resetBounds();
        this->resetSprites();
    }

    void runSprites(Arduboy2Base * arduboy)
    {
        uint8_t numsprites = this->numsprites;
        for(uint8_t i = 0; i < numsprites; i++)
        {
            RcSprite<InternalStateBytes> * sprite = &this->sprites[i];
            if(!ISSPRITEACTIVE((*sprite)))
                continue;
            
            if(sprite->behavior)
                sprite->behavior(sprite, arduboy);
        }
    }

    //Sort sprites within the sprite contiainer (only affects the sorted list). returns number of active sprites
    uint8_t sortSprites(uflot playerX, uflot playerY)
    {
        SFixed<11,4> fposx = (SFixed<11,4>)playerX;
        SFixed<11,4> fposy = (SFixed<11,4>)playerY;

        //Make a temp sort array on stack
        uint8_t numsprites = this->numsprites;
        uint8_t usedSprites = 0;
        SSprite<InternalStateBytes> * sorted = this->sortedSprites;
        
        // Calc distance. Also, sort elements (might as well, we're already here)
        for (uint8_t i = 0; i < numsprites; ++i)
        {
            RcSprite<InternalStateBytes> * sprite = &this->sprites[i];

            if (!ISSPRITEACTIVE((*sprite)))
                continue;

            SSprite<InternalStateBytes> toSort;
            SFixed<11,4> dpx = (SFixed<11,4>)sprite->x - fposx;
            SFixed<11,4> dpy = (SFixed<11,4>)sprite->y - fposy;
            toSort.distance = dpx * dpx + dpy * dpy; // sqrt not taken, unneeded
            toSort.sprite = sprite;

            //Insertion sort (it's faster for small arrays; if you increase sprite count to some 
            //absurd number, change this to something else).
            int8_t insertPos = usedSprites - 1;

            while(insertPos >= 0 && sorted[insertPos].distance < toSort.distance)
            {
                sorted[insertPos + 1] = sorted[insertPos];
                insertPos--;
            }

            sorted[insertPos + 1] = toSort;
            usedSprites++;
        }

        return usedSprites;
    }

    RcSprite<InternalStateBytes> * addSprite(float x, float y, uint8_t frame, uint8_t shrinkLevel, int8_t heightAdjust, void (* func)(RcSprite<InternalStateBytes> *,Arduboy2Base*))
    {
        uint8_t numsprites = this->numsprites;
        for(uint8_t i = 0; i < numsprites; i++)
        {
            RcSprite<InternalStateBytes> * sprite = &this->sprites[i];
            if(!ISSPRITEACTIVE((*sprite)))
            {
                sprite->x = muflot(x);
                sprite->y = muflot(y);
                sprite->frame = frame;
                sprite->state = 1 | ((shrinkLevel << 1) & RSSTATESHRINK) | (heightAdjust < 0 ? 16 : 0) | ((abs(heightAdjust) << 3) & RSTATEYOFFSET);
                sprite->behavior = func;
                return sprite;
            }
        }

        return NULL;
    }

    //Get the first bounding box (in order of ID) which intersects this point
    RcBounds * firstColliding(uflot x, uflot y)
    {
        uint8_t numbounds = this->numbounds;
        for (uint8_t i = 0; i < numbounds; i++)
        {
            if (!ISSPRITEACTIVE((this->bounds[i])))
                continue;

            if(this->bounds[i].colliding(x, y))
                return &this->bounds[i];
        }

        return NULL;
    }
};


