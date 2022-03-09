#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_memory_editor.h"
#include "imfilebrowser.h"

#define DISASSEMBLY_LINES 60

//Color defines
#define BUTTON_W 120
#define BUTTON_H 35
#define RED ImVec4(1, 0, 0, 1)
#define GREEN ImVec4(0, 1, 0, 1)
#define BLUE ImVec4(0, 0, 1, 1)
#define LIGHTGREEN ImVec4(0x90 / 255.0f, 0xee / 255.0f, 0x90 / 255.0f , 1)
#define FRAMEACTIVE ImVec4(0.260f, 0.590f, 0.980f, 0.670f)
#define ALICEBLUE ImVec4( 0xf0 / 255.0f, 0xf8 / 255.0f, 0xff / 255.0f , 1)
#define LIGHTGRAY ImVec4( 0x7f / 255.0f, 0x7f / 255.0f, 0x7f / 255.0f , 1)
#define DEFCOLOR ImVec4(0.260f, 0.590f, 0.980f, 0.400f)

//ImGui flags
#define INPUT_FLAGS ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase
#define INPUT_ENTER INPUT_FLAGS | ImGuiInputTextFlags_EnterReturnsTrue
#define MAIN_WINDOW ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | \
ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_MenuBar | \
ImGuiWindowFlags_NoBringToFrontOnFocus

inline u16 jumpaddr;

inline int lineoffset;
inline int gui_running;

inline bool is_jump;

inline int item_num = 0;
inline bool is_pc = false;

inline bool style_editor = false;
inline bool trace_logger = false;

inline string flag_names = "NVUBDIZC";
inline bool flag_values[8] = { };
inline char jumpto[5] = { 0 };

inline bool follow_pc;

inline u16 inputaddr;
inline char bpaddrtext[5] = { 0 };
inline int item_id = 0;
inline u8 bptype = 0;

inline MemoryEditor mem_edit;

inline ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

void gui_update();
void gui_show_display();
void gui_show_disassembly();
void gui_show_memory();
void gui_show_registers();
void gui_show_rom_info();
void gui_show_breakpoints();
void gui_show_logger();
void gui_show_buttons();
void gui_show_menu();
void gui_open_dialog();