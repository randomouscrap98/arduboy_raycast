#include "ArduboyRaycast_Sprite.h"

void RcSpriteGroup::resetSprites()
{
    memset(this->sprites, 0, sizeof(RcSprite) * this->numsprites);
}

void RcSpriteGroup::resetBounds()
{
    memset(this->bounds, 0, sizeof(RcBounds) * this->numbounds);
}

void RcSpriteGroup::resetAll()
{
    this->resetBounds();
    this->resetSprites();
}

void RcSpriteGroup::runSprites(Arduboy2Base * arduboy)
{
    uint8_t numsprites = this->numsprites;
    for(uint8_t i = 0; i < numsprites; i++)
    {
        RcSprite * sprite = &this->sprites[i];
        if(!ISSPRITEACTIVE((*sprite)))
            continue;
        
        if(sprite->behavior)
            sprite->behavior(sprite, arduboy);
    }
}

//Sort sprites within the sprite contiainer (only affects the sorted list). returns number of active sprites
uint8_t RcSpriteGroup::sortSprites(uflot playerX, uflot playerY)
{
    SFixed<11,4> fposx = (SFixed<11,4>)playerX;
    SFixed<11,4> fposy = (SFixed<11,4>)playerY;

    //Make a temp sort array on stack
    uint8_t numsprites = this->numsprites;
    uint8_t usedSprites = 0;
    SSprite * sorted = this->tempsorting;
    
    // Calc distance. Also, sort elements (might as well, we're already here)
    for (uint8_t i = 0; i < numsprites; ++i)
    {
        RcSprite * sprite = &this->sprites[i];

        if (!ISSPRITEACTIVE((*sprite)))
            continue;

        SSprite toSort;
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

RcSprite * RcSpriteGroup::addSprite(float x, float y, uint8_t frame, uint8_t shrinkLevel, int8_t heightAdjust, behavior_func func)
{
    uint8_t numsprites = this->numsprites;
    for(uint8_t i = 0; i < numsprites; i++)
    {
        RcSprite * sprite = &this->sprites[i];
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
