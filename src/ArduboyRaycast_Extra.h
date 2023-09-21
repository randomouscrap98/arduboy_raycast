#pragma once

#include "ArduboyRaycast_Utils.h"
#include "ArduboyRaycast_Map.h"
#include "ArduboyRaycast_Sprite.h"

// Attempt to move the player by the given amount. The player state will be set accordingly if something happened.
void tryMovement(RcPlayer * player, RcSpriteGroup * sprites, float movement, float rotation, bool (* solidChecker)(uint8_t,uint8_t));