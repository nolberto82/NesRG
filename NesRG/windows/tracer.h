#pragma once

#include "types.h"

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

inline bool logging;

inline ofstream outFile;

inline vector<disasmentry> vdentry;

void log_to_file(u16 pc);
void create_close_log(bool status);
vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers = false, bool memory_access = false);