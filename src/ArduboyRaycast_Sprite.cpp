#include "ArduboyRaycast_Sprite.h"

void resetSprites(RcSpriteGroup * group)
{
    memset(group->sprites, 0, sizeof(RcSprite) * group->numsprites);
}

void resetBounds(RcSpriteGroup * group)
{
    memset(group->bounds, 0, sizeof(RcBounds) * group->numbounds);
}

void resetGroup(RcSpriteGroup * group)
{
    resetBounds(group);
    resetSprites(group);
}

void runSprites(RcSpriteGroup * group, Arduboy2Base * arduboy)
{
    uint8_t numsprites = group->numsprites;
    for(uint8_t i = 0; i < numsprites; i++)
    {
        if(!ISSPRITEACTIVE(group->sprites[i]))
            continue;
        
        if(group->sprites[i].behavior)
            group->sprites[i].behavior(&group->sprites[i], arduboy);
    }
}

//Sort sprites within the sprite contiainer (only affects the sorted list). returns number of active sprites
uint8_t sortSprites(uflot playerX, uflot playerY, RcSpriteGroup * group)
{
    SFixed<11,4> fposx = (SFixed<11,4>)playerX;
    SFixed<11,4> fposy = (SFixed<11,4>)playerY;

    //Make a temp sort array on stack
    uint8_t numsprites = group->numsprites;
    uint8_t usedSprites = 0;
    SSprite * sorted = group->tempsorting;
    
    // Calc distance. Also, sort elements (might as well, we're already here)
    for (uint8_t i = 0; i < numsprites; ++i)
    {
        RcSprite * sprite = &group->sprites[i];

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

RcSprite * addSprite(RcSpriteGroup * group, float x, float y, uint8_t frame, uint8_t shrinkLevel, int8_t heightAdjust, behavior_func func)
{
    uint8_t numsprites = group->numsprites;
    for(uint8_t i = 0; i < numsprites; i++)
    {
        RcSprite * sprite = &group->sprites[i];
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
