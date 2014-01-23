//
//  Box.cpp
//  SDLNetBoxGameClient
//
//  Created by Evan Chapman on 1/21/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#include "Box.h"
using namespace std;

SDL_Renderer* Box::renderer = 0;

Box::Box(){
    color.r = 155;
    color.g = 105;
    color.b = 155;
    color.a = 255;
    
    rect.x = 10;
    rect.y = 10;
    rect.w = 100;
    rect.h = 100;
    
}


void Box::Render()
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
    
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderDrawRect(renderer, &rect);
}

