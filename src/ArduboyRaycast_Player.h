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

    uflot calcNewX(float movement, float strafe)
    {
        return this->posX + this->dirX * movement + this->dirY * strafe;
    }

    uflot calcNewY(float movement, float strafe)
    {
        return this->posY + this->dirY * movement - this->dirX * strafe;
    }

    // Attempt to move the player the given
    void tryMovement(float movement, float movementStrafe, float rotation, bool (* solidChecker)(uflot,uflot))
    {
        if(movement || movementStrafe)
        {
            uflot posX = this->posX;
            uflot posY = this->posY;
            uflot newPosX = this->calcNewX(movement, movementStrafe);
            uflot newPosY = this->calcNewY(movement, movementStrafe);

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

    float getAngle()
    {
        float result = atan2(this->dirY, this->dirX);
        if(result < 0)
            result += 2 * M_PI;
        return result;
    }

    //Attempt to move the player the given delta movement and rotation, using the given solidity checker for position
    inline void tryMovement(float movement, float rotation, bool (* solidChecker)(uflot,uflot))
    {
        this->tryMovement(movement, 0, rotation, solidChecker);
    }
};
