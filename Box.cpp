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
float         Box::xOff = 0;
float         Box::yOff = 0;

Box::Box(){
    mRect = new SDL_Rect;
    mColor.r = 200;
    mColor.g = 200;
    mColor.b = 200;
    mColor.a = 255;
    
    mRect->x = 10;
    mRect->y = 10;
    mRect->w = 300;
    mRect->h = 300;
    
    mCurrentSpeed = 0;
    
    StopShooting();
    StopMoving();
}


void Box::Update( Uint32 delta )
{
    if ( IsOnline() )
    {
    if ( mIsMoving )
    {
        if (mCurrentSpeed < mMaxSpeed )
        {
            mCurrentSpeed += mAccelSpeed;
        }
        
        if (mCurrentSpeed >= mMaxSpeed )
        {
            mCurrentSpeed = mMaxSpeed;
        }
    } else
    {
        mCurrentSpeed -= mDecelSpeed;
        if (mCurrentSpeed <= 0 )
        {
            mDir.up     = false;
            mDir.down   = false;
            mDir.right  = false;
            mDir.left   = false;
            
            mCurrentSpeed = 0;
        }
        
    }

    if ( mDir.up )
    {
        mRect->y -= mCurrentSpeed;
    }
    
    if ( mDir.down )
    {
        mRect->y += mCurrentSpeed;
    }
    
    if ( mDir.right )
    {
        mRect->x += mCurrentSpeed;
    }
    
    if ( mDir.left )
    {
        mRect->x -= mCurrentSpeed;
    }
    }

}


void Box::Render()
{ 

    if ( IsOnline() )
    {
    SDL_Rect* finalRect = new SDL_Rect;
    
    finalRect->x = mRect->x;
    finalRect->y = mRect->y;
    
    finalRect->w = mRect->w;
    finalRect->h = mRect->h;
    
    finalRect->x -= xOff;
    finalRect->y -= yOff;
    
    SDL_SetRenderDrawColor(renderer, mColor.r,mColor.g, mColor.b, mColor.a);
    SDL_RenderFillRect(renderer, finalRect);
    
    SDL_SetRenderDrawColor(renderer,255,255,255,255);
    SDL_RenderDrawRect(renderer, finalRect);
    }
}

