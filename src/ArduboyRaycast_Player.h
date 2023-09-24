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

    uflot calcNewX(float movement)
    {
        return this->posX + this->dirX * movement;
    }

    uflot calcNewY(float movement)
    {
        return this->posY + this->dirY * movement;
    }

    //Attempt to move the player the given delta movement and rotation, using the given solidity checker for position
    void tryMovement(float movement, float rotation, bool (* solidChecker)(uflot,uflot))
    {
        if(movement)
        {
            uflot posX = this->posX;
            uflot posY = this->posY;
            uflot newPosX = this->calcNewX(movement);
            uflot newPosY = this->calcNewY(movement);

            if (solidChecker(newPosX, posY))
                newPosX = posX;
            if (solidChecker(posX, newPosY))
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
