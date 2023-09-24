#pragma once

#include <ArduboyRaycast.h>

constexpr uint8_t ELLERHZCHANCE = 2; //This is actually 1 / 2 chance
constexpr uint8_t ELLERVTCHANCE = 2;
constexpr uint8_t ELLERROWSIZE = 15; // Maps are 15x15 (must be odd + fit within 16x16)

// Normally you would tell the maze generator what wall you want to use but I'm being lazy.
constexpr uint8_t ELLERWALL = 2;

// Using some algorithm called "Eller's algorithm", which is constant memory. Note that width and height 
// are generated width and height; we don't use the width/height provided by map
void ellerMaze(RcMap * map, uint8_t width, uint8_t height, RcPlayer * player)
{
    map->fillMap(ELLERWALL);

    // Width and height MUST be odd.
    if(!(width & 1)) width -= 1;
    if(!(height & 1)) height -= 1;

    // Also need to know which set each cell is in (basic row)
    uint16_t row[ELLERROWSIZE]; 
    uint16_t setId = 0;

    uint8_t xStart = 1;
    uint8_t yStart = 1;
    uint8_t xEnd = width - 2;
    uint8_t yEnd = height - 2;

    //Main algorithm loop (once per row)
    for(uint8_t y = yStart; y <= yEnd; y += 2)
    {
        // Need to initialize every other block to be a "cell". Also initialize the 
        // row set tracker
        for(uint8_t x = xStart; x <= xEnd; x += 2)
        {
            map->setCell(x, y, 0);

            // This works no matter which row we're on because the top row has a 
            // full ceiling, and thus will "initialize" the row sets to all individual
            // sets. Otherwise, the row retains the set from the previous row.
            if(map->getCell(x, y - 1) != 0)
                row[x >> 1] = ++setId;
        }
        
        // Now iterate over cells again. For each one, we simply, with some probability, 
        // join our cell with the next. We actually look backwards to make things easier
        for(uint8_t x = xStart + 2; x <= xEnd; x += 2)
        {
            uint8_t xrow = x >> 1;
            // Tear down the wall between them if the sets don't match and either a random
            // chance OR we're on the last row
            if(row[xrow - 1] != row[xrow] && (random(ELLERHZCHANCE) == 0 || y == yEnd)) {
                // Merge set by flood filling essentially
                uint8_t ovr = row[xrow];
                uint8_t flood = row[xrow - 1];
                for(uint8_t i = xrow; i < ELLERROWSIZE; i++)
                {
                    if(row[i] != ovr)
                        break;
                    row[i] = flood;
                }
                map->setCell(x - 1, y, 0);
            }
        }

        // Iterate over the cells one last time, but only if this isn't the last row. 
        if(y != yEnd)
        {
            for (uint8_t x = xStart; x <= xEnd; x += 2)
            {
                // Randomly connect with the next row down, but it's a bit compute intensive
                // because we must continuously scan the rest of the row to see if this 
                // is the last instance of this set AND there are no other sets, and if so, 
                // 100% must connect. (because memory bound)
                uint8_t isLast = 1;
                for(uint8_t i = x + 2; i <= xEnd; i += 2)
                {
                    if(row[(x >> 1)] == row[(i >> 1)])
                    {
                        isLast = 0;
                        break;
                    }
                }

                uint8_t mustConnect = isLast;

                if(isLast)
                {
                    //Full scan whole row to see if any cells in our set
                    //connect to the next row
                    for(uint8_t i = xStart; i <= xEnd; i += 2)
                    {
                        if(row[(x >> 1)] == row[(i >> 1)] && map->getCell(i, y + 1) == 0)
                        {
                            mustConnect = 0;
                            break;
                        }
                    }
                }

                if(mustConnect || random(ELLERVTCHANCE) == 0)
                {
                    // DON'T need to set the row, we scan for walls in the next iteration
                    map->setCell(x, y + 1, 0);
                }
            }
        }
    } 

    // Normally we'd put an exit but that's not the kind of game we're making
    //map->setCell(xEnd + 1, yEnd, TILEEXIT);

    player->posX = 1.6; 
    player->posY = 1.6;

    // Face the correct direction.
    if(map->getCell(xStart + 1, yStart) == 0) {
        player->dirX = 1;
        player->dirY = 0;
    }
    else {
        player->dirY = 1;
        player->dirX = 0;
    }
}

