//
//  bullet.cpp
//  SDLNetBoxGame
//
//  Created by Evan Chapman on 1/23/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#include "bullet.h"


bullet::bullet()
{
    mRect = new SDL_Rect;
    mRect->w = 100;
    mRect->h = 100;
    
    mColor.r = 255;
    mColor.g = 50;
    mColor.b = 50;
    mCurrentSpeed = 0;
    StopMoving();
    SetOnline();
}

bullet::~bullet()
{
    
}

