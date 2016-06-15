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
    along with Chimp Out!.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CHIMPMOBILE_H
#define CHIMPMOBILE_H

#include <vector>
#include "ChimpObject.h"
#include "ChimpTile.h"

namespace chimp
{

class ChimpMobile : public ChimpObject
{
protected:
    bool runningRight, runningLeft, sprinting, boundLeft, boundRight, boundTop, boundBottom,
         respawn;
    ChimpObject* platform; // pointer to Object this Mobile is standing on, null if none
    Coordinate coordInitial;
    std::string scriptInit, scriptBehavior;
    int maxJumps; // maximum number of jumps before landing
    int numJumps; // current number of jumps since last standing

    float accelerationY, velocityX, velocityY;
    float run_impulse, run_accel, jump_impulse, multi_jump_impulse, jump_accel, stop_factor, sprint_factor,
          resistance_x, resistance_y;
    
public:
    ChimpMobile(SDL_Renderer* const rend, const ChimpTile& til, const int pX = 0, const int pY = 0,
                const int tilesX = 1, const int tilesY = 1, Faction frnds = FACTION_VOID, Faction enms = FACTION_VOID);
    virtual ~ChimpMobile() {}

    void initialize(const ChimpGame& game);
    
    virtual void runRight();
    virtual void runLeft();
    virtual void stopRunningRight();
    virtual void stopRunningLeft();
    virtual void stopRunning();
    //virtual void activate();
    virtual void deactivate();
    virtual void jump();
    void stopJumping();
    void sprint();
    void stopSprinting();
    virtual void reset();

    float getAccelerationY() const { return accelerationY; }
    void setAccelerationY(const float accel) { accelerationY = accel; }
    float getVelocityX() const { return velocityX; }
    void setVelocityX(const float velocity) { velocityX = velocity; }
    float getVelocityY() const { return velocityY; }
    void setVelocityY(const float velocity) { velocityY = velocity; }
    float getRunImpulse() const { return run_impulse; }
    void setRunImpulse(const float impulse);
    float getRunAccel() const { return run_accel; }
    void setRunAccel(const float accel) { run_accel = accel; }
    float getJumpImpulse() const { return jump_impulse; }
    void setJumpImpulse(const float impulse) { jump_impulse = impulse; }
    float getMultiJumpImpulse() const { return multi_jump_impulse; }
    void setMultiJumpImpulse(const float fraction) { multi_jump_impulse = fraction; }
    float getJumpAccel() const { return jump_accel; }
    void setJumpAccel(const float accel) { jump_accel = accel; }
    float getStopFactor() const { return stop_factor; }
    void setStopFactor(const float factor) { stop_factor = factor; }
    float getSprintFactor() const { return sprint_factor; }
    void setSprintFactor(const float factor) { sprint_factor = factor; }
    float getResistanceX() const { return resistance_x; }
    void setResistanceX(const float resistance) { resistance_x = resistance; }
    float getResistanceY() const { return resistance_y; }
    void setResistanceY(const float resistance);
    float getInitialX() const { return coordInitial.x; }
    void setInitialX(const float x) { coordInitial.x = x; }
    float getInitialY() const { return coordInitial.y; }
    void setInitialY(const float y) { coordInitial.y = y; }
    bool getBoundLeft() const { return boundLeft; }
    void setBoundLeft(bool b) { boundLeft = b; }
    bool getBoundRight() const { return boundRight; }
    void setBoundRight(bool b) { boundRight = b; }
    bool getBoundTop() const { return boundTop; }
    void setBoundTop(bool b) { boundTop = b; }
    bool getBoundBottom() const { return boundBottom; }
    void setBoundBottom(bool b) { boundBottom = b; }
    bool getRespawn() const { return respawn; }
    void setRespawn(const bool pd) { respawn = pd; }
    int getMaxJumps() const { return maxJumps; }
    bool setMaxJumps(const int max);
    std::string getScriptBehavior() const { return scriptBehavior; }
    bool setScriptBehavior(const std::string& behav);
    std::string getScriptInit() const { return scriptInit; }
    bool setScriptInit(const std::string& behav);

    bool hasPlatform() const { return platform; }

    virtual void update(const ObjectVector& objects, ChimpGame& game, const Uint32 time);
    
    //ChimpMobile& operator=(const ChimpMobile& rhs);
    
protected:
    void accelerateRight();
    void accelerateLeft();
    void runScript(std::string& script, lua_State* const luast);
};

} // namespace chimp

#endif // CHIMPMOBILE_H


























