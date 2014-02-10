// ============================================================================
// [Include Section]
// ============================================================================
#include "CommonIncludes.h"

#include "Box.h"
#include "bullet.h"



using namespace std;

#define MAX_SOCKETS 20
#define MAX_CLIENTS 20
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
    //Methods
	SdlApplication();
	~SdlApplication();
	
	enum APP_STATE
	{ APP_OK = 0,APP_FAILED = 1};
	
	int init(int width, int height);
    
	void destroy();
	int run(int width, int height);
    
	void onEvent(SDL_Event* ev);
    void HandleKeys( void );
    void Update( Uint32 delta );
	void Render();
    
    void ShootBullet( int x , int y , direction direction , bool friendly , SDL_Color color);
    void CheckBulletCollisions( void );
    void CheckBounds( void );
    
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
    
    void ConvertMessageToAction( string msg );//Takes rcvd msg and turns to action
    
    
    //App Members
    SDL_Window *win;//SDL window
	SDL_Renderer *renderer;///<Rendering frame
	bool _running;///<Used to determine if app is looping
    string userPrompt;///<Captures user selection when prompted
    
    
    //Network Members
    IPaddress ip;///<Blank IP holder
    
    //Server Object to handle server communication
    
    bool isServer = false;///<Flag if running server
    bool isClient = false;///<Flag if connecting as client
    
    SDLNet_SocketSet serverSocketSet;///<Used for listening as server
    SDLNet_SocketSet clientSocketSet;///<Set of sockets from clients
    
    TCPsocket server;///<Server socket
    TCPsocket client[MAX_CLIENTS];///<Array of client sockets
    bool      clientFree[MAX_CLIENTS];///<Array of flags to tell if client socket free
    
    int GUID = -1;///<ID to handle network messages
    
    string userName;///<Used for players name in game
    string rcvMsg;///<Used to capture received data
    string sndMsg[10];///<Used to send data

    int clientNumber;///<Indicates which client you are on server
    int totalClients = 0;///<Number of clients on server
    int currentBox = 0;///<Indicates currently selected box from server
    bool isConnected;///<Flag used to determine if connected to server
    
    
    //Render Members
    double scale,finalScale;//<Used to zoom view
    int TOTAL_BULLETS = 1000;///<Handles how many bullets available
    int totalBoxes = 0;///<Count of boxes (controlled by clients) in game
    float distance;///<Indicates how far apart all boxes are
    float finalXOff,finalYOff;///<Used to transition to offset values
    float xOff,yOff;///<Offsets determined by distance between boxes

    
    //Logic Members
    
    int speedNormal = 60;///<Normal rate of movement
    int speedFast = 100;///<Faster rate of movement when player holds boost button
    bool isBoosting = false;///<Flag for if player is boosting
    int boxSpeed = speedNormal;///<Current speed of player
    
    Box* box[MAX_CLIENTS];///<Array of boxes on screen
    bullet* bullets[MAX_BULLETS];///<Array of bullets on screen
    int availableBullet = 0;///<Index of last available bullet
    bullet* stars[MAX_STARS];///<Array of stars on screen
    
    SDL_Rect tempRect;
    SDL_Color tempColor;
    
    Box* nullBox;///<Null Box used for when a box is yet to be made
    Box* playerBox = nullBox;///<Pointer to players box
    Box* leftBox = nullBox;///<Points to leftmost box
    Box* rightBox = nullBox;///<Points to rightmost box
    Box* topBox = nullBox;///<Points to topmost box
    Box* bottomBox = nullBox;///<Points to bottommost box
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
    
    
initInternet:{
    cout << "Connecting to internet ..." << endl;
    if ( SDLNet_Init() !=-1){
        cout << "Connected to internet." << endl;
    } else {
        cout << "Could not connect to internet.";
    }
}
    
    
promptUser:{
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
        
        cout << "Please enter your user name: " << endl;
        cin.clear();
        getline(cin,userName);
        
        cout << "Enter IP address of server (press enter for localhost)" << endl;
        userPrompt = "";
        cin.clear();
        getline(cin,userPrompt);
        
        if (userPrompt == ""){
            userPrompt = "127.0.0.1";
        }
        
    }
}
    
connectToServer:{
    //Attempt to connect to Server
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
}
 
    
    //Connect to/Listen on server port
    /*
     For server, this simply opens listening port
     For Clients, this is the first socket set activity to server
     */
    server = SDLNet_TCP_Open(&ip);
    
    
clearSockets:{
    //Clear out entire client socket set
    for (int i = 0 ; i < MAX_CLIENTS ; i++){
        client[i] = NULL;
        box[i] = NULL;
        clientFree[i] = true;
    }
}
    
    
gameInit:{
    //Create NullBox
    nullBox = new Box();
    nullBox->xOff = 0;
    
    //Set renderers
    Box::renderer = renderer;
    bullet::Box::renderer = renderer;
    
    
    //Clear out boxes
    for (int i = 0 ; i < MAX_CLIENTS ; i++)
    {
        box[ i ] = new Box();
        box[ i ]->SetGUID( i );
    }
    
    
    //Clear out bullets
    for (int i = 0 ; i < MAX_BULLETS ; i++ )
    {
        bullets[i] = NULL;
    }
    
}
    
    //Server init
    if (isServer){
        SDL_SetWindowTitle(win, "SDL NetBox - Server");
        //Allocate open sockets for clients to connect to
        serverSocketSet = SDLNet_AllocSocketSet(1);
        clientSocketSet = SDLNet_AllocSocketSet(MAX_SOCKETS);
        
        
        //add server listening socket
        if ( server != NULL ){
            SDLNet_TCP_AddSocket(serverSocketSet, server);
            isConnected = true;
            
        } else{
            cout << "Server not listening..." << endl;
            _running = false;
            isConnected = false;
        }
    }
    
    
    
    Box::renderer = renderer;
    
    if (isClient){
        playerBox = NULL;
        playerBox = new Box();
        
        playerBox->SetOnline();
        playerBox->SetRectX( rand() % 1000 );
        playerBox->SetRectY( rand() % 1000 );
        playerBox->SetName( userName );
        
        int color = rand() % 4;
        
        cout << "Color seed: " << color << endl;
        switch ( color ) {
            case 0:
                playerBox->SetColorRed( 200 + rand() % 50 );
                playerBox->SetColorGreen( 0 + rand() % 50 );
                playerBox->SetColorBlue( 0 + rand() % 50 );
                break;
                
            case 1:
                playerBox->SetColorRed( 0 + rand() % 50 );
                playerBox->SetColorGreen( 0 + rand() % 50 );
                playerBox->SetColorBlue( 200 + rand() % 50 );
                break;
                
            case 2:
                playerBox->SetColorRed( 0 + rand() % 50 );
                playerBox->SetColorGreen( 200 + rand() % 50 );
                playerBox->SetColorBlue( 0 + rand() % 50 );
                break;
                
            case 3:
                playerBox->SetColorRed( 200 + rand() % 50 );
                playerBox->SetColorGreen( 0 + rand() % 50 );
                playerBox->SetColorBlue( 200 + rand() % 50 );
                break;
                
            default:
                playerBox->SetColorRed( 200 + rand() % 50 );
                playerBox->SetColorGreen( 200 + rand() % 50 );
                playerBox->SetColorBlue( 0 + rand() % 50 );;
                break;
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
                        //sndMsg[0] = "join." + std::to_string(playerBox->GUID()) + ";";
                    }
                }
            } while (attemptConnect);
        }
        cout << "Welcome, " << userName << " to NetBox..." << endl;
        
    }
    

        
        for (int i = 0 ; i < MAX_STARS ; i++ )
        {
            stars[i] = new bullet();
            stars[i]->SetRectX( -HORIZONTAL_BOUNDS  + rand()% HORIZONTAL_BOUNDS*2);
            stars[i]->SetRectY( -VERTICAL_BOUNDS + rand()% VERTICAL_BOUNDS*2);
            
            stars[i]->SetRectW( 25 );
            stars[i]->SetRectH( 25 );
            
            stars[i]->SetColorRed( 200 );
            stars[i]->SetColorGreen( 200 );
            stars[i]->SetColorBlue( 200 );

            
            stars[i]->SetNormalSpeed( 1 + rand() % 20);
            stars[i]->MoveNormal();
            stars[i]->Move();
            stars[i]->SetOnline();
            
            int dir = rand() % 4;
            
            switch (dir) {
                case 0:
                    stars[i]->Dir()->up = true;
                    break;
                    
                case 1:
                    stars[i]->Dir()->down = true;
                    break;
                    
                case 2:
                    stars[i]->Dir()->left = true;
                    break;
                    
                case 3:
                    stars[i]->Dir()->right = true;
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
            //CheckBulletCollisions();
            CheckBounds();
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
	}
}

void SdlApplication::Render()
{
    //finalScale = .3;
    
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
        
        if (playerBox->IsMoving())
        {
            playerBox->Dir()->up   = false;
            playerBox->Dir()->down = false;
            playerBox->Dir()->left    = false;
            playerBox->Dir()->right    = false;
        }
        
        playerBox->StopMoving();
        
        if (SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_LEFT || SDL_GetMouseState(NULL, NULL) == SDL_BUTTON_LEFT && SDL_BUTTON_RIGHT)
        {
            if (isClient){
                ShootBullet(
                            playerBox->Rect()->x + playerBox->Rect()->w/2,
                            playerBox->Rect()->y + playerBox->Rect()->h/2 , *playerBox->Dir(),
                            true,playerBox->Color());
                playerBox->Shoot();
                sndMsg[0] =  sndMsg[0] + "shoot.yes" + ";";
                
            } else {
                playerBox->StopShooting();
                sndMsg[0] =  sndMsg[0] + "shoot.no" + ";";
            }
        } else {
            if (isClient)
            {
                playerBox->StopShooting();
                sndMsg[0] =  sndMsg[0] + "shoot.no" + ";";
            }
        }
        
        
        if ( state[ SDL_SCANCODE_F ] ) {
            SDL_SetWindowFullscreen(win, SDL_WINDOW_FULLSCREEN);
        }
        
        
        if ( state[ SDL_SCANCODE_W ] ) {
            if (isClient)
            {
                playerBox->Dir()->up = true;


                
                playerBox->Move();
                sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->Rect()->y) + ";";
            }
        }
        
        
        if ( state[ SDL_SCANCODE_S ] ) {
            if (isClient)
            {
                playerBox->Dir()->down = true;


                
                playerBox->Move();
                sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->Rect()->y) + ";";
            }
        }
        
        if ( state[ SDL_SCANCODE_A ] ) {
            if (isClient)
            {
                playerBox->Dir()->left = true;

                
                playerBox->Move();
                sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->Rect()->x) + ";";
            }
        }
        
        if ( state[ SDL_SCANCODE_D ] ) {
            if (isClient)
            {
                playerBox->Dir()->right = true;

                
                playerBox->Move();
                sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->Rect()->x) + ";";
            }
        }
        
        if ( state[ SDL_SCANCODE_LSHIFT ] ) {
            playerBox->MoveFast();
        } else {
            playerBox->MoveNormal();
        }
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
                
                
                char finalMsg[1000];
                totalClients++;
                sprintf(finalMsg, "accept.%i" , freeSpot);
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
        for (int i = 0 ; i < MAX_CLIENTS ; i++ )
        {
            if ( !clientFree[ i ] )
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
                        
                        //delete box[i+1];
                        box[i+1]->SetOffline();
                        client[i] = NULL;
                        clientFree[i] = true;
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
}



void SdlApplication::BroadcastToClients()
{
    if (totalClients > 0)
    {
        for (int i  = 0 ; i < MAX_CLIENTS ; i++)
        {
            if (box[i] != NULL){
                sndMsg[0] = sndMsg[0] + "box." + std::to_string(box[i]->GUID())  + ";alive.0" +
                ";name." + box[i]->Name() +
                ";x." + std::to_string(box[i]->Rect()->x) +
                ";y." + std::to_string(box[i]->Rect()->y) +
                ";red." + std::to_string(box[i]->Color().r) +
                ";green." + std::to_string(box[i]->Color().g) +
                ";blue." + std::to_string(box[i]->Color().b) +
                ";";
                
                if (box[i]->IsShooting())
                {
                    sndMsg[0] = sndMsg[0] + "shoot.yes;";
                } else {
                    sndMsg[0] = sndMsg[0] + "shoot.no;";
                }
                
            } else {
                sndMsg[0] = sndMsg[0] + "box." + std::to_string(box[i]->GUID()) + ";dead.0;";
            }
        }
        
        
        //Broadcast message back to all clients
        for (int clientNum = 0 ; clientNum < MAX_CLIENTS ; clientNum++)
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
        "red." + std::to_string(playerBox->Color().r) +
        ";green." + std::to_string(playerBox->Color().g) +
        ";blue." + std::to_string(playerBox->Color().b) +
        ";x." + std::to_string(playerBox->Rect()->x) +
        ";y." + std::to_string(playerBox->Rect()->y) +
        ";name." + playerBox->Name()  +  ";end;";
        
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
            intValue = atoi(currentValue.c_str());
            
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
                if (currentBox != GUID && box[ currentBox ] != NULL ){
                    if ( box[currentBox]->Name() != currentValue )
                    {
                        box[currentBox]->SetName( currentValue );
                        cout << box[currentBox]->Name() << " has just joined " << endl;
                    }
                    box[currentBox]->SetName( currentValue );
                }
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "x")
            {
                if (currentBox != GUID && box[ currentBox ] != NULL ){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->SetRectX( intValue );
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "direction")
            {
                //cout << "Current value for dir:" << endl;
                //cout << currentValue << endl;
                intValue = atoi(currentValue.c_str());
                
                
                if (currentBox != GUID && box[ currentBox ] != NULL ){
                    switch (intValue) {
                        case 0:
                            box[ currentBox ]->Dir()->up = true;
                            break;
                            
                        case 1:
                            box[ currentBox ]->Dir()->down = true;
                            break;
                            
                            
                        case 2:
                            box[ currentBox ]->Dir()->left = true;
                            break;
                            
                            
                        case 3:
                            box[ currentBox ]->Dir()->right = true;
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
                if (currentBox != GUID && box[ currentBox ] != NULL ){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->SetRectY( intValue );
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "join")
            {
                intValue = atoi(currentValue.c_str());
                //cout << "Added box " << intValue << endl;
                //box[intValue] = new Box();
                //box[intValue]->SetGUID(intValue);
                box[ intValue ]->SetOnline();
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
                playerBox->SetGUID( intValue );
                GUID = playerBox->GUID();
                
                cout << "Connected as client " << playerBox->GUID() << endl;
                
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "alive")
            {
                if ( box[currentBox] == NULL && currentBox != GUID)
                {
                    cout << "Client " << intValue << " has joined..." << endl;
                    box[currentBox] = new Box();
                    totalClients++;
                }
                
                box[ currentBox ]->SetOnline();
                
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "red")
            {
                if (currentBox != GUID && box[ currentBox ] != NULL )
                {
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->SetColorRed( intValue );
                }
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "green")
            {
                if (currentBox != GUID && box[ currentBox ] != NULL ){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->SetColorGreen( intValue );
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "blue")
            {
                if (currentBox != GUID && box[ currentBox ] != NULL ){
                    intValue = atoi(currentValue.c_str());
                    box[currentBox]->SetColorBlue( intValue );
                }
                
                currentAction = "";
                currentValue = "";
            }
            
            if (currentAction == "shoot")
            {
                if (currentBox != GUID && box[ currentBox ] != NULL )
                {
                    if (currentValue == "yes")
                    {
                        if ( box[ currentBox ] != NULL )
                        {
                            box[currentBox]->Shoot();
                        }
                    }  else {
                        box[currentBox]->StopShooting();
                    }
                }
                
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "name")
            {
                //cout << "Set name to " << currentValue << endl;
                if (currentBox != GUID && box[ currentBox ] != NULL )
                {
                    box[currentBox]->SetName( currentValue );
                }
                currentAction = "";
                currentValue = "";
            }
            
            
            if (currentAction == "dead")
            {
                //cout << "Box " << currentBox << " left..." << endl;
                if (box[currentBox] != NULL && currentBox != GUID)
                {
                    cout << "Received action dead " << endl;
                    cout << box[currentBox]->Name() + " just left..." << endl;
                    //delete box[currentBox];
                    box[ currentBox ]->SetOffline();
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
    
    if (playerBox !=NULL && isClient){
        playerBox->SetGUID( GUID );
        playerBox->Update( delta );
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

    
    for ( int i = 0 ; i < MAX_CLIENTS ; i++ )
    {
        if (box[i] != NULL )
        {
            if (box[i]->IsShooting()){
                ShootBullet(box[i]->Rect()->x + box[i]->Rect()->w/2, box[i]->Rect()->y + box[i]->Rect()->h/2, *box[i]->Dir() ,false,box[i]->Color());
            }
        }
    }
    
    
    TOTAL_BULLETS = totalBoxes * 1000;
    
    if (isClient)
    {
        playerBox->SetColorRed( 100 );
        playerBox->SetColorGreen( 100 );
        playerBox->SetColorBlue( 100 );
        
        switch (GUID) {
            case 1:
                playerBox->SetColorRed( 255 );
                break;
                
            case 2:
                playerBox->SetColorGreen( 255 );
                break;
                
            case 3:
                playerBox->SetColorBlue( 255 );
                break;
                
            case 4:
                playerBox->SetColorRed( 255 );
                playerBox->SetColorGreen( 255 );
                break;
                
            case 5:
                playerBox->SetColorGreen( 255 );
                playerBox->SetColorBlue( 255 );
                break;
                
                
            case 6:
                playerBox->SetColorRed( 255 );
                playerBox->SetColorBlue( 255 );
                break;
                
                
            default:
                playerBox->SetColorRed( 100  * (GUID +1) );
                playerBox->SetColorGreen( 100 * (GUID +1) );
                playerBox->SetColorBlue( 100  * (GUID +1) );
                break;
        }
    }
    
}

void SdlApplication::CheckBounds( void )
{
    
    if ( playerBox->Rect()->x > HORIZONTAL_BOUNDS)
    {
        playerBox->SetRectX( HORIZONTAL_BOUNDS );
    }
    
    if ( playerBox->Rect()->x < -HORIZONTAL_BOUNDS)
    {
        playerBox->SetRectX( -HORIZONTAL_BOUNDS );
    }
    
    
    if ( playerBox->Rect()->y < -VERTICAL_BOUNDS)
    {
        playerBox->SetRectY( -VERTICAL_BOUNDS );
    }
    
    if ( playerBox->Rect()->y > VERTICAL_BOUNDS)
    {
        playerBox->SetRectY( VERTICAL_BOUNDS );
    }
    
    
    
    for ( int i = 0 ; i < MAX_STARS ; i++ )
    {
        
        if ( stars[i]->Rect()->x > HORIZONTAL_BOUNDS )
        {
            stars[i]->SetRectX( -HORIZONTAL_BOUNDS );
            stars[i]->SetRectY( rand() % VERTICAL_BOUNDS );
            
        }
        
        if ( stars[i]->Rect()->x < -HORIZONTAL_BOUNDS)
        {
            
            stars[i]->SetRectX( HORIZONTAL_BOUNDS );
            stars[i]->SetRectY( rand() % VERTICAL_BOUNDS );
        }
        
        
        if ( stars[i]->Rect()->y > VERTICAL_BOUNDS )
        {
            stars[i]->SetRectY( -VERTICAL_BOUNDS );
            stars[i]->SetRectX( rand() % HORIZONTAL_BOUNDS );
        }
        
        if ( stars[i]->Rect()->y < -VERTICAL_BOUNDS )
        {
            stars[i]->SetRectY( VERTICAL_BOUNDS );
            stars[i]->SetRectX( rand() % HORIZONTAL_BOUNDS );
        }
        
    }
    
    
    
    
    
}

void SdlApplication::ShootBullet( int x , int y , direction direction , bool friendly, SDL_Color color)
{
    //Create new bullet if needed
    if (bullets[availableBullet] == NULL)
    {
        bullets[availableBullet] = new bullet();
    }
    bullets[availableBullet]->isFriendly = friendly;
    bullets[availableBullet]->SetRectX( x );
    bullets[availableBullet]->SetRectY( y );
    
    
    bullets[availableBullet]->SetColorRed( color.r );
    bullets[availableBullet]->SetColorGreen( color.g );
    bullets[availableBullet]->SetColorBlue( color.b );

    
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
                if (SDL_IntersectRect(bullets[i]->Rect(), playerBox->Rect(), &intersect) == SDL_TRUE)
                {
                    cout << "collision" << endl;
                    playerBox->Rect()->x = finalXOff + rand() % 100;
                    playerBox->Rect()->y = finalYOff + rand() % 100;
                    
                    sndMsg[0] =  sndMsg[0] + "y." + std::to_string(playerBox->Rect()->y) + ";";
                    sndMsg[0] =  sndMsg[0] + "x." + std::to_string(playerBox->Rect()->x) + ";";
                }
            }
        }
    }
}



void SdlApplication::CalculateFrameDistance( void )
{
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
            if (box[i]->IsOnline())
            {
                //cout << "Real box" << endl;
                noBoxes = false;
                totalBoxes++;
                if (box[i]->Rect()->x > rightBox->Rect()->x)
                {
                    rightBox = box[i];
                }
                
                if (box[i]->Rect()->x < leftBox->Rect()->x)
                {
                    leftBox = box[i];
                }
                
                if (box[i]->Rect()->y > bottomBox->Rect()->y)
                {
                    bottomBox = box[i];
                }
                
                if (box[i]->Rect()->y < topBox->Rect()->y)
                {
                    topBox = box[i];
                }
            }
        }
        

        distance = sqrtf(   pow((leftBox->Rect()->x - rightBox->Rect()->x ),2)   +
                            pow((topBox->Rect()->y - bottomBox->Rect()->y),2) );
        
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
        
        
        finalXOff = (leftBox->Rect()->x) - (SCREEN_WIDTH  * (distance/1000)) - (SCREEN_WIDTH/2) * (distance/10000);
        
        
        finalYOff = (topBox->Rect()->y - topBox->Rect()->h) - (SCREEN_HEIGHT * (distance/1000)) - (SCREEN_HEIGHT/2) * (distance/10000);
        
        
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









