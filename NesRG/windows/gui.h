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

class Cpu;
class Memory;
class Ppu;
class Gfx;

class Gui
{
private:
	Cpu* cpu = nullptr;
	Memory* mem = nullptr;
	Ppu* ppu = nullptr;
	Gfx* gfx = nullptr;

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

public:
	Gui() {}
	~Gui()
	{
		ImGui_ImplSDLRenderer_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();
	}

	void set_obj(Cpu* cpu, Memory* mem, Ppu* ppu, Gfx* gfx)
	{
		this->cpu = cpu; this->mem = mem; this->ppu = ppu; this->gfx = gfx;
	}

	bool init();
	void update();
	void show_disassembly(ImGuiIO io);
	void show_buttons(ImGuiIO io);
	void show_memory();
	void show_breakpoints();
	void show_registers(ImGuiIO io);
	void show_menu();
	void step(bool stepping, bool over = false);
	void input(ImGuiIO io);
	void log_to_file(u16 pc);
	void create_close_log(bool status);
	vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers = false, bool memory_access = false);
};