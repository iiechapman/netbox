//
//  bullet.h
//  SDLNetBoxGame
//
//  Created by Evan Chapman on 1/23/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#ifndef __SDLNetBoxGame__bullet__
#define __SDLNetBoxGame__bullet__

#include <iostream>
#include <SDL2/SDL.h>
#include <string>
#include "Box.h"
using namespace std;


class bullet : public Box {
public:
                        bullet();
                        ~bullet();
    void                Update( Uint32 delta);
    
    dir                 direction;
    float               speed;
    bool                isFriendly;
    
private:
    
    
    
};



#endif /* defined(__SDLNetBoxGame__bullet__) */
