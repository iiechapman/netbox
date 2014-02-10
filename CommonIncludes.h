//
//  CommonIncludes.h
//  SDLNetBoxGame
//
//  Created by Evan Chapman on 2/10/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#ifndef __SDLNetBoxGame__CommonIncludes__
#define __SDLNetBoxGame__CommonIncludes__

#include <iostream>
#include <string>
#include <string>
#include <cmath>
#include <iomanip>


#include <SDL2/SDL.h>
#include <SDL2_net/SDL_net.h>



struct direction{
    bool up = false;
    bool down = false;
    bool left = false;
    bool right = false;
};


#endif /* defined(__SDLNetBoxGame__CommonIncludes__) */
