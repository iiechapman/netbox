//
//  Box.h
//  SDLNetBoxGameClient
//
//  Created by Evan Chapman on 1/21/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#ifndef __SDLNetBoxGameClient__Box__
#define __SDLNetBoxGameClient__Box__

#include "CommonIncludes.h"
#include <SDL2_image/SDL_image.h>


using namespace std;


/**
 * \class Box
 * \brief Renderable square for use in game
 * \note In Progress
 * \author Evan Chapman
 * \version 1
 * \date Jan 21 2014
 * Contact: iiechapman\@gmail.com
 */


class Box{
public:
                            Box();
    void                    Render( void );
    void                    Update( Uint32 delta);
    
    void                    SetRectX( int val ){ mRect->x = val; }
    void                    SetRectY( int val ){ mRect->y = val; }
    void                    SetRectW( int val ){ mRect->w = val; };
    void                    SetRectH( int val ){ mRect->h = val; };
    
    SDL_Rect*               Rect( void ){return mRect;}
    
    void                    SetRect( SDL_Rect* rect ) { mRect = rect; }
    void                    SetColor( SDL_Color color ){ mColor = color; }
    const SDL_Color         Color( void ) const { return mColor;}
    
    void                    SetColorRed( int val ){ mColor.r = val; }
    void                    SetColorGreen( int val ){ mColor.g = val; }
    void                    SetColorBlue( int val ){ mColor.b = val; }
    
    
    const bool              IsShooting( void ) const {return mIsShooting; }
    void                    Shoot( void ){ mIsShooting = true; }
    void                    StopShooting( void ){ mIsShooting = false; }
    
    void                    SetGUID( int guid ){ mGUID = guid; }
    void                    SetName( string name ){ mName = name;}
    const string            Name( void )const { return mName;}
    const int               GUID( void )const {return mGUID;}
    
    direction*              Dir( void ){ return &mDir;}
    
    void                    SetNormalSpeed( int val ){ mSpeedNormal   = val; }
    void                    SetFastSpeed( int val ){ mSpeedFast     = val; }
    void                    SetSlowSpeed( int val ){ mSpeedSlow     = val; }
    
    const bool              IsMoving( void ){ return mIsMoving;}
    void                    Move( void ){ mIsMoving = true; }
    void                    StopMoving( void ){ mIsMoving = false;}
    
    void                    MoveFast( void ){ mMaxSpeed = mSpeedFast; }
    void                    MoveSlow( void ){ mMaxSpeed = mSpeedSlow; }
    void                    MoveNormal( void ){ mMaxSpeed = mSpeedNormal; }
    
    void                    Boost( void ){ mIsBoosting = true; }
    void                    StopBoost( void ){ mIsBoosting = false;}
    
    void                    SetOnline( void ){ mIsOnline = true;}
    void                    SetOffline( void ){ mIsOnline = false;}
    const bool              IsOnline( void ){ return mIsOnline;}
    
    
    
    static SDL_Renderer*    renderer;
    static float            xOff,yOff;
    
    
protected:
    //Render Components
    SDL_Rect*               mRect;
    SDL_Color               mColor;
    
    //Game Components
    string                  mName;
    bool                    mIsShooting;
    int                     mGUID = -1;
    direction               mDir;
    
    //Physical Components
    float                   mSpeedFast = 400;
    float                   mSpeedNormal = 100;
    float                   mSpeedSlow = 100;
    float                   mCurrentSpeed;
    float                   mMaxSpeed = mSpeedNormal;
    float                   mAccelSpeed = mMaxSpeed * .1;
    float                   mDecelSpeed = mAccelSpeed * .1;

    bool                    mIsMoving;
    bool                    mIsBoosting;
    bool                    mIsOnline = false;
    
    
private:
};


#endif /* defined(__SDLNetBoxGameClient__Box__) */















