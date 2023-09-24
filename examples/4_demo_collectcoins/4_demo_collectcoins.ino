/*
    In this example, we're going to make a "full" demo game where
    you go around a map and collect coins. This will showcase:
    - Procedural map generation (using a maze generation algorithm)
    - Animated / moving sprites
    - Removing sprites from the pool
    - Collision detection with sprites
    - Nonstandard rendering widths (and how it increases performance)
    - Partial rendering instead of using arduboy.clear
*/
#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>
#include <ArduboyTones.h>

// Include our maze generator. You can go look at the code but it's
// not too important for our example, just know it generates a maze
// and places the player in a safe spot, facing the right direction.
#include "mazegen.h"

// Resources
#include "bg.h"
#include "tilesheet.h"
#include "spritesheet.h"
#include "coin_b.h"

// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 35; // because we're not rendering full width, we can increase the fps
constexpr float MOVESPEED = 3.0f / FRAMERATE;
constexpr float ROTSPEED = 3.25f / FRAMERATE;

// Store the number of internal bytes we're using for user data within sprites to make 
// it easier to write functions and pass around data types. We need to store a pointer
// to a bounds, so the number of bytes should essentially become 2
constexpr uint8_t NUMINTERNALBYTES = sizeof(RcBounds *);

// Some constants for the coin sprite. If you look at the coin sprite itself,
// you'll notice it's very small within its 16x16 frame. Sprites (currently)
// must always be 16x16 to increase performance; having multiple sizes would
// require multiple code paths and perhaps multiple data types.
constexpr uint8_t COINBASEFRAME = 0;
constexpr uint8_t COINSIZEINDEX = 2;
constexpr int8_t COINBASEHEIGHT = 0;

// The number of coins we generate in the maze (required to win)
constexpr uint8_t MAXNUMCOINS = 16;

// Enumeration to represent what "state" our game is in. There are many ways
// to do this, you don't have to do it like this.
enum GameState {
    Normal,
    WinScreen
};

// We're going to generate 16 sprites and use EVERY slot. Also notice we're 
// not using the full width, this will greatly increase performance (and we 
// can use the unused width for something else). We're actually using some
// of the internal state here, to store a pointer to the bounds for this coin
RcContainer<MAXNUMCOINS, NUMINTERNALBYTES, 100, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

// Some globals for gameplay
uint8_t foundCoins = 0;
GameState state = GameState::Normal;

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);


// This is the "behavior" function for our coins. It uses the functionality
// from the sprite animation example to create a frame animated, bobbing coin
void coinAnimation(RcSprite<NUMINTERNALBYTES> * sprite)
{
    sprite->frame = COINBASEFRAME + ((arduboy.frameCount & 0b11000) >> 3);
    sprite->setHeight(4 * sin(arduboy.frameCount / 6.0));
}

// Since we're setting up "coins" a lot, let's make a function that
// adds a coin for us. The only thing that changes is the position this time. 
// We're also taking a "cell" value instead of a float value, because we're
// ALWAYS going to place it in the center of the cell
RcSprite<NUMINTERNALBYTES> * addCoin(uint8_t x, uint8_t y)
{
    // Create a coin sprite and put a generous bounds around it for us to walk into
    RcSprite<NUMINTERNALBYTES> * sprite = raycast.sprites.addSprite(x + 0.5, y + 0.5, COINBASEFRAME, COINSIZEINDEX, COINBASEHEIGHT, &coinAnimation);
    RcBounds * bounds = raycast.sprites.addSpriteBounds(sprite, 0.75);

    // This is important: you can use sprite internal state to link the bounds to the sprite.
    // There are better ways to do this, this is just an example. We're copying the pointer for the
    // bounds object into the internal state for the sprite. If we had more memory, I would provide 
    // this functionality for you, but I just can't. I might add helper functions in the future which
    // do this if for you only if asked if it's a common enough task (it might be)
    memcpy(sprite->intstate, &bounds, sizeof(RcBounds *));

    return sprite;
}


// It's up to you to provide the "collision" detection function, but
// I have some functions to help you out. 
bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any bounding boxes
    return raycast.worldMap.getCell(x.getInteger(), y.getInteger()) != 0 || 
        raycast.sprites.firstColliding(x, y) != NULL;
}

// A function which will do the logic for "hitting" a sprite bounding box.
void hitBounds(RcBounds * bounds)
{
    if(bounds == NULL)
        return;
    
    // Unfortunately, as of right now, bounding boxes themselves do not have internal storage,
    // so we instead have to scan the sprites to find the one it's linked to. This may 
    // change in the future. Also, if you don't need this flexibility, you could simply
    // make sure the bounding boxes for sprites are stored at the SAME position within their
    // array as the sprites. This is a more manual task but may be better in... well, most cases.
    // Just wanted to show off how to use internal sprite data.
    for(uint8_t i = 0; i < raycast.sprites.numsprites; i++)
    {
        RcSprite<NUMINTERNALBYTES> * sprite = &raycast.sprites.sprites[i];

        // Skip inactive sprites
        if(!sprite->isActive()) continue;

        // Pull the "pointer" out of the internal sprite data. There are shorter ways to do this
        RcBounds * bptr;
        memcpy(&bptr, sprite->intstate, sizeof(RcBounds *));

        // This is our sprite, time to remove it! Also, since we know we hit a coin, let's 
        // play a sound and increase the found counter
        if(bptr == bounds)
        {
            sound.tone(600, 64);
            sprite->setActive(false); //This immediately removes the sprite from the pool
            bounds->setActive(false);
            foundCoins++;
            drawInventory();
            break;
        }
    }
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

    // Here we're doing something different than before: we want to see if we run
    // into any coins. If we do, we should remove the coin and update the coin count
    // BEFORE we try the movement, as coin bounding boxes will halt the player.
    // You can do this any way you'd like, you don't have to use the bounding box system.
    uflot newX = raycast.player.calcNewX(movement);
    uflot newY = raycast.player.calcNewY(movement);
    hitBounds(raycast.sprites.firstColliding(newX, newY));

    raycast.player.tryMovement(movement, rotation, &isSolid);
}


// Draw the "coin" inventory on the side. You could of course use the unused screen area for anything
void drawInventory()
{
    // Clear out the side 
    arduboy.fillRect(raycast.render.VIEWWIDTH, 0, WIDTH - raycast.render.VIEWWIDTH, HEIGHT, BLACK);

    constexpr uint8_t COINDRAWX = 104;

    // We're going to be extra simple and simply draw unfilled coins for any not-found coin,
    // and filled in coins for the ones that ARE found.
    for(uint8_t i = 0; i < MAXNUMCOINS; i++)
        Sprites::drawOverwrite(COINDRAWX + (i % 2) * 12, (i / 2) * 8, coin_b, i < foundCoins ? 1 : 0);
}

// This function will create a maze and populate it with 16 coins. It will also reset some
// globals and make sure everything is set nice and good for a "new game"
void generateNew()
{
    arduboy.clear();

    foundCoins = 0;
    state = GameState::Normal;

    ellerMaze(&raycast.worldMap, RCMAXMAPDIMENSION, RCMAXMAPDIMENSION, &raycast.player);

    // Now try VERY HARD to place all 16 coins (ie retry with random generation until
    // all slots are filled. we don't want coins on top of each other and we don't want
    // coins in walls, or at the player start position)
    uint8_t coins_generated = 0;

    while(coins_generated < MAXNUMCOINS)
    {
        uint8_t rndx = 1 + random(14);
        uint8_t rndy = 1 + random(14);

        // The cell is filled, don't put coins in walls!
        if(raycast.worldMap.getCell(rndx, rndy))
            continue;
        
        // Oops, this is the player position!
        if(rndx == raycast.player.posX.getInteger() && rndy == raycast.player.posY.getInteger())
            continue;
        
        bool overlapping = false;

        // Scan the sprite array to see if we overlap other coins
        for(uint8_t i = 0; i < raycast.sprites.numsprites; i++)
        {
            RcSprite<NUMINTERNALBYTES> * sprite = &raycast.sprites.sprites[i];
            if(sprite->isActive() && sprite->x.getInteger() == rndx && sprite->y.getInteger() == rndy)
            {
                overlapping = true;
                continue;
            }
        }

        // OK, we finally found a good place!
        if(!overlapping)
        {
            addCoin(rndx, rndy);
            coins_generated++;
        }
    }

    // And then draw the initial inventory since it's only drawn on update
    drawInventory();
}


// The normal arduino setup. Here, we can copy our map out of program memory and into 
// the map buffer in memory. 
void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 
    arduboy.initRandomSeed();

    // Make our coins an "exact" size scaling (not really necessary, I just like it)
    raycast.render.spritescaling[COINSIZEINDEX] = 9.0/16;

    generateNew();
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    arduboy.pollButtons();

    // Render one of two screens based on the overall game state
    if(state == GameState::Normal)
    {
        // Process player interaction, or however you'd like to do it
        movement();

        // After movement, we know if we've won. Or you could do this whenever, idk
        if(foundCoins == MAXNUMCOINS)
        {
            state = GameState::WinScreen;
            return;
        }

        // Notice something: we're drawing a background but this time it doesn't take up the
        // entire screen, since our raycast function is only 100 wide. So there's a section
        // on the right that remains static. This increases performance, as we can simply
        // redraw that area on the right only when something "interesting" happens.
        Sprites::drawOverwrite(0, 0, raycastBg, 0);

        // Then just do a raycast iteration. This also runs the sprite behavior functions!
        raycast.runIteration(&arduboy);
    }
    else
    {
        arduboy.clear();
        arduboy.setCursor(36,28);
        arduboy.print("Winner!");

        if(arduboy.justPressed(A_BUTTON))
            generateNew();
    }

    arduboy.display();
}
