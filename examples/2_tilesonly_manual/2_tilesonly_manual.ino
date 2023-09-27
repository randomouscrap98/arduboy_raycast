#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>

// Resources
#include "bg_full.h"
#include "tilesheet.h"

// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 35;   //35 is a safe number for fullscreen raycasting (no sprites)
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;


// This is the same example as `basic_tilesonly`, just with manual setup to ensure
// no sprite code is used. You can compare the compiled sizes if you wish, I found
// it to be (as of the time of writing) over 2KiB saved.

// When using the simple container `RcContainer`, it brings in all the sprite code,
// as it still calls the sprite rendering and management functions. If you absolutely need
// the maximum amount of program space and somehow don't need 3D sprites, you can 
// set up the raycaster manually.

// First, you have to create your buffers. I can't malloc them for you in the classes,
// so it's better to just make global buffers. Doing it this way, you can also make
// larger or smaller maps, though note that maps larger than 16x16 will require code
// changes in the library since assumptions are made to reduce precision + increase
// performance
uint8_t mapBuffer[RCMAXMAPDIMENSION * RCMAXMAPDIMENSION];
// if you were using sprites, you'd create your sprite buffers here too.

// Then create the containers necessary to hold the buffers
RcMap worldMap {
    mapBuffer,
    RCMAXMAPDIMENSION,
    RCMAXMAPDIMENSION 
};

// And now the final bits: player and renderer.
RcPlayer player;
RcRender<WIDTH,HEIGHT> renderer;

Arduboy2 arduboy;


// Some static map in progmem. You could load this however you want. Note that maps by default are
// only 16x16 in the raycaster. It's fast to load a new map (basically instant)
constexpr uint8_t mymap[RCMAXMAPDIMENSION * RCMAXMAPDIMENSION] PROGMEM = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 0, 0, 2, 0, 0, 3, 0, 3, 0, 0, 4, 0, 0, 1,
    1, 0, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 4, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 0, 1,
    1, 0, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 5, 0, 5, 0, 1,
    1, 0, 7, 7, 7, 0, 6, 6, 6, 0, 5, 0, 0, 5, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 6, 0, 0, 5, 0, 5, 5, 0, 1,
    1, 0, 7, 7, 7, 0, 6, 6, 6, 0, 5, 0, 0, 5, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 5, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// A function which determines if a cell is solid. This is up to you: you may have
// your own scheme for whether a block is solid or not. Luckily in our case, any
// nonzero block is solid
bool isSolid(uflot x, uflot y)
{
    // If youve seen the `basic_tilesonly` example, you might notice that the RcContainer
    // simply contains all those other special classes and barely has any functionality on its
    // own. As a result, this collision detection doesn't have to change
    return worldMap.getCell(x.getInteger(), y.getInteger()) != 0;
}

// You have to write this yourself but I provide a helper function which automatically does 
// bounds checking on the map and other defined bounding boxes, as long as you provide a 
// function which says whether something is a collision (see above)
void movement()
{
    float movement = 0;
    float rotation = 0;

    // Simple movement forward and backward
    if (arduboy.pressed(UP_BUTTON))
        movement = MOVESPEED;
    if (arduboy.pressed(DOWN_BUTTON))
        movement = -MOVESPEED;

    // Simple rotation
    if (arduboy.pressed(RIGHT_BUTTON))
        rotation = -ROTSPEED;
    if (arduboy.pressed(LEFT_BUTTON))
        rotation = ROTSPEED;

    // Same here: instead of getting player out of the container like in the previous example, 
    // we can simply go straight to the player to try movement
    player.tryMovement(movement, rotation, &isSolid);
}

// Just an example of setting the light level, this isn't necessary
void flashlight()
{
    // Example of perhaps a "flashlight"
    if (arduboy.pressed(A_BUTTON))
        renderer.setLightIntensity(3.0); 
    else
        renderer.setLightIntensity(1.0);
}


// The normal arduino setup. Here, we can copy our map out of program memory and into 
// the map buffer in memory. 
void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 

    // Note: since you're not using the container, you have to set the player position yourself
    // the first time. To be fair, the container only assumes a reasonable position
    player.posX = 1.5;
    player.posY = 1.5;
    player.dirX = 0;
    player.dirY = 1;

    // Aaanndd also since it's manual, you need to assign the tilesheet the renderer will use
    renderer.tilesheet = tilesheet;

    // This is how you'd load a map out of program memory. Note that `worldMap` is a convenience
    // wrapper around the buffer, but the data is only actually stored within the buffer. So you 
    // could access it either directly through mapBuffer or through the worldMap.mapBuffer variable.
    memcpy_P(mapBuffer, mymap, RCMAXMAPDIMENSION * RCMAXMAPDIMENSION);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player interaction, or however you'd like to do it
    movement();
    flashlight();

    // It's too expensive to draw a true raycast floor, so you're stuck with just drawing a background 
    // that's "good enough". The background is large enough to clear the screen, hence why we didn't call
    // "clearRaycast" above
    Sprites::drawOverwrite(0, 0, bg_full, 0);

    // Instead of calling 'runIteration' on the container, we simply directly call the 
    // renderer to render the raycast walls. We don't have to do the sprite stuff
    renderer.raycastWalls(&player, &worldMap, &arduboy);

    arduboy.display();
}
