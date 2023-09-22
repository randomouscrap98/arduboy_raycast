#pragma once

#include "ArduboyRaycast_Utils.h"

// Representation of the player in an rcmap
class RcPlayer 
{
public:
    uflot posX;
    uflot posY;
    float dirX; //These HAVE TO be float, or something with a lot more precision
    float dirY; 

    void initPlayerDirection(float angle, float fov)
    {
        this->dirX = fov * cos(angle);
        this->dirY = fov * sin(angle);
    }

    //Attempt to move the player the given delta movement and rotation, using the given solidity checker for position
    void tryMovement(float movement, float rotation, bool (* solidChecker)(uint8_t,uint8_t))
    {
        if(movement)
        {
            uflot posX = this->posX;
            uflot posY = this->posY;
            uflot newPosX = posX + this->dirX * movement;
            uflot newPosY = posY + this->dirY * movement;

            if (solidChecker(newPosX.getInteger(), posY.getInteger()))
                newPosX = posX;
            if (solidChecker(posX.getInteger(), newPosY.getInteger()))
                newPosY = posY;

            this->posX = newPosX;
            this->posY = newPosY;
        }

        if(rotation)
        {
            float oldDirX = this->dirX;
            this->dirX = this->dirX * cos(rotation) - this->dirY * sin(rotation);
            this->dirY = oldDirX * sin(rotation) + this->dirY * cos(rotation);
        }
    }
};
