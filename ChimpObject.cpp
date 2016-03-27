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

#include "ChimpObject.h"

ChimpObject::ChimpObject(SDL_Texture* tex, SDL_Rect& texRect, SDL_Rect& collRect, SDL_Renderer* rend,
                         const int positionX, const int positionY)
    : ChimpObject(tex, texRect, collRect, rend, positionX, positionY, 1, 1) {}

ChimpObject::ChimpObject(SDL_Texture* tex, SDL_Rect& texRect, SDL_Rect& collRect, SDL_Renderer* rend,
                         const int positionX, const int positionY, const int tilesX, const int tilesY)
    : texture(tex), textureRect(texRect), collisionRect(collRect), renderer(rend), width(textureRect.w*tilesX),
      height(textureRect.h*tilesY)
{
    positionRect.w = textureRect.w;
    positionRect.h = textureRect.h;
    positionRect.x = positionX - (positionRect.w >> 1);
    positionRect.y = SCREEN_HEIGHT - positionY - height;
    approx_zero_float = RUN_IMPULSE / 4.0;
    approx_zero_y = int( ceil(GRAVITY / RESISTANCE_Y * APPROX_ZERO_Y_FACTOR) );
}

void ChimpObject::render()
{
    SDL_Rect pos = positionRect;
    for(int x = 0; x < width; x += textureRect.w)
        for(int y = 0; y < height; y += textureRect.h)
        {
            pos.x = positionRect.x + x;
            pos.y = positionRect.y + y;
            SDL_RenderCopy(renderer, texture, &textureRect, &pos);
        }
}

bool ChimpObject::touches(const ChimpObject &other) const
{
    /*return    positionRect.x          <= other.positionRect.x + other.width
           && positionRect.x + width  >= other.positionRect.x
           && positionRect.y          <= other.positionRect.y + other.height
           && positionRect.y + height >= other.positionRect.y;*/
    
    return    getCollisionLeft()   <= other.getCollisionRight()
           && getCollisionRight()  >= other.getCollisionLeft()
           && getCollisionTop()    <= other.getCollisionBottom()
           && getCollisionBottom() >= other.getCollisionTop();
}

bool ChimpObject::touchesAtBottom(const ChimpObject& other) const
{
    /*return    approxZeroI(positionRect.y + height - other.positionRect.y)
           && positionRect.x         <= other.positionRect.x + other.width
           && positionRect.x + width >= other.positionRect.x;*/
    
    return    approxZeroI( getCollisionBottom() - other.getCollisionTop() )
           && getCollisionLeft()  <= other.getCollisionRight()
           && getCollisionRight() >= other.getCollisionLeft();
}







































