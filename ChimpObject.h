/*
    Copyright 2016 Jeffrey Thomas Piercy
  
    This file is part of Chimp Out!.

    Chimp Out! is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Chimp Out! is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHIMPOBJECT_H
#define CHIMPOBJECT_H

#include <SDL2/SDL.h>
#include <iostream>
#include <memory>
#include "SDLUtils.h"
#include "cleanup.h"
#include "ChimpConstants.h"
#include "ChimpTile.h"
#include "Coordinate.h"

using std::cout;
using std::endl;

namespace chimp
{

enum Faction { FACTION_VOID = 0, FACTION_PLAYER = 1<<0, FACTION_BADDIES = 1<<1 };

class ChimpObject
{
protected:
    ChimpTile tile;
    SDL_Renderer* renderer;
    const int width, height;
    Coordinate coord, center;
    float approx_zero_float, approx_zero_y;
    SDL_RendererFlip flip;
    Faction friends, enemies;
    
public:
    ChimpObject(const ChimpTile& til, SDL_Renderer* rend, const int pX,
                const int pY, const int tilesX = 1, const int tilesY = 1, Faction frnds = FACTION_VOID,
                Faction enms = FACTION_VOID);
    
    inline int getX() const { return coord.x; }
    inline int getY() const { return SCREEN_HEIGHT - coord.y - tile.textureRect.h; }
    inline int getCenterX() const { return coord.x + center.x; }
    inline int getCenterY() const { return coord.y + center.y; }
    inline int getWidth() const { return width; }
    inline int getHeight() const { return height; }
    inline int getTrueX() const { return coord.x; }
    inline int getTrueY() const { return coord.y; }
    inline int getTexRectW() const { return tile.textureRect.w; }
    inline int getTexRectH() const { return tile.textureRect.h; }
    inline int collisionLeft() const { return coord.x + tile.collisionRect.x; }
    inline int collisionRight() const { return coord.x + width - tile.collisionRect.w; }
    inline int collisionTop() const { return coord.y + tile.collisionRect.y; }
    inline int collisionBottom() const {return coord.y + height - tile.collisionRect.h; }
    /*inline int getCollisionSizeLeft() const { return collisionRect.x; }
    inline int getCollisionSizeRight() const { return collisionRect.w; }
    inline int getCollisionSizeTop() const { return collisionRect.y; }
    inline int getCollisionSizeBottom() const { return collisionRect.h; }*/
    
    inline bool touches(const ChimpObject& other) const;
    inline bool touchesAtBottom(const ChimpObject& other) const;
    
    virtual void render();
    
    float getApproxZeroFloat() { return approx_zero_float; }
    float getApproxZeroY() { return approx_zero_y; }
    
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual bool isJumper() const { return false; }
    virtual void setJumper(bool b) {}
    virtual float getAccelerationY() const { return 0; }
    virtual void setAccelerationY(const float accel) {}
    virtual float getVelocityX() const { return 0; }
    virtual void setVelocityX(const float velocity) {}
    virtual float getVelocityY() const { return 0; }
    virtual void setVelocityY(const float velocity) {}
    virtual void update(std::vector<std::unique_ptr<ChimpObject>>& objects) {}
    virtual void runRight() {}
    virtual void runLeft() {}
    virtual float getRunImpulse() const { return 0; }
    virtual void setRunImpulse(const float impulse) {}
    virtual float getRunAccel() const { return 0; }
    virtual void setRunAccel(const float accel) {}
    virtual float getJumpImpulse() const { return 0; }
    virtual void setJumpImpulse(const float impulse) {}
    virtual float getDoubleJumpFraction() const { return 0; }
    virtual void setDoubleJumpFraction(const float fraction) {}
    virtual float getJumpAccel() const { return 0; }
    virtual void setJumpAccel(const float accel) {}
    virtual float getStopFactor() const { return 0; }
    virtual void setStopFactor(const float factor) {}
    virtual float getSprintFactor() const { return 0; }
    virtual void setSprintFactor(const float factor) {}
    virtual float getResistanceX() const { return 0; }
    virtual void setResistanceX(const float resistance) {}
    virtual float getResistanceY() const { return 0; }
    virtual void setResistanceY(const float resistance) {}
    virtual int getFriends() const { return 0; }
    virtual int getEnemies() const { return 0; }
    #pragma GCC diagnostic pop
    
protected:
    inline bool approxZeroF(const float f) const { return f > -approx_zero_float && f < approx_zero_float; }
    inline bool approxZeroI(const int i) const { return i > -approx_zero_y && i < approx_zero_y; }
};

inline bool ChimpObject::touches(const ChimpObject &other) const
{
    return    collisionLeft()   <= other.collisionRight()
           && collisionRight()  >= other.collisionLeft()
           && collisionTop()    <= other.collisionBottom()
           && collisionBottom() >= other.collisionTop();
}

inline bool ChimpObject::touchesAtBottom(const ChimpObject& other) const
{
    return    collisionBottom() <= other.collisionTop()
           && collisionBottom() + approx_zero_y > other.collisionTop()
           && collisionLeft()  <= other.collisionRight()
           && collisionRight() >= other.collisionLeft();
}

} // namespace chimp

#endif // CHIMPOBJECT_H















