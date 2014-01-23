// ============================================================================
// [Include Section]
// ============================================================================
#include <SDL2/SDL.h>
#include <SDL2_net/SDL_net.h>
#include <iostream>
#include <iomanip>
#include <string>
#include "Box.h"

using namespace std;

#define MAX_SOCKETS 20
#define MAX_CLIENTS 20
#define MAX_MESSAGES 20
#define SEND_MAX 100000

// ============================================================================
// [SdlApplication]
// ============================================================================
#define APPTITLE "SDL Net Box Game"

struct SdlApplication
{
	SdlApplication();
	~SdlApplication();
	
	enum APP_STATE
	{
		APP_OK = 0,
		APP_FAILED = 1
	};
	
	int init(int width, int height);

	void destroy();
	int run(int width, int height);

	void onEvent(SDL_Event* ev);
    void HandleKeys( void );
	void Render();
    
    void NetworkUpdate( void );
    void ListenAsServer( void );
    void CheckForNewClient( void );
    void CheckForClientData( void );
    void BroadcastToClients( void );
    
    void ClearMessageList( void );
    void ListenToServer( void );
    void SendToServer( void );
    
    void ConvertMessageToAction( string msg );

    SDL_Window *win;
	SDL_Renderer *renderer;///<Rendering frame
    
	bool _running;///<Used to determine if app is looping

    
    IPaddress ip;
    
    int numMessages = 0;
    
    bool isServer = false;///<Flag if running server
    bool isClient = false;///<Flag if connecting as client
    
    SDLNet_SocketSet serverSocketSet;///<Used for listening as server
    SDLNet_SocketSet clientSocketSet;///<Set of sockets from clients
    
    TCPsocket server;///<Server socket
    TCPsocket client[MAX_CLIENTS];///<Array of client sockets
    bool      clientFree[MAX_CLIENTS];///<Array of flags to tell if client socket free
    
    string userName;///<Used for players name in game
    
    string rcvMsg;///<Used to capture received data
    string sndMsg[MAX_MESSAGES];///<Used to send data

    
    int clientNumber;
    int totalClients = 0;///<Number of clients on server
    int currentBox = 0;

    bool isConnected;///<Flag used to determine if connected to server
    bool isConnecting;///<Flag used to determine if connecting to server

    string userPrompt;///<Captures user selection when prompted
    
    Box* box[MAX_CLIENTS];///<Array of boxes on screen

    Box* playerBox = 0;///<Pointer to players box
    
    Box* testBox = 0;///<Box class used for testing
    
};

SdlApplication::SdlApplication() :
_running(false)
{
}

SdlApplication::~SdlApplication()
{
	destroy();
}

int SdlApplication::init(int width, int height)
{
	// Initialize the SDL library.
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0)
	{
		fprintf(stderr, "SDL_Init() failed: %s\n", SDL_GetError());
		return APP_FAILED;
	}
	
	win = SDL_CreateWindow(APPTITLE, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    cout << setw(42) << "---------------------------" << endl;
    cout << setw(35) <<  "SDL NetBox Game" << endl;
    cout << setw(40) << "By Evan Chapman 2014" << endl;
    cout << setw(42) << "---------------------------" << endl;
    cout << endl << endl << endl;
    
    cout << "Connecting to internet ..." << endl;
    if ( SDLNet_Init() !=-1){
        cout << "Connected to internet." << endl;
    } else {
        cout << "Could not connect to internet.";
    }
    
    cout << "Please enter your user name: " << endl;
    cin.clear();
    getline(cin,userName);
    
    //Connect to server, or setup as server
    cout << "Press enter to connect to server, or type 1 to host server..." << endl;
    
    cin.clear();
    getline(cin, userPrompt);
    
    if (userPrompt == "1"){
        cout << "Chose to host server..." << endl;
        isServer = true;
        isClient = false;
    } else {
        isServer = false;
        isClient = true;
        
        cout << "Enter IP address of server (press enter for localhost)" << endl;
        userPrompt = "";
        cin.clear();
        getline(cin,userPrompt);
        
        if (userPrompt == ""){
            userPrompt = "127.0.0.1";
        }
        
    }
    
    if (isClient)
    {
        cout << "Attempting to connect to " << userPrompt << endl;
        if (SDLNet_ResolveHost(&ip, userPrompt.c_str(), 1234) == 0){
            cout << "Connected to ip" << endl;
        } else {
            cout << "Could not connect to remote ip" << endl;
        }
    } else {
        if ( SDLNet_ResolveHost(&ip, NULL, 1234) == 0 ){
            cout << "Setup server..." << endl;
        } else {
            cout << "Could not setup server..." << endl;
        }
    }
    
    //Connect to/Listen on server port
    /*
     For server, this simply opens listening port
     For Clients, this is the first socket set activity to server
     */
    server = SDLNet_TCP_Open(&ip);
    
    //Clear out entire client socket set
    for (int i = 0 ; i < MAX_CLIENTS ; i++){
        client[i] = NULL;
        box[i] = NULL;
        clientFree[i] = true;
    }

    //Server init
    if (isServer){
        SDL_SetWindowTitle(win, "SDL NetBox - Server");
        //Allocate open sockets for clients to connect to
        serverSocketSet = SDLNet_AllocSocketSet(1);
        clientSocketSet = SDLNet_AllocSocketSet(MAX_SOCKETS);
        
        
        //add server listening socket
        if ( server ){
            SDLNet_TCP_AddSocket(serverSocketSet, server);
            isConnected = true;
    
        } else{
            cout << "Server not listening..." << endl;
            _running = false;
            isConnected = false;
        }
    }
    
    
    //client init
    if (isClient){
        SDL_SetWindowTitle(win, "SDL NetBox - Client");
        serverSocketSet = SDLNet_AllocSocketSet(1);
        
        if ( server ){
            int activeSockets;
            //add server output socket
            SDLNet_TCP_AddSocket(serverSocketSet, server);
            
            bool attemptConnect = true;
            do
            {
                cout << "Checking for active server (1 second timeout)..." << endl;
                activeSockets = SDLNet_CheckSockets(serverSocketSet, 1000);
                
                if ( activeSockets !=0 )
                {
                    attemptConnect = false;
                    cout << "Found active server" << endl;
                    cout << "Connecting to server..." << endl;
                    
                    int serverResponse = SDLNet_SocketReady(server);
                    
                    if (serverResponse != 0)
                    {
                        cout << "Connected to server!" << endl;
                        sndMsg[0] = "join.0;";
                    }
                }
            } while (attemptConnect);
        }
        cout << "Welcome, " << userName << " to NetBox..." << endl;
        
    }
    

    Box::renderer = renderer;
    
    if (isClient){
        playerBox = new Box();
    }
    
	// Success.
	return APP_OK;
}

void SdlApplication::destroy()
{
	if (win)
	{
		SDL_DestroyWindow(win);
		SDL_DestroyRenderer(renderer);
		SDL_Quit();
	}
}

int SdlApplication::run(int width, int height)
{
	// Initialize application.
	int state = init(width, height);
	if (state != APP_OK) return state;
	
	// Enter to the SDL event loop.
	SDL_Event ev;
	_running = true;
	
	while (_running)
	{
    
        if ( server ){
            isConnected = true;
        } else {
            isConnected = false;
        }

        SDL_PollEvent(&ev);
		onEvent(&ev);
        
        HandleKeys();
        
        NetworkUpdate();
        
		Render();
		
		if (_running == false)
		{
            cout << "Disconnecting..." << endl;
            
            for (int i = 0 ; i < totalClients ; i++){
                SDLNet_TCP_Close(client[i]);
            }
            
            SDLNet_FreeSocketSet(clientSocketSet);
            SDLNet_FreeSocketSet(serverSocketSet);
            SDLNet_TCP_Close(server);
            SDLNet_Quit();
            
			break;
		}
	}
	
	return APP_OK;
}

void SdlApplication::onEvent(SDL_Event* ev)
{
	switch (ev->type)
	{
		case SDL_QUIT:
			_running = false;
			break;
			
		case SDL_KEYDOWN:
		{
			if (ev->key.keysym.sym == SDLK_ESCAPE)
			{
				_running = false;
			}
		}
	}
}

void SdlApplication::Render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    if (isClient){
        playerBox->Render();
    }
    
    for (int i = 0 ; i < MAX_CLIENTS ; i++)
    {
        if ( box[i] != NULL)
        {
            cout << "Rendering box" << endl;
            box[i]->Render();
        }
    }

    
    SDL_RenderPresent(renderer);
    
}

void SdlApplication::HandleKeys( void  )
{
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    
    if ( state[ SDL_SCANCODE_W ] ) {
        if (isClient)
        {
        playerBox->rect.y-=5;
        
        sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->rect.y) + ";";
        }
    }
    
    if ( state[ SDL_SCANCODE_S ] ) {
        if (isClient)
        {
        playerBox->rect.y+=5;
        sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->rect.y) + ";";
        }
    }
    
    if ( state[ SDL_SCANCODE_A ] ) {
        if (isClient)
        {
            playerBox->rect.x-=5;
        sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->rect.x) + ";";
        }
    }
    
    if ( state[ SDL_SCANCODE_D ] ) {
        if (isClient)
        {
        playerBox->rect.x+=5;

        sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->rect.x) + ";";
        }
    }
}


//Entry
int main(int argc, char* argv[])
{
	SdlApplication app;
	return app.run(640, 480);
}

void SdlApplication::NetworkUpdate( void ){
    if ( isServer )
    {
        ListenAsServer();
    }
    
    if ( isClient )
    {
        ListenToServer();
        SendToServer();
    }
}

void SdlApplication::ListenAsServer( void )
{
    CheckForNewClient();
    CheckForClientData();
    BroadcastToClients();
}

void SdlApplication::CheckForNewClient()
{
    //Check if new client connected
    //cout << "Total Clients: " << totalClients << endl;
    //cout << "Checking if new client joined..." << endl;
    int activeSockets = SDLNet_CheckSockets(serverSocketSet, 10);
    //if new client joined
    if (activeSockets > 0)
    {
        int socketActivity = SDLNet_SocketReady(server);
        cout << "Found active socket..." << endl;
        //If client requests to join
        if (socketActivity !=0)
        {
            cout << "Looking for open socket..." << endl;
            // if we have room to add another client
            if (totalClients < MAX_CLIENTS)
            {
                int freeSpot = -1;
                for (int i = 0 ; i < MAX_CLIENTS ; i++)
                {
                    if ( clientFree[i])
                    {
                        freeSpot = i ;
                        clientFree[i] = false;
                        break;
                    }
                }
                
                //Save client to free spot
                cout << "Assigning client spot " << freeSpot << endl;
                client[freeSpot] = SDLNet_TCP_Accept(server);
                //add new client socket to set
                SDLNet_TCP_AddSocket(clientSocketSet, client[freeSpot]);
                totalClients++;
                
                rcvMsg = "accept.0;";
    
                
                SDLNet_TCP_Send(client[freeSpot], rcvMsg.c_str(), static_cast<unsigned int>( rcvMsg.length() ));
                
                cout << "Sent message: " << rcvMsg << endl;
                cout << "Accepted new client..." << endl;
                cout << "Total Clients: " << totalClients << endl;
            
            }else {
                cout << "Rejected client, no free spots..." << endl;
                
                rcvMsg  = "NO";
                
    
                TCPsocket tempClient = SDLNet_TCP_Accept(server);
                
                 SDLNet_TCP_Send(tempClient, rcvMsg.c_str(), static_cast<unsigned int>( rcvMsg.length()));
                
                SDLNet_TCP_Close(tempClient);
            }
        }
    }
}


void SdlApplication::CheckForClientData()
{
    //Check all clients for activity
    //cout << "Checking for client activity..." << endl;
    
    int activeSockets =  SDLNet_CheckSockets(clientSocketSet, 10);

    if ( activeSockets > 0 ) {
        for (int i = 0 ; i < totalClients ; i++ )
        {
            int clientActivity = SDLNet_SocketReady(client[i]);
            
            if (clientActivity !=0)
            {
                char msg[SEND_MAX];
                
                currentBox = i+1;
                int receivedData = SDLNet_TCP_Recv(client[i], &msg, SEND_MAX);
                
                //first check if client  disconnected
                if (receivedData <= 0)
                {
                    cout << "Client disconnected ... " << endl;
                    SDLNet_TCP_DelSocket(clientSocketSet, client[i]);
                    SDLNet_TCP_Close(client[i]);
                    
                    delete box[i+1];
                    box[i+1] = NULL;
                    client[i] = NULL;
                    clientFree[i] = true;
                    sndMsg[0] = sndMsg[0] + "box." + std::to_string(i) + ";leave.0;";
                    totalClients--;
                    cout << "Total Clients: " << totalClients << endl;
                } else{
                    cout << "Received message from client: " << currentBox << endl;
                    cout << msg << endl;
                    
                    ConvertMessageToAction( msg );
                }
            }
        }
    }
}



void SdlApplication::BroadcastToClients()
{
    
    if (totalClients > 0)
    {
    for (int i  = 0 ; i <= totalClients ; i++)
    {
        if (box[i] != NULL){
        sndMsg[0] = sndMsg[0] + "box." + std::to_string(i) + ";x." + std::to_string(box[i]->rect.x) + ";y." + std::to_string(box[i]->rect.y)+ ";";
        }
    }
    
        //Broadcast message back to all clients
        for (int clientNum = 0 ; clientNum < totalClients ; clientNum++)
        {
            if (clientFree[clientNum] == false){
            int length = static_cast<unsigned int>(strlen( sndMsg[0].c_str() ) + 1 );
                SDLNet_TCP_Send(client[clientNum], sndMsg[0].c_str(),length);
            }
        }
    }
    sndMsg[0] = "";
}



void SdlApplication::ListenToServer( void )
{
    int serverActive = SDLNet_CheckSockets(serverSocketSet, 10);
    
    if (serverActive != 0)
    {
        int msgReceived = SDLNet_SocketReady(server);
        
        if (msgReceived !=0 )
        {
            char msg[SEND_MAX];
            
            int dataReceived = SDLNet_TCP_Recv(server, &msg, SEND_MAX);
            
            if (dataReceived <= 0)
            {
                cout << "Server disconnected..." << endl;
                _running = false;

            } else {
                cout << "Received message: " << endl;
                cout << msg << endl;
                
                ConvertMessageToAction( msg );
            }
        }
    }
}

void SdlApplication::SendToServer( void )
{
    if (sndMsg[0] != "")
    {
    sndMsg[0] =  sndMsg[0] + "end;";

    cout << "send mess: " << sndMsg[0].c_str() << endl;
    
    int length = static_cast<unsigned int>(strlen( sndMsg[0].c_str() ) + 1 );
    SDLNet_TCP_Send(server, sndMsg[0].c_str(),length);
    
    }
    sndMsg[0] = "";
}



/*
 msg format:
 action.data;end;
 
 or for multiple pieces of data:
 action.data;action.data;end;
 
 ie:
 x.10;
 name.Evan;
 
 x.10;y.20;name.evan;end;
 
 */

void SdlApplication::ConvertMessageToAction( string msg )
{
    string currentAction = "";
    string currentValue = "";
    
    int intValue = 0;
    
    bool searchingForAction = true;
    bool searchingForValue = false;
    bool foundValue = false;
    bool isSearching = true;
    
    char currentChar = '\0';
    int index = 0;
    
    cout << "Turning message into action..." << endl;
    while ( isSearching ) {
        currentChar = msg[index];
        
        if ( searchingForAction )
        {
            if (currentChar == '.')
            {
                cout << "Found action: " << currentAction << endl;
                searchingForAction = false;
                searchingForValue = true;
            } else if (currentChar != '.' && currentChar != ';'){
                currentAction = currentAction + currentChar;
            }
        }
        
        if ( searchingForValue )
        {
            cout << "Searching for value..." << endl;
            if (currentChar == ';')
            {
                foundValue = true;
                searchingForAction = true;
                searchingForValue = false;
            } else if (currentChar != '.' && currentChar != ';'){
                currentValue = currentValue + currentChar;   
            }
        }
        
        

        //Turn message into actions
        if (foundValue)
        {
        if (currentAction == "end")
        {
            cout << "Finished decoding ..." << endl;
            isSearching = false;
            searchingForValue = false;
            searchingForAction = false;
            currentAction = "";
            currentValue = "";
        }
        
        if (currentAction == "name")
        {
        }
        
        if (currentAction == "x")
        {
            cout << "Changed x" << endl;
            intValue = atoi(currentValue.c_str());
            box[currentBox]->rect.x = intValue;
            
            currentAction = "";
            currentValue = "";
        }
        
        if (currentAction == "y")
        {
            cout << "Changed y" << endl;
            intValue = atoi(currentValue.c_str());
            box[currentBox]->rect.y = intValue;
            currentAction = "";
            currentValue = "";
        }
        
        if (currentAction == "w")
        {
        }
        
        if (currentAction == "h")
        {
        }
        
        if (currentAction == "red")
        {
        }
        
        if (currentAction == "green")
        {
        }
        
        if (currentAction == "blue")
        {
        }
        
        if (currentAction == "join")
        {
            cout << "Added box " << totalClients << endl;
            box[totalClients] = new Box();
            currentAction = "";
            currentValue = "";
        }
        
        if (currentAction == "box")
        {
            intValue = atoi(currentValue.c_str());
            currentBox = intValue;
            
            if ( box[currentBox] == NULL)
            {
                cout << "Created new box: " << intValue << endl;
                box[currentBox] = new Box();
            }
            
            currentAction = "";
            currentValue = "";
        }
            
            
        if (currentAction == "clients")
        {
            intValue = atoi(currentValue.c_str());
            totalClients = intValue;
            currentAction = "";
            currentValue = "";
        }
            
        if (currentAction == "clientnum")
        {
            intValue = atoi(currentValue.c_str());
            clientNumber = intValue;
            currentAction = "";
            currentValue = "";
        }
            
        if (currentAction == "accept")
        {
            isSearching = false;
            searchingForValue = false;
            searchingForAction = false;
            currentAction = "";
            currentValue = "";
        }
        
            
            
        if (currentAction == "add")
        {
            cout << "Added box " << totalClients << endl;
            box[totalClients] = new Box();
            currentAction = "";
            currentValue = "";
        }
            
        
        if (currentAction == "leave")
        {
            cout << "Box " << currentBox << " left..." << endl;
            delete box[currentBox];
            box[currentBox] = 0;
            clientFree[currentBox] = true;
            currentAction = "";
            currentValue = "";
        }
            
            foundValue = false;
        }
        
        index++;
        if (currentChar == '\0'){
            isSearching = false;
        }
        if (index >= SEND_MAX){
            isSearching = false;
        }
    }//end while
    
}
































