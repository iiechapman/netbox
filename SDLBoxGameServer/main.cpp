// ============================================================================
// [Include Section]
// ============================================================================
#include <SDL2/SDL.h>

// ============================================================================
// [SdlApplication]
// ============================================================================
#define APPTITLE "SDL Net Box Game"
// SdlApplication is nothing more than thin wrapper to SDL library. You need
// just to instantiate it and call run() to enter the SDL event loop.
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

	void Render();

	bool _running;
	SDL_Window *win;
	SDL_Renderer *renderer;
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
	
	while (SDL_WaitEvent(&ev))
	{
		onEvent(&ev);
		Render();
		
		if (_running == false)
		{
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
    
    
}





//Entry
int main(int argc, char* argv[])
{
	SdlApplication app;
	return app.run(640, 480);
}



















