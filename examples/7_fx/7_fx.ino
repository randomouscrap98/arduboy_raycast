/*
    In this example, we're going to make a demo where you can
    go between one of two zones. It will showcase:
    - Loading multiple maps from program memory
    - Loading sprites from memory (or an exmaple anyway)
    - Creating non-solid tiles to act as doorways
    - Using bounding boxes to transition to new areas
    - Setting more aspects of the rendering technique 
      to differentiate inside vs outside
*/

// #define RCSMALLLOOPS

#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyFX.h>
#include <ArduboyRaycastFX.h>

// Resources
#include "bg_full.h"
#include "fxdata/fxdata.h"


// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 30; // Too many sprites + fullscreen for 35
constexpr float MOVESPEED = 2.25f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

// Since we're using this number so many times in template types, might 
// as well make it a constant.
constexpr uint8_t NUMINTERNALBYTES = 1;
constexpr uint8_t NUMSPRITES = 16;

// Once again we pick 16 sprites just in case we need them. 16 is a decent number
// to not take up all the memory but still have enough to work with.
RcContainer<NUMSPRITES, NUMINTERNALBYTES, WIDTH, HEIGHT> raycast(tilesheet, spritesheet, spritesheetMask);

Arduboy2 arduboy;

// The map determines the backgrund to use, need to store it. We also store some
// other pointers related to the current map for ease of use later
// const uint8_t * currentBackground;

// Here, we create a function to provide during movement to say whether
// something is solid. This is pretty similar to others, but we've designated
// a certain tile as non-solid so we can walk through it. You can do this
// however you want.
bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any (solid) bounding boxes
    uint8_t tile = raycast.worldMap.getCell(x.getInteger(), y.getInteger());

    //return (tile != 0 && tile != MyTiles::OutdoorRockOpening) || 
    return (tile != 0) || 
        raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL;
}

// You have to write this yourself but I provide a helper function which automatically does 
// bounds checking on the map and other defined bounding boxes, as long as you provide a 
// function which says whether something is a collision (see above)
void movement()
{
    float movement = 0;
    float rotation = 0;

    // Simple movement forward and backward. Might as well also let the user use B
    // to move forward for "tank" controls
    if (arduboy.pressed(UP_BUTTON) || arduboy.pressed(B_BUTTON))
        movement = MOVESPEED;
    if (arduboy.pressed(DOWN_BUTTON))
        movement = -MOVESPEED;

    // Simple rotation
    if (arduboy.pressed(RIGHT_BUTTON))
        rotation = -ROTSPEED;
    if (arduboy.pressed(LEFT_BUTTON))
        rotation = ROTSPEED;

    raycast.player.tryMovement(movement, rotation, &isSolid);
}

void setup()
{
    arduboy.boot();
    arduboy.flashlight();
    arduboy.setFrameRate(FRAMERATE); 
    FX_INIT();

    raycast.render.spritescaling[2] = 0.75;

    for(int i = 0; i < RCMAXMAPDIMENSION; i++) {
        raycast.worldMap.setCell(i, 0, 2);
        raycast.worldMap.setCell(i, RCMAXMAPDIMENSION - 1, 2);
        raycast.worldMap.setCell(0, i, 2);
        raycast.worldMap.setCell(RCMAXMAPDIMENSION - 1, i, 2);
    }

    for(int i = 2; i < RCMAXMAPDIMENSION - 2; i += 2)
        for(int j = 2; j < RCMAXMAPDIMENSION - 2; j += 3)
            raycast.worldMap.setCell(i, j, 1);

    for(int i = 0; i < NUMSPRITES; i++)
    {
        uint8_t tile = rand() % 2;
        RcSprite<NUMINTERNALBYTES> * sp = raycast.sprites.addSprite(1.5 + (rand() % 9), 2.5 + (rand() % 9), 1 + tile, 2 - tile, 9 - 2 * tile, NULL);
        raycast.sprites.addSpriteBounds(sp, 0.5 + 0.25 * tile, true);
    }
    // for(int i = 3; i < RCMAXMAPDIMENSION - 2; i++)
    // {
    // }

    //raycast.render.setLightIntensity(3.0);
    //Assign custom sizes for scaling
    // raycast.render.spritescaling[0] = 1.0;
    // raycast.render.spritescaling[1] = 0.8;
    // raycast.render.spritescaling[2] = 0.6;
    // raycast.render.spritescaling[3] = 0.4;
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player movement + interaction
    movement();

    // Draw the correct background for the area. 
    raycast.render.drawRaycastBackground(&arduboy, bg_full);
    //raycast.render.clearRaycast(&arduboy);
    raycast.runIteration(&arduboy);

    FX::display(false);
    //arduboy.display();
}
