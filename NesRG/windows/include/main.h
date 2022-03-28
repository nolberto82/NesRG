#pragma once

#include "types.h"

void main_update();
void main_step();
void main_step_over();
void main_step_frame();
void main_step_scanline(u16 lines);
void main_save_state(u8 slot);
void main_load_state(u8 slot);
void set_spacing(int count);
