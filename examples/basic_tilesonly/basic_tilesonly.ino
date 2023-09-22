#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>

// Resources
#include "bg_full.h"
#include "tilesheet.h"

// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 30;   //30 is a safe number for raycasting
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;

// We're going to use the simple container, which wraps many requirements into a single container.
// Note with this container, it's a bit harder to have finer control, but you generally don't need it.
// The first number in the template is the number of sprites, this has a great impact on the amount
// of memory used. Second is the amount of bytes within each sprite you can use for state. Unfortunately,
// you have to set each to at least 1. Then comes the raycast rendering width and height. Raycasting
// is computationally intensive, so you should take up some of the screen with an inventory or something.
// You then pass in the tilesheet, spritesheet, and spritesheet mask pointers. If you're not using 
// sprites (like us), you can just pass null for those last two
RcContainer<1, 1, WIDTH, HEIGHT> raycast(tilesheet, NULL, NULL);

Arduboy2 arduboy;

// Some static map in progmem. You could load this however you want. Note that maps by default are
// only 16x16 in the raycaster. It's fast to load a new map (basically instant)
constexpr uint8_t mymap[] PROGMEM = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// A function which determines if a cell is solid. This is up to you: you may have
// your own scheme for whether a block is solid or not. Luckily in our case, any
// nonzero block is solid
bool isSolid(uint8_t x, uint8_t y)
{
    return raycast.worldMap.getCell(x, y) != 0;
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

    raycast.tryMovement(movement, rotation, &isSolid);
}

void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 

    // This is how you'd load a map out of program memory
    memcpy_P(raycast.mapBuffer, mymap, raycast.worldMap.width * raycast.worldMap.height);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process movement, or however you'd like to do it
    movement();

    // If you're trying to be optimal, you could only clear the raycasting portion using this
    // raycast.instance.clearRaycast(&arduboy);

    // It's too expensive to draw a true raycast floor, so you're stuck with just drawing a background 
    // that's "good enough". The background is large enough to clear the screen, hence why we didn't call
    // "clearRaycast" above
    Sprites::drawOverwrite(0, 0, bg_full, 0);

    // Then just do a raycast iteration
    raycast.runIteration(&arduboy);
}
