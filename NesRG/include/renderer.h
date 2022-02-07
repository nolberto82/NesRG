#pragma once

#include "types.h"

bool render_init();
void render_frame(u32* screen_pixels);
void render_input();
void render_clean();

inline const u8* ctrl_keys;

inline FPSmanager fpsman;
inline SDL_Texture* screen;
inline SDL_Renderer* renderer;
inline SDL_Window* window;