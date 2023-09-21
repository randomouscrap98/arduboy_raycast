#pragma once

#include <Arduboy2.h>
#include <FixedPoints.h>

// I modify these for testing, just nice to have it abstracted
typedef SFixed<7,8> flot;
typedef UFixed<8,8> uflot;

// A very tiny float, but generally enough to place you within the map.
// Map max is currently 16x16, so 4 bits is enough to place anything within.
// NOTE: NOT ENOUGH FOR RENDERING, NEED MORE FLOAT PRECISION!
typedef UFixed<4,4> muflot;

// Funny hack constants. We're working with very small ranges, so 
// the values have to be picked carefully. The following must remain true:
// 1 / NEARZEROFIXED < MAXFIXED. It may even need to be < MAXFIXED / 2
constexpr uflot MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 1.0f / 128; // Prefer accuracy (fixed decimal exact)

// Mod the given menu position m the given amount v against a max of x
#define menumod(m, v, x) m = ((m) + (v) + (x)) % (x)

// Make value odd by subtracting 1 if necessary
#define oddify(v) if((v & 1) == 0) v -= 1

// Only works for 16x16 textures
inline uint16_t readTextureStrip16(const uint8_t * tex, uint8_t tile, uint8_t strip)
{
    //32 is a constant: 16x16 textures take up 32 bytes
    const uint8_t * tofs = tex + tile * 32 + strip;
    //textureOffset16(tex, tile, strip);
    return pgm_read_byte(tofs) + 256 * pgm_read_byte(tofs + 16);
}

// A rectangle starting at x, y and having side w, h
struct MRect {
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t w = 0;
    uint8_t h = 0;
};

// Clear screen in a fast block. Note that y will be shifted down and y2
// shifted up to the nearest multiple of 8 to be byte aligned, so you 
// may not get the exact box you want. X2 and Y2 are exclusive
void fastClear(Arduboy2Base * arduboy, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2)
{
    uint8_t yEnd = (y2 >> 3) + (y2 & 7 ? 1 : 0);
    //Arduboy2 fillrect is absurdly slow; I have the luxury of doing this instead
    for(uint8_t i = y >> 3; i < yEnd; ++i)
        memset(arduboy->sBuffer + (i * 128) + x, 0, x2 - x);
}

//For SOME REASON, it's less generated code to call these four things than to call any 
//rectangle function, or to even make this a proper function. Oh well
#define FASTRECT(a,x,y,x2,y2,c) \
    a.drawFastVLine(x,  y,  (y2) - (y), c); \
    a.drawFastVLine(x2, y,  (y2) - (y), c); \
    a.drawFastHLine(x,  y,  (x2) - (x), c); \
    a.drawFastHLine(x,  y2, (x2) - (x), c);

//void fastRect(Arduboy2Base * arduboy, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2, uint8_t color)
//{
//    arduboy->drawFastVLine(x, y, y2 - y, color); //VIEWWIDTH + 1, 0, HEIGHT, WHITE);
//    arduboy->drawFastVLine(x2, y, y2- y, color);
//    arduboy->drawFastHLine(x, y, x2 - x, color);
//    arduboy->drawFastHLine(x, y2, x2 - x, color);
//}

// Left shift lookup table for 1 << N
constexpr uint16_t shift1Lookup16[17] = { 
    1, 2, 4, 8, 16, 32, 64, 128,
    256, 512, 1024, 2048, 4096, 8192, 16384, 32768,
    32768 //Safety
};

constexpr uint8_t shift1Lookup8[9] = {
    1, 2, 4, 8, 16, 32, 64, 128,
    128 //Safety
};

#define fastlshift8(x) shift1Lookup8[x]
#define fastlshift16(x) shift1Lookup16[x]
//#define fastlshift8(x) pgm_read_byte(shift1Lookup8 + (x))
//#define fastlshift16(x) pgm_read_word(shift1Lookup16 + (x))

// Taken from https://github.com/tiberiusbrown/arduboy_minigolf/blob/master/div.cpp
constexpr uint16_t DIVISORS[256] PROGMEM =
{
    65535, 65535, 32767, 21845, 16383, 13107, 10922,  9362,
     8191,  7281,  6553,  5957,  5461,  5041,  4681,  4369,
     4095,  3855,  3640,  3449,  3276,  3120,  2978,  2849,
     2730,  2621,  2520,  2427,  2340,  2259,  2184,  2114,
     2047,  1985,  1927,  1872,  1820,  1771,  1724,  1680,
     1638,  1598,  1560,  1524,  1489,  1456,  1424,  1394,
     1365,  1337,  1310,  1285,  1260,  1236,  1213,  1191,
     1170,  1149,  1129,  1110,  1092,  1074,  1057,  1040,
     1023,  1008,   992,   978,   963,   949,   936,   923,
      910,   897,   885,   873,   862,   851,   840,   829,
      819,   809,   799,   789,   780,   771,   762,   753,
      744,   736,   728,   720,   712,   704,   697,   689,
      682,   675,   668,   661,   655,   648,   642,   636,
      630,   624,   618,   612,   606,   601,   595,   590,
      585,   579,   574,   569,   564,   560,   555,   550,
      546,   541,   537,   532,   528,   524,   520,   516,
      511,   508,   504,   500,   496,   492,   489,   485,
      481,   478,   474,   471,   468,   464,   461,   458,
      455,   451,   448,   445,   442,   439,   436,   434,
      431,   428,   425,   422,   420,   417,   414,   412,
      409,   407,   404,   402,   399,   397,   394,   392,
      390,   387,   385,   383,   381,   378,   376,   374,
      372,   370,   368,   366,   364,   362,   360,   358,
      356,   354,   352,   350,   348,   346,   344,   343,
      341,   339,   337,   336,   334,   332,   330,   329,
      327,   326,   324,   322,   321,   319,   318,   316,
      315,   313,   312,   310,   309,   307,   306,   304,
      303,   302,   300,   299,   297,   296,   295,   293,
      292,   291,   289,   288,   287,   286,   284,   283,
      282,   281,   280,   278,   277,   276,   275,   274,
      273,   271,   270,   269,   268,   267,   266,   265,
      264,   263,   262,   261,   260,   259,   258,   257,
};

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
