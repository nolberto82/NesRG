#pragma once

#include "types.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "imgui_memory_editor.h"
#include "imfilebrowser.h"

//Color defines
#define TEXTSIZE 512
#define BUTTONSIZE_X 120
#define RED ImVec4(1, 0, 0, 1)
#define GREEN ImVec4(0, 1, 0, 1)
#define BLUE ImVec4(0, 0, 1, 1)
#define LIGHTGRAY ImVec4( 0xd0 / 255.0f, 0xd0 / 255.0f, 0xd0 / 255.0f , 1)

//GUI defines
#define DEBUG_W 400
#define DEBUG_H 450
#define DEBUG_X 5
#define DEBUG_Y 25

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

inline u16 inputaddr;

inline int lineoffset;
inline int item_id;
inline int gui_running;

inline bool logging;
inline bool stepping;
inline bool is_jump;

inline ofstream outFile;

inline vector<disasmentry> vdentry;
inline vector<string> nesfiles;

bool gui_init();
void gui_update();
void gui_show_disassembly(ImGuiIO io);
void gui_show_buttons(ImGuiIO io);
void gui_show_memory();
void gui_show_breakpoints();
void gui_show_registers(ImGuiIO io);
void gui_show_menu();
void gui_step(bool stepping, bool over = false);
void gui_input(ImGuiIO io);
void log_to_file(u16 pc);
void create_close_log(bool status);
vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers = false, bool memory_access = false);
void gui_clean();