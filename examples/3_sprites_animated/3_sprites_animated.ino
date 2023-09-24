/*
    In this example, we're going to simply place coins in a room. Some will
    bob up and down as an animation, some will do a rotation animation, 
    and some will do both. All of them will showcase the "behavior" function
    of sprites, which allows you to attach a function to a sprite to have it
    do something every frame.

    You can't do anything with the coins yet, but in a later example, we'll
    show how to detect collision with the sprites and remove them.

    We also show how to load "smaller" maps out of program memory, if for 
    instance you don't need to use the whole thing. It requires slightly
    more code to load, but you can simply pull it out into a function and
    load any size map (up to 16x16).
*/
#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>

// Resources
#include "bg_full.h"
#include "tilesheet.h"
#include "spritesheet.h"

// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 30; 
constexpr float MOVESPEED = 2.5f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

// We're only using a portion of the 16x16 map. Instead of defining the whole thing,
// we only define the section we're using, and are just more careful with how we load
// it (see later)
constexpr uint8_t SUBMAPHEIGHT = 9;
constexpr uint8_t SUBMAPWIDTH = 8;

// Some constants for the coin sprite. If you look at the coin sprite itself,
// you'll notice it's very small within its 16x16 frame. Sprites (currently)
// must always be 16x16 to increase performance; having multiple sizes would
// require multiple code paths and perhaps multiple data types.
constexpr uint8_t COINBASEFRAME = 0;
constexpr uint8_t COINSIZEINDEX = 2;
constexpr int8_t COINBASEHEIGHT = 0;

// We're only going to have 6 sprites on the map, but let's have room for 10
// just in case. We still don't need more than 1 byte for internal storage
// (technically we need 0 but we can't have 0-sized arrays)
RcContainer<10, 1, WIDTH, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

Arduboy2 arduboy;

// Some static map in progmem. You could load this however you want. Note that maps by default are
// only 16x16 in the raycaster. It's fast to load a new map (basically instant). Here I'm
// only using a small section in the corner, so instead of defining the whole map, I'm 
// only using a portion of it. We'll need to load it special later; in previous examples we
// did 'memcpy_P' to load from program memory, but now the format of our map is not quite the 
// same as the format of the raycaster map.
constexpr uint8_t mymap[SUBMAPWIDTH * SUBMAPHEIGHT] PROGMEM = {
    1, 1, 1, 1, 1, 1, 1, 1, 
    1, 0, 0, 0, 0, 0, 0, 1, 
    1, 0, 2, 2, 0, 2, 0, 1, 
    1, 0, 2, 0, 0, 2, 0, 1, 
    1, 0, 0, 0, 2, 2, 0, 3, 
    1, 0, 2, 0, 0, 0, 0, 1, 
    1, 0, 2, 2, 0, 2, 0, 1, 
    1, 0, 0, 0, 0, 0, 0, 1, 
    1, 1, 1, 1, 1, 1, 1, 1, 
};

// It's up to you to provide the "collision" detection function, but
// I have some functions to help you out. 
bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any bounding boxes
    return raycast.worldMap.getCell(x.getInteger(), y.getInteger()) != 0 || 
        raycast.sprites.firstColliding(x, y) != NULL;
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

    raycast.player.tryMovement(movement, rotation, &isSolid);
}

// Since we're setting up "coins" a lot, let's make a function that
// adds a coin for us. The only thing that changes is the position
// and the functionality, so we'll add those as parameters
RcSprite<1> * addCoin(float x, float y, void (* behavior)(RcSprite<1> *))
{
    return raycast.sprites.addSprite(x, y, COINBASEFRAME, COINSIZEINDEX, COINBASEHEIGHT, behavior);
}

// Here's the interesting part: we're going to define behavior functions for the sprites.
// A behavior function takes in a sprite and modifies its properties so it can "do things"

// First, let's make a frame animation behavior. It'll only work for coin, since it's hardcoded 
// to know what the base frame is, but you could make a generic one if you store the base
// frame inside the sprite's internal data. Remember you can define how many bytes you need
// of internal data (we chose 1 since we're not using it)
void coinBehaviorFrameAnimate(RcSprite<1> * sprite)
{
    // We have it rotate at a speed 1/8th of the overall framerate of the game.
    // We have 4 frames of animation, which is 2 bits. We select 2 bits out of the frame
    // count 3 bits up from the bottom, which we know will change once every 8 frames.
    sprite->frame = COINBASEFRAME + ((arduboy.frameCount & 0b11000) >> 3);
}

// Next, let's make a "bob up and down" behavior. This will make the coin 
// appear to have a hovering effect. But since this function has no animation,
// the coin will not spin.
void coinBehaviorHover(RcSprite<1> * sprite)
{
    // We'll set the vertical offset to be the sin of the framecount divided
    // by some amount to slow it down. sin usually takes radians, you can adjust
    // the division to speed up or slow down the hover. We then multiply sin by
    // 4 so it hovers a bit more than just a small range
    sprite->setHeight(4 * sin(arduboy.frameCount / 6.0));
}

// Finally, we'll combine the two to make a behavior that's both spinning and hovering.
void coinBehaviorAnimateHover(RcSprite<1> * sprite)
{
    coinBehaviorFrameAnimate(sprite);
    coinBehaviorHover(sprite);
}

// The normal arduino setup. Here, we can copy our map out of program memory and into 
// the map buffer in memory. 
void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 

    // Let's not start in the corner this time
    raycast.player.posX = 6.5;
    raycast.player.posY = 5.5;
    raycast.player.dirX = -1;
    raycast.player.dirY = 0;

    // This is how you'd load a map out of program memory. Remember that the map in memory is
    // fixed to 16x16, but we're only using a smaller portion of it. In that case, we simply
    // loop over the rows and load them one at a time rather than all at once. We offset into
    // the larger in-memory array by the full RCMAXMAPDIMENSION, and offset into our smaller
    // program memory array by its own width. 
    for(uint8_t i = 0; i < SUBMAPHEIGHT; i++)
        memcpy_P(raycast.mapBuffer + i * RCMAXMAPDIMENSION, mymap + i * SUBMAPWIDTH, SUBMAPWIDTH);

    // This is a bit of a weird one: sprites can only be one of four size modifiers, but you can define
    // what those 4 sizes are. By default, those sizes are 1.5, 1.0, 0.5, and 0.25. 0.25 is too small,
    // so we modify that last value to 9/16 (yes, larger than 0.5, it doesn't matter what order they're in). 
    // This gives you great control over the sprite sizes, as you could have a unique 4 sizes for every 
    // level, or change it during gameplay for interesting effects. Note that
    // sizes have a VERY LOW precision, and can only go in fractional increments of 1/16.
    raycast.render.spritescaling[COINSIZEINDEX] = 9.0/16;

    // And now let's add some coins! We'll have a mix of frame animated coins,
    // bobbing up and down coins, and some with the special "both" function!
    addCoin(4.5, 5.5, &coinBehaviorFrameAnimate);
    addCoin(1.5, 7.5, &coinBehaviorFrameAnimate);
    addCoin(6.5, 1.5, &coinBehaviorHover);
    addCoin(4.5, 2.5, &coinBehaviorHover);
    addCoin(1.5, 1.5, &coinBehaviorAnimateHover);
    addCoin(6.5, 7.5, &coinBehaviorAnimateHover);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player interaction, or however you'd like to do it
    movement();

    // here's a funny hack: the "drawOverwite" function is very complicated, but we know that our 
    // background buffer will take up the whole screen, so we can just memcpy it into the screen buffer
    // and save both code and some cycles. This isn't a reliable optimization and doesn't save
    // much, just thought I'd show it. Not including 'drawOverwrite' saves 84 bytes in the output
    // and about 3% cpu
    memcpy_P(arduboy.sBuffer, bg_full, 1024);

    // Then just do a raycast iteration. This also runs the sprite behavior functions!
    raycast.runIteration(&arduboy);

    arduboy.display();
}
