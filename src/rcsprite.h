#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

#include "utils.h"

// As with raycast.h, I unfortunately require you to make changes to this file if you
// want different results.
constexpr uint8_t RSINTERNALSTATEBYTES = 2;

// These are bitmasks to get data out of state
constexpr uint8_t RSSTATEACTIVE = 0b00000001;
constexpr uint8_t RSSTATESHRINK = 0b00000110;
constexpr uint8_t RSTATEYOFFSET = 0b11111000;


// Try to make this fit into as little space as possible
struct RcSprite {
    muflot x; //Precision for x/y is low but doesn't really need to be high
    muflot y;
    uint8_t frame = 0;
    uint8_t state = 0; // First bit is active, next 2 are how many times to /2 for size
    void (* behavior)(RcSprite*,Arduboy2Base*) = NULL;

    uint8_t intstate[RSINTERNALSTATEBYTES];
};

typedef void (* behavior_func)(RcSprite *,Arduboy2Base *);

// Sorted sprite. Useful to keep original sprite list index as sprite ids 
struct SSprite {
    RcSprite * sprite; 
    SFixed<11,4> distance;    //Unfortunately, distance kinda has to be large... 12 bits = 4096, should be more than enough
};

// A box representing bounds that the player shouldn't be able to walk into. Not necessarily tied to a sprite
struct RcBounds {
    muflot x1;
    muflot y1;
    muflot x2;
    muflot y2;
    uint8_t state; //some of the same as RSprite, where & 1 = active
};

#define ISSPRITEACTIVE(s) (s.state & RSSTATEACTIVE)

struct RcSpriteGroup
{
    RcSprite * sprites;
    SSprite * tempsorting;
    const uint8_t numsprites;
    RcBounds * bounds;
    const uint8_t numbounds;
};

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
