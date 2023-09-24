#pragma once

#include "ArduboyRaycast_Sprite.h"

#define ISSPRITEACTIVE(s) (s.state & RSSTATEACTIVE)

template<uint8_t InternalStateBytes>
class RcSpriteGroup
{
public:
    RcSprite<InternalStateBytes> * sprites;
    SSprite<InternalStateBytes> * sortedSprites;
    RcBounds * bounds;
    uint8_t numsprites;
    uint8_t numbounds;

    RcSprite<InternalStateBytes> * operator[](uint8_t index)
    {
        return this->sprites[index];
    }

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

    void runSprites()
    {
        uint8_t numsprites = this->numsprites;
        for(uint8_t i = 0; i < numsprites; i++)
        {
            RcSprite<InternalStateBytes> * sprite = &this->sprites[i];
            if(!ISSPRITEACTIVE((*sprite)))
                continue;
            
            if(sprite->behavior)
                sprite->behavior(sprite);
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

    // Attempt to add a sprite to the sprite list. Activates the sprite immediately and fills out some of the more 
    // complicated fields.
    RcSprite<InternalStateBytes> * addSprite(float x, float y, uint8_t frame, uint8_t sizeLevel, int8_t heightAdjust, void (* func)(RcSprite<InternalStateBytes> *))
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
                sprite->setActive(true);
                sprite->setHeight(heightAdjust);
                sprite->setSizeIndex(sizeLevel);
                sprite->behavior = func;
                return sprite;
            }
        }

        return NULL;
    }

    // Attempt to add a bounds to the bounds list.
    RcBounds * addBounds(float x1, float y1, float x2, float y2, bool solid)
    {
        uint8_t numbounds = this->numbounds;
        for(uint8_t i = 0; i < numbounds; i++)
        {
            RcBounds * bounds = &this->bounds[i];
            if(!ISSPRITEACTIVE((*bounds)))
            {
                bounds->x1 = muflot(x1);
                bounds->x2 = muflot(x2);
                bounds->y1 = muflot(y1);
                bounds->y2 = muflot(y2);
                bounds->setActive(true);
                bounds->setSolid(solid);
                return bounds;
            }
        }

        return NULL;
    }

    // Add a bounds for the given sprite to the bounds array. Since sprites are billboards and 
    // always rotate to face the player, this shortcut function only allows you to give one dimension
    // of a square box surrounding said sprite. The bounding box will NOT move with the sprite, that's
    // up to you. You can add extra data to sprites to store the pointer to the bounding box (2 bytes)
    // and thus have a link between the two.
    RcBounds * addSpriteBounds(RcSprite<InternalStateBytes> * sprite, float size, bool solid)
    {
        float halfsize = size / 2;
        return this->addBounds((float)sprite->x - halfsize, (float)sprite->y - halfsize, (float)sprite->x + halfsize, (float)sprite->y + halfsize, solid);
    }

    void deleteBounds(RcBounds * bounds) { bounds->state = 0; }
    void deleteSprite(RcSprite<InternalStateBytes> * sprite) { sprite->state = 0; }

    //Get the first bounding box (in order of ID) which intersects this point. Optionally restrict 
    //by bounds that have a nonzero value with the statemask
    RcBounds * firstColliding(uflot x, uflot y, uint8_t statemask)
    {
        uint8_t numbounds = this->numbounds;
        for (uint8_t i = 0; i < numbounds; i++)
        {
            if (!ISSPRITEACTIVE((this->bounds[i])))
                continue;

            if(!statemask || (this->bounds[i].state & statemask))
            {
                if(this->bounds[i].colliding(x, y))
                    return &this->bounds[i];
            }
        }

        return NULL;
    }
};


