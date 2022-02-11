#pragma once

#include "types.h"

typedef struct
{
	std::string line;
	int size;
}disasmentry;

inline bool logging;

inline ofstream outFile;

void log_to_file(u16 pc);
void create_close_log(bool status);
vector<disasmentry> get_trace_line(u16 pc, bool get_registers);
stringstream format_string(int width, u16 adrr, u8* bytes);
