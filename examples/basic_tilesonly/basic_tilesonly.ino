#include <Arduboy2.h>
#include <FixedPoints.h>

#include "../../lib/raycast.h"

Arduboy2 arduboy;

void setup()
{
    arduboy.begin();
    arduboy.setFrameRate(30); //30 is a safe number for raycasting
}

void loop()
{
    if(!arduboy.nextFrame()) return;
}
