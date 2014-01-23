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


class Box{
public:
                        Box();
    void                Render( void );
    void                Update( Uint32 delta);
    
    SDL_Rect            rect;
    SDL_Color           color;
    string              name;
    int                 clientNumber;
    static              SDL_Renderer* renderer;
    
private:
};


#endif /* defined(__SDLNetBoxGameClient__Box__) */
