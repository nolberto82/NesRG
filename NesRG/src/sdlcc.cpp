#include "sdlcc.h"
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

		SDL_WindowFlags flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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

		if (!SDL::window)
		{
			printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
			return false;
		}

		// GL 3.0 + GLSL 130
		SDL::glsl_version = "#version 130";
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

		// Create window with graphics context
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

		SDL::context = SDL_GL_CreateContext(SDL::window);
		if (!SDL::context)
			return false;

		SDL_GL_MakeCurrent(SDL::window, SDL::context);
		SDL_GL_SetSwapInterval(1); // Enable vsync

		screen = set_texture(PPU::screen_pix.data(), 256, 240);
		nametable = set_texture(PPU::ntable_pix.data(), 512, 480);
		pattern = set_texture(PPU::patt_pix[0].data(), 128, 128);

		return true;
	}
	void input_new()
	{
		ctrl_keys = SDL_GetKeyboardState(NULL);
		newkeys.f1 = ctrl_keys[SDL_SCANCODE_F1];
		newkeys.f2 = ctrl_keys[SDL_SCANCODE_F2];
		newkeys.f9 = ctrl_keys[SDL_SCANCODE_F9];
		newkeys.lshift = ctrl_keys[SDL_SCANCODE_LSHIFT];
	}

	void input_old()
	{
		oldkeys.f1 = newkeys.f1;
		oldkeys.f2 = newkeys.f2;
		oldkeys.f9 = newkeys.f9;
	}

	void render_screen_debug(GLuint texture, u32* pixels, float w, float h, float menubarheight)
	{
		double vasp = w / h;
		glViewport(0, 0, w, h - menubarheight);

		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex2f(0.0 + w, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex2f(0.0 + w, 0.0 + h / vasp);
			glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 0.0 + h / vasp);
		}
		glEnd();
	}

	void render_screen(GLuint texture, u32* pixels, float w, float h, float menubarheight)
	{
		glViewport(0, 0, w, h - menubarheight);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, w, h, 0, -1, 1);

		glClear(GL_COLOR_BUFFER_BIT);

		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 240, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		glBegin(GL_QUADS);
		{
			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
			glTexCoord2f(1.0, 0.0); glVertex2f(0.0 + w, 0.0);
			glTexCoord2f(1.0, 1.0); glVertex2f(0.0 + w, 0.0 + h);
			glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 0.0 + h);
		}
		glEnd();
	}

	void update_nametable()
	{
		for (int i = 0; i < 4; i++)
		{
			glBindTexture(GL_TEXTURE_2D, nametable);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, PPU::ntable_pix.data());
		}
	}

	void update_pattern()
	{
		for (int i = 0; i < 4; i++)
		{
			glBindTexture(GL_TEXTURE_2D, nametable);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, PPU::patt_pix[i].data());
		}
	}

	GLuint set_texture(u32* pixels, float w, float h)
	{
		GLuint texture;
		glEnable(GL_TEXTURE_2D);
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		return texture;
	}

	void clean()
	{
		glDeleteTextures(1, &screen);
		glDeleteTextures(1, &nametable);
		glDeleteTextures(1, &pattern);

		SDL_GL_DeleteContext(context);

		if (renderer)
			SDL_DestroyRenderer(renderer);

		if (window)
			SDL_DestroyWindow(window);

		SDL_Quit();
	}
}

