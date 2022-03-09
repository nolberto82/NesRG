#include "sdlcore.h"
#include "controls.h"
#include "ppu.h"

bool sdl_init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	int flags = SDL_WINDOW_RESIZABLE;
	sdl.window = SDL_CreateWindow("Nes RG", 10, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, flags);

	if (!sdl.window)
	{
		printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
		return false;
	}

	sdl.renderer = SDL_CreateRenderer(sdl.window, -1, SDL_RENDERER_ACCELERATED);

	if (!sdl.renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	//create textures
	sdl.screen = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);

	if (!sdl.screen)
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
		return false;
	}

	sdl.ntscreen = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, NES_SCREEN_WIDTH * 2, NES_SCREEN_HEIGHT * 2);

	if (!sdl.ntscreen)
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
		return false;
	}

	sdl.sprscreen = SDL_CreateTexture(sdl.renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, 128, 128);

	if (!sdl.sprscreen)
	{
		printf("Failed to create texture: %s\n", SDL_GetError());
		return false;
	}

	//Enable transparency
	SDL_SetRenderDrawBlendMode(sdl.renderer, SDL_BLENDMODE_BLEND);

	SDL_initFramerate(&sdl.fpsman);
	SDL_setFramerate(&sdl.fpsman, 60);

	return true;
}

void sdl_frame(u32* screen_pixels, int state)
{
	if (state == cstate::scanlines || state == cstate::cycles)
	{
		int x = lp.v & 0x1f;
		int y = (lp.v & 0x3e0) >> 5;

		SDL_SetRenderTarget(sdl.renderer, sdl.screen);

		//SDL_Rect rect = { 0,  ppu.scanline, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
		//SDL_Rect rect2 = { 0,  ppu.scanline, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
		//if (state == cstate::cycles)
		//{
		SDL_Rect rect = { 0,  ppu.scanline + 1, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
		SDL_Rect rect2 = { ppu.cycle,  ppu.scanline, NES_SCREEN_WIDTH, 1 };
		//}


		SDL_UpdateTexture(sdl.screen, NULL, screen_pixels, NES_SCREEN_WIDTH * sizeof(unsigned int));
		SDL_RenderCopy(sdl.renderer, sdl.screen, NULL, NULL);

		sdl_overlay(rect, rect2);

		SDL_SetRenderTarget(sdl.renderer, NULL);
	}
	else
	{
		SDL_UpdateTexture(sdl.screen, NULL, screen_pixels, NES_SCREEN_WIDTH * sizeof(unsigned int));
		SDL_RenderCopy(sdl.renderer, sdl.screen, NULL, NULL);
	}
}

void sdl_nttable()
{
	int x = 0, y = 0;
	int sx = (lp.t & 0x1f);
	int sy = (lp.t & 0x3e0) >> 5;

	SDL_SetRenderTarget(sdl.renderer, sdl.ntscreen);

	for (int i = 0; i < 4; i++)
	{
		SDL_Rect rect = { x, y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
		SDL_UpdateTexture(sdl.ntscreen, &rect, ppu.ntable_pixels[i], NES_SCREEN_WIDTH * sizeof(unsigned int));
		SDL_RenderCopy(sdl.renderer, sdl.ntscreen, NULL, NULL);

		x = 256;
		if (i == 1)
		{
			x = 0;
			y = 240;
		}
	}

	u8 nametable = (lp.t & 0xc00) >> 10;
	s16 fy = (lp.v & 0x7000) >> 12;
	s16 fx = (lp.x & 7);
	u16 xp = (sx * 8 + fx) + (nametable & 1) * 256;
	u16 yp = (sy * 8 + fy) + ((nametable & 2) >> 1) * 240;
	SDL_Rect overlay = { xp, yp, 256, 240 };
	SDL_Rect overlay2 = { xp, yp, 256, 240 };

	sdl_overlay(overlay, overlay2);

	if (xp + 256 >= 512)
	{
		overlay = { 0, yp, xp - 256, 240 };
		sdl_overlay(overlay, overlay2);
	}

	if (yp + 240 >= 480)
	{
		overlay = { xp, 0, 256, yp - 240 };
		sdl_overlay(overlay, overlay2);
	}

	if (xp + 256 >= 512 && xp + 256 >= 480)
	{
		overlay = { 0, 0, xp - 256, yp - 240 };
		sdl_overlay(overlay, overlay2);
	}

	SDL_RenderCopy(sdl.renderer, sdl.ntscreen, NULL, NULL);

	SDL_SetRenderTarget(sdl.renderer, NULL);
}

void sdl_sprites()
{
	if (!sdl.renderer)
		return;

	SDL_SetRenderTarget(sdl.renderer, sdl.sprscreen);

	//SDL_Rect rect = { x, y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
	SDL_UpdateTexture(sdl.sprscreen, NULL, ppu.sprite_pixels, 256 * sizeof(unsigned int));
	SDL_RenderCopy(sdl.renderer, sdl.sprscreen, NULL, NULL);

	SDL_SetRenderTarget(sdl.renderer, NULL);
}

void sdl_overlay(SDL_Rect rect, SDL_Rect rect2)
{
	SDL_SetRenderDrawColor(sdl.renderer, 255, 255, 255, 80);
	SDL_RenderFillRect(sdl.renderer, &rect);
	//SDL_SetRenderDrawColor(sdl.renderer, 0, 255, 0, 255);
	SDL_RenderDrawRect(sdl.renderer, &rect2);
}

void sdl_input_new()
{
	sdl.ctrl_keys = SDL_GetKeyboardState(NULL);
	newkeys.f1 = sdl.ctrl_keys[SDL_SCANCODE_F1];
	newkeys.lshift = sdl.ctrl_keys[SDL_SCANCODE_LSHIFT];
}

void sdl_input_old()
{
	oldkeys.f1 = newkeys.f1;
}

void sdl_clean()
{
	if (sdl.sprscreen)
		SDL_DestroyTexture(sdl.sprscreen);

	if (sdl.ntscreen)
		SDL_DestroyTexture(sdl.ntscreen);

	if (sdl.controller)
		SDL_GameControllerClose(sdl.controller);

	if (sdl.screen)
		SDL_DestroyTexture(sdl.screen);

	if (sdl.renderer)
		SDL_DestroyRenderer(sdl.renderer);

	if (sdl.window)
		SDL_DestroyWindow(sdl.window);

	SDL_Quit();
}