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
    rect.w = 100;
    rect.h = 100;
    
    color.r = 255;
    color.g = 50;
    color.b = 50;
    speed = 0;
}

bullet::~bullet()
{
    
}

void bullet::Update( Uint32 delta )
{
    switch (direction) {
        case dirUp:
            rect.y -= speed * delta;
            break;
            
        case dirDown:
            rect.y += speed * delta;
            break;
            
        case dirLeft:
            rect.x -= speed * delta;
            break;
            
        case dirRight:
            rect.x += speed * delta;
            break;
            
        default:
            break;
    }
}
