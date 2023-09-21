#include "ArduboyRaycast_Extra.h"

// Attempt to move the player by the given amount. The player state will be set accordingly if something happened.
void tryMovement(RcPlayer * player, RcSpriteGroup * sprites, float movement, float rotation, bool (* solidChecker)(uint8_t,uint8_t))
{
    if(movement)
    {
        uflot posX = player->posX;
        uflot posY = player->posY;
        uflot newPosX = posX + player->dirX * movement;
        uflot newPosY = posY + player->dirY * movement;

        if (solidChecker(newPosX.getInteger(), posY.getInteger()))
            newPosX = posX;
        if (solidChecker(posX.getInteger(), newPosY.getInteger()))
            newPosY = posY;

        if(sprites)
        {
            uint8_t numbounds = sprites->numbounds;
            for (uint8_t i = 0; i < numbounds; i++)
            {
                RcBounds *bounds = &sprites->bounds[i];
                if (!ISSPRITEACTIVE((*bounds)))
                    continue;

                if(bounds->colliding(newPosX, posY))
                    newPosX = posX;
                if(bounds->colliding(posX, newPosY))
                    newPosY = posY;
            }
        }

        player->posX = newPosX;
        player->posY = newPosY;
    }

    if(rotation)
    {
        float oldDirX = player->dirX;
        player->dirX = player->dirX * cos(rotation) - player->dirY * sin(rotation);
        player->dirY = oldDirX * sin(rotation) + player->dirY * cos(rotation);
    }
}