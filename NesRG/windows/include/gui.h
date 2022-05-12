#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_memory_editor.h"
#include "imfilebrowser.h"

#define DISASSEMBLY_LINES 30

//Color defines
#define BUTTON_W 90
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
#define MAIN_WINDOW ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_MenuBar | \
ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | \
ImGuiWindowFlags_NoMove

namespace GUI
{
	inline u16 jumpaddr;

	inline bool releasemode;

	inline int lineoffset;
	inline int running;
	inline int item_num = 0;

	inline bool is_jump;
	inline bool is_pc = false;

	inline bool style_editor = false;
	inline bool trace_logger = false;
	inline bool mem_viewer = false;
	inline bool debug_viewer = false;
	inline bool emu_rom = false;
	inline bool emu_run = false;
	inline bool emu_reset = false;
	inline bool debug_enable = false;
	inline bool ppu_enable = false;

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

	void update(ImGuiIO io);
	void show_display();
	void show_ppu_debug();
	void show_disassembly();
	void show_memory();
	void show_registers();
	void show_breakpoints();
	void show_buttons();
	void show_logger();
	void show_menu();
	void open_dialog();
	void run_emu();
	void reset_emu();
	bool init();
}