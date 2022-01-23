#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_sdl.h"

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
	std::string looptext;
	int size;
}disasmentry;

struct Debugger
{
public:
	bool init();
	void update();
	void show_disassembly();
	void show_breakpoints();
	void clean();

private:
	int lineoffset = 0;
	int item_id = 0;

	bool logging = false;
	bool stepping = false;

	void show_registers();
	void show_buttons(u16& inputaddr, bool& is_jump, ImGuiIO io);
	void input(ImGuiIO io);
	void step(bool stepping = false);
	std::vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers = false);
	disasmdata get_disasm_entry(u8 op, u16 pc);
};