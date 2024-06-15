#pragma once

#include <FixedPoints.h>
#include <Arduboy2.h>
#include <ArduboyFX.h>

#include "ArduboyRaycast_Utils.h"
#include "ArduboyRaycast_Map.h"
#include "ArduboyRaycast_Player.h"
#include "ArduboyRaycast_SpriteGroup.h"
#include "ArduboyRaycast_Shading.h"

// Available flags for compilation
// #define RCSMALLLOOPS           // The raycaster makes use of loop unrolling, which adds about 1.5kb code. This removes that but performance severely drops

// Debug flags 
// #define RCGENERALDEBUG       // Must be set for any of the othere to work
// #define RCLINEHEIGHTDEBUG      // Display information about lineheight (only draws a few lines)
// #define RCPRINTSPRITEDATA      // Display information about certain sprite-related properties

#ifdef RCGENERALDEBUG
#include <Tinyfont.h>
#endif

// -------------- CALCULATED / ASSUMED CONSTANTS, TRY NOT TO TOUCH ------------------------
constexpr uint8_t BWIDTH = WIDTH >> 3;
// Some assumptions (please try to follow these instead of changing them)
constexpr uint8_t RCEMPTY = 0;
constexpr uint8_t RCTILESIZE = 32;
constexpr uint8_t MIPMAPOFS[8] PROGMEM = {
    // Offsets into the TILE area to find the mipmap per "step" value.
    0, 128, 160, 160, 168, 168, 168, 168
    //0, 4, 6, 6, 7, 7, 7, 7
};
constexpr uint8_t MIPMAPWIDTHS[8] PROGMEM = {
    // The mipmap levels for each step value
    32, 16, 8, 8, 4, 4, 4, 4
};
constexpr uint8_t MIPMAPBYTES[8] PROGMEM = {
    // The bytes per stripe for each step value
    4, 2, 1, 1, 1, 1, 1, 1
};
// ------------------------------------------------------------------------------

struct MipMapInfo {
    uint8_t step;
    uint8_t offset;
    uint8_t width;
    uint8_t bytes;
};

MipMapInfo lastMipmapInfo;
MipMapInfo get_mipmap_info(uint8_t mipmap) {
    if(mipmap == lastMipmapInfo.step) return lastMipmapInfo;
    lastMipmapInfo.step = mipmap;
    lastMipmapInfo.offset = pgm_read_byte(MIPMAPOFS + mipmap);
    lastMipmapInfo.width = pgm_read_byte(MIPMAPWIDTHS + mipmap);
    lastMipmapInfo.bytes = pgm_read_byte(MIPMAPBYTES + mipmap);
}


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

    MipMapInfo mminfo;

    uflot texXInit;
    uflot texYInit;
    uflot stepX = 0;
    uflot stepY = 0;

    uflot transformY;
};

enum RcShadingType : uint8_t
{
    None,
    Black,
    White
};

struct RcShadeInfo
{
    uint8_t shading;
    RcShadingType type;
};

#define RCMASKTOP(shading, shade, yofs) \
    if(shading.type == RcShadingType::White) shade &= pgm_read_byte(shade_mask + yofs); \
    else shade |= ~pgm_read_byte(shade_mask + yofs);
#define RCMASKBOTTOM(shading, shade, yofs) \
    if(shading.type == RcShadingType::White) shade &= ~pgm_read_byte(shade_mask + yofs); \
    else shade |= pgm_read_byte(shade_mask + yofs);

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
    static constexpr uflot SPRITEVIEWEXENTSION = 1;

    uflot lightintensity = 1.0;     // Impacts view distance + shading even when no shading applied
    uint24_t tilesheet;
    uint24_t spritesheet;
    uint24_t spritesheet_mask;
    muflot spritescaling[4] = { 1.5, 1.0, 0.5, 0.25 };

    uint8_t cornershading = 1;
    RcShadingType shading = RcShadingType::Black;
    RcShadingType altWallShading = RcShadingType::Black;
    RcShadingType spriteShading = RcShadingType::None; //Sprite shading is kinda weird

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

    // Calculate the appropriate shading for this wall slice given our rendering config
    inline RcShadeInfo calculateShading(uflot distance, uint8_t x, RcShadingType shading)
    {
        if(shading == RcShadingType::None)
        {
            return RcShadeInfo {
                0xFF,
                RcShadingType::Black
            };
        }
        else
        {
            RcShadeInfo result {
                calcShading(distance, x, this->_darkness),
                shading
            };
            if(shading == RcShadingType::White)
                result.shading = ~result.shading;
            return result;
        }
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
        uint8_t startMapIndex = map->getIndex(pmapX, pmapY);
        uflot pmapofsX = p->posX - pmapX;
        uflot pmapofsY = p->posY - pmapY;
        flot fposX = (flot)p->posX, fposY = (flot)p->posY;
        flot dX = p->dirX, dY = p->dirY;
        uflot viewdistance = this->_viewdistance;
        uflot * distCache = this->_distCache;

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
                    stepY = -map->width;
                    sideDistY = pmapofsY * deltaDistY;
                }
                else {
                    stepY = map->width;
                    sideDistY = (1 - pmapofsY) * deltaDistY;
                }
            }

            uint8_t side;           // was a NS or a EW wall hit?
            uint8_t mapIndex = startMapIndex;
            uflot perpWallDist = 0;   // perpendicular distance (not real distance)
            uint8_t tile;

            // perform DDA. A do/while loop is ever-so-slightly faster it seems?
            do
            {
                // jump to next map square, either in x-direction, or in y-direction
                if (sideDistX < sideDistY) {
                    perpWallDist = sideDistX; // Remember that sideDist is actual distance and not distance only in 1 direction
                    sideDistX += deltaDistX;
                    mapIndex += stepX;
                    side = 0; //0 = xside hit
                }
                else {
                    perpWallDist = sideDistY;
                    sideDistY += deltaDistY;
                    mapIndex += stepY;
                    side = 1; //1 = yside hit
                }
                tile = map->map[mapIndex];
            }
            while (perpWallDist < viewdistance && tile == RCEMPTY);

            //Only calc distance for every other point to save a lot of memory (100 bytes)
            if((x & 1) == 0)
                distCache[x >> 1] = perpWallDist;

            // If the above loop was exited without finding a tile, there's nothing to draw
            if(tile == RCEMPTY) continue;

            // Figure out NOW what the line height and mipmap level is is
            float invLineHeight = INVHEIGHT * (float)perpWallDist; 
            uint8_t mipmap = RCTILESIZE * invLineHeight;
            if(mipmap > 7) return;
            MipMapInfo mminfo = get_mipmap_info(mipmap);

            //NOTE: wallX technically can only be positive, but I'm using flot to save a tiny amount from casting
            flot wallX = side ? fposX + (flot)perpWallDist * rayDirX : fposY + (flot)perpWallDist * rayDirY;
            uint8_t texX = uint8_t((wallX - floorFixed(wallX)) * mminfo.width); //.getFraction isn't what you think!
            if((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0)) texX = mminfo.width - 1 - texX;

            UFixed<16,16> step = mminfo.width * invLineHeight;
            uint16_t lineHeight = (invLineHeight <= MINLDISTANCE) ? MAXLHEIGHT : (uint16_t)(1 / invLineHeight);
            uint32_t texData;

            if((side & x) && this->altWallShading != RcShadingType::None)
                texData = this->altWallShading == RcShadingType::Black ? 0x0 : 0xFFFFFFFF;
            else
                FX::readDataObject<uint32_t>(this->tilesheet + tile * 172 + mminfo.offset + texX * mminfo.bytes, texData);

            #ifdef RCLINEHEIGHTDEBUG
            tinyfont.setCursor(16, x * 16);
            tinyfont.println(lineHeight);
            tinyfont.setCursor(16, x * 16 + 8);
            tinyfont.println((float)perpWallDist);
            if(x > 2) break;
            #endif

            //ending should be exclusive
            drawWallLine(x, lineHeight, step, calculateShading(perpWallDist, x, this->shading), texData, arduboy);
        }
    }

    //Draw a single raycast wall line. Will only draw specifically the wall line and will clip out all the rest
    //(so you can predraw a ceiling and floor before calling raycast)
    //void drawWallLine(uint8_t x, uflot distance, RcShadeInfo shading, uint16_t texData, Arduboy2Base * arduboy)
    void drawWallLine(uint8_t x, uint16_t lineHeight, UFixed<16,16> step, RcShadeInfo shading, uint32_t texData, Arduboy2Base * arduboy)
    {
        // ------- BEGIN CRITICAL SECTION -------------
        int16_t halfLine = lineHeight >> 1;
        uint8_t yStart = max(0, MIDSCREENY - halfLine);
        uint8_t yEnd = min(VIEWHEIGHT, MIDSCREENY + halfLine); //EXCLUSIVE

        //Everyone prefers the high precision tiles (and for some reason, it's now faster? so confusing...)
        UFixed<16,16> texPos = (yStart + halfLine - MIDSCREENY) * step;

        //These four variables are needed as part of the loop unrolling system
        uint16_t bofs;
        uint8_t texByte;
        uint8_t thisWallByte = yStart;
        TOBYTECOUNT(thisWallByte);
        uint8_t * sbuffer = arduboy->sBuffer;
        uint8_t shade = shading.shading;

        uint8_t accustep = (step.getFraction() >> 8);
        uint8_t accum = (texPos.getFraction() >> 8);
        texData >>= texPos.getInteger();

        //Pull wall byte, save location
        #define _WALLREADBYTE() bofs = thisWallByte * WIDTH + x; texByte = sbuffer[bofs];
        //Write previously read wall byte, go to next byte
        #define _WALLWRITENEXT(mixin) if(shading.type == RcShadingType::Black) { sbuffer[bofs] = (texByte & shade) mixin; } else { sbuffer[bofs] = (texByte | shade) mixin;} thisWallByte++;
        //Work for setting bits of wall byte. Use an imperfect overflow accumulator to approximate stepping through texture.
        #define _WALLBITUNROLL(bm,nbm) \
            if(texData & 1) texByte |= (bm); \
            else texByte &= (nbm); \
            accum += accustep; \
            if (accum < accustep) { texData >>= 1; }

            //asm volatile( \
            //    "add %[accum], %[step]    \n" \
            //    "brcc .+4       \n" \
            //    "lsr %B[td]     \n" \
            //    "ror %A[td]     \n" \
            //    : [accum] "+&r" (accum), \
            //      [td] "+&r" (texData)   \
            //    : [step] "r" (accustep) \
            //); 
        
        // The above volatile section is a special fractional accumulator which uses the carry bit to determine
        // if an additional right shift of the texture is in order

        _WALLREADBYTE();

        #ifndef RCSMALLLOOPS

        uint8_t startByte = thisWallByte; //The byte within which we start, always inclusive
        uint8_t endByte = yEnd;
        TOBYTECOUNT(endByte);

        uint8_t yofs = yStart & 7;

        //First and last bytes are tricky
        if(yofs)
        {
            uint8_t endFirst = min((startByte + 1) * 8, yEnd);
            uint8_t bm = fastlshift8(yofs);

            for (uint8_t i = yStart; i < endFirst; i++) {
                _WALLBITUNROLL(bm, (~bm));
                bm <<= 1;
            }

            //Move to next, like it never happened (but mask shading)
            RCMASKTOP(shading, shade, yofs);
            _WALLWRITENEXT();
            _WALLREADBYTE();
            shade = shading.shading;
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

        yofs = yEnd & 7;
        //Last byte, but only need to do it if we don't simply span one byte
        if(yofs && startByte != endByte)
        {
            uint8_t bm = 1;
            for (uint8_t i = thisWallByte * 8; i < yEnd; i++) {
                _WALLBITUNROLL(bm, (~bm));
                bm <<= 1;
            }

            //"Don't repeat yourself": that ship has sailed. Anyway, only write the last byte if we need to, otherwise
            //we could legitimately write outside the bounds of the screen.
            RCMASKBOTTOM(shading, shade, yofs);
            if(cornershading) { _WALLWRITENEXT(& ~(fastlshift8(yofs))); }
            else { _WALLWRITENEXT(); }
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

        if(cornershading && yEnd != VIEWHEIGHT) { _WALLWRITENEXT(& ~(fastlshift8(yEnd & 7))); }
        else { _WALLWRITENEXT(); }

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

        // Nice quick shortcut to get out for sprites behind us (and ones that are too close / far)
        if (transformYT < MINSPRITEDISTANCE ) //|| transformYT > _viewdistance + SPRITEVIEWEXENTSION) //_sviewdistance)
            return result;

        float invTransformYT = 1 / transformYT;
        float transformXT = calc->invDet * (player->dirY * spriteX - player->dirX * spriteY);

        // int16 because easy overflow! if x is much larger than y, then you're effectively multiplying 50 by map width.
        //  NOTE: this is the CENTER of the sprite, not the edge (thankfully)
        int16_t spriteScreenX = int16_t(MIDSCREENX * (1 + transformXT * invTransformYT));

        // calculate the dimensions of the sprite on screen. All sprites are square. Size mods go here
        // using 'transformY' instead of the real distance prevents fisheye
        float scale = (float)this->spritescaling[(sprite->state & RSSTATESIZE) >> 1];
        uint16_t spriteHeight = uint16_t(VIEWHEIGHT * invTransformYT * scale);
        uint16_t spriteWidth = spriteHeight;

        // calculate lowest and highest pixel to fill. Sprite screen/start X and Sprite screen/start Y
        // Because we have 1 fewer bit to store things, we unfortunately need an int16
        int16_t ssX = -(spriteWidth >> 1) + spriteScreenX; // Offsets go here, but modified by distance or something?
        int16_t ssXe = ssX + spriteWidth;                  // EXCLUSIVE

        // Get out if sprite is completely outside view
        if (ssXe < 0 || ssX > VIEWWIDTH)
            return result;

        // Calculate vertical shift from top 5 bits of state
        uint8_t yShiftBits = sprite->state;
        TOBYTECOUNT(yShiftBits); //((sprite->state >> 1) >> 1) >> 1;
        int16_t yShift = yShiftBits ? int16_t((yShiftBits & 16 ? -(yShiftBits & 15) : (yShiftBits & 15)) * max(scale, 1.0) * 2.0 * invTransformYT) : 0;
        // The above didn't work without float math, didn't feel like figuring out the ridiculous type casting

        int16_t ssY = -(spriteHeight >> 1) + MIDSCREENY + yShift;
        int16_t ssYe = ssY + spriteHeight; // EXCLUSIVE

        if (ssYe < 0 || ssY > VIEWHEIGHT)
            return result;

        float invHeight = 1.0 / spriteHeight;
        uint8_t mipmap = RCTILESIZE * invHeight;
        if(mipmap > 7) return;
        result.mminfo = get_mipmap_info(mipmap);

        result.drawStartY = ssY < 0 ? 0 : ssY; // Because of these checks, we can store them in 1 byte stuctures
        result.drawEndY = ssYe > VIEWHEIGHT ? VIEWHEIGHT : ssYe;
        result.drawStartX = ssX < 0 ? 0 : ssX;
        result.drawEndX = ssXe > VIEWWIDTH ? VIEWWIDTH : ssXe;

        // Setup stepping to avoid costly mult (and div) in critical loops
        // These float divisions happen just once per sprite, hopefully that's not too bad.
        // There used to be an option to set the precision of sprites but it didn't seem to make any difference
        float stepXf = (float)result.mminfo.width / spriteWidth;
        float stepYf = result.mminfo.width * invHeight;

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
        const uint24_t spritesheet = this->spritesheet;
        const uint24_t spritesheet_Mask = this->spritesheet_mask;
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

            uint8_t drawStartByte = drawData.drawStartY;
            TOBYTECOUNT(drawStartByte); 
            uint8_t drawEndByte = drawData.drawEndY;
            TOBYTECOUNT(drawEndByte); 
            uint32_t texData = 0;
            uint32_t texMask = 0;

            uint8_t accumStart = drawData.texYInit.getFraction();
            uint8_t accustep = drawData.stepY.getFraction();
            uint8_t preshift = drawData.texYInit.getInteger();

            uint8_t x = drawData.drawStartX;

            // ------- BEGIN CRITICAL SECTION -------------
            do //For every strip (x)
            {
                //If the sprite is hidden, skip this line. Lots of calculations bypassed!
                if (drawData.transformY < distCache[x >> 1])
                {
                    uint8_t tx = texX.getInteger();

                    FX::readDataObject<uint32_t>(spritesheet + fr * 172 + tx * drawData.mminfo.bytes + drawData.mminfo.offset, texData);
                    //FX::readDataObject<uint32_t>(spritesheet_Mask + fr * 172 + tx * drawData.mminfo.bytes + drawData.mminfo.offset, texMask);
                    texMask = 0xFFFFFFFF;
                    texData >>= preshift;
                    texMask >>= preshift;

                    //A small optimization for small sprites
                    if(!texMask) goto SKIPSPRITESTRIPE;

                    RcShadeInfo shading = this->calculateShading(drawData.transformY, x, this->spriteShading);
                    uint8_t shade = shading.shading;

                    //These five variables (including texData+texMask) are needed as part of the loop unrolling system
                    uint16_t bofs;
                    uint8_t texByte;
                    uint8_t maskByte;
                    uint8_t thisWallByte = drawStartByte;

                    uint8_t accum = accumStart;

                    //Pull screen byte, save location
                    #define _SPRITEREADSCRBYTE() bofs = thisWallByte * WIDTH + x; texByte = sbuffer[bofs]; maskByte = 0;
                    //Write previously read screen byte, go to next byte
                    #define _SPRITEWRITESCRNEXT() if(shading.type == RcShadingType::Black) { sbuffer[bofs] = (texByte & (shade | ~maskByte)); } else { sbuffer[bofs] = (texByte | (shade & maskByte));} thisWallByte++;
                    //Work for setting bits of screen byte
                    #define _SPRITEBITUNROLL(bm,nbm) \
                        if (texMask & 1) { if (texData & 1) texByte |= bm; else texByte &= nbm; maskByte |= bm; } \
                        accum += accustep; \
                        if (accum < accustep) { texData >>= 1; texMask >>= 1; }

                        // asm volatile( \
                        //     "add %[accum], %[step]    \n" \
                        //     "brcc .+8       \n" \
                        //     "lsr %B[td]     \n" \
                        //     "ror %A[td]     \n" \
                        //     "lsr %B[td2]     \n" \
                        //     "ror %A[td2]     \n" \
                        //     : [accum] "+&r" (accum), \
                        //       [td] "+&r" (texData),  \
                        //       [td2] "+&r" (texMask)  \
                        //     : [step] "r" (accustep) \
                        // ); \
                        //if(fullstep) { texMask >>= fullstep; texData >>= fullstep; }

                    _SPRITEREADSCRBYTE();

                    #ifndef RCSMALLLOOPS

                    uint8_t yofs = drawData.drawStartY & 7;

                    //First and last bytes are tricky
                    if(yofs)
                    {
                        uint8_t endFirst = min((drawStartByte + 1) * 8, drawData.drawEndY);
                        uint8_t bm = fastlshift8(yofs);

                        for (uint8_t i = drawData.drawStartY; i < endFirst; i++)
                        {
                            _SPRITEBITUNROLL(bm, (~bm));
                            bm <<= 1;
                        }

                        //Move to next, like it never happened
                        RCMASKTOP(shading, shade, yofs);
                        _SPRITEWRITESCRNEXT();
                        _SPRITEREADSCRBYTE();
                        shade = shading.shading;
                    }

                    //Now the unrolled loop
                    while (thisWallByte < drawEndByte)
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

                    yofs = drawData.drawEndY & 7;

                    //Last byte, but only need to do it if we end in the middle of a byte and don't simply span one byte
                    if(yofs && drawStartByte != drawEndByte)
                    {
                        uint8_t endStart = thisWallByte * 8;
                        uint8_t bm = fastlshift8(endStart & 7);
                        for (uint8_t i = endStart; i < drawData.drawEndY; i++)
                        {
                            _SPRITEBITUNROLL(bm, (~bm));
                            bm <<= 1;
                        }

                        //Only need to set the last byte if we're drawing in it of course
                        RCMASKBOTTOM(shading, shade, yofs);
                        _SPRITEWRITESCRNEXT();
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
                    //if(drawData.drawEndY & 7)
                    _SPRITEWRITESCRNEXT();
                    //sbuffer[bofs] = texByte;

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
