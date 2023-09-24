#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>

// Resources
#include "bg_full.h"
#include "tilesheet.h"
#include "spritesheet.h"

// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 25;   //Sprites are a bit heavier; at this full width resolution, you'll get frame drops.
constexpr float MOVESPEED = 2.5f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

// Let's have 16 sprites and again only 1 byte of internal storage. This 
// time, we actually do have a spritesheet, so let's load those
RcContainer<16, 1, WIDTH, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

Arduboy2 arduboy;

// Some static map in progmem. You could load this however you want. Note that maps by default are
// only 16x16 in the raycaster. It's fast to load a new map (basically instant). Here I'm
// only using a small section in the corner
constexpr uint8_t mymap[RCMAXMAPDIMENSION * RCMAXMAPDIMENSION] PROGMEM = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 2, 0, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
};

// It's up to you to provide the "collision" detection function, but
// I have some functions to help you out. 
bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any bounding boxes
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

// The normal arduino setup. Here, we can copy our map out of program memory and into 
// the map buffer in memory. 
void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 

    // Example of how to increase the view distance (lowers performance). Default is 1.0.
    // I don't think brightness is linear? I don't remember...
    raycast.render.setLightIntensity(4.0);

    // And let's place our player at the door, facing in. Note that if you want
    // to apply an FOV (not supported yet?) you'll want to call a function instead.
    // The FOV is intrinsically tied to the player direction because of how 
    // raycasting works
    raycast.player.posX = 6.5;
    raycast.player.posY = 4.5;
    raycast.player.dirX = -1;
    raycast.player.dirY = 0;

    // This is how you'd load a map out of program memory
    memcpy_P(raycast.mapBuffer, mymap, RCMAXMAPDIMENSION * RCMAXMAPDIMENSION);

    // Now, we're going to setup the sprites manually here but feel free to load your 
    // sprite location from some buffer or something. The "addSprite" function returns
    // a pointer to your sprite within the sprite buffer, or NULL if one couldn't be
    // added (ie the sprite buffer is full). For most sprites, we're also going to add
    // a bounding box so that our "isSolid" function will know where the player can't 
    // walk (see: raycast.sprites.firstColliding)

    // A temp pointer for later
    RcSprite<1> * s;

    // First is a chandelier right in the middle of the room. Note that we're using the
    // y offset to put it higher up, and we don't need a bounding box because the player
    // can't run into it. The scaling is '1', but note that this isn't the direct scaling,
    // it's unfortunately only an index into the scaling lookup table. 1 in this case is
    // "1.0", 2 is "0.5" and 3 is "0.25". 0 is "1.5". The "-15" shifts the chandelier up 
    // to the ceiling. Note that -15 to 15 is the total range of vertical movement.
    // That final "NULL" is a behavior function that gets processed against the sprite 
    // every frame. This is mostly for moving or animated sprites.
    raycast.sprites.addSprite(4, 4.5, 1, 1, -15, NULL);

    // Define some constants so we don't go crazy
    constexpr uint8_t TABLEOFFSET = 8;
    constexpr uint8_t TABLESIZE = 2;
    constexpr uint8_t TABLESPRITE = 4;
    constexpr float TABLEBOUNDS = 0.5;

    // Next, let's put a table in each wall of the room where there isn't a door. 
    // These will have a bounding box: see how we get the sprite and use it to
    // create a small area around the table where the player can't move. We 
    // also shrink the table so it's not absolutely massive, and move it down
    // a bit so it looks like it's on the floor.
    s = raycast.sprites.addSprite(4, 1.5, TABLESPRITE, TABLESIZE, TABLEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TABLEBOUNDS, true);
    s = raycast.sprites.addSprite(4, 7.5, TABLESPRITE, TABLESIZE, TABLEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TABLEBOUNDS, true);
    s = raycast.sprites.addSprite(1.5, 4.5, TABLESPRITE, TABLESIZE, TABLEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TABLEBOUNDS, true);

    constexpr uint8_t VASEOFFSET = 2;
    constexpr uint8_t VASESIZE = 1;
    constexpr uint8_t VASESPRITE = 3;
    constexpr float VASEBOUNDS = 1.0;

    // Big vases in the corners of the rooms. Note that when you get close to them, the game
    // lags. It's difficult to draw both the walls AND the sprites, and when the sprite is big,
    // it has to basically do a full screen draw twice. You can try to avoid this by reducing
    // the view width of the raycaster, which is a direct savings, and making the bounding boxes
    // for big sprites larger so players can't get close.
    s = raycast.sprites.addSprite(1.4, 1.4, VASESPRITE, VASESIZE, VASEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, VASEBOUNDS, true);
    s = raycast.sprites.addSprite(1.4, 7.6, VASESPRITE, VASESIZE, VASEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, VASEBOUNDS, true);
    s = raycast.sprites.addSprite(6.6, 1.4, VASESPRITE, VASESIZE, VASEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, VASEBOUNDS, true);
    s = raycast.sprites.addSprite(6.6, 7.6, VASESPRITE, VASESIZE, VASEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, VASEBOUNDS, true);

    constexpr uint8_t TREEOFFSET = 8;
    constexpr uint8_t TREESIZE = 2;
    constexpr uint8_t TREESPRITE = 0;
    constexpr float TREEBOUNDS = 0.5;

    // Let's place trees on some sides of the pillar
    s = raycast.sprites.addSprite(2.5, 3.3, TREESPRITE, TREESIZE, TREEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TREEBOUNDS, true);
    s = raycast.sprites.addSprite(2.5, 5.7, TREESPRITE, TREESIZE, TREEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TREEBOUNDS, true);
    s = raycast.sprites.addSprite(5.5, 3.3, TREESPRITE, TREESIZE, TREEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TREEBOUNDS, true);
    s = raycast.sprites.addSprite(5.5, 5.7, TREESPRITE, TREESIZE, TREEOFFSET, NULL);
    raycast.sprites.addSpriteBounds(s, TREEBOUNDS, true);

    // And then hide the "thing" (whatever it is) in a corner. It won't have 
    // collision. Also make it a bit smaller
    raycast.sprites.addSprite(2, 7.25, 2, 2, 8, NULL);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player interaction, or however you'd like to do it
    movement();

    // It's too expensive to draw a true raycast floor, so you're stuck with just drawing a background 
    // that's "good enough". The background is large enough to clear the screen, hence why we didn't call
    // "clearRaycast" above
    Sprites::drawOverwrite(0, 0, bg_full, 0);

    // Then just do a raycast iteration
    raycast.runIteration(&arduboy);

    arduboy.display();
}
