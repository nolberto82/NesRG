#pragma once

#define SDL_MAIN_HANDLED

#include "types.h"

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL2_framerate.h>



struct SDLGfx
{
public:
	bool init();
	void update();
	void input();
	void begin_frame();
	void end_frame();
	void clean();

	SDL_Window* get_window() { return window; }
	SDL_Renderer* get_renderer() { return renderer; }

	bool running = 0;

private:

	//FPSmanager fpsman;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Texture* font = nullptr;
};

extern SDLGfx gfx;