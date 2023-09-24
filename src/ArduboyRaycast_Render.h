#pragma once

#include <FixedPoints.h>
#include <Arduboy2.h>

#include "ArduboyRaycast_Utils.h"
#include "ArduboyRaycast_Map.h"
#include "ArduboyRaycast_Player.h"
#include "ArduboyRaycast_SpriteGroup.h"
#include "ArduboyRaycast_Shading.h"

// Available flags for compilation
// #define RCNOCORNERSHADOWS      // Remove the corner shadows (used to increase readability of walls against floor)
// #define RCWHITEFOG             // Make distance shading white instead of black
// #define RCNOALTWALLSHADING     // Perpendicular walls are half-shaded to increase readability, this removes that
// #define RCSMALLLOOPS           // The raycaster makes use of loop unrolling, which adds about 1.5kb code. This removes that but performance severely drops

// Debug flags 
// #define RCGENERALDEBUG       // Must be set for any of the othere to work
// #define RCLINEHEIGHTDEBUG      // Display information about lineheight (only draws a few lines)
// #define RCPRINTSPRITEDATA      // Display information about certain sprite-related properties

#ifndef RCWALLSHADING
#define RCWALLSHADING 1           // 0 = no wall shading, 1 = shading, 2 = half resolution shading. "LIGHTINTENSITY" still affects draw distance regardless
#endif

#ifdef RCGENERALDEBUG
#include <Tinyfont.h>
#endif

// -------------- CALCULATED / ASSUMED CONSTANTS, TRY NOT TO TOUCH ------------------------
constexpr uint8_t BWIDTH = WIDTH >> 3;
// Some assumptions (please try to follow these instead of changing them)
constexpr uint8_t RCEMPTY = 0;
constexpr uint8_t RCTILESIZE = 16;
// ------------------------------------------------------------------------------


// A container for precalculated sprite information. These are calculations we
// don't want to do per-frame
struct RcSpriteDrawPrecalc
{
    float fposX;
    float fposY;
    float invDet;
};


// A container for calculated sprite draw data. You know a calculation was not performed
// if stepX and stepY are 0
struct RcSpriteDrawData
{
    uint8_t drawStartX;
    uint8_t drawStartY;
    uint8_t drawEndY;
    uint8_t drawEndX;

    uflot texXInit;
    uflot texYInit;
    uflot stepX = 0;
    uflot stepY = 0;

    uflot transformY;
};


// Raycast renderer container, tracks data used for raycasting + lets you render raycasting
template<uint8_t W, uint8_t H>
class RcRender
{
public:
    static constexpr uint8_t VIEWWIDTH = W;
    static constexpr uint8_t VIEWHEIGHT = H;
    static constexpr uint8_t VIEWHEIGHTBYTES = VIEWHEIGHT >> 3;
    static constexpr uint8_t MIDSCREENY = VIEWHEIGHT / 2;
    static constexpr uint8_t MIDSCREENX = VIEWWIDTH / 2;
    static constexpr uflot INVWIDTH = 1.0 / VIEWWIDTH;
    static constexpr float INVHEIGHT = 1.0 / VIEWHEIGHT;
    static constexpr flot INVWIDTH2 = 2.0f / VIEWWIDTH;
    static constexpr uint16_t MAXLHEIGHT = 32768;
    static constexpr float MINLDISTANCE = 1.0f / MAXLHEIGHT;
    static constexpr float MINSPRITEDISTANCE = 0.2;

    uflot lightintensity = 1.0;     // Impacts view distance + shading even when no shading applied
    const uint8_t * tilesheet = NULL;
    const uint8_t * spritesheet = NULL;
    const uint8_t * spritesheet_mask = NULL;
    muflot spritescaling[4] = { 1.5, 1.0, 0.5, 0.25 };

    // I want these to be private but they're needed elsewhere
    uflot _viewdistance = 4.0;      // Calculated value
    uflot _darkness = 1.0;          // Calculated value
    uflot _distCache[VIEWWIDTH / 2]; // Half distance resolution means sprites will clip 1 pixel into walls sometimes but otherwise...

    #ifdef RCGENERALDEBUG
    Tinyfont * tinyfont;
    #endif

    // Clear the area represented by this raycaster
    inline void clearRaycast(Arduboy2Base * arduboy)
    {
        fastClear(arduboy, 0, 0, VIEWWIDTH, VIEWHEIGHT);
    }

    // Draw a fast(?) raycast background, assumed to start at 0,0. Your background should
    // NOT have the usual width/height. Will underdraw in the Y direction if VIEWHEIGHT not 
    // a multiple of 8
    inline void drawRaycastBackground(Arduboy2Base * arduboy, const uint8_t * bg)
    {
        for(uint8_t i = 0; i < VIEWHEIGHTBYTES; ++i)
            memcpy_P(arduboy->sBuffer + i * WIDTH, bg + i * VIEWWIDTH, VIEWWIDTH);
    }

    // Set the light intensity for raycasting. Performs several expensive calculations, only set this
    // when necessary
    void setLightIntensity(uflot intensity)
    {
        // Skip complicated calcs
        if(intensity == this->lightintensity)
            return;

        this->lightintensity = intensity;
        this->_viewdistance = sqrt(BAYERGRADIENTS * (float)intensity);
        this->_darkness = 1 / intensity;
    }

    // The full function for raycasting. 
    void raycastWalls(RcPlayer * p, RcMap * map, Arduboy2Base * arduboy)
    {
        uint8_t pmapX = p->posX.getInteger();
        uint8_t pmapY = p->posY.getInteger();
        uflot pmapofsX = p->posX - pmapX;
        uflot pmapofsY = p->posY - pmapY;
        flot fposX = (flot)p->posX, fposY = (flot)p->posY;
        flot dX = p->dirX, dY = p->dirY;
        const uint8_t * tilesheet = this->tilesheet;
        uflot darkness = this->_darkness;
        uflot viewdistance = this->_viewdistance;
        uflot * distCache = this->_distCache;

        uint8_t shade = 0;
        uint8_t texX = 0;
        uint16_t texData = 0;

        uint8_t lastTile = RCEMPTY;
        uint8_t lastTexX = 0;

        for (uint8_t x = 0; x < VIEWWIDTH; x++)
        {
            flot cameraX = x * INVWIDTH2 - 1; // x-coordinate in camera space

            // The camera plane is a simple -90 degree rotation on the player direction (as required for this algorithm).
            flot rayDirX = dX + dY * cameraX;
            flot rayDirY = dY - dX * cameraX;

            // length of ray from one x or y-side to next x or y-side. But we prefill it with
            // some initial data which has to be massaged later.
            uflot deltaDistX = (uflot)abs(rayDirX); //Temp value; may not be used
            uflot deltaDistY = (uflot)abs(rayDirY); //same

            // length of ray from current position to next x or y-side
            uflot sideDistX = MAXFIXED;
            uflot sideDistY = MAXFIXED;

            // what direction to step in x or y-direction (either +1 or -1)
            int8_t stepX = 0;
            int8_t stepY = 0;

            // With this DDA stepping algorithm, have to be careful about making too-large values
            // with our tiny fixed point numbers. Make some arbitrarily small cutoff point for
            // even trying to deal with steps in that direction. As long as the map size is 
            // never larger than 1 / NEARZEROFIXED on any side, it will be fine (that means
            // map has to be < 100 on a side with this)
            if(deltaDistX > NEARZEROFIXED) {
                deltaDistX = uReciprocalNearUnit(deltaDistX); 
                if (rayDirX < 0) {
                    stepX = -1;
                    sideDistX = pmapofsX * deltaDistX;
                }
                else {
                    stepX = 1;
                    sideDistX = (1 - pmapofsX) * deltaDistX;
                }
            }
            if(deltaDistY > NEARZEROFIXED) {
                deltaDistY = uReciprocalNearUnit(deltaDistY); 
                if (rayDirY < 0) {
                    stepY = -1;
                    sideDistY = pmapofsY * deltaDistY;
                }
                else {
                    stepY = 1;
                    sideDistY = (1 - pmapofsY) * deltaDistY;
                }
            }

            uint8_t side;           // was a NS or a EW wall hit?
            uint8_t mapX = pmapX;   // which box of the map the ray collision is in
            uint8_t mapY = pmapY;
            uflot perpWallDist = 0;   // perpendicular distance (not real distance)
            uint8_t tile = RCEMPTY;   // tile that was hit by ray

            // perform DDA. A do/while loop is ever-so-slightly faster it seems?
            do
            {
                // jump to next map square, either in x-direction, or in y-direction
                if (sideDistX < sideDistY) {
                    perpWallDist = sideDistX; // Remember that sideDist is actual distance and not distance only in 1 direction
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0; //0 = xside hit
                }
                else {
                    perpWallDist = sideDistY;
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1; //1 = yside hit
                }
                // Check if ray has hit a wall
                tile = map->getCell(mapX, mapY);
            }
            while (perpWallDist < viewdistance && tile == RCEMPTY);

            //Only calc distance for every other point to save a lot of memory (100 bytes)
            if((x & 1) == 0)
            {
                distCache[x >> 1] = perpWallDist;

                #if RCWALLSHADING == 2
                //Since we're in here anyway, do the half-res shading too (if requested)
                shade = calcShading(perpWallDist, x, darkness);
                #endif
            }

            #if RCWALLSHADING == 1
            //Full res shading happens every frame of course
            shade = calcShading(perpWallDist, x, darkness);
            #elif RCWALLSHADING == 0
            //If you're not having any wall shading, it's always fully set
            shade = 0xFF;
            #endif

            #ifndef RCNOALTWALLSHADING
            //Every other row of alt walls get no shading to make them easier to read
            if(side & x) shade = 0;
            #endif

            // If the above loop was exited without finding a tile, there's nothing to draw
            if(tile == RCEMPTY) continue;

            //NOTE: wallX technically can only be positive, but I'm using flot to save a tiny amount from casting
            flot wallX = side ? fposX + (flot)perpWallDist * rayDirX : fposY + (flot)perpWallDist * rayDirY;
            wallX -= floorFixed(wallX); //.getFraction isn't working!
            texX = uint8_t(wallX * RCTILESIZE);
            if((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0)) texX = RCTILESIZE - 1 - texX;

            //Loading the texture is more expensive than this if statement
            if(tile != lastTile || texX != lastTexX)
            {
                texData = readTextureStrip16(tilesheet, tile, texX);
                lastTile = tile;
                lastTexX = texX;
            }

            // Calculate half height of line to draw on screen. We already know the distance to the wall.
            // We can truncate the total height if too close to the wall right here and now and avoid future checks.
            //uint16_t lineHeight = (perpWallDist <= MINLDISTANCE) ? MAXLHEIGHT : (VIEWHEIGHT / (float)perpWallDist);

            #ifdef RCLINEHEIGHTDEBUG
            tinyfont.setCursor(16, x * 16);
            tinyfont.println(lineHeight);
            tinyfont.setCursor(16, x * 16 + 8);
            tinyfont.println((float)perpWallDist);
            if(x > 2) break;
            #endif

            //ending should be exclusive
            drawWallLine(x, perpWallDist, shade, texData, arduboy);
        }
    }

    //Draw a single raycast wall line. Will only draw specifically the wall line and will clip out all the rest
    //(so you can predraw a ceiling and floor before calling raycast)
    void drawWallLine(uint8_t x, uflot distance, uint8_t shade, uint16_t texData, Arduboy2Base * arduboy)
    {
        // ------- BEGIN CRITICAL SECTION -------------
        float invLineHeight = INVHEIGHT * (float)distance; 
        UFixed<16,16> step = RCTILESIZE * invLineHeight;

        uint16_t lineHeight = (invLineHeight <= MINLDISTANCE) ? MAXLHEIGHT : (uint16_t)(1 / invLineHeight);

        int16_t halfLine = lineHeight >> 1;
        uint8_t yStart = max(0, MIDSCREENY - halfLine);
        uint8_t yEnd = min(VIEWHEIGHT, MIDSCREENY + halfLine); //EXCLUSIVE

        //Everyone prefers the high precision tiles (and for some reason, it's now faster? so confusing...)
        UFixed<16,16> texPos = (yStart + halfLine - MIDSCREENY) * step;

        //These four variables are needed as part of the loop unrolling system
        uint16_t bofs;
        uint8_t texByte;
        uint8_t thisWallByte = (((yStart >> 1) >> 1) >> 1);
        uint8_t * sbuffer = arduboy->sBuffer;

        uint8_t fullstep = step.getInteger();
        uint8_t accustep = (step.getFraction() >> 8);
        uint8_t accum = (texPos.getFraction() >> 8);
        texData >>= texPos.getInteger();

        // Different kind of shade check for white fog
        #ifdef RCWHITEFOG
        #define _WALLSHADECHECK(bm) !(shade & (bm)) ||
        #else
        #define _WALLSHADECHECK(bm) (shade & (bm)) &&
        #endif

        //Pull wall byte, save location
        #define _WALLREADBYTE() bofs = thisWallByte * WIDTH + x; texByte = sbuffer[bofs];
        //Write previously read wall byte, go to next byte
        #define _WALLWRITENEXT() sbuffer[bofs] = texByte; thisWallByte++;
        //Work for setting bits of wall byte. Use an imperfect overflow accumulator to approximate stepping through texture.
        #define _WALLBITUNROLL(bm,nbm) \
            if(_WALLSHADECHECK(bm) (texData & 1)) texByte |= (bm); \
            else texByte &= (nbm); \
            asm volatile( \
                "add %[accum], %[step]    \n" \
                "brcc .+4       \n" \
                "lsr %B[td]     \n" \
                "ror %A[td]     \n" \
                : [accum] "+&r" (accum), \
                  [td] "+&r" (texData)    \
                : [step] "r" (accustep) \
            ); \
            if(fullstep) { texData >>= fullstep; }
        
        // The above volatile section is a special fractional accumulator which uses the carry bit to determine
        // if an additional right shift of the texture is in order

        _WALLREADBYTE();

        #ifndef RCSMALLLOOPS

        uint8_t startByte = thisWallByte; //The byte within which we start, always inclusive
        uint8_t endByte = (((yEnd >> 1) >> 1) >> 1);  //The byte to end the unrolled loop on. Could be inclusive or exclusive

        //First and last bytes are tricky
        if(yStart & 7)
        {
            uint8_t endFirst = min((startByte + 1) * 8, yEnd);
            uint8_t bm = fastlshift8(yStart & 7);

            for (uint8_t i = yStart; i < endFirst; i++)
            {
                _WALLBITUNROLL(bm, (~bm));
                bm <<= 1;
            }

            //Move to next, like it never happened
            _WALLWRITENEXT();
            _WALLREADBYTE();
        }

        // Now the unrolled loop
        while (thisWallByte < endByte)
        {
            _WALLBITUNROLL(0b00000001, 0b11111110);
            _WALLBITUNROLL(0b00000010, 0b11111101);
            _WALLBITUNROLL(0b00000100, 0b11111011);
            _WALLBITUNROLL(0b00001000, 0b11110111);
            _WALLBITUNROLL(0b00010000, 0b11101111);
            _WALLBITUNROLL(0b00100000, 0b11011111);
            _WALLBITUNROLL(0b01000000, 0b10111111);
            _WALLBITUNROLL(0b10000000, 0b01111111);
            _WALLWRITENEXT();
            _WALLREADBYTE();
        }

        //Last byte, but only need to do it if we don't simply span one byte
        if((yEnd & 7) && startByte != endByte)
        {
            uint8_t endStart = thisWallByte * 8;
            uint8_t bm = fastlshift8(endStart & 7);
            for (uint8_t i = endStart; i < yEnd; i++)
            {
                _WALLBITUNROLL(bm, (~bm));
                bm <<= 1;
            }

            //"Don't repeat yourself": that ship has sailed. Anyway, only write the last byte if we need to, otherwise
            //we could legitimately write outside the bounds of the screen.
            #ifndef RCNOCORNERSHADOWS
            sbuffer[bofs] = texByte & ~(fastlshift8(yEnd & 7));
            #else
            sbuffer[bofs] = texByte;
            #endif
        }

        #else // No loop unrolling

        //Funny hack; code is written for loop unrolling first, so we have to kind of "fit in" to the macro system
        if((yStart & 7) == 0) thisWallByte--;
        uint8_t bm = fastlshift8(yStart & 7);

        do
        {
            uint8_t bidx = yStart & 7;

            // Every new byte, save the current (previous) byte and load the new byte from the screen. 
            // This might be wasteful, as only the first and last byte technically need to pull from the screen. 
            if(bidx == 0) {
                bm = 1;
                _WALLWRITENEXT();
                _WALLREADBYTE();
            }

            _WALLBITUNROLL(bm, ~bm);
            bm <<= 1;
        }
        while(++yStart < yEnd);

        //The above loop specifically can't end where bidx = 0 and thus placing us outside the writable area. 
        //Note that corner shadows are SLIGHTLY different between loop unrolled and not: we MUST move yEnd up,
        //but the previous does not, giving perhaps a better effect
        #ifndef RCNOCORNERSHADOWS
        sbuffer[bofs] = texByte & ~(fastlshift8((yEnd - 1) & 7));
        #else
        sbuffer[bofs] = texByte;
        #endif

        #endif

        // ------- END CRITICAL SECTION -------------
    }


    //Precalculate some sprite drawing stuff, happens before any loop, not tied to a sprite
    RcSpriteDrawPrecalc precalcSpriteDraw(RcPlayer * player)
    {
        RcSpriteDrawPrecalc result;

        result.fposX = (float)player->posX;
        result.fposY = (float)player->posY;
        result.invDet = 1.0 / (player->dirY * player->dirY + player->dirX * player->dirX); // required for correct matrix multiplication

        return result;
    }

    template<uint8_t InternalStateBytes>
    RcSpriteDrawData calcSpriteDraw(RcSpriteDrawPrecalc * calc, RcPlayer * player, RcSprite<InternalStateBytes> * sprite)
    {
        RcSpriteDrawData result;

        float spriteX = (float)sprite->x - calc->fposX;
        float spriteY = (float)sprite->y - calc->fposY;

        float transformYT = calc->invDet * (player->dirX * spriteX + player->dirY * spriteY); // this is actually the depth inside the screen, that what Z is in 3D

        // Nice quick shortcut to get out for sprites behind us (and ones that are too close)
        if (transformYT < MINSPRITEDISTANCE)
            return result;

        float transformXT = calc->invDet * (player->dirY * spriteX - player->dirX * spriteY);

        // int16 because easy overflow! if x is much larger than y, then you're effectively multiplying 50 by map width.
        //  NOTE: this is the CENTER of the sprite, not the edge (thankfully)
        int16_t spriteScreenX = int16_t(MIDSCREENX * (1 + transformXT / transformYT));

        // calculate the dimensions of the sprite on screen. All sprites are square. Size mods go here
        // using 'transformY' instead of the real distance prevents fisheye
        uint16_t spriteHeight = uint16_t(VIEWHEIGHT / transformYT * (float)this->spritescaling[(sprite->state & RSSTATESIZE) >> 1]);
        uint16_t spriteWidth = spriteHeight;

        // calculate lowest and highest pixel to fill. Sprite screen/start X and Sprite screen/start Y
        // Because we have 1 fewer bit to store things, we unfortunately need an int16
        int16_t ssX = -(spriteWidth >> 1) + spriteScreenX; // Offsets go here, but modified by distance or something?
        int16_t ssXe = ssX + spriteWidth;                  // EXCLUSIVE

        // Get out if sprite is completely outside view
        if (ssXe < 0 || ssX > VIEWWIDTH)
            return result;

        // Calculate vertical shift from top 5 bits of state
        uint8_t yShiftBits = ((sprite->state >> 1) >> 1) >> 1;
        int16_t yShift = yShiftBits ? int16_t((yShiftBits & 16 ? -(yShiftBits & 15) : (yShiftBits & 15)) * 2.0 / transformYT) : 0;
        // The above didn't work without float math, didn't feel like figuring out the ridiculous type casting

        int16_t ssY = -(spriteHeight >> 1) + MIDSCREENY + yShift;
        int16_t ssYe = ssY + spriteHeight; // EXCLUSIVE

        if (ssYe < 0 || ssY > VIEWHEIGHT)
            return result;

        result.drawStartY = ssY < 0 ? 0 : ssY; // Because of these checks, we can store them in 1 byte stuctures
        result.drawEndY = ssYe > VIEWHEIGHT ? VIEWHEIGHT : ssYe;
        result.drawStartX = ssX < 0 ? 0 : ssX;
        result.drawEndX = ssXe > VIEWWIDTH ? VIEWWIDTH : ssXe;

        // Setup stepping to avoid costly mult (and div) in critical loops
        // These float divisions happen just once per sprite, hopefully that's not too bad.
        // There used to be an option to set the precision of sprites but it didn't seem to make any difference
        float stepXf = (float)RCTILESIZE / spriteWidth;
        float stepYf = (float)RCTILESIZE / spriteHeight;

        result.texXInit = (result.drawStartX - ssX) * stepXf; // This unfortunately needs float because of precision glitches
        result.texYInit = (result.drawStartY - ssY) * stepYf;
        result.stepX = stepXf;
        result.stepY = stepYf;
        result.transformY = (uflot)transformYT;

        #ifdef RCPRINTSPRITEDATA
        //Clear a section for us to use
        constexpr uint8_t sdh = 10;
        arduboy->fillRect(0, HEIGHT - sdh, VIEWWIDTH, sdh, BLACK);
        //Print some junk
        tinyfont->setCursor(0, HEIGHT - sdh);
        tinyfont->print((float)transformXT, 4);
        tinyfont->print(" Y");
        tinyfont->print(ssY);
        tinyfont->setCursor(0, HEIGHT - sdh + 5);
        tinyfont->print((float)transformYT, 4);
        tinyfont->print(" W");
        tinyfont->print(spriteWidth);
        #endif

        return result;
    }


    template<uint8_t InternalStateBytes>
    void drawSprites(RcPlayer * player, RcSpriteGroup<InternalStateBytes> * group, Arduboy2Base * arduboy)
    {
        uint8_t usedSprites = group->sortSprites(player->posX, player->posY);

        // Buffers, we pull them out like this just to make it a little easier (might remove later)
        const uint8_t * spritesheet = this->spritesheet;
        const uint8_t * spritesheet_Mask = this->spritesheet_mask;
        uint8_t * sbuffer = arduboy->sBuffer;
        uflot * distCache = this->_distCache;

        RcSpriteDrawPrecalc precalc = precalcSpriteDraw(player);

        // after sorting the sprites, do the projection and draw them. We know all sprites in the array are active,
        // since we're looping against the sorted array.
        for (uint8_t i = 0; i < usedSprites; i++)
        {
            //Get the current sprite so we don't have to dereference multiple pointers
            RcSprite<InternalStateBytes> * sprite = group->sortedSprites[i].sprite;

            RcSpriteDrawData drawData = calcSpriteDraw(&precalc, player, sprite);

            // Skip drawing, it was determined nothing was needed
            if(drawData.stepX == 0 && drawData.stepY == 0) continue;

            uflot texX = drawData.texXInit;
            uint8_t fr = sprite->frame;

            uint8_t drawStartByte = (((drawData.drawStartY >> 1) >> 1) >> 1); //right shift 3 without loop
            uint8_t drawEndByte = (((drawData.drawEndY >> 1) >> 1) >> 1);  //The byte to end the unrolled loop on. Could be inclusive or exclusive
            uint16_t texData = 0;
            uint16_t texMask = 0;

            //uint8_t lastAccum;
            uint8_t accumStart = drawData.texYInit.getFraction();
            uint8_t accustep = drawData.stepY.getFraction();
            uint8_t fullstep = drawData.stepY.getInteger();
            uint8_t preshift = drawData.texYInit.getInteger();

            uint8_t x = drawData.drawStartX;

            // ------- BEGIN CRITICAL SECTION -------------
            do //For every strip (x)
            {
                //If the sprite is hidden, skip this line. Lots of calculations bypassed!
                if (drawData.transformY < distCache[x >> 1])
                {
                    uint8_t tx = texX.getInteger();

                    texData = readTextureStrip16(spritesheet, fr, tx) >> preshift;
                    texMask = readTextureStrip16(spritesheet_Mask, fr, tx) >> preshift;

                    //A small optimization for small sprites
                    if(!texMask) goto SKIPSPRITESTRIPE;

                    //These five variables (including texData+texMask) are needed as part of the loop unrolling system
                    uint16_t bofs;
                    uint8_t texByte;
                    uint8_t thisWallByte = drawStartByte;

                    uint8_t accum = accumStart;

                    //Pull screen byte, save location
                    #define _SPRITEREADSCRBYTE() bofs = thisWallByte * WIDTH + x; texByte = sbuffer[bofs];
                    //Write previously read screen byte, go to next byte
                    #define _SPRITEWRITESCRNEXT() sbuffer[bofs] = texByte; thisWallByte++;
                    //Work for setting bits of screen byte
                    #define _SPRITEBITUNROLL(bm,nbm) \
                        if (texMask & 1) { if (texData & 1) texByte |= bm; else texByte &= nbm; } \
                        asm volatile( \
                            "add %[accum], %[step]    \n" \
                            "brcc .+8       \n" \
                            "lsr %B[td]     \n" \
                            "ror %A[td]     \n" \
                            "lsr %B[td2]     \n" \
                            "ror %A[td2]     \n" \
                            : [accum] "+&r" (accum), \
                              [td] "+&r" (texData),  \
                              [td2] "+&r" (texMask)  \
                            : [step] "r" (accustep) \
                        ); \
                        if(fullstep) { texMask >>= fullstep; texData >>= fullstep; }

                    _SPRITEREADSCRBYTE();

                    #ifndef RCSMALLLOOPS

                    //First and last bytes are tricky
                    if(drawData.drawStartY & 7)
                    {
                        uint8_t endFirst = min((drawStartByte + 1) * 8, drawData.drawEndY);
                        uint8_t bm = fastlshift8(drawData.drawStartY & 7);

                        for (uint8_t i = drawData.drawStartY; i < endFirst; i++)
                        {
                            _SPRITEBITUNROLL(bm, (~bm));
                            bm <<= 1;
                        }

                        //Move to next, like it never happened
                        _SPRITEWRITESCRNEXT();
                        _SPRITEREADSCRBYTE();
                    }

                    //Now the unrolled loop
                    while(thisWallByte < drawEndByte)
                    {
                        _SPRITEBITUNROLL(0b00000001, 0b11111110);
                        _SPRITEBITUNROLL(0b00000010, 0b11111101);
                        _SPRITEBITUNROLL(0b00000100, 0b11111011);
                        _SPRITEBITUNROLL(0b00001000, 0b11110111);
                        _SPRITEBITUNROLL(0b00010000, 0b11101111);
                        _SPRITEBITUNROLL(0b00100000, 0b11011111);
                        _SPRITEBITUNROLL(0b01000000, 0b10111111);
                        _SPRITEBITUNROLL(0b10000000, 0b01111111);
                        _SPRITEWRITESCRNEXT();
                        _SPRITEREADSCRBYTE();
                    }

                    //Last byte, but only need to do it if we end in the middle of a byte and don't simply span one byte
                    if((drawData.drawEndY & 7) && drawStartByte != drawEndByte)
                    {
                        uint8_t endStart = thisWallByte * 8;
                        uint8_t bm = fastlshift8(endStart & 7);
                        for (uint8_t i = endStart; i < drawData.drawEndY; i++)
                        {
                            _SPRITEBITUNROLL(bm, (~bm));
                            bm <<= 1;
                        }

                        //Only need to set the last byte if we're drawing in it of course
                        sbuffer[bofs] = texByte;
                    }

                    #else // No loop unrolling

                    uint8_t y = drawData.drawStartY;

                    //Funny hack; code is written for loop unrolling first, so we have to kind of "fit in" to the macro system
                    if((drawData.drawStartY & 7) == 0) thisWallByte--;

                    do
                    {
                        uint8_t bidx = y & 7;

                        // Every new byte, save the current (previous) byte and load the new byte from the screen. 
                        // This might be wasteful, as only the first and last byte technically need to pull from the screen. 
                        if(bidx == 0) {
                            _SPRITEWRITESCRNEXT();
                            _SPRITEREADSCRBYTE();
                        }

                        uint8_t bm = fastlshift8(bidx);
                        _SPRITEBITUNROLL(bm, ~bm);
                    }
                    while(++y < drawData.drawEndY); //EXCLUSIVE

                    //The above loop specifically CAN'T reach the last byte, so although it's wasteful in the case of a 
                    //sprite ending at the bottom of the screen, it's still better than always incurring an if statement... maybe.
                    sbuffer[bofs] = texByte;

                    #endif

                }

                SKIPSPRITESTRIPE:
                //This ONE step is why there has to be a big if statement up there. 
                texX += drawData.stepX;
            }
            while(++x < drawData.drawEndX); //EXCLUSIVE
            // ------- END CRITICAL SECTION -------------

        }
    }
};
