#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

// These are bitmasks to get data out of state
constexpr uint8_t RSSTATEACTIVE = 0b00000001;
constexpr uint8_t RSSTATESIZE = 0b00000110;
constexpr uint8_t RSTATEYOFFSET = 0b11111000;

#define ISSPRITEACTIVE(s) (s.state & RSSTATEACTIVE)


// Try to make this fit into as little space as possible
template<uint8_t InternalStateBytes>
class RcSprite 
{
public:
    muflot x; //Precision for x/y is low but doesn't really need to be high
    muflot y;
    uint8_t frame = 0;
    uint8_t state = 0; // First bit is active, next 2 are how many times to /2 for size
    void (* behavior)(RcSprite<InternalStateBytes> *,Arduboy2Base*) = NULL;

    uint8_t intstate[InternalStateBytes];
};

//typedef void (* behavior_func)(RcSprite *,Arduboy2Base *);

// Sorted sprite. Useful to keep original sprite list index as sprite ids 
template<uint8_t InternalStateBytes>
struct SSprite {
    RcSprite<InternalStateBytes> * sprite; 
    SFixed<11,4> distance;    //Unfortunately, distance kinda has to be large... 12 bits = 4096, should be more than enough
};

// A box representing bounds that the player shouldn't be able to walk into. Not necessarily tied to a sprite
class RcBounds 
{
public:
    muflot x1;
    muflot y1;
    muflot x2;
    muflot y2;
    uint8_t state; //some of the same as RSprite, where & 1 = active

    inline bool colliding(uflot x, uflot y) {
        return x > this->x1 && x < this->x2 && y > this->y1 && y < this->y2;
    }
};
