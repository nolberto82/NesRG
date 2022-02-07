#include "renderer.h"
#include "controls.h"

bool render_init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Nes RG", 10, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0);

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

	//create textures
	screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);

	if (!screen)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	SDL_initFramerate(&fpsman);
	SDL_setFramerate(&fpsman, 60);

	return true;
}

void render_frame(u32* screen_pixels)
{
	SDL_UpdateTexture(screen, NULL, screen_pixels, NES_SCREEN_WIDTH * sizeof(unsigned int));
	SDL_RenderCopy(renderer, screen, NULL, NULL);
}

void render_input()
{
	ctrl_keys = SDL_GetKeyboardState(NULL);
}

void render_clean()
{
	SDL_DestroyTexture(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}