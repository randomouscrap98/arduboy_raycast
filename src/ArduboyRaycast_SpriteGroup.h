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
        return &this->sprites[index];
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

    // Run a common function against all active sprites.
    void runSpritesCommon(void (* func)(RcSprite<InternalStateBytes> *))
    {
        uint8_t numsprites = this->numsprites;
        for(uint8_t i = 0; i < numsprites; i++)
        {
            if(this->sprites[i].isActive())
                func(&this->sprites[i]);    
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
    RcSprite<InternalStateBytes> * addSprite(muflot x, muflot y, uint8_t frame, uint8_t sizeLevel, int8_t heightAdjust, void (* func)(RcSprite<InternalStateBytes> *))
    {
        uint8_t numsprites = this->numsprites;
        for(uint8_t i = 0; i < numsprites; i++)
        {
            RcSprite<InternalStateBytes> * sprite = &this->sprites[i];
            if(!ISSPRITEACTIVE((*sprite)))
            {
                sprite->x = x; //muflot(x);
                sprite->y = y; //muflot(y);
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
    RcBounds * addBounds(muflot x1, muflot y1, muflot x2, muflot y2, bool solid)
    {
        uint8_t numbounds = this->numbounds;
        for(uint8_t i = 0; i < numbounds; i++)
        {
            RcBounds * bounds = &this->bounds[i];
            if(!ISSPRITEACTIVE((*bounds)))
            {
                bounds->x1 = x1; //muflot(x1);
                bounds->x2 = x2; //muflot(x2);
                bounds->y1 = y1; //muflot(y1);
                bounds->y2 = y2; //muflot(y2);
                bounds->setActive(true);
                bounds->setSolid(solid);
                return bounds;
            }
        }

        return NULL;
    }

    // A shortcut function to add simple square bounds around the sprite and link the two together.
    // It is equivalent to calling 'addBounds' then calling 'linkSpriteBounds' with all the error
    // handling involved. Not that linked bounds do NOT move with the sprite, that's up to you.
    // Linking is only done to more easily find a sprite from a bounds, and vice-versa.
    RcBounds * addSpriteBounds(RcSprite<InternalStateBytes> * sprite, muflot size, bool solid)
    {
        muflot halfsize = size / 2;
        RcBounds * result = this->addBounds(sprite->x - halfsize, sprite->y - halfsize, sprite->x + halfsize, sprite->y + halfsize, solid);
        if(result)
        {
            RcBounds * bounds = this->linkSpriteBounds(sprite, result);
            if(bounds == NULL)
            {
                this->deleteBounds(result);
                result = NULL;
            }
            else
            {
                result = bounds;
            }
        }
        return result;
    }

    // Link a sprite and bounds together. This is only an "imaginary" link for use with finding sprites
    // based on bounds, and does not impact anything else in the library. For instance, you still
    // need to delete both sprite and bounds separately. This is a relatively expensive function
    // (for these low powered systems), only call this when creating sprites! NO safety checks are
    // performed; if you pass in a sprite or bounds that are not contained within this group, the
    // results are undefined (and almost certainly broken!)
    RcBounds * linkSpriteBounds(RcSprite<InternalStateBytes> * sprite, RcBounds * bounds)
    {
        uint8_t spriteIndex = sprite - this->sprites;
        uint8_t oldBoundsIndex = bounds - this->bounds;
        if(spriteIndex == oldBoundsIndex) return bounds;    // Already linked
        if(this->numbounds <= spriteIndex) return NULL;     // Not enough space to 'link' the bounds
        RcBounds existing = this->bounds[spriteIndex];
        this->bounds[spriteIndex] = this->bounds[oldBoundsIndex];
        this->bounds[oldBoundsIndex] = existing;
        return &this->bounds[spriteIndex];
    }

    // If a sprite is linked to a bounds, get the sprite from the bounds. Note that the 
    // functionality is undefined if the sprite + bounds are not linked (no check is performed)
    RcSprite<InternalStateBytes> * getLinkedSprite(RcBounds * bounds)
    {
        return &this->sprites[bounds - this->bounds];
    }

    // If a sprite is linked to a bounds, get the bounds from the sprite. Note that 
    // the functionality is undefined if the sprite + bounds are not linked
    RcBounds * getLinkedBounds(RcSprite<InternalStateBytes> * sprite)
    {
        return &this->bounds[sprite - this->sprites];
    }

    void deleteBounds(RcBounds * bounds) { bounds->state = 0; }
    void deleteSprite(RcSprite<InternalStateBytes> * sprite) { sprite->state = 0; }
    void deleteLinked(RcBounds * bounds) {
        RcSprite<InternalStateBytes> * sprite = this->getLinkedSprite(bounds);
        this->deleteSprite(sprite);
        this->deleteBounds(bounds);
    }
    void deleteLinked(RcSprite<InternalStateBytes> * sprite) {
        RcBounds * bounds = this->getLinkedBounds(sprite);
        this->deleteSprite(sprite);
        this->deleteBounds(bounds);
    }

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


