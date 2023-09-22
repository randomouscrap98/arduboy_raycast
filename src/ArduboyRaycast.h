#pragma once

#include <FixedPoints.h>
#include <Arduboy2.h>

#include "ArduboyRaycast_Utils.h"
#include "ArduboyRaycast_Map.h"
#include "ArduboyRaycast_Sprite.h"


// Unless you ask for custom flags, raycaster just sets some sane defaults
#ifndef RCCUSTOMFLAGS
// Visualization flags (barely impacts performance) 
#define CORNERSHADOWS       // Shadows at the bottom of walls help differentiate walls from floor
#define WALLSHADING 1       // Unset = no wall shading, 1 = shading, 2 = half resolution shading. "LIGHTINTENSITY" still affects draw distance regardless
#define ALTWALLSHADING      // Make perpendicular walls half-shaded to increase readability
//#define WHITEFOG            // Make distance shading white instead of black
// Optimization flags (greatly impacts performance... I think?) 
#define CRITICALLOOPUNROLLING   // This adds a large (~1.5kb) amount of code but significantly increases performance, especially sprites
// Debug flags 
// #define LINEHEIGHTDEBUG      // Display information about lineheight (only draws a few lines)
#endif

// -------------- CALCULATED / ASSUMED CONSTANTS, TRY NOT TO TOUCH ------------------------
// Screen
constexpr uint8_t BWIDTH = WIDTH >> 3;
// Distance stuff; if you need to change this for some reason, idk just reconsider I guess
// Some assumptions (please try to follow these instead of changing them)
constexpr uint8_t RCEMPTY = 0;
constexpr uint8_t RCTILESIZE = 16;
// ------------------------------------------------------------------------------


// A container to hold raycasting state. 
template<uint8_t W, uint8_t H>
class RaycastInstance
{
    public:
        static constexpr uint8_t VIEWWIDTH = W;
        static constexpr uint8_t VIEWHEIGHT = H;
        static constexpr uint8_t MIDSCREENY = VIEWHEIGHT / 2;
        static constexpr uint8_t MIDSCREENX = VIEWWIDTH / 2;
        static constexpr flot INVWIDTH = 1.0 / VIEWWIDTH;
        static constexpr flot INVHEIGHT = 1.0 / VIEWHEIGHT;
        static constexpr flot INVWIDTH2 = 2.0f / VIEWWIDTH;
        static constexpr uint8_t LDISTSAFE = 16;
        static constexpr uflot MINLDISTANCE = 1.0f / LDISTSAFE;
        static constexpr uint16_t MAXLHEIGHT = VIEWHEIGHT * LDISTSAFE;
        static constexpr float MINSPRITEDISTANCE = 0.2;

        uflot lightintensity = 1.0;     // Impacts view distance + shading even when no shading applied
        const uint8_t * tilesheet = NULL;
        const uint8_t * spritesheet = NULL;
        const uint8_t * spritesheet_mask = NULL;

    // I want these to be private but they're needed elsewhere
        uflot _viewdistance = 4.0;      // Calculated value
        uflot _darkness = 1.0;          // Calculated value
        uflot _distCache[VIEWWIDTH / 2]; // Half distance resolution means sprites will clip 1 pixel into walls sometimes but otherwise...

        // Set the light intensity for raycasting. Performs several expensive calculations, only set this
        // when necessary
        void setLightIntensity(uflot intensity);

        // The full function for raycasting. 
        void raycastWalls(RcPlayer * p, RcMap * map, Arduboy2Base * arduboy);

        void drawSprites(RcPlayer * player, RcSpriteGroup * group, Arduboy2Base * arduboy);

        inline void clearRaycast(Arduboy2Base * arduboy)
        {
            fastClear(arduboy, 0, 0, VIEWWIDTH, VIEWHEIGHT);
        }

        //Draw a single raycast wall line. Will only draw specifically the wall line and will clip out all the rest
        //(so you can predraw a ceiling and floor before calling raycast)
        void drawWallLine(uint8_t x, uint16_t lineHeight, uint8_t shade, uint16_t texData, Arduboy2Base * arduboy);
};
