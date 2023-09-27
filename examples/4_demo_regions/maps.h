#pragma once

// Used to give us the special fixed point definitions and
// other data types used by the raycaster library
#include <ArduboyRaycast_Utils.h>
#include <ArduboyRaycast_Render.h>

// So we can link against real data
#include "bg_full.h"
#include "bg_full_inverted.h"
#include "spritesheet.h"


// To make defining and loading maps easier, we create some types to represent
// map information then store them in program memory. You could also store this
// on the FX chip (very handy). 

// This is VERY similar to the raycast sprite data, I just
// didn't want to initiate all the fields, and this is more flexible.
// You could just store the raycast sprite data directly (in fact it
// may be better that way)
struct SpriteInfo
{
    uint8_t Frame;
    float X;
    float Y;
    float Collision;
    uint8_t Size;
    uint8_t Height;
};

// Definition for a bounding box we want to place in the world which 
// will load a new map.
struct LoadingZone
{
    float X;
    float Y;
    float Size;
    uint8_t LoadMap;
};


// Information for a "whole map". Problem is:
// we may have some dynamic amount of data for certain thingss, such
// as sprites. While we could simply have fullsize maps and fullsize
// arrays for max sprites, that's a bit wasteful, so I've opted for
// a more complicated approach of linking everything with pointers.
struct MapInfo
{
    const uint8_t * MapData;
    const SpriteInfo * SpriteData;
    const LoadingZone * LoadingZones;
    uint8_t Width;
    uint8_t Height;
    muflot SpawnX;
    muflot SpawnY;
    const uint8_t * Background;
    RcShadingType ShadeType;
    RcShadingType AltShading;
    muflot LightLevel;
};


// Since we're storing MapInfo in a complicated object but in a nice array,
// we can store all the bits of data individually. Perhaps that's not the 
// best way, so you can do this however you want. Just wanted to show that
// you could load the information into the raycaster library however you want.
constexpr uint8_t CaveMap[8 * 8] PROGMEM = {
    4,  4,  4,  4,  4,  4,  4,  4,
    4,  0,  0,  0,  0,  2,  2,  4,
   13,  0,  0,  0,  0,  0,  2,  4,
    4,  0,  0,  1,  1,  0,  0,  4,
    4,  0,  0,  1,  1,  0,  0,  4,
    4,  2,  0,  1,  0,  0,  2,  4,
    4,  0,  0,  0,  0,  2,  2,  4,
    4,  4,  4,  4,  4,  4,  4,  4,
};

constexpr float BarrelCollision = 0.5;
constexpr uint8_t BarrelSize = 2;
constexpr uint8_t BarrelHeight = 8;

constexpr float CrateCollision = 0.75;
constexpr uint8_t CrateSize = 2;
constexpr uint8_t CrateHeight = 8;

constexpr float RockCollision = 0.85;
constexpr uint8_t RockSize = 1;
constexpr uint8_t RockHeight = 6;


constexpr SpriteInfo CaveSprites[] PROGMEM = {
    SpriteInfo {
        MySprites::Barrel,
        1.5, 1.3,
        BarrelCollision,
        BarrelSize, BarrelHeight
    },
    SpriteInfo {
        MySprites::Barrel,
        2.0, 1.3,
        BarrelCollision,
        BarrelSize, BarrelHeight
    },
    SpriteInfo {
        MySprites::Barrel,
        1.3, 1.7,
        BarrelCollision,
        BarrelSize, BarrelHeight
    },
    SpriteInfo {
        MySprites::Barrel,
        4.3, 1.3,
        BarrelCollision,
        BarrelSize, BarrelHeight
    },
    SpriteInfo {
        MySprites::Crate,
        1.4, 4.6,
        CrateCollision,
        CrateSize, CrateHeight 
    },
    SpriteInfo {
        MySprites::Crate,
        4.4, 5.4,
        CrateCollision,
        CrateSize, CrateHeight
    },
    SpriteInfo {
        MySprites::Rock,
        6.5, 3.5,
        RockCollision,
        RockSize, RockHeight
    },
    SpriteInfo {
        MySprites::Rock,
        6.4, 4.6,
        RockCollision,
        RockSize, RockHeight
    },
    SpriteInfo {
        MySprites::Rock,
        1.5, 6.5,
        RockCollision,
        RockSize, RockHeight
    },
    // Instead of having to store a size somewhere, and because I don't think 
    // items stored in progrmem have an appropriate size tied to them (or maybe
    // they do?), we simply end these kinds of arrays with a special struct
    // that indicates it's the end
    SpriteInfo { 0, 0, 0 }
};

constexpr LoadingZone CaveLoadingZones[] PROGMEM = {
    LoadingZone {
        0.5, 2.5, 1.0, 1 // In the cave entrance, load the outdoor map
    },
    LoadingZone { 0, 0, 0 }
};


constexpr uint8_t OverworldMap[8 * 8] PROGMEM = {
   10, 10, 10, 10, 10, 10, 10, 10,
   10,  0,  0,  0,  0,  0,  0, 10,
   10,  0,  0,  0,  0,  0, 12, 12,
   11,  0,  0,  0,  0,  0, 12, 12,
   10,  0,  0,  0,  0,  0, 13, 12,
   10,  0,  0,  0,  0,  0, 12, 12,
   10,  0,  0,  0,  0,  0,  0, 10,
   10, 10, 10, 10, 10, 10, 10, 10,
};

constexpr float TreeCollision = 1.05;
constexpr uint8_t TreeSize = 0;
constexpr uint8_t TreeHeight = 0;

constexpr float Grave1Collision = 0.65;
constexpr uint8_t Grave1Size = 2;
constexpr uint8_t Grave1Height = 8;

constexpr float Grave2Collision = 0.75;
constexpr uint8_t Grave2Size = 1;
constexpr uint8_t Grave2Height = 6;

constexpr float KeyCollision = 0;
constexpr uint8_t KeySize = 3;
constexpr uint8_t KeyHeight = 13;

constexpr SpriteInfo OverworldSprites[] PROGMEM = {
    SpriteInfo {
        MySprites::DeadTree,
        1.5, 1.5,
        TreeCollision,
        TreeSize, TreeHeight
    },
    SpriteInfo {
        MySprites::DeadTree,
        2.0, 6.0,
        TreeCollision,
        TreeSize, TreeHeight
    },
    SpriteInfo {
        MySprites::DeadTree,
        4.5, 2.5,
        TreeCollision,
        TreeSize, TreeHeight
    },
    SpriteInfo {
        MySprites::Grave2,
        6.5, 1.5,
        Grave2Collision,
        Grave2Size, Grave2Height 
    },
    SpriteInfo {
        MySprites::Grave2,
        3.5, 5.0,
        Grave2Collision,
        Grave2Size, Grave2Height 
    },
    SpriteInfo {
        MySprites::Grave1,
        3.0, 1.5,
        Grave1Collision,
        Grave1Size, Grave1Height 
    },
    SpriteInfo {
        MySprites::Grave1,
        4.0, 1.5,
        Grave1Collision,
        Grave1Size, Grave1Height 
    },
    SpriteInfo {
        MySprites::Grave1,
        3.0, 3.0,
        Grave1Collision,
        Grave1Size, Grave1Height 
    },
    SpriteInfo {
        MySprites::Grave1,
        4.5, 6.5,
        Grave1Collision,
        Grave1Size, Grave1Height 
    },
    SpriteInfo {
        MySprites::Grave1,
        3.2, 6.3,
        Grave1Collision,
        Grave1Size, Grave1Height 
    },
    SpriteInfo {
        MySprites::Key,
        6.8, 6.2,
        KeyCollision,
        KeySize, KeyHeight
    },
    SpriteInfo { 0, 0, 0 }
};

constexpr LoadingZone OverworldLoadingZones[] PROGMEM = {
    LoadingZone {
        6.5, 4.5, 1.0, 0  // In the cave entrance, load the cave map
    },
    LoadingZone { 0, 0, 0 }
};


// All information about all maps! Or at least, links to the information lol
constexpr MapInfo AllMaps[] PROGMEM = {
    MapInfo {
        CaveMap, 
        CaveSprites,
        CaveLoadingZones,
        8, 8,       // Map size
        1.5, 2.5,   // Player spawn
        bg_full,
        RcShadingType::Black,
        RcShadingType::Black,
        1.0         // Light level
    },
    MapInfo {
        OverworldMap, 
        OverworldSprites,
        OverworldLoadingZones,
        8, 8,       // Map size
        5.5, 4.5,   // Player spawn
        bg_full_inverted,
        RcShadingType::White,
        RcShadingType::None,
        2.0         // Light level
    }
};
