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

#include <cmath>
#include "ChimpCharacter.h"

namespace chimp
{

/**
 * @brief ChimpCharacter::ChimpCharacter()
 * @param tilRn Character's run ChimpTiles
 * @param tilJmp Character's jump ChimpTiles
 * @param tilIdl Character's idle ChimpTiles
 * @param rend SDL renderer that should be drawn to
 * @param pX Object's initial x-position
 * @param pY Object's initial y-position
 * @param tilesX How many times the ChimpTile should be tiled to the right.
 * @param tilesY How many times the ChimpTile should be tiled down.
 * @param frnds Factions to which the Object belongs.
 * @param enms Factions which the Object can deal damage to.
 * @param maxH Charcter's maximum health
 */
ChimpCharacter::ChimpCharacter(SDL_Renderer* const rend, const TileVec& tilRn, const TileVec& tilJmp, const TileVec& tilIdl,
                               const int pX, const int pY, const int tilesX, const int tilesY, Faction frnds,
                               Faction enms, const int maxH)
    : ChimpMobile(rend, tilIdl[0], pX, pY, tilesX, tilesY, frnds, enms), tilesRun(tilRn), tilesJump(tilJmp),
      tilesIdle(tilIdl), maxHealth(maxH)
{
    health = maxHealth;
    vulnerable = true;
    idleTime = 0;
}

void ChimpCharacter::initialize(const ChimpGame& game)
{
    health = maxHealth;
    ChimpMobile::initialize(game);
}

bool ChimpCharacter::setTilesIdle(const TileVec &vec)
{
    if(vec.empty())
        return false;
    tilesIdle = vec;
    return true;
}

bool ChimpCharacter::setTilesRun(const TileVec &vec)
{
    if(vec.empty())
        return false;
    tilesRun = vec;
    return true;
}

bool ChimpCharacter::setTilesJump(const TileVec &vec)
{
    if(vec.empty())
        return false;
    tilesJump = vec;
    return true;
}

/**
 * @brief ChimpCharacter::runRight()
 * 
 * Called when a Character begins running right.
 */
void ChimpCharacter::runRight()
{
    idleTime = 0;
    if(!runningRight)
    {
        moveStart.x = coord.x;
        tile = tilesRun[0];
        tileIndex = 0;
    }
    ChimpMobile::runRight();
}

/**
 * @brief ChimpCharacter::runLeft()
 * 
 * Called when a Character begins running left.
 */
void ChimpCharacter::runLeft()
{
    idleTime = 0;
    if(!runningLeft)
    {
        moveStart.x = coord.x;
        tile = tilesRun[0];
        tileIndex = 0;
    }
    ChimpMobile::runLeft();
}

/**
 * @brief ChimpCharacter::jump()
 * 
 * Called when a Character tries to jump.
 */
void ChimpCharacter::jump()
{
    idleTime = 0;
    if(platform)
    {
        moveStart.x = coord.x;
        tile = tilesJump[0];
        tileIndex = 0;
    }
    ChimpMobile::jump();
}

/**
 * @brief ChimpCharacter::reset()
 * 
 * Calls ChimpMobile::reset(). Sets health to maxHealth.
 */
void ChimpCharacter::reset()
{
    ChimpMobile::reset();
    health = maxHealth;
}

bool ChimpCharacter::setMaxHealth(const int heal)
{
    if(heal >= 0)
    {
        maxHealth = heal;
        return true;
    }
    return false;
}

/**
 * @brief ChimpCharacter::update
 * 
 * Calls ChimpMobile::update(). This method is where Characters take damage and/or die.
 * 
 * @param objects Vector for the game layer in which this Character resides.
 * @param screen Current window for this Character's game layer.
 * @param world Game world boundaries object.
 */
void ChimpCharacter::update(const ObjectVector& objects, ChimpGame& game, const Uint32 time)
{    
    if(active && vulnerable)
        for(const ObjectPointer& obj : objects)
        {
            if( !obj->getDamageTop() && (platform == &*obj || touchesAtBottom(*obj)) )
                continue;
            if( touches(*obj) && (friends & obj->getEnemies()) )
            {
                float x = getCenterX() - obj->getCenterX();
                float y = getCenterY() - obj->getCenterY();
                float invMag = 1.0 / std::sqrt(x*x + y*y);
                
                velocityX = DAMAGE_VELOCITY * x * invMag;
                velocityY = DAMAGE_VELOCITY * y * invMag;
                
                health -= DAMAGE;
                
                if(health <= 0)
                    deactivate();
                else
                {
                    setVulnerable(false);
                    SDL_AddTimer(INVULNERABLE_TIME, vulnerableTimer, this);
                }
            }
        }
    
    ChimpMobile::update(objects, game, time);
    
    if(coord.y > SCREEN_HEIGHT + height)
        health = 0;
}

/**
 * @brief ChimpCharacter::render()
 * 
 * Calls ChimpMobile::update(). Animates this Character by cycling through the appropriate ChimpTile vector.
 * 
 * @param screen Current window for this Character's game layer.
 */
void ChimpCharacter::render(const IntBox& screen)
{
    if(!platform)
    {
        if(int(coord.y-moveStart.y) / PIXELS_PER_FRAME_Y)
        {
            tileIndex = (tileIndex+1) % tilesJump.size();
            tile = tilesJump[tileIndex];
            moveStart.y = coord.y;
        }
    }
    else if(runningLeft || runningRight)
    {
        size_t in = std::abs((int)(coord.x-moveStart.x) / PIXELS_PER_FRAME_X) % tilesRun.size();
        if(tileIndex != in)
        {
            tile = tilesRun[in];
            tileIndex = in;
        }
    }
    else
    {
        Uint32 time = SDL_GetTicks();
        if(idleTime == 0)
        {
            tileIndex = 0;
            tile = tilesIdle[tileIndex];
            idleTime = time;
        }
        else if( (time - idleTime) / TIME_PER_FRAME )
        {
            tileIndex = (tileIndex+1) % tilesIdle.size();
            tile = tilesIdle[tileIndex];
            idleTime = time;
        }
    }
    
    if(!vulnerable)
    {
        SDL_SetTextureColorMod(tile.texture, 255, 0, 0);
        ChimpMobile::render(screen);
        SDL_SetTextureColorMod(tile.texture, 255, 255, 255);
    }
    else
        ChimpMobile::render(screen);
}

} // namespace chimp























