#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>


// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 30;   //30 is a safe number for raycasting
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;

// You should have some constants for the map. Note that currently, the library
// only supports maps up to 16x16.
constexpr uint8_t MAPWIDTH = RCMAXMAPDIMENSION;
constexpr uint8_t MAPHEIGHT = RCMAXMAPDIMENSION;

// You must create a buffer for the map, then a container to hold it.
uint8_t mapBuffer[MAPWIDTH * MAPHEIGHT];
RcMap mapData {
    mapBuffer,
    MAPWIDTH,
    MAPHEIGHT
};

// You must also track the player position and direction, which is all together
// in one struct
RcPlayer player;

Arduboy2 arduboy;


void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 
}

void loop()
{
    if(!arduboy.nextFrame()) return;
}
