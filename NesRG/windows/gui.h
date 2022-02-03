#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_memory_editor.h"
#include "imfilebrowser.h"

#define TEXTSIZE 512
#define BUTTONSIZE_X 120

enum bptype
{
	bp_read = 1,
	bp_write = 2,
	bp_exec = 4
};

enum bpaddtype
{
	add_bp,
	add_edit,
};

typedef struct
{
	u16 offset;
	std::string name;
	std::string oper;
	std::string pctext;
	std::string regtext;
	std::string dtext;
	std::string bytetext;
	std::string cycles;
	int size;
}disasmentry;

struct Gui
{
	int lineoffset = 0;
	int item_id = 0;
	u16 inputaddr = 0;
	int running = 0;

	bool logging = false;
	bool stepping = false;
	bool is_jump = false;

	ofstream outFile;

	vector<disasmentry> vdentry;
	vector<string> nesfiles;

	Gui(){}
	~Gui() 
	{
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL_DestroyTexture(display.texture);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		SDL_Quit();
	}

	bool init_gui();
	void update_gui();
	void show_disassembly(ImGuiIO io);
	void show_buttons(ImGuiIO io);
	void show_memory();
	void show_breakpoints();
	void show_registers(ImGuiIO io);
	void show_menu();
	void step_gui(bool stepping, bool over = false);
	void input(ImGuiIO io);
	void render_frame();
	void log_to_file(u16 pc);
	void create_close_log(bool status);
	vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers = false, bool memory_access = false);

	texture_t display = {};
	u32 disp_pixels[256 * 240];
	u32 tile_pixels[8 * 8];

	SDL_Renderer* renderer = nullptr;
	SDL_Window* window = nullptr;
};

extern Gui gui;