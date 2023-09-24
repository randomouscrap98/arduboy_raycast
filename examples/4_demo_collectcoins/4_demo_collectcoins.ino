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
constexpr uint8_t FRAMERATE = 40; 
constexpr float MOVESPEED = 3.0f / FRAMERATE;
constexpr float ROTSPEED = 3.25f / FRAMERATE;

// Store the number of internal bytes we're using for user data within sprites to make 
// it easier to write functions and pass around data types. We need to store a pointer
// to a bounds, so the number of bytes should essentially become 2
constexpr uint8_t NUMINTERNALBYTES = sizeof(RcBounds *);
constexpr uint8_t COINBITFLAG = 0x80;

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

// The winning melody
const uint16_t winsong[] PROGMEM = {
    NOTE_C5,128, 0,64, NOTE_C5,64, 0,32, NOTE_C5,64, 0,16, NOTE_G5,256,
    TONES_END
};

// We're going to generate 16 sprites and use EVERY slot. Also notice we're 
// not using the full width, this will greatly increase performance (and we 
// can use the unused width for something else). We're actually using some
// of the internal state here, to store a pointer to the bounds for this coin
RcContainer<MAXNUMCOINS, NUMINTERNALBYTES, 100, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

// Some globals for gameplay
uint8_t foundCoins = 0;
GameState state = GameState::Normal;

// These end up being expensive to calculate (sin and float math yeesh!), so 
// doing it for EVERY coin is wasteful. We can cache the same values and use
// them for all coin animations
uint8_t currentCoinFrame = COINBASEFRAME;
int8_t currentCoinHeight = 0;

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);


// This is the "behavior" function for our coins. It uses the functionality
// from the sprite animation example to create a frame animated, bobbing coin
void coinAnimation(RcSprite<NUMINTERNALBYTES> * sprite)
{
    sprite->frame = currentCoinFrame; //COINBASEFRAME + ((arduboy.frameCount & 0b11000) >> 3);
    sprite->setHeight(currentCoinHeight); //4 * sin(arduboy.frameCount / 6.0));
}

// Since we're setting up "coins" a lot, let's make a function that
// adds a coin for us. The only thing that changes is the position this time. 
// We're also taking a "cell" value instead of a float value, because we're
// ALWAYS going to place it in the center of the cell
RcSprite<NUMINTERNALBYTES> * addCoin(uint8_t x, uint8_t y)
{
    // Create a coin sprite and put a generous bounds around it for us to walk into.
    RcSprite<NUMINTERNALBYTES> * sprite = raycast.sprites.addSprite(x + 0.5, y + 0.5, COINBASEFRAME, COINSIZEINDEX, COINBASEHEIGHT, &coinAnimation);
    // Coins are NOT solid, notice the false at the end. You can create bounds that are simply used 
    // for hit detection. 
    RcBounds * bounds = raycast.sprites.addSpriteBounds(sprite, 0.85, false);
    // Here, we use one of the bits of state to mark this as a coin for hit detection. 
    // Later, when we scan for collisions, we can pass in that same bitmask to only find
    // detections against coins. Since coins are the only thing we have, this isn't necessary,
    // but it's important to know! I would say only the top 4 bits are safe in the state byte.
    bounds->state |= COINBITFLAG;

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
    // any (solid) bounding boxes
    return raycast.worldMap.getCell(x.getInteger(), y.getInteger()) != 0 || 
        raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL;
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

// See if we're collecting any coins. 
void testCoinCollection()
{
    // Go scan for any coin collisions. This should be exceptionally fast, especially compared with
    // raycast rendering, and shouldn't make a difference to performance (if it does, let me know)
    RcBounds * bounds = raycast.sprites.firstColliding(raycast.player.posX, raycast.player.posY, COINBITFLAG);

    // Nothing found, nothing to do
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
            foundCoins++;
            if(foundCoins == MAXNUMCOINS)
            {
                sound.tones(winsong);
                state = GameState::WinScreen;
            }
            else
            {
                sound.tone(500 + foundCoins * 10, 32, 700 + foundCoins * 10, 32);
            }
            sprite->setActive(false); //This immediately removes the sprite from the pool
            bounds->setActive(false);
            drawInventory();
            break;
        }
    }
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

    // Get rid of (ll sprites (this is important to call!!)
    raycast.sprites.resetAll();

    foundCoins = 0;
    state = GameState::Normal;

    ellerMaze(&raycast.worldMap, RCMAXMAPDIMENSION, RCMAXMAPDIMENSION, &raycast.player);

    // Now try VERY HARD to place all 16 coins (ie retry with random generation until
    // all slots are filled. we don't want coins on top of each other and we don't want
    // coins in walls, or at the player start position)
    uint8_t coins_generated = 0;

    while(coins_generated < MAXNUMCOINS)
    {
        // Find a random location for the coin
        uint8_t rndx = 1 + random(14);
        uint8_t rndy = 1 + random(14);

        // The cell is filled, don't put coins in walls!
        if(raycast.worldMap.getCell(rndx, rndy))
            continue;
        
        // Oops, this is the player position!
        if(rndx == raycast.player.posX.getInteger() && rndy == raycast.player.posY.getInteger())
            continue;
        
        bool overlapping = false;

        // Scan the sprite array to see if we overlap other coins slots
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


void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 
    arduboy.initRandomSeed();

    // Make our coins an "exact" size scaling (not really necessary, I just like it)
    raycast.render.spritescaling[COINSIZEINDEX] = 9.0/16;

    // Generate the first maze
    generateNew();
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    arduboy.pollButtons();

    // Calculate these ONCE per frame, it's expensive! You should do this for any repeat animations!
    currentCoinFrame = COINBASEFRAME + ((arduboy.frameCount & 0b11000) >> 3);
    currentCoinHeight = 4 * sin(arduboy.frameCount / 6.0);

    // Render one of two screens based on the overall game state
    if(state == GameState::Normal)
    {
        // Process player movement + interaction
        movement();
        testCoinCollection();

        // Notice something: we're drawing a background but this time it doesn't take up the
        // entire screen, since our raycast function is only 100 wide. So there's a section
        // on the right that remains static. This increases performance, as we can simply
        // redraw that area on the right only when something "interesting" happens 
        // (like when you collect a coin). The arduboy2 library gives you this flexibility,
        // might as well use it! EDIT: I'm not using the arduboy library for this, since 
        // the background is byte aligned and we can optimize the draw. Plus it gets rid
        // of quite a few bytes, not using drawOverwrite at all.
        raycast.render.drawRaycastBackground(&arduboy, raycastBg);
        //Sprites::drawOverwrite(0, 0, raycastBg, 0);

        // Then just do a raycast iteration. This also runs the sprite behavior functions!
        raycast.runIteration(&arduboy);
    }
    else
    {
        arduboy.clear();
        arduboy.setCursor(40, 28);
        arduboy.print("Winner!");

        if(arduboy.justPressed(A_BUTTON))
            generateNew();
    }

    arduboy.display();
}
