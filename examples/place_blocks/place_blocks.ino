#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>

// Just for fun
#include <ArduboyTones.h>

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
ArduboyTones sound(arduboy.audio.enabled);

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

    raycast.movePlayer(movement, rotation, &isSolid);
}

// Function for letting the player place or remove blocks.
void playerblock()
{
    // Use integers just in case it's somehow outside the map (unlikely). 
    // Because of performance, the player's position is a "fixed point", which has to be
    // cast up to a float when used with player direction. Sorry!
    int8_t blockX = (float)raycast.player.posX + raycast.player.dirX;
    int8_t blocky = (float)raycast.player.posY + raycast.player.dirY;

    // Do some initial checks: if we're outside the array or if the block we're
    // working on is indestructible, don't do anything!


    if(arduboy.justPressed(A_BUTTON))
    {

    }
    else if(arduboy.justPressed(B_BUTTON))
    {

    }

    // Example of perhaps a "flashlight"
    if (arduboy.pressed(A_BUTTON))
        raycast.instance.setLightIntensity(3.0); 
    else
        raycast.instance.setLightIntensity(1.0);
}


// The normal arduino setup. Here, we can copy our map out of program memory and into 
// the map buffer in memory. 
void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 

    // Since we're letting the player set blocks, let's simply fill in the perimeter of the 
    // room with the "unbreakable" tile.
    for(uint8_t y = 0; y < raycast.worldMap.height; y++)
    {
        raycast.worldMap.setCell(0, y, 1);
        raycast.worldMap.setCell(raycast.worldMap.width - 1, y, 1);
    }
    for(uint8_t x = 0; x < raycast.worldMap.width; x++)
    {
        raycast.worldMap.setCell(x, 0, 1);
        raycast.worldMap.setCell(x, raycast.worldMap.height - 1, 1);
    }
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    arduboy.pollButtons();

    // Process player interaction, or however you'd like to do it
    movement();
    playerblock();

    // It's too expensive to draw a true raycast floor, so you're stuck with just drawing a background 
    // that's "good enough". The background is large enough to clear the screen, hence why we didn't call
    // "clearRaycast" above
    Sprites::drawOverwrite(0, 0, bg_full, 0);

    // Then just do a raycast iteration
    raycast.runIteration(&arduboy);

    arduboy.display();
}
