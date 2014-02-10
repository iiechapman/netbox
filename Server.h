//
//  Server.h
//  SDLNetBoxGame
//
//  Created by Evan Chapman on 1/30/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#ifndef __SDLNetBoxGame__Server__
#define __SDLNetBoxGame__Server__

#include "CommonIncludes.h"
#include <SDL2_net/SDL_net.h>

using namespace std;

/*
 ===============================================================================
 
 Server
 Handles clients joining and leaving, receiving and sending messages to clients,
 as well as broadcast client messages to all clients
 
 ===============================================================================
 */


struct ClientSocket{
    TCPsocket socket;
    bool      isFree;
};


class Server{
    
public:
                            Server();
                            ~Server();
    
    void                    Init( void );
    void                    Update( void );
    
    const bool              IsOnline( void ) {return mIsOnline;}
    const int               NumberOfClients( void ){return mTotalClients;}
    
private:
    void                    ClearClientSockets( void );

    void                    CheckForNewClient( void );
    void                    CheckForClientData( void );
    void                    BroadcastToClients( void );
    
    const bool              SendMessage( TCPsocket receiver , string msg);
    
    static const int        MAX_CLIENTS = 20;
    static const int        NEW_CLIENT_TIMEOUT = 20;
    static const int        MESSAGE_RECEIVE_TIMEOUT = 10;
    static const int        SEND_MAX = 100000;
    
    bool                    mIsOnline;
    
    IPaddress               mIPAddress;
    TCPsocket               mListenSocket;
    ClientSocket            mClient[MAX_CLIENTS];
    
    
    SDLNet_SocketSet        mClientSockets;
    SDLNet_SocketSet        mListenSet;
    
    string                  mMsgReceived;
    string                  mMsgToSend;
    
    int                     mTotalClients;
    
};






#endif /* defined(__SDLNetBoxGame__Server__) */
