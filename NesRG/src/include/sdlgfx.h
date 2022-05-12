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

	inline const u8* ctrl_keys;
	inline bool frame_limit = true;

	inline FPSmanager fpsman;
	inline SDL_Texture* screen;
	inline SDL_Texture* ntscreen;
	inline SDL_Texture* patscreen;
	inline SDL_Texture* sprscreen;
	inline SDL_Renderer* renderer;
	inline SDL_Window* window;
	inline SDL_GameController* controller;
}

struct Keys
{
	bool f1, f9, lshift;
};

extern Keys newkeys, oldkeys;