#pragma once

#define SDL_MAIN_HANDLED

#define MAX_LETTERS 128

#include "types.h"

#include <SDL.h>
//#include <SDL_image.h>
//#include <SDL2_framerate.h>

struct texture_t
{
	int w;
	int h;
	SDL_Texture* texture;
};

struct stext
{
	int x, y, w, h;
};

struct SDLGfx
{
public:
	bool init();
	void set_gui_callback(std::function<void()> function);
	void update();
	void input();
	void draw_string(const char* text, int x, int y, int size, SDL_Color c);
	void render_frame();
	void clean();

	bool running = 0;

	std::function<void()> debug_gui;

	texture_t display = {};
	texture_t tile = {};
	u8 disp_pixels[256 * 240 * 4];
	u32 tile_pixels[8 * 8];

	//FPSmanager fpsman;
	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
	SDL_Texture* font = nullptr;

private:

};

extern SDLGfx gfx;