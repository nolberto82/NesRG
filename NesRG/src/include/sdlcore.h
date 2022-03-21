#pragma once

#include "types.h"

bool sdl_init();
void sdl_frame(u32* screen_pixels, int state);
void sdl_nttable();
void sdl_sprites();
void sdl_overlay(SDL_Rect rect, SDL_Rect rect2);
void sdl_input_new();
void sdl_input_old();
void sdl_clean();

struct Keys
{
	bool f1, lshift;
};

struct SdlCore
{
	const u8* ctrl_keys;
	bool frame_limit = true;

	FPSmanager fpsman;
	SDL_Texture* screen;
	SDL_Texture* ntscreen;
	SDL_Texture* patscreen[2];
	SDL_Texture* sprscreen;
	SDL_Renderer* renderer;
	SDL_Window* window;
	SDL_GameController* controller;
};

extern SdlCore sdl;
extern Keys newkeys, oldkeys;