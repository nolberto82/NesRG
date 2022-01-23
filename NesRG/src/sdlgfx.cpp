#include "sdlgfx.h"

bool SDLGfx::init()
{
	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Z80 Emulator", 10, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, window_flags);

	if (!window)
	{
		printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	SDL_initFramerate(&fpsman);
	SDL_setFramerate(&fpsman, 60);

	return true;
}

void SDLGfx::update()
{
}

void SDLGfx::input()
{
	int wheel = 0;

	//memset(keystick1, 0, sizeof(keystick1));
#if _WIN64

#else

#endif

}

void SDLGfx::begin_frame()
{

	SDL_SetRenderDrawColor(renderer, 114, 144, 154, 255);
	SDL_RenderClear(renderer);
}

void SDLGfx::end_frame()
{
	SDL_framerateDelay(&fpsman);
	SDL_RenderPresent(renderer);
}

void SDLGfx::clean()
{
	//SDL_DestroyTexture(tile.texture);
	//SDL_DestroyTexture(display.texture);
	//SDL_DestroyTexture(chars.texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
