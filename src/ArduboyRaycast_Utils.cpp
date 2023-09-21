#include "ArduboyRaycast_Utils.h"

void fastClear(Arduboy2Base * arduboy, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2)
{
    uint8_t yEnd = (y2 >> 3) + (y2 & 7 ? 1 : 0);
    //Arduboy2 fillrect is absurdly slow; I have the luxury of doing this instead
    for(uint8_t i = y >> 3; i < yEnd; ++i)
        memset(arduboy->sBuffer + (i * 128) + x, 0, x2 - x);
}

// Get 1/x where x is unit range only
flot fReciprocalUnit(flot x)
{
    if(x.getInteger())
        return 1; //This is dumb
    return flot::fromInternal(pgm_read_word(DIVISORS + (x.getInternal() & 0xFF))) * (x < 0 ? -1 : 1);
}
// Get 1/x where x is unit range only (uflot)
uflot uReciprocalUnit(uflot x)
{
    if(x.getInteger())
        return 1; //This is dumb
    return uflot::fromInternal(pgm_read_word(DIVISORS + (x.getInternal() & 0xFF)));
}

// Reciprocal of ALMOST unit length (ie 2 to -2) Reduces precision when in the outer range
uflot uReciprocalNearUnit(uflot x)
{
    if(x.getInteger())
        return uflot::fromInternal(pgm_read_word(DIVISORS + ((x * 0.5).getInternal() & 0xFF))) * 0.5;
    else
        return uflot::fromInternal(pgm_read_word(DIVISORS + (x.getInternal() & 0xFF)));
}

flot fReciprocalNearUnitNoSign(flot x)
{
    if(x.getInteger())
        return flot::fromInternal(pgm_read_word(DIVISORS + ((x * 0.5).getInternal() & 0xFF))) * 0.5;
    else
        return flot::fromInternal(pgm_read_word(DIVISORS + (x.getInternal() & 0xFF)));
}
