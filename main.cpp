﻿/*
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

#include <fstream>
#include <iostream>
#include <memory>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_gamecontroller.h>
#include <string>
#include <vector>
#include "cleanup.h"
#include "ChimpConstants.h"
#include "ChimpGame.h"
#include "ChimpObject.h"
#include "ChimpMobile.h"
#include "ChimpCharacter.h"
#include "ChimpTile.h"
#include "SDLUtils.h"

using std::cout;
using std::endl;

typedef std::vector<std::unique_ptr<chimp::ChimpObject>> ObjectVector;

bool loadChimpTextures(std::vector<chimp::ChimpTile>& tiles, std::vector<SDL_Texture*>& textures,
                       SDL_Renderer* renderer);
void addController(int id);
void pushObject(ObjectVector& objects, chimp::ChimpTile& til, SDL_Renderer* renderer, const int x, const int y,
                 const int tilesX, const int tilesY);
void pushMobile(ObjectVector& objects, chimp::ChimpTile& til, SDL_Renderer* renderer, const int x, const int y,
                const int tilesX, const int tilesY);
void pushCharacter(ObjectVector& objects, chimp::ChimpTile& til, SDL_Renderer* renderer, const int x, const int y,
                   const int tilesX, const int tilesY, int maxH, chimp::Faction friends, chimp::Faction enemies);

inline void keyDown(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed);
inline void keyUp(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed);
inline void buttonDown(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed);
inline void buttonUp(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed);
inline void axisMotion(SDL_Event& event, chimp::ChimpGame* game);

inline void drawHUD(chimp::ChimpGame* game, SDL_Renderer* renderer, TTF_Font* font, SDL_Texture* healthTex);

chimp::ChimpGame* generateWorld1(std::vector<chimp::ChimpTile> &tiles, SDL_Renderer* renderer);
chimp::ChimpGame* generateWorld2(std::vector<chimp::ChimpTile> &tiles, SDL_Renderer* renderer);

int main(int argc, char** argv)
{
    SDL_Window* window;
    SDL_Renderer* renderer;
    TTF_Font* font;
    std::vector<SDL_GameController*> controllers;
    std::vector<SDL_Texture*> textures;
    std::vector<chimp::ChimpTile> tiles;
    
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_TIMER) < 0)
    {
        logSDLError(std::cout, "SDL_Init");
        return 1;
    }
    window = SDL_CreateWindow("Chimp Out!", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if(window == nullptr)
    {
        logSDLError(std::cout, "CreateWindow");
        SDL_Quit();
        return 1;
    }
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(renderer == nullptr)
    {
        logSDLError(std::cout, "CreateRenderer");
        cleanup(window);
        SDL_Quit();
        return 1;
    }
    if( (IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG )
    {
        logSDLError(std::cout, "IMG_Init");
        cleanup(window, renderer);
        SDL_Quit();
        return 1;
    }
    if(TTF_Init() != 0)
    {
        logSDLError(std::cout, "TTF_Init");
        cleanup(window, renderer);
        SDL_Quit();
        return 1;
    }
    font = TTF_OpenFont( (ASSETS_PATH + FONT_FILE).c_str(), FONT_SIZE );
    if(font == nullptr)
    {
        logSDLError(std::cout, "TTF_OpenFont");
        cleanup(window, renderer, font);
        SDL_Quit();
        return 1;
    }
    if( !loadChimpTextures(tiles, textures, renderer) )
    {
        cleanup(window, renderer, font, &tiles);
        SDL_Quit();
        return 1;
    }
    if(SDL_GameControllerAddMappingsFromFile( CONTROLLER_MAP_FILE.c_str() ) == -1)
        logSDLError(std::cout, "GameControllerAddMappingsFromFile");
    for(int i = 0; i < SDL_NumJoysticks(); ++i)
    {
        if( SDL_IsGameController(i) )
            controllers.push_back( SDL_GameControllerOpen(i) );
    }
    
    SDL_Event event;
    bool quit = false;
    bool keyJumpPressed = false;
    chimp::ChimpGame* game = generateWorld1(tiles, renderer);
    SDL_Texture* healthTex = renderText(TEXT_HEALTH, font, FONT_COLOR, renderer);
    int health = -1;
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    
    /*while(!quit)
    {
        SDL_PollEvent(&event);
        if(event.type == SDL_CONTROLLERBUTTONDOWN && event.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
            quit = true;
    }
    quit = false;*/
    
    while(!quit)
    {
        while( SDL_PollEvent(&event) )
        {
            switch(event.type)
            {
            case SDL_QUIT:
                quit = true;
                continue;
            case SDL_KEYDOWN:
                keyDown(event, game, keyJumpPressed);
                break;
            case SDL_KEYUP:
                keyUp(event, game, keyJumpPressed);
                break;
            case SDL_CONTROLLERBUTTONDOWN:
                buttonDown(event, game, keyJumpPressed);
                break;
            case SDL_CONTROLLERBUTTONUP:
                buttonUp(event, game, keyJumpPressed);
                break;
            case SDL_CONTROLLERAXISMOTION:
                axisMotion(event, game);
                break;
            }
        }
        
        SDL_RenderClear(renderer);
        (*game).render();
        drawHUD(game, renderer, font, healthTex);
        SDL_RenderPresent(renderer);
        
        SDL_Delay(1);
    }
    
    delete game;
    cleanup(window, renderer, font, &tiles);
    
    return 0;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
bool loadChimpTextures(std::vector<chimp::ChimpTile>& tiles, std::vector<SDL_Texture*>& textures,
                       SDL_Renderer* renderer)
{
    std::string line;
    int sub1, sub2;
    int count;
    bool countLoaded;
    
    // Load Textures
    std::ifstream textureData(TEXTURES_FILE);
    if( !textureData.is_open() )
    {
        std::cout << "Couldn't open texture data file." << std::endl;
        return false;
    }
    countLoaded = false;
    while(!countLoaded)
    {
        std::getline(textureData, line);
        sub1 = line.find(TEXTURE_DELIMITER);
        if(sub1 == std::string::npos)
            continue;
        sub2 = line.find(TEXTURE_COMMENT);
        if(sub2 != std::string::npos && sub2 < sub1)
            continue;
        
        ++sub1;
        sub2 = line.find(";", sub1);
        count = std::stoi( line.substr(sub1, sub2-sub1) );
        countLoaded = true;
    }
    for(int i = 0; i < count; ++i)
    {
        std::getline(textureData, line);
        sub1 = line.find(TEXTURE_DELIMITER);
        if(sub1 == std::string::npos)
        {
            --i;
            continue;
        }
        sub2 = line.find(TEXTURE_COMMENT);
        if(sub2 != std::string::npos && sub2 < sub1)
            continue;
        ++sub1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        textures.push_back( loadTexture(ASSETS_PATH + line.substr(sub1, sub2-sub1), renderer) );
        
        if(textures.back() == nullptr)
            return false;
    }
    textureData.close();
    
    // Load Tiles
    std::ifstream tileData(TILES_FILE);
    if( !tileData.is_open() )
    {
        std::cout << "Couldn't open tile data file." << std::endl;
        return false;
    }
    countLoaded = false;
    while(!countLoaded)
    {
        std::getline(tileData, line);
        sub1 = line.find(TEXTURE_DELIMITER);
        if(sub1 == std::string::npos)
            continue;
        sub2 = line.find(TEXTURE_COMMENT);
        if(sub2 != std::string::npos && sub2 < sub1)
            continue;
        
        ++sub1;
        sub2 = line.find(";", sub1);
        count = std::stoi( line.substr(sub1, sub2-sub1) );
        countLoaded = true;
    }
    for(int i = 0; i < count; ++i)
    {
        std::getline(tileData, line);
        sub1 = line.find(TEXTURE_DELIMITER);
        if(sub1 == std::string::npos)
        {
            --i;
            continue;
        }
        sub2 = line.find(TEXTURE_COMMENT);
        if(sub2 != std::string::npos && sub2 < sub1)
        {
            --i;
            continue;
        }
        
        tiles.push_back( chimp::ChimpTile() );
        
        ++sub1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().texture = textures[std::stoi( line.substr(sub1, sub2-sub1) )];
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        const int scale = std::stoi( line.substr(sub1, sub2-sub1) );
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().textureRect.x = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().textureRect.y = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().textureRect.w = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().textureRect.h = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().collisionRect.x = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().collisionRect.w = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().collisionRect.y = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
        
        sub1 = sub2 + 1;
        sub2 = line.find(TEXTURE_DELIMITER, sub1);
        tiles.back().collisionRect.h = std::stoi( line.substr(sub1, sub2-sub1) ) / scale;
    }
    tileData.close();
    
    return true;
}
#pragma GCC diagnostic pop

/*void addController(int id)
{
    if( SDL_IsGameController(id) )
    {
        SDL_GameController* pad = SDL_GameControllerOpen(id);
        if(pad)
        {
            SDL_Joystick* joy = SDL_GameControllerGetJoystick(pad);
            int instanceID = SDL_JoystickInstanceID(joy);
        }
    }
}*/

/*void pushObject(ObjectVector& objects, chimp::ChimpTile& til, SDL_Renderer* renderer, const int x, const int y,
                 const int tilesX, const int tilesY)
{
    objects.push_back(std::unique_ptr<chimp::ChimpObject>(
                          new chimp::ChimpObject(til, renderer, x, y, tilesX, tilesY) ));
}

void pushMobile(ObjectVector &objects, chimp::ChimpTile &til, SDL_Renderer *renderer, const int x, const int y,
                const int tilesX, const int tilesY)
{
    objects.push_back(std::unique_ptr<chimp::ChimpMobile>(
                          new chimp::ChimpMobile(til, renderer, x, y, tilesX, tilesY) ));
}


void pushCharacter(ObjectVector& objects, chimp::ChimpTile& til, SDL_Renderer* renderer, const int x, const int y,
                   const int tilesX, const int tilesY, int maxH, chimp::Faction friends, chimp::Faction enemies)
{
    objects.push_back(std::unique_ptr<chimp::ChimpCharacter>(
                          new chimp::ChimpCharacter(til, renderer, x, y, tilesX, tilesY, maxH, friends, enemies) ));
}*/

chimp::ChimpGame* generateWorld1(std::vector<chimp::ChimpTile> &tiles, SDL_Renderer* renderer)
{
    chimp::ChimpGame* game = new chimp::ChimpGame( renderer, chimp::ChimpCharacter(tiles[0], renderer, SCREEN_WIDTH>>1,
                                                   400, 1, 1, chimp::FACTION_PLAYER, chimp::FACTION_BADDIES, 100) );
    
    (*game).getPlayer().setScreenBoundLeft(true);
    (*game).getPlayer().setScreenBoundRight(true);
    (*game).pushObj(chimp::MID, tiles[1], 0, 120, 8, 1);
    (*game).pushObj(chimp::MID, tiles[1], SCREEN_WIDTH / 10, 0, SCREEN_WIDTH / tiles[1].textureRect.w + 1, 3);
    (*game).pushChar(chimp::MID, tiles[2], -35, 160, 1, 1, 100, chimp::FACTION_BADDIES, chimp::FACTION_PLAYER);
    (*game).getObjBack(chimp::MID).setRunAccel(RUN_ACCEL / 3.8);
    (*game).getObjBack(chimp::MID).runRight();
    (*game).pushChar(chimp::MID, tiles[2], SCREEN_WIDTH, 160, 1, 1, 100, chimp::FACTION_BADDIES,
                          chimp::FACTION_PLAYER);
    (*game).getObjBack(chimp::MID).setRunAccel(RUN_ACCEL / 4.0);
    (*game).getObjBack(chimp::MID).runLeft();
    (*game).getObjBack(chimp::MID).setJumper(true);
    (*game).getObjBack(chimp::MID).setJumpImpulse(JUMP_IMPULSE * 0.75);
    (*game).pushObj(chimp::BACK, tiles[4], 0, -tiles[4].textureRect.h + 25, SCREEN_WIDTH/tiles[4].textureRect.w+1, 1);
    (*game).pushObj(chimp::FORE, tiles[5], SCREEN_WIDTH*0.75, 10, 1, 1);
    
    return game;
}


chimp::ChimpGame* generateWorld2(std::vector<chimp::ChimpTile> &tiles, SDL_Renderer* renderer)
{
    chimp::ChimpGame* game = new chimp::ChimpGame( renderer, chimp::ChimpCharacter(tiles[9], renderer, SCREEN_WIDTH>>1,
                                                   400, 1, 1, chimp::FACTION_PLAYER, chimp::FACTION_BADDIES, 100) );
    
    (*game).getPlayer().setScreenBoundLeft(true);
    (*game).getPlayer().setScreenBoundRight(true);
    (*game).pushObj(chimp::MID, tiles[7], 0, 9, 3, 1);
    (*game).pushObj(chimp::MID, tiles[8], tiles[7].textureRect.w*3, 9, 1, 1);
    
    return game;
}

inline void keyDown(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed)
{
    switch(event.key.keysym.sym)
    {
    case SDLK_RIGHT:
        (*game).getPlayer().runRight();
        break;
    case SDLK_LEFT:
        (*game).getPlayer().runLeft();
        break;
    case SDLK_UP:
    case SDLK_SPACE:
        if(!keyJumpPressed)
        {
            (*game).getPlayer().jump();
            keyJumpPressed = true;
        }
        break;
    case SDLK_x:
        (*game).getPlayer().sprint();
        break;
    }
}

inline void keyUp(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed)
{
    switch(event.key.keysym.sym)
    {
    case SDLK_RIGHT:
        (*game).getPlayer().stopRunningRight();
        break;
    case SDLK_LEFT:
        (*game).getPlayer().stopRunningLeft();
        break;
    case SDLK_UP:
    case SDLK_SPACE:
        (*game).getPlayer().stopJumping();
        keyJumpPressed = false;
        break;
    case SDLK_x:
        (*game).getPlayer().stopSprinting();
        break;
    }
}

inline void buttonDown(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed)
{
    if(event.cbutton.button == SDL_CONTROLLER_BUTTON_A && !keyJumpPressed)
    {
        (*game).getPlayer().jump();
        keyJumpPressed = true;
    }
    else if(event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
        (*game).getPlayer().sprint();
}

inline void buttonUp(SDL_Event& event, chimp::ChimpGame* game, bool& keyJumpPressed)
{
    if(event.cbutton.button == SDL_CONTROLLER_BUTTON_A)
    {
        (*game).getPlayer().stopJumping();
        keyJumpPressed = false;
    }
    else if(event.cbutton.button == SDL_CONTROLLER_BUTTON_X)
        (*game).getPlayer().stopSprinting();
}

inline void axisMotion(SDL_Event& event, chimp::ChimpGame* game)
{
    if(event.caxis.axis == 0)
    {
        if(event.caxis.value > JOYSTICK_DEAD_ZONE)
            (*game).getPlayer().runRight();
        else if(event.caxis.value < -JOYSTICK_DEAD_ZONE)
            (*game).getPlayer().runLeft();
        else
            (*game).getPlayer().stopRunning();
    }
}

inline void drawHUD(chimp::ChimpGame* game, SDL_Renderer* renderer, TTF_Font* font, SDL_Texture* healthTex)
{
    static int oldHealth, w1, w2, h, x;
    static SDL_Texture* currentHealthTex;
    
    if(oldHealth != game->getPlayer().getHealth() || !currentHealthTex)
    {
        SDL_DestroyTexture(currentHealthTex);
        oldHealth = game->getPlayer().getHealth();
        std::string healthString = std::to_string(oldHealth);
        currentHealthTex = renderText(healthString, font, FONT_COLOR, renderer);
        SDL_QueryTexture(healthTex, NULL, NULL, &w1, &h);
        SDL_QueryTexture(currentHealthTex, NULL, NULL, &w2, &h);
    }
    
    //x = (SCREEN_WIDTH - w1 - w2)>>1;
    x = (SCREEN_WIDTH>>1) - w1;
    renderTexture(healthTex, renderer, x, 10);
    renderTexture(currentHealthTex, renderer, x + w1, 10);
}































