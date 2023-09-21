#include "ArduboyRaycast_Shading.h"

inline uint8_t calcShading(uflot perpWallDist, uint8_t x, const uflot DARKNESS)
{
    uint8_t dither = floorFixed(perpWallDist * DARKNESS * perpWallDist).getInteger();
    return (dither >= BAYERGRADIENTS) ? 0 : pgm_read_byte(b_shading + (dither * 4) + (x & 3));
}