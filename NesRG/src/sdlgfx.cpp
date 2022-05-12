#include "sdlgfx.h"
#include "controls.h"
#include "ppu.h"

namespace SDL
{
	bool init()
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
		{
			printf("Couldn't initialize SDL: %s\n", SDL_GetError());
			return false;
		}

		int flags = SDL_WINDOW_RESIZABLE;
		SDL::window = SDL_CreateWindow("Nes RG", 10, 40, APP_WIDTH, APP_HEIGHT, flags);

		if (!SDL::window)
		{
			printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
			return false;
		}

		SDL::renderer = SDL_CreateRenderer(SDL::window, -1, SDL_RENDERER_ACCELERATED);

		if (!SDL::renderer)
		{
			printf("Failed to create renderer: %s\n", SDL_GetError());
			return false;
		}

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

		//create textures
		SDL::screen = SDL_CreateTexture(SDL::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);

		if (!SDL::screen)
		{
			printf("Failed to create texture: %s\n", SDL_GetError());
			return false;
		}

		SDL::ntscreen = SDL_CreateTexture(SDL::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, NES_SCREEN_WIDTH * 2, NES_SCREEN_HEIGHT * 2);

		if (!SDL::ntscreen)
		{
			printf("Failed to create texture: %s\n", SDL_GetError());
			return false;
		}

		SDL::sprscreen = SDL_CreateTexture(SDL::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, PATTERN_WIDTH, PATTERN_HEIGHT);

		if (!SDL::sprscreen)
		{
			printf("Failed to create texture: %s\n", SDL_GetError());
			return false;
		}

		SDL::patscreen = SDL_CreateTexture(SDL::renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, PATTERN_WIDTH, PATTERN_HEIGHT);

		if (!SDL::patscreen)
		{
			printf("Failed to create texture: %s\n", SDL_GetError());
			return false;
		}

		//Enable transparency
		SDL_SetRenderDrawBlendMode(SDL::renderer, SDL_BLENDMODE_BLEND);

		SDL_initFramerate(&SDL::fpsman);
		//SDL_setFramerate(&SDL::fpsman, 60);

		return true;
	}

	void draw_frame(u32* screen_pixels, int state)
	{
		if (state == cstate::scanlines || state == cstate::cycles)
		{
			int x = lp.v & 0x1f;
			int y = (lp.v & 0x3e0) >> 5;

			SDL_SetRenderTarget(SDL::renderer, SDL::screen);
			SDL_Rect rect = { 0,  PPU::scanline + 1, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
			SDL_Rect rect2 = { PPU::cycle,  PPU::scanline, NES_SCREEN_WIDTH, 1 };
			SDL_UpdateTexture(SDL::screen, NULL, screen_pixels, NES_SCREEN_WIDTH * sizeof(unsigned int));
			SDL_RenderCopy(SDL::renderer, SDL::screen, NULL, NULL);
			draw_overlay(rect, rect2);
			SDL_SetRenderTarget(SDL::renderer, NULL);
		}
		else
		{
			//SDL_Rect dstrect = { 0, 25, 256 * 3,240 * 3 };
			SDL_UpdateTexture(SDL::screen, NULL, screen_pixels, NES_SCREEN_WIDTH * sizeof(unsigned int));
			SDL_RenderCopy(SDL::renderer, SDL::screen, NULL, NULL);
		}
	}

	void draw_nttable()
	{
		int x = 0, y = 0;
		int sx = (lp.v - 2 & 0x1f);
		int sy = (lp.v & 0x3e0) >> 5;

		SDL_SetRenderTarget(SDL::renderer, SDL::ntscreen);

		for (int i = 0; i < 4; i++)
		{
			SDL_Rect rect = { x, y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
			SDL_UpdateTexture(SDL::ntscreen, &rect, PPU::ntable_pixels[i], NES_SCREEN_WIDTH * sizeof(unsigned int));
			SDL_RenderCopy(SDL::renderer, SDL::ntscreen, NULL, NULL);

			x = 256;
			if (i == 1)
			{
				x = 0;
				y = 240;
			}
		}

		u8 nametable = (lp.t & 0xc00) >> 10;
		s16 fy = (lp.t & 0x7000) >> 12;
		s16 fx = (lp.fx & 7);
		u16 xp = (sx * 8 + fx) + (nametable & 1) * 256;
		u16 yp = (sy * 8 + fy) + ((nametable & 2) >> 1) * 240;
		SDL_Rect overlay = { xp, yp, 256, 240 };
		SDL_Rect overlay2 = { xp, yp, 256, 240 };

		draw_overlay(overlay, overlay2);

		if (xp + 256 >= 512)
		{
			overlay = { 0, yp, xp - 256, 240 };
			draw_overlay(overlay, overlay2);
		}

		if (yp + 240 >= 480)
		{
			overlay = { xp, 0, 256, yp - 240 };
			draw_overlay(overlay, overlay2);
		}

		if (xp + 256 >= 512 && xp + 256 >= 480)
		{
			overlay = { 0, 0, xp - 256, yp - 240 };
			draw_overlay(overlay, overlay2);
		}

		SDL_RenderCopy(SDL::renderer, SDL::ntscreen, NULL, NULL);

		SDL_SetRenderTarget(SDL::renderer, NULL);
	}

	void draw_sprites()
	{
		if (!SDL::renderer)
			return;

		SDL_SetRenderTarget(SDL::renderer, SDL::sprscreen);

		//SDL_Rect rect = { x, y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT };
		SDL_UpdateTexture(SDL::sprscreen, NULL, PPU::sprite_pixels, 256 * sizeof(unsigned int));
		SDL_RenderCopy(SDL::renderer, SDL::sprscreen, NULL, NULL);

		SDL_SetRenderTarget(SDL::renderer, NULL);
	}

	void draw_overlay(SDL_Rect rect, SDL_Rect rect2)
	{
		SDL_SetRenderDrawColor(SDL::renderer, 255, 255, 255, 80);
		SDL_RenderFillRect(SDL::renderer, &rect);
		//SDL_SetRenderDrawColor(SDL::renderer, 0, 255, 0, 255);
		SDL_RenderDrawRect(SDL::renderer, &rect2);
	}

	void input_new()
	{
		SDL::ctrl_keys = SDL_GetKeyboardState(NULL);
		newkeys.f1 = SDL::ctrl_keys[SDL_SCANCODE_F1];
		newkeys.f9 = SDL::ctrl_keys[SDL_SCANCODE_F9];
		newkeys.lshift = SDL::ctrl_keys[SDL_SCANCODE_LSHIFT];
	}

	void input_old()
	{
		oldkeys.f1 = newkeys.f1;
		oldkeys.f9 = newkeys.f9;
	}

	void clean()
	{
		if (SDL::sprscreen)
			SDL_DestroyTexture(SDL::sprscreen);

		if (SDL::ntscreen)
			SDL_DestroyTexture(SDL::ntscreen);

		if (SDL::patscreen)
			SDL_DestroyTexture(SDL::patscreen);

		if (SDL::controller)
			SDL_GameControllerClose(SDL::controller);

		if (SDL::screen)
			SDL_DestroyTexture(SDL::screen);

		if (SDL::renderer)
			SDL_DestroyRenderer(SDL::renderer);

		if (SDL::window)
			SDL_DestroyWindow(SDL::window);

		SDL_Quit();
	}
}