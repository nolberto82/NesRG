#include "sdlgfx.h"

struct stext otext[MAX_LETTERS];

bool SDLGfx::init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Nes RG", 10, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0);

	if (!window)
	{
		printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	//SDL_Surface* surface = IMG_Load("assets/emu-font.bmp");

	//if (surface == NULL)
	//{
	//	printf("Unable to load image! SDL_image Error: %s\n", IMG_GetError());
	//	return false;
	//}

	//int height = surface->h / 8;
	//int width = surface->w / 8;

	//int n = 0;

	//for (int y = 0; y < height; y++)
	//{
	//	for (int x = 0; x < width; x++)
	//	{
	//		otext[n].x = x * 8;
	//		otext[n].y = y * 8;
	//		otext[n].w = 8;
	//		otext[n].h = 8;
	//		n++;
	//	}
	//}

	//SDL_initFramerate(&fpsman);
	//SDL_setFramerate(&fpsman, 60);

	//create textures
	display.w = 256; display.h = 240;
	tile.w = 8; tile.h = 8;

	display.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, display.w, display.h);
	tile.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, display.w, display.h);

	return true;
}

void SDLGfx::set_gui_callback(std::function<void()> function)
{
	debug_gui = function;
}

void SDLGfx::update()
{
}

void SDLGfx::input()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
				running = 0;
				break;
		}
	}
}

void SDLGfx::draw_string(const char* text, int x, int y, int size, SDL_Color c)
{
	//for (; *text; text++)
	//{
	//	u8 ch = *text;

	//	if (ch == '\n')
	//		break;


	//	if (ch == ' ')
	//	{
	//		x += size;
	//		continue;
	//	}

	//	if (ch >= 0 && ch <= 9)
	//		continue;

	//	SDL_Rect rsrc = { otext[ch].x,otext[ch].y, 8, 8 };
	//	SDL_Rect rdst = { x, y, size, size };
	//	SDL_RendererFlip flip{};
	//	SDL_SetTextureColorMod(font, c.r, c.g, c.b);
	//	SDL_RenderCopyEx(renderer, gfx.font, &rsrc, &rdst, 1, NULL, flip);
	//	//SDL_RenderCopy(gfx.renderer, gfx.font, &rsrc, &rdst);
	//	x += size;
	//}

	//SDL_SetRenderTarget(renderer, NULL);
	//SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
}

void SDLGfx::render_frame()
{
	//SDL_Rect rect = { 0, -24, 256, 240 };
	SDL_UpdateTexture(display.texture, NULL, disp_pixels, display.w * sizeof(unsigned int));
	SDL_RenderCopy(renderer, display.texture, NULL, NULL);
	//SDL_RenderPresent(renderer);
}

void SDLGfx::clean()
{
	SDL_DestroyTexture(tile.texture);
	SDL_DestroyTexture(display.texture);
	SDL_DestroyTexture(font);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}
