// ============================================================================
// [Include Section]
// ============================================================================
#include <SDL2/SDL.h>
#include <SDL2_net/SDL_net.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include "Box.h"
#include "bullet.h"



using namespace std;

#define MAX_SOCKETS 20
#define MAX_CLIENTS 20
#define MAX_MESSAGES 20
#define SEND_MAX 100000
#define MAX_STARS 5000
#define MAX_BULLETS 100000
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define HORIZONTAL_BOUNDS 20000
#define VERTICAL_BOUNDS 20000


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
    void Update( Uint32 delta );
	void Render();
    
    void ShootBullet( int x , int y , dir direction , bool friendly , SDL_Color color);
    void CheckBulletCollisions( void );
    void CheckBulletBounds( void );
    
    void CalculateFrameDistance( void );
    
    //Network Functions
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
    
    int GUID = 0;
    
    string userName;///<Used for players name in game
    
    string currentName ="";
    
    float farLeft = 0;
    float farRight = 0;
    float farBottom = 0;
    float farTop = 0;
    float farWidth, farHeight;
    
    double scale,finalScale;
    
    string rcvMsg;///<Used to capture received data
    string sndMsg[MAX_MESSAGES];///<Used to send data

    bool deleteBox = false;
    int deadBox = 0;
    
    bool shootPressed = false;
    
    int clientNumber;
    int totalClients = 0;///<Number of clients on server
    int currentBox = 0;

    int TOTAL_BULLETS = 1000;
    
    int totalBoxes = 0;
    
    float distance;
    
    float finalXOff,finalYOff;
    float xOff,yOff;
    bool isConnected;///<Flag used to determine if connected to server
    bool isConnecting;///<Flag used to determine if connecting to server

    string userPrompt;///<Captures user selection when prompted
    
    int speedNormal = 60;
    int speedFast = 100;
    bool isBoosting = false;
    int boxSpeed = speedNormal;
    
    Box* box[MAX_CLIENTS];///<Array of boxes on screen
    Box* playerBox = 0;///<Pointer to players box
    
    Box* nullBox;
    
    Box* leftBox = nullBox;
    Box* rightBox = nullBox;
    Box* topBox = nullBox;
    Box* bottomBox = nullBox;
    
    bullet* bullets[MAX_BULLETS];
    
    bullet* stars[MAX_STARS];
    
    int availableBullet =0;
    dir playerDir = dirUp;
    bool changeDir = true;
    
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
	
    srand((int)time(NULL));
    
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
    
    nullBox = new Box();
    nullBox->xOff = 0;
    
    Box::renderer = renderer;
    bullet::Box::renderer = renderer;
    
    //Clear out bullets
    for (int i = 0 ; i < MAX_BULLETS ; i++ )
    {
        bullets[i] = NULL;
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
        char buffer[1000];
        sprintf(buffer, "SDL NetBox - Client - %s" , userName.c_str());
        SDL_SetWindowTitle(win, buffer);
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
        playerBox->rect.x = rand() % 10000;
        playerBox->rect.y = rand() % 10000;
        playerBox->name = userName;
        
        int color = rand() % 4;
        
        cout << "Color seed: " << color << endl;
        switch (color) {
            case 0:
                playerBox->color.r = 200 + rand() % 50 ;
                playerBox->color.g = 0 + rand() % 50 ;
                playerBox->color.b = 200 + rand() % 50 ;
                break;
                
            case 1:
                playerBox->color.r = 200 + rand() % 50 ;
                playerBox->color.g = 200 + rand() % 50 ;
                playerBox->color.b = 0 + rand() % 50 ;
                break;
                
            case 2:
                playerBox->color.r = 0 + rand() % 50 ;
                playerBox->color.g = 200 + rand() % 50 ;
                playerBox->color.b = 200 + rand() % 50 ;
                break;
                
            case 3:
                playerBox->color.r = 0 + rand() % 50 ;
                playerBox->color.g = 200 + rand() % 50 ;
                playerBox->color.b = 0 + rand() % 50 ;
                break;
                
            default:
                playerBox->color.r = 200 + rand() % 50 ;
                playerBox->color.g = 200 + rand() % 50 ;
                playerBox->color.b = 200 + rand() % 50 ;
                break;
        }
        
        for (int i = 0 ; i < MAX_STARS ; i++ )
        {
            stars[i] = new bullet();
            stars[i]->rect.x = -HORIZONTAL_BOUNDS  + rand()% HORIZONTAL_BOUNDS*2;
            stars[i]->rect.y = -VERTICAL_BOUNDS + rand()% VERTICAL_BOUNDS*2;
            
            stars[i]->rect.w = 25;
            stars[i]->rect.h = 25;
            
            stars[i]->color.r = 200;
            stars[i]->color.g = 200;
            stars[i]->color.b = 200;
            
            stars[i]->speed = 1 + rand() % 20;
            
            int dir = rand() % 4;
            
            switch (dir) {
                case 0:
                    stars[i]->direction = dirUp;
                    break;
                    
                case 1:
                    stars[i]->direction = dirDown;
                    break;
                    
                    
                case 2:
                    stars[i]->direction = dirLeft;
                    break;
                    
                case 3:
                    stars[i]->direction = dirRight;
                    break;
                    
                    
                default:
                    break;
            }
            
        }
        
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

        while (SDL_PollEvent(&ev))
        {
            onEvent(&ev);
        }
        
        HandleKeys();
        Update( 1 );
        CalculateFrameDistance();
        
        if (isClient)
        {
        CheckBulletCollisions();
        CheckBulletBounds();
        }
        NetworkUpdate();
        

        
		Render();
		
		if (_running == false)
		{
            cout << "Disconnecting..." << endl;
            
            for (int i = 0 ; i < totalClients ; i++){
                SDLNet_TCP_Close(client[i]);
            }
            
            if (isServer){
                SDLNet_FreeSocketSet(clientSocketSet);
            }
            
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
            
        case SDL_MOUSEBUTTONDOWN:
        {
        
        }
            
	}
}

void SdlApplication::Render()
{
    ///TODO: Set Scale To fit all players
    //finalScale = 1;
    
    SDL_RenderSetScale(renderer,finalScale,finalScale);

    
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    
    if (isClient)
    {
    //Render all stars
    for ( int i = 0 ; i < MAX_STARS ; i++ )
    {
        if (stars[i] != NULL )
        {
            stars[i]->Render();
        }
    }
    }
    
    //Render all bullets
    for ( int i = 0 ; i < TOTAL_BULLETS ; i++ )
    {
        if (bullets[i] != NULL )
        {
            bullets[i]->Render();
        }
    }
    
    
    
    
    //Render all clients
    for (int i = 0 ; i < MAX_CLIENTS ; i++)
    {
        if ( box[i] != NULL)
        {
            //cout << "Rendering box" << endl;
            box[i]->Render();
        }
    }

    
    if (isClient){
        playerBox->Render();
    }
    
    SDL_RenderPresent(renderer);
    
}

void SdlApplication::HandleKeys( void  )
{
    if (isClient)
    {
    const Uint8 *state = SDL_GetKeyboardState(NULL);
    
    if (SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_LEFT || SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_LEFT && SDL_BUTTON_RIGHT)
    {
        if (isClient){
                shootPressed = true;
                ShootBullet(
                            playerBox->rect.x + playerBox->rect.w/2,
                            playerBox->rect.y + playerBox->rect.h/2 , playerBox->direction,
                            true,playerBox->color);
                playerBox->isShooting = true;
                sndMsg[0] =  sndMsg[0] + "shoot.yes" + ";";
            
            } else {
                playerBox->isShooting = false;
                sndMsg[0] =  sndMsg[0] + "shoot.no" + ";";
            }
    } else {
        if (isClient)
        {
            shootPressed = false;
            playerBox->isShooting = false;
            sndMsg[0] =  sndMsg[0] + "shoot.no" + ";";
        }
    }

    if ( state[ SDL_SCANCODE_W ] ) {
        if (isClient)
        {
            playerBox->rect.y-=boxSpeed;
            if (changeDir){
                playerBox->direction = dirUp;
                sndMsg[0] =  sndMsg[0] + "direction." + std::to_string(playerBox->direction) + ";";
            }
            sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->rect.y) + ";";
            
        }
    }
    
    
        if ( state[ SDL_SCANCODE_F ] ) {
            SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);
        }
    
    if ( state[ SDL_SCANCODE_S ] ) {
        if (isClient)
        {
        playerBox->rect.y+=boxSpeed;
            if (changeDir){
            playerBox->direction = dirDown;
                sndMsg[0] =  sndMsg[0] + "direction." + std::to_string(playerBox->direction) + ";";
            }
        sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->rect.y) + ";";
        }
    }
    
    if ( state[ SDL_SCANCODE_A ] ) {
        if (isClient)
        {
            if (changeDir){
            playerBox->direction = dirLeft;
                sndMsg[0] =  sndMsg[0] + "direction." + std::to_string(playerBox->direction) + ";";
            }
            playerBox->rect.x-=boxSpeed;
            sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->rect.x) + ";";
        }
    }
    
    if ( state[ SDL_SCANCODE_D ] ) {
        if (isClient)
        {
        playerBox->rect.x+=boxSpeed;
            if (changeDir){
                playerBox->direction = dirRight;
                sndMsg[0] =  sndMsg[0] + "direction." + std::to_string(playerBox->direction) + ";";
            }
        sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->rect.x) + ";";
        }
    }
    
    if ( state[ SDL_SCANCODE_LSHIFT ] ) {
        isBoosting = true;
    } else {
        isBoosting = false;
    }

    if (isBoosting) {
        boxSpeed = speedFast;
    } else {
        boxSpeed = speedNormal;
    }

        if (SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_RIGHT)
        {
            changeDir = false;
        } else {
            changeDir = true;
        }
    
//    if ( state[ SDL_SCANCODE_SPACE ] ) {
//        if (isClient){
//            if (!shootPressed)
//            {
//            shootPressed = true;
//            ShootBullet(
//                        playerBox->rect.x + playerBox->rect.w/2,
//                        playerBox->rect.y + playerBox->rect.h/2 , playerBox->direction,
//                        true);
//            playerBox->isShooting = true;
//            sndMsg[0] =  sndMsg[0] + "shoot.yes" + ";";
//            } else {
//            playerBox->isShooting = false;
//            sndMsg[0] =  sndMsg[0] + "shoot.no" + ";";
//            }
//        }
//    } else {
//        if (isClient)
//        {
//            shootPressed = false;
//            playerBox->isShooting = false;
//            sndMsg[0] =  sndMsg[0] + "shoot.no" + ";";
//        }
//        
//    }
}
}


//Entry
int main(int argc, char* argv[])
{
	SdlApplication app;
	return app.run(SCREEN_WIDTH, SCREEN_HEIGHT);
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
                
                char finalMsg[1000];
                sprintf(finalMsg, "accept.%i" , totalClients);
                rcvMsg = finalMsg;
    
                
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
                    deleteBox = true;
                    totalClients--;
                    cout << "Total Clients: " << totalClients << endl;
                } else{
                    //cout << "Received message from client: " << currentBox << endl;
                    //cout << msg << endl;
                    
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
    for (int i  = 0 ; i < MAX_CLIENTS ; i++)
    {
        if (box[i] != NULL){
        sndMsg[0] = sndMsg[0] + "box." + std::to_string(i)  + ";alive.0" +
            ";name." + box[i]->name +
            ";x." + std::to_string(box[i]->rect.x) +
            ";y." + std::to_string(box[i]->rect.y) +
            ";red." + std::to_string(box[i]->color.r) +
            ";green." + std::to_string(box[i]->color.g) +
            ";blue." + std::to_string(box[i]->color.b) +
            ";direction." + std::to_string(box[i]->direction) +
            ";";
            
            if (box[i]->isShooting)
            {
                sndMsg[0] = sndMsg[0] + "shoot.yes;";
            } else {
                sndMsg[0] = sndMsg[0] + "shoot.no;";
            }
            
        } else {
            sndMsg[0] = sndMsg[0] + "box." + std::to_string(i) + ";dead.0;";
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
                //cout << "Received message: " << endl;
                //cout << msg << endl;
                
                ConvertMessageToAction( msg );
            }
        }
    }
}

void SdlApplication::SendToServer( void )
{
    if (sndMsg[0] != "")
    {
        sndMsg[0] =  sndMsg[0] +
        ";red." + std::to_string(playerBox->color.r) +
        ";green." + std::to_string(playerBox->color.g) +
        ";blue." + std::to_string(playerBox->color.b) +
        ";name." + playerBox->name  +  ";end;";

        //cout << "send mess: " << sndMsg[0].c_str() << endl;
    
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
    
    //cout << "Turning message into action..." << endl;
    while ( isSearching ) {
        currentChar = msg[index];
        
        if ( searchingForAction )
        {
            if (currentChar == '.')
            {
                //cout << "Found action: " << currentAction << endl;
                searchingForAction = false;
                searchingForValue = true;
            } else if (currentChar != '.' && currentChar != ';'){
                currentAction = currentAction + currentChar;
            }
        }
        
        if ( searchingForValue )
        {
            //cout << "Searching for value..." << endl;
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
                //cout << "Finished decoding ..." << endl;
                isSearching = false;
                searchingForValue = false;
                searchingForAction = false;
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "name")
            {
                if (currentBox != GUID){
                box[currentBox]->name = currentValue;
                }
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "x")
            {
                if (currentBox != GUID){
                intValue = atoi(currentValue.c_str());
                box[currentBox]->rect.x = intValue;
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "direction")
            {
                //cout << "Current value for dir:" << endl;
                //cout << currentValue << endl;
                intValue = atoi(currentValue.c_str());
                
               
                if (currentBox != GUID){
                switch (intValue) {
                    case 0:
                        box[currentBox]->direction = dirUp;
                        break;
                     
                    case 1:
                        box[currentBox]->direction = dirDown;
                        break;
                        
                        
                    case 2:
                        box[currentBox]->direction = dirLeft;
                        break;
                        
                        
                    case 3:
                        box[currentBox]->direction = dirRight;
                        break;
                        
                    default:
                        break;
                }
                }
                
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "y")
            {
                if (currentBox != GUID){
                intValue = atoi(currentValue.c_str());
                box[currentBox]->rect.y = intValue;
                }
                currentAction = "";
                currentValue = "";
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
                
                intValue = atoi(currentValue.c_str());
                playerBox->GUID = intValue;
                
                cout << "Connected as client " << playerBox->GUID << endl;
                
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "alive")
            {
                if ( box[currentBox] == NULL && currentBox != GUID)
                {
                    cout << "Created new box: " << intValue << endl;
                    box[currentBox] = new Box();
                    totalClients++;
                    
                }
                
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "red")
            {
                if (currentBox != GUID){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->color.r = intValue;
                }
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "green")
            {
                if (currentBox != GUID){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->color.g = intValue;
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "blue")
            {
                if (currentBox != GUID){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->color.b = intValue;
                }
                
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "shoot")
            {
                if (currentBox != GUID)
                {
                if (currentValue == "yes")
                {
                    //cout << "Turning on shoot" << endl;
                    box[currentBox]->isShooting = true;
                } else {
                    box[currentBox]->isShooting = false;
                }
                }
                
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "name")
            {
                //cout << "Set name to " << currentValue << endl;
                if (currentBox != GUID)
                {
                    box[currentBox]->name = currentValue;
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "dead")
            {
                //cout << "Box " << currentBox << " left..." << endl;
                if (box[currentBox] != NULL && currentBox != GUID)
                {
                    cout << box[currentBox]->name + " just left..." << endl;
                    delete box[currentBox];
                    box[currentBox] = 0;
                    totalClients--;
                    
                    //clientFree[currentBox] = true;
                }
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



void SdlApplication::Update( Uint32 delta )
{
    
    if (playerBox !=NULL){
        GUID = playerBox->GUID;
    }
    
    for ( int i = 0 ; i < TOTAL_BULLETS ; i++ )
    {
        if ( bullets[i] != NULL) {
            bullets[i]->Update( delta );
        }
    }
    
    if (isClient)
    {
    for ( int i = 0 ; i < MAX_STARS ; i++ )
    {
        if ( stars[i] != NULL) {
            stars[i]->Update( delta );
        }
    }
    
    }
    
    
    if ( finalScale < scale * 3)
    {
        finalScale += .001;
    }
    
    if (finalScale > scale * 3){
        finalScale -=  .001;
    }
    
    finalScale = scale * 3 ;
    
    if (finalXOff < xOff) {
        finalXOff += .01;
    }
    
    
    if (finalXOff > xOff) {
        finalXOff -= .01;
    }
    
    if (finalYOff < yOff) {
        finalYOff += .01;
    }
    
    
    if (finalYOff > yOff) {
        finalYOff -= .01;
    }
    
    
    Box::xOff = finalXOff;
    Box::yOff = finalYOff;
    
    bullet::Box::xOff = Box::xOff;
    bullet::Box::yOff = Box::yOff;
    
    
//    if (isClient)
//    {
//    if (playerBox->rect.x + playerBox->rect.w > SCREEN_WIDTH){
//        playerBox->rect.x = SCREEN_WIDTH - playerBox->rect.w;
//    }
//    
//    if (playerBox->rect.x < 0){
//        playerBox->rect.x = 0;
//    }
//    
//    
//    if (playerBox->rect.y  + playerBox->rect.h > SCREEN_HEIGHT){
//        playerBox->rect.y = SCREEN_HEIGHT - playerBox->rect.h;
//    }
//    
//    if (playerBox->rect.y < 0){
//        playerBox->rect.y = 0;
//    }
//    }
    
    
    
    for ( int i = 0 ; i < MAX_CLIENTS ; i++ )
    {
        if (box[i] != NULL )
        {
            if (box[i]->isShooting){
            ShootBullet(box[i]->rect.x + box[i]->rect.w/2, box[i]->rect.y + box[i]->rect.h/2, box[i]->direction ,false,box[i]->color);
            }
        }
    }
    
    
    TOTAL_BULLETS = totalBoxes * 1000;
    
    if (isClient)
    {
    playerBox->color.r = 100;
    playerBox->color.g = 100;
    playerBox->color.b = 100;
    
    switch (GUID) {
        case 1:
            playerBox->color.r = 255;
            break;
            
        case 2:
            playerBox->color.g = 255;
            break;
       
        case 3:
            playerBox->color.b = 255;
            break;
            
        case 4:
            playerBox->color.r = 255;
            playerBox->color.g = 255;
            break;
            
        case 5:
            playerBox->color.g = 255;
            playerBox->color.b = 255;
            break;
            
            
        case 6:
            playerBox->color.r = 255;
            playerBox->color.b = 255;
            break;
            
            
        default:
            playerBox->color.r = 50 * GUID;
            playerBox->color.g = 50 * GUID;
            playerBox->color.b = 50 * GUID;
            break;
        }
    }
    
}

void SdlApplication::CheckBulletBounds( void )
{
   
    if ( playerBox->rect.x > HORIZONTAL_BOUNDS)
    {
        playerBox->rect.x = HORIZONTAL_BOUNDS;
    }
    
    if ( playerBox->rect.x < -HORIZONTAL_BOUNDS)
    {
        playerBox->rect.x = -HORIZONTAL_BOUNDS;
    }
    
   
    if ( playerBox->rect.y < -VERTICAL_BOUNDS)
    {
        playerBox->rect.y = -VERTICAL_BOUNDS;
    }
    
    if ( playerBox->rect.y > VERTICAL_BOUNDS)
    {
        playerBox->rect.y = VERTICAL_BOUNDS;
    }
    
    

    for ( int i = 0 ; i < MAX_STARS ; i++ )
    {

            if ( stars[i]->rect.x > HORIZONTAL_BOUNDS )
            {
                stars[i]->rect.x = -HORIZONTAL_BOUNDS;
                stars[i]->rect.y = rand() % VERTICAL_BOUNDS;

            }
            
            if ( stars[i]->rect.x < -HORIZONTAL_BOUNDS)
            {

                stars[i]->rect.x = HORIZONTAL_BOUNDS;
                stars[i]->rect.y = rand() % VERTICAL_BOUNDS;
            }
            
            
            if ( stars[i]->rect.y > VERTICAL_BOUNDS )
            {
                stars[i]->rect.y = -VERTICAL_BOUNDS ;
                stars[i]->rect.x = rand() % HORIZONTAL_BOUNDS;
            }
            
            if ( stars[i]->rect.y < -VERTICAL_BOUNDS )
            {
                stars[i]->rect.y = VERTICAL_BOUNDS ;
                stars[i]->rect.x = rand() % HORIZONTAL_BOUNDS;
            }
        
    }
    
    

    
    
}

void SdlApplication::ShootBullet( int x , int y , dir direction , bool friendly, SDL_Color color)
{
    //Create new bullet if needed
    if (bullets[availableBullet] == NULL)
    {
        bullets[availableBullet] = new bullet();
    }
    bullets[availableBullet]->isFriendly = friendly;
    bullets[availableBullet]->direction = direction;
    bullets[availableBullet]->rect.x = x;
    bullets[availableBullet]->rect.y = y;
    

    bullets[availableBullet]->color.r = color.r;
    bullets[availableBullet]->color.g = color.g;
    bullets[availableBullet]->color.b = color.b;
    
    availableBullet++;
    if (availableBullet >= TOTAL_BULLETS){
        availableBullet = 0;
    }
    
}




void SdlApplication::CheckBulletCollisions( void )
{
    for (int i = 0 ;  i < MAX_BULLETS; i++) {
        if (bullets[i] != NULL)
        {
        if (!bullets[i]->isFriendly)
        {
            SDL_Rect intersect;
            if (SDL_IntersectRect(&bullets[i]->rect, &playerBox->rect, &intersect) == SDL_TRUE)
            {
                cout << "collision" << endl;
                playerBox->rect.x = finalXOff + rand() % 1000;
                playerBox->rect.y = finalYOff + rand() % 1000;
                
                sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->rect.y) + ";";
                sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->rect.x) + ";";
            }
        }
        }
    }
}



void SdlApplication::CalculateFrameDistance( void )
{
//    farBottom = 0;
//    farTop = 0;
//    farLeft = 0;
//    farRight = 0;
    scale = 1;
    
    
    leftBox = nullBox;
    rightBox = nullBox;
    topBox = nullBox;
    bottomBox = nullBox;
    
    if ( true )
    {
        if (isClient)
        {

            leftBox = playerBox;
            rightBox = playerBox;
            topBox = playerBox;
            bottomBox = playerBox;
            
        }
        
        
        bool noBoxes = true;
        
        totalBoxes = 0;
        
    for ( int i = 0 ; i < MAX_CLIENTS ; i++ )
    {
        if (box[i] != NULL)
        {
            //cout << "Real box" << endl;
            noBoxes = false;
            totalBoxes++;
            if (box[i]->rect.x > rightBox->rect.x)
            {
                rightBox = box[i];
            }
            
            if (box[i]->rect.x < leftBox->rect.x)
            {
                leftBox = box[i];
            }
            
            if (box[i]->rect.y > bottomBox->rect.y)
            {
                bottomBox = box[i];
            }
            
            if (box[i]->rect.y < topBox->rect.y)
            {
                topBox = box[i];
            }
        }
    }
        
        
    
    farWidth = farRight - farLeft;
    farHeight = farBottom - farTop;
    
        // cout << "Far Width: " << farWidth << endl;
        // cout << "Far Height: " << farHeight << endl;


//                    cout << "Top: " << farTop << endl;
//                    cout << "Bottom: " << farBottom << endl;
//                    cout << "Right: " << farRight << endl;
//                    cout << "Left: " << farLeft << endl;
        
        //point farright,farbottom  / farleft,fartop
 
        
        distance = sqrtf(  pow((leftBox->rect.x - rightBox->rect.x),2)  + pow((topBox->rect.y - bottomBox->rect.y),2) );
    
//        cout << "Dist: " << distance << endl;
        
        scale = 100 / (distance);
        
//        if (distance < 0 )
//        {
//            distance = .0000001;
//        }
        
        
        if (noBoxes) {
            scale = .2;
            xOff = 0;
            yOff = 0;
        }
        
        
        finalXOff = (leftBox->rect.x) - (SCREEN_WIDTH  * (distance/1000)) - (SCREEN_WIDTH/2) * (distance/10000);


        finalYOff = (topBox->rect.y - topBox->rect.h) - (SCREEN_HEIGHT * (distance/1000)) - (SCREEN_HEIGHT/2) * (distance/10000);
        
    
//        if (scale <= 0){
//            scale = 1;
//        }
//        
        if (scale > 5)
        {
            scale = 5;
        }
        
//    cout << "Scale :" << scale << endl;
    }
    
}









