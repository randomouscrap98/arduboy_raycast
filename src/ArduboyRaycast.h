#pragma once

#include <FixedPoints.h>
#include <Arduboy2.h>

#include "ArduboyRaycast_Utils.h"
#include "ArduboyRaycast_Map.h"
#include "ArduboyRaycast_Sprite.h"


// These are the things you can set pre-emptively, they absolutely MUST be constants
#ifndef RCVIEWWIDTH
#define RCVIEWWIDTH WIDTH
#endif
#ifndef RCVIEWHEIGHT
#define RCVIEWHEIGHT HEIGHT
#endif

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
// Graphics constants
constexpr uint8_t VIEWWIDTH = RCVIEWWIDTH;
constexpr uint8_t VIEWHEIGHT = RCVIEWHEIGHT;
// Screen
constexpr uint8_t MIDSCREENY = VIEWHEIGHT / 2;
constexpr uint8_t MIDSCREENX = VIEWWIDTH / 2;
constexpr flot INVWIDTH = 1.0 / VIEWWIDTH;
constexpr flot INVHEIGHT = 1.0 / VIEWHEIGHT;
constexpr flot INVWIDTH2 = 2.0f / VIEWWIDTH;
constexpr uint8_t BWIDTH = WIDTH >> 3;
// Distance stuff; if you need to change this for some reason, idk just reconsider I guess
constexpr uint8_t LDISTSAFE = 16;
constexpr uflot MINLDISTANCE = 1.0f / LDISTSAFE;
constexpr uint16_t MAXLHEIGHT = VIEWHEIGHT * LDISTSAFE;
constexpr float MINSPRITEDISTANCE = 0.2;
// Some assumptions (please try to follow these instead of changing them)
constexpr uint8_t RCEMPTY = 0;
constexpr uint8_t RCTILESIZE = 16;
// ------------------------------------------------------------------------------


// A container to hold raycasting state. 
struct RaycastState
{
    uflot lightintensity = 1.0;     // Impacts view distance + shading even when no shading applied
    const uint8_t * tilesheet = NULL;
    const uint8_t * spritesheet = NULL;
    const uint8_t * spritesheet_mask = NULL;

    uflot _viewdistance = 4.0;      // Calculated value
    uflot _darkness = 1.0;          // Calculated value
    uflot _distCache[50]; // Half distance resolution means sprites will clip 1 pixel into walls sometimes but otherwise...
};

// Set the light intensity for raycasting. Performs several expensive calculations, only set this
// when necessary
void setLightIntensity(RaycastState * state, uflot intensity);

// Full clear specifically the raycast area. Note that if your view height is not aligned to a byte boundary,
// this will overclear the raycast area.
void clearRaycast(Arduboy2Base * arduboy);

//Draw a single raycast wall line. Will only draw specifically the wall line and will clip out all the rest
//(so you can predraw a ceiling and floor before calling raycast)
void drawWallLine(uint8_t x, uint16_t lineHeight, uint8_t shade, uint16_t texData, Arduboy2Base * arduboy);

// The full function for raycasting. 
void raycastWalls(RcPlayer * p, RcMap * map, Arduboy2Base * arduboy, RaycastState * state);

void drawSprites(RcPlayer * player, RcSpriteGroup * group, Arduboy2Base * arduboy, RaycastState * state);
