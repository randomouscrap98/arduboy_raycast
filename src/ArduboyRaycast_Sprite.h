#pragma once

#include <FixedPoints.h>

#include "ArduboyRaycast_Utils.h"

// These are bitmasks to get data out of state
constexpr uint8_t RSSTATEACTIVE = 0b00000001;
constexpr uint8_t RSSTATESIZE = 0b00000110;
constexpr uint8_t RSSTATEYOFFSET = 0b11111000;

constexpr uint8_t RBSTATEACTIVE = 0b00000001;
constexpr uint8_t RBSTATESOLID = 0b00000010;


// Try to make this fit into as little space as possible
template<uint8_t InternalStateBytes>
class RcSprite 
{
public:
    muflot x; //Precision for x/y is low but doesn't really need to be high
    muflot y;
    uint8_t frame = 0;
    uint8_t state = 0; // First bit is active, next 2 are how many times to /2 for size
    void (* behavior)(RcSprite<InternalStateBytes> *) = NULL;

    uint8_t intstate[InternalStateBytes];

    void setActive(bool active) {
        this->state = (this->state & ~RSSTATEACTIVE) | (active ? RSSTATEACTIVE : 0);
    }

    void setSizeIndex(uint8_t size) {
        this->state = (this->state & ~RSSTATESIZE) | ((size << 1) & RSSTATESIZE);
    }

    void setHeight(int8_t height) {
        this->state = (this->state & ~RSSTATEYOFFSET) | (height < 0 ? 128 : 0) | ((abs(height) << 3) & RSSTATEYOFFSET);
    }

    inline bool isActive() {
        return this->state & RSSTATEACTIVE;
    }

    inline uint8_t getSizeIndex() {
        return (this-> state & RSSTATESIZE) >> 1;
    }

    inline int8_t getHeight() {
        return (this-> state & RSSTATEYOFFSET) >> 3;
    }
};

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

    void setActive(bool active) {
        this->state = (this->state & ~RBSTATEACTIVE) | (active ? RBSTATEACTIVE : 0);
    }

    void setSolid(bool solid) {
        this->state = (this->state & ~RBSTATESOLID) | (solid ? RBSTATESOLID : 0);
    }

    inline bool isActive() {
        return this->state & RBSTATEACTIVE;
    }

    inline bool isSolid() {
        return this->state & RBSTATESOLID;
    }
};
