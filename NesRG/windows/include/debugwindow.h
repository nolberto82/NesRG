#pragma once

#include "types.h"

#include "gui.h"

void debug_update();
void debug_show_disassembly(ImGuiIO io);
void debug_show_breakpoints();
void debug_show_memory();
void debug_create_textbox(string l, string id, u16 regval, int width);
void debug_bp_buttons(string l, u8 bp, u8* type, float width);
void debug_render_memory(MemoryEditor mem_edit, vector<u8> mem);
void debug_create_reg_text(string regname, string format, string format2, u16 regval);
void debug_show_registers(ImGuiIO io);
void debug_show_buttons(ImGuiIO io);