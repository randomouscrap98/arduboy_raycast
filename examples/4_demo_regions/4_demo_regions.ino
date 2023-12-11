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
#include <Arduboy2.h>
#include <FixedPoints.h>
#include <ArduboyRaycast.h>

// Resources
#include "bg_full.h"
#include "bg_full_inverted.h"
#include "tilesheet.h"
#include "spritesheet.h"

#include "maps.h"


// Gameplay constants. You don't have to define these, but it's nice to have
constexpr uint8_t FRAMERATE = 30; // Too many sprites + fullscreen for 35
constexpr float MOVESPEED = 2.25f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

// Since we're using this number so many times in template types, might 
// as well make it a constant.
constexpr uint8_t NUMINTERNALBYTES = 1;


// Once again we pick 16 sprites just in case we need them. 16 is a decent number
// to not take up all the memory but still have enough to work with.
RcContainer<16, NUMINTERNALBYTES, WIDTH, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

Arduboy2 arduboy;

// The map determines the backgrund to use, need to store it. We also store some
// other pointers related to the current map for ease of use later
const uint8_t * currentBackground;
const LoadingZone * currentZoneArray;

// Here, we create a function to provide during movement to say whether
// something is solid. This is pretty similar to others, but we've designated
// a certain tile as non-solid so we can walk through it. You can do this
// however you want.
bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any (solid) bounding boxes
    uint8_t tile = raycast.worldMap.getCell(x.getInteger(), y.getInteger());

    return (tile != 0 && tile != MyTiles::OutdoorRockOpening) || 
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

// Test for area transition. If it's detected, we move to that new area!
void testAreaTransition()
{
    // Go scan for area bounding boxes. We reserve the top two bits of the bounds state flag to indicate
    // the transition area ID. This means each room could have 3 map transition areas (0 couldn't be represented). 
    // By passing in the bitmask, the colliding function will only return bounds which have one of those bits set, and we 
    // won't get things like the collision boxes for decorative sprites
    RcBounds * bounds = raycast.sprites.firstColliding(raycast.player.posX, raycast.player.posY, 0b11000000);

    // Nothing found, nothing to do
    if(bounds == NULL)
        return;
    
    // Remember we added 1 to the zone to make it nonzero, so we must undo that now
    uint8_t zone = (bounds->state >> 6) - 1;

    // Need to go lookup the current zone. Luckily we stored a pointer to the current list of zones
    // to make life easier!
    LoadingZone nextZone;
    memcpy_P(&nextZone, currentZoneArray + zone, sizeof(LoadingZone));

    // Now we just load the new area!
    loadArea(nextZone.LoadMap);
}

// Load the given area
void loadArea(uint8_t area)
{
    // Clear out old stuff
    raycast.sprites.resetAll();

    // Go into the maps and pull out the struct representing the map
    MapInfo map; 
    memcpy_P(&map, AllMaps + area, sizeof(MapInfo));

    // Now load the map and set the background and such. See how we can
    // set the parameters of rendering whenever we want? You could set it
    // in the middle of gameplay even, though it would be jarring
    currentBackground = map.Background;
    currentZoneArray = map.LoadingZones;
    raycast.render.shading = map.ShadeType;
    raycast.render.spriteShading = map.ShadeType;
    raycast.render.altWallShading = map.AltShading;
    raycast.render.setLightIntensity((uflot)map.LightLevel);

    // We keep the default player facing direction because why not.
    raycast.player.posX = (uflot)map.SpawnX;
    raycast.player.posY = (uflot)map.SpawnY;

    // Now copy the map into memory! Since we're using off-sized maps, we
    // have to copy them line by line, carefully
    for(uint8_t y = 0; y < map.Height; y++)
        memcpy_P(raycast.worldMap.map + (y * RCMAXMAPDIMENSION), map.MapData + (y * map.Width), map.Width);
    
    // For each sprite, well.. just set it up! We don't know how big the array is so we keep
    // going forever until we find the special "ending sprite"
    for(uint8_t i = 0; i < 255; i++)
    {
        SpriteInfo spinfo;
        memcpy_P(&spinfo, map.SpriteData + i, sizeof(SpriteInfo));
        if(spinfo.X == 0 && spinfo.Y == 0) break; // The special ending zone

        RcSprite<NUMINTERNALBYTES> * sprite = raycast.sprites.addSprite(spinfo.X, spinfo.Y, spinfo.Frame, spinfo.Size, spinfo.Height, NULL);

        if(spinfo.Collision > 0)
            raycast.sprites.addSpriteBounds(sprite, spinfo.Collision, true);
    }
    
    // For each loading zone, make an unlinked bounding box. Because of the nature of linked
    // sprites + bounding boxes, it's more efficient to do this after sprites (but it's not a big deal)
    for(uint8_t i = 0; i < 255; i++)
    {
        LoadingZone zone;
        memcpy_P(&zone, map.LoadingZones + i, sizeof(LoadingZone));
        if(zone.Size == 0) break; // The special ending zone
        float halfSize = zone.Size / 2;
        RcBounds * bounds = raycast.sprites.addBounds(zone.X - halfSize, zone.Y - halfSize, zone.X + halfSize, zone.Y + halfSize, false);
        // Now set the zone. When transitioning, we'll lookup the zone information again, we just need
        // to store the ID (+1)
        bounds->state |= ((i + 1) << 6);
    }
    
}


void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(FRAMERATE); 

    //Assign custom sizes for scaling
    raycast.render.spritescaling[0] = 1.0;
    raycast.render.spritescaling[1] = 0.8;
    raycast.render.spritescaling[2] = 0.6;
    raycast.render.spritescaling[3] = 0.4;

    loadArea(0);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player movement + interaction
    movement();
    testAreaTransition();

    // Draw the correct background for the area. 
    raycast.render.drawRaycastBackground(&arduboy, currentBackground);

    raycast.runIteration(&arduboy);

    arduboy.display();
}
