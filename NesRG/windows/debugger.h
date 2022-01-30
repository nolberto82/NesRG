#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_memory_editor.h"

#define TEXTSIZE 512

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

struct Debugger
{
	int lineoffset = 0;
	int item_id = 0;
	u16 inputaddr = 0;

	bool logging = false;
	bool stepping = false;
	bool is_jump = false;

	ofstream outFile;

	vector<disasmentry> vdentry;
	vector<string> nesfiles;

	bool init();
	void update();
	void show_disassembly(ImGuiIO io);
	void show_buttons();
	void show_memory();
	void show_breakpoints();
	void show_registers(ImGuiIO io);
	void show_menu();
	void show_roms();
	void step(bool stepping);
	void input(ImGuiIO io);
	void log_to_file(u16 pc);
	void create_close_log(bool status);
	vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers = false);
	void get_rom_files();
	void clean();
};