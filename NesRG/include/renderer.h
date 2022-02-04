#pragma once

#include "types.h"

class Gfx
{
public:
	Gfx() {}
	~Gfx()
	{
		SDL_DestroyTexture(screen);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		SDL_Quit();
	}

	bool init();
	void render_frame(u32* screen_pixels);
	
	//static SDL_Texture* get_screen_texture() { return screen; };
	//static SDL_Renderer* get_renderer() { return renderer; };
	//static SDL_Window* get_window() { return window; };

	static SDL_Texture* screen;
	static SDL_Renderer* renderer;
	static SDL_Window* window;
};