#pragma once

#include "types.h"

namespace SDL
{
	bool init();
	void draw_frame(u32* screen_pixels, int state);
	void draw_nttable();
	void draw_sprites();
	void draw_overlay(SDL_Rect rect, SDL_Rect rect2);
	void input_new();
	void input_old();
	void clean();
}

struct Keys
{
	bool f1, f9, lshift;
};

struct SdlGfx
{
	const u8* ctrl_keys;
	bool frame_limit = true;

	FPSmanager fpsman;
	SDL_Texture* screen;
	SDL_Texture* ntscreen;
	SDL_Texture* patscreen[3];
	SDL_Texture* sprscreen;
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_GameController* controller;
};

extern SdlGfx sdl;
extern Keys newkeys, oldkeys;