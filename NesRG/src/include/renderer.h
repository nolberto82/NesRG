#pragma once

#include "types.h"

bool render_init();
void render_frame(u32* screen_pixels, int state);
void render_nttable();
void render_sprites();
void render_overlay(SDL_Rect rect, SDL_Rect rect2);
void render_input();
void render_clean();

struct Renderer
{
	const u8* ctrl_keys;
	bool frame_limit = true;

	FPSmanager fpsman;
	SDL_Texture* screen;
	SDL_Texture* ntscreen;
	SDL_Texture* sprscreen;
	SDL_Renderer* renderer;
	SDL_Window* window;
};

extern Renderer gfx;