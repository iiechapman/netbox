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
    if ( mTotalClients > 0 )
    {
        for ( int i = 0 ; i < MAX_CLIENTS ; i++ )
        {
            ///TODO: read from array of objects and convert state to string, then bundle string in message to send to all clients
        }
        
        //Send state message to all clients
        for (int clientNum = 0 ; clientNum < mTotalClients ; clientNum++ )
        {
            if ( !mClient[ clientNum ].isFree )
            {
                SendMessage( mClient[ clientNum ].socket, mMsgToSend );
            }
        }
    }
    mMsgToSend = ""; //Clear state message if not needed
}



/*
 ====================
 Server::CheckForClientData
 ====================
 */
void Server::CheckForClientData()
{
    int numActiveSockets = SDLNet_CheckSockets( mListenSet, MESSAGE_RECEIVE_TIMEOUT );
    
    if ( numActiveSockets > 0 )
    {
        for ( int i = 0 ; i < MAX_CLIENTS ; i++ )
        {
            if ( !mClient[ i ].isFree )
            {
                int clientActivity = SDLNet_SocketReady( mClient[ i ].socket);
                if ( clientActivity )
                {
                    int dataReceived = SDLNet_TCP_Recv( mClient[ i ].socket, &mMsgReceived, SEND_MAX);
                    
                    if ( dataReceived <= 0 )//Client disconnected
                    {
                        cout << "Client Disconnected..." << endl;
                        SDLNet_TCP_DelSocket( mListenSet , mClient[ i ].socket );
                        SDLNet_TCP_Close( mClient[ i ].socket );
                        mClient[ i ].socket = NULL;
                        mClient[ i ].isFree = true;
                        mTotalClients--;
                        cout << "Total clients " << mTotalClients << "..." << endl;
                    } else
                    {
                        ///TODO: Add message to action queue
                    }
                }
            }
        }
    }
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
                cout << "Assigning client new spot on server..." << endl;
                mClient[freespot].socket = SDLNet_TCP_Accept( mListenSocket );
                SDLNet_TCP_AddSocket(mListenSet , mClient[ freespot ].socket );
                mTotalClients++;
                
                mMsgToSend = "accept." + std::to_string( mTotalClients );
                SendMessage( mClient[ freespot ].socket ,  mMsgToSend );
                cout << "Accepted new client to spot " << mTotalClients << endl;
            } else //No Free spots, reject client
            {
                cout << "Rejected client due to no free spots..." << endl;
                mMsgToSend = "reject.0";
                
                //Connect to client
                TCPsocket tempClient = SDLNet_TCP_Accept( mListenSocket );
                SendMessage( tempClient , mMsgToSend );
                SDLNet_TCP_Close(tempClient);
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
    //Send message, if message isnt sent correctly, return false
    if ( SDLNet_TCP_Send(
                         receiver ,
                         msg.c_str() ,
                         static_cast<unsigned int>( msg.length())) < static_cast<unsigned int>( msg.length()))
    {
        return false;
    }
    
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





















