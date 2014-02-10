//
//  bullet.h
//  SDLNetBoxGame
//
//  Created by Evan Chapman on 1/23/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#ifndef __SDLNetBoxGame__bullet__
#define __SDLNetBoxGame__bullet__

#include "CommonIncludes.h"


#include "Box.h"
using namespace std;


class bullet : public Box {
public:
                        bullet();
                        ~bullet();
    
    bool                isFriendly;
    
private:
    
    
    
};



#endif /* defined(__SDLNetBoxGame__bullet__) */
