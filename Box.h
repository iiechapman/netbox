//
//  Box.h
//  SDLNetBoxGameClient
//
//  Created by Evan Chapman on 1/21/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#ifndef __SDLNetBoxGameClient__Box__
#define __SDLNetBoxGameClient__Box__

#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <string>

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

enum dir {dirUp = 0,dirDown,dirLeft,dirRight};

class Box{
public:
                            Box();
    void                    Render( void );
    void                    Update( Uint32 delta);
    
    SDL_Rect                rect;
    SDL_Color               color;
    string                  name;
    static SDL_Renderer*    renderer;
    static float            xOff,yOff;
    bool                    isShooting;
    int                     GUID;
    dir                     direction;
    SDL_Texture*            texture;
    
    
private:
    
    int                     speed;
    int                     speedFast;
    int                     speedNormal;
    int                     speedSlow;
    
    bool                    isBoosting;
    
    
private:
};


#endif /* defined(__SDLNetBoxGameClient__Box__) */
