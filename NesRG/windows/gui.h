#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_memory_editor.h"
#include "imfilebrowser.h"

//Color defines
#define BUTTON_W 120
#define BUTTON_H 35
#define RED ImVec4(1, 0, 0, 1)
#define GREEN ImVec4(0, 1, 0, 1)
#define BLUE ImVec4(0, 0, 1, 1)
#define LIGHTGRAY ImVec4( 0xd0 / 255.0f, 0xd0 / 255.0f, 0xd0 / 255.0f , 1)

//Gui defines
#define DEBUG_W 500
#define DEBUG_H 450
#define DEBUG_X 5
#define DEBUG_Y 25
#define MEM_W 550
#define MEM_H 350

//ImGui flags
#define INPUT_FLAGS ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase

enum bptype
{
	bp_read = 1,
	bp_write = 2,
	bp_exec = 4,
	bp_vread = 8,
	bp_vwrite = 16,
};

enum bpaddtype
{
	add_bp,
	add_edit,
};

inline u16 inputaddr;

inline int lineoffset;
inline int gui_running;

inline bool stepping;
inline bool is_jump;

bool gui_init();
void gui_update();
void gui_show_disassembly(ImGuiIO io);
void gui_show_buttons(ImGuiIO io);
void gui_show_memory();
void gui_show_breakpoints();
void gui_show_registers(ImGuiIO io);
void gui_show_menu();
void gui_step(bool stepping, bool over = false);
void gui_step_over();
void gui_input(ImGuiIO io);
void gui_clean();