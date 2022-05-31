#pragma once

#include "types.h"

namespace SDL
{
	bool init();
	void input_new();
	void input_old();
	void render_screen(GLuint texture, u32* pixels, float w, float h, float menubarheight);
	void update_nametable();
	void update_pattern();
	GLuint set_texture(u32* pixels, float w, float h);
	void clean();

	inline const u8* ctrl_keys;
	inline bool frame_limit = true;

	inline GLuint screen;
	inline GLuint nametable;
	inline GLuint pattern;

	inline const char* glsl_version;

	inline SDL_Window* window;
	inline SDL_GLContext context;
	inline SDL_GameController* controller;
}

struct Keys
{
	bool f1, f9, lshift;
};

extern Keys newkeys, oldkeys;