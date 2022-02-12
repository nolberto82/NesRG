#pragma once

#include "types.h"

bool render_init();
void render_frame(u32* screen_pixels);
void render_nttable();
void render_overlay(SDL_Rect rect);
void render_input();
void render_clean();

struct Renderer
{
	const u8* ctrl_keys;
	bool frame_limit = true;

	FPSmanager fpsman;
	SDL_Texture* screen;
	SDL_Texture* ntscreen;
	SDL_Renderer* renderer;
	SDL_Window* window;
};

extern Renderer gfx;