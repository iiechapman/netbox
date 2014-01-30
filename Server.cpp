//
//  Server.cpp
//  SDLNetBoxGame
//
//  Created by Evan Chapman on 1/30/14.
//  Copyright (c) 2014 Evan Chapman. All rights reserved.
//

#include "Server.h"


/*
 ====================
 Server::Server
 ====================
 */

Server::Server()
{
    cout << "Created server object..." << endl;
}

/*
 ====================
 Server::~Server
 ====================
 */

Server::~Server()
{
    SDLNet_FreeSocketSet( mClientSockets );
    SDLNet_FreeSocketSet( mListenSet );
    SDLNet_TCP_Close( mListenSocket );
    cout << "Deleted server object..." << endl;
}

/*
 ====================
 Server::BroadcastToClients
 ====================
 */
void Server::BroadcastToClients()
{
    
}

/*
 ====================
 Server::CheckForClientData
 ====================
 */
void Server::CheckForClientData()
{
    
}


/*
 ====================
 Server::CheckForNewClient
 ====================
 */
void Server::CheckForNewClient()///<Checks if new client connected
{
    //Check listen set for activity
    int numActiveSockets = SDLNet_CheckSockets(mListenSet, NEW_CLIENT_TIMEOUT);
    
    //If Activity on listen set, check if ready to receive
    if ( numActiveSockets > 0 )
    {
        //Listen on socket for client
        int socketReady = SDLNet_SocketReady( mListenSocket );
        
        //If client is requesting to join, add to client socket list
        if ( socketReady !=0 )
        {
            //If there is a free spot, place client in that spot
            if ( mTotalClients < MAX_CLIENTS )
            {
                int freespot = -1;
                
                for (int i = 0 ; i < MAX_CLIENTS ; i++ )
                {
                    if ( mClient[i].isFree )
                    {
                        mClient[i].isFree = false;
                        freespot = i;
                        break;
                    }
                }
            } else //No Free spots, reject client
            {
                cout << "Rejected client due to no free spots..." << endl;
                mMsgToSend = "reject";
                
                //Connect to client
                TCPsocket tempClient = SDLNet_TCP_Accept( mListenSocket );
                SendMessage( tempClient , mMsgToSend );
            }
        }
    }
}


/*
 ====================
 Server::SendMessage
 ====================
 */
const bool Server::SendMessage( TCPsocket receiver , string msg)
{
    
    SDLNet_TCP_Send( receiver ,
                    msg.c_str() ,
                    static_cast<unsigned int>( msg.length()));
    
    
    
    return true;
}


/*
 ====================
 Server::Update
 ====================
 */
void Server::Update( void )
{
    CheckForNewClient();
    CheckForClientData();
    BroadcastToClients();
}


/*
 ====================
 Server::ClearClientSockets
 ====================
 */
void Server::ClearClientSockets()///<Clears out client sockets for beginning use
{
    for ( int i = 0 ; i < MAX_CLIENTS ; i++)
    {
        mClient[i].isFree = true;
        mClient[i].socket = NULL;
    }
}


/*
 ====================
 Server::Init
 ====================
 */
void Server::Init()
{
    //Clear out socket array
    ClearClientSockets();
    
    mListenSocket = SDLNet_TCP_Open( &mIPAddress );
    
    mListenSet       = SDLNet_AllocSocketSet( 1 );
    mClientSockets      = SDLNet_AllocSocketSet( MAX_CLIENTS );
    
    if ( mListenSocket != NULL )
    {
        SDLNet_TCP_AddSocket(mListenSet, mListenSocket);
        mIsOnline = true;
    } else {
        cout << "Server could not establish connection..." << endl;
        mIsOnline = false;
    }
}





















