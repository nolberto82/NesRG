#include "renderer.h"
#include "controls.h"
#include "ppu.h"

bool render_init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	gfx.window = SDL_CreateWindow("Nes RG", 10, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0);

	if (!gfx.window)
	{
		printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
		return false;
	}

	gfx.renderer = SDL_CreateRenderer(gfx.window, -1, SDL_RENDERER_ACCELERATED);

	if (!gfx.renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	//create textures
	gfx.screen = SDL_CreateTexture(gfx.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);

	if (!gfx.screen)
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
		return false;
	}


	gfx.ntscreen = SDL_CreateTexture(gfx.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, NES_SCREEN_WIDTH * 2, NES_SCREEN_HEIGHT * 2);

	if (!gfx.ntscreen)
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
		return false;
	}

	//Enable transparency
	SDL_SetRenderDrawBlendMode(gfx.renderer, SDL_BLENDMODE_BLEND);

	SDL_initFramerate(&gfx.fpsman);
	SDL_setFramerate(&gfx.fpsman, 60);

	return true;
}

void render_frame(u32* screen_pixels)
{
	SDL_UpdateTexture(gfx.screen, NULL, screen_pixels, NES_SCREEN_WIDTH * sizeof(unsigned int));
	SDL_RenderCopy(gfx.renderer, gfx.screen, NULL, NULL);
}

void render_nttable()
{
	int x = 0, y = 0;
	int sx = (lp.t & 0x1f);
	int sy = (lp.t & 0x3e0) >> 5;

	if (!gfx.renderer)
		return;

	SDL_SetRenderTarget(gfx.renderer, gfx.ntscreen);

	for (int i = 0; i < 4; i++)
	{
		SDL_Rect rect = { x, y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
		SDL_UpdateTexture(gfx.ntscreen, &rect, ppu.ntable_pixels[i], NES_SCREEN_WIDTH * sizeof(unsigned int));
		SDL_RenderCopy(gfx.renderer, gfx.ntscreen, NULL, NULL);

		x = 256;
		if (i == 1)
		{
			x = 0;
			y = 240;
		}
	}

	SDL_RenderCopy(gfx.renderer, gfx.ntscreen, NULL, NULL);

	SDL_SetRenderTarget(gfx.renderer, NULL);
}

void render_overlay(SDL_Rect rect)
{
	SDL_SetRenderDrawColor(gfx.renderer, 255, 255, 255, 80);
	SDL_RenderFillRect(gfx.renderer, &rect);
	SDL_SetRenderDrawColor(gfx.renderer, 0, 255, 0, 255);
	SDL_RenderDrawRect(gfx.renderer, &rect);
}

void render_input()
{
	gfx.ctrl_keys = SDL_GetKeyboardState(NULL);
}

void render_clean()
{
	if (gfx.ntscreen)
		SDL_DestroyTexture(gfx.ntscreen);

	if (gfx.screen)
		SDL_DestroyTexture(gfx.screen);

	if (gfx.renderer)
		SDL_DestroyRenderer(gfx.renderer);

	if (gfx.window)
		SDL_DestroyWindow(gfx.window);

	SDL_Quit();
}