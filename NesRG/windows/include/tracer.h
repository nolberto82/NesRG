#pragma once

#include "types.h"

typedef struct
{
	string line;
	u8 size;
	u16 pc;
	bool isjump;
}disasmentry;

inline bool logging;

inline ofstream outFile;

void log_to_file(u16 pc);
void create_close_log(bool status);
vector<disasmentry> get_trace_line(u16 pc, bool get_registers, bool get_cycles);
