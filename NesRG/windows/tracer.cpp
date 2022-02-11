#include "tracer.h"
#include "mem.h"
#include "ppu.h"

void log_to_file(u16 pc)
{
	char text[TEXTSIZE] = { 0 };
	vector<disasmentry> entry = get_trace_line(pc, true);

	for (const auto& e : entry)
	{
		outFile << e.line << '\n';
	}
}

void create_close_log(bool status)
{
	logging = status;

	if (logging)
	{
		outFile.open("cpu_trace.log");
		outFile << "FCEUX 2.6.1 - Trace Log File\n";
	}
	else
	{
		outFile.close();
	}
}

vector<disasmentry> get_trace_line(u16 pc, bool get_registers)
{
	u8 b[3] = { 0 };
	b[0] = rbd(pc);
	b[1] = rbd(pc + 1);
	b[2] = rbd(pc + 2);



	int size = 0;
	int asize = 0;
	const char* name;
	int mode;
	char data[TEXTSIZE] = { 0 };

	name = disasm[b[0]].name;
	size = disasm[b[0]].size;
	mode = disasm[b[0]].mode;

	switch (mode)
	{
		case addrmode::impl:
		case addrmode::accu:
		{
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], name);
			break;
		}
		case addrmode::imme:
		{
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], name, b[1]);
			break;
		}
		case addrmode::zerp:
		{
			u16 a = get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], name, b[1], rbd(a));
			break;
		}
		case addrmode::zerx:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], a + reg.x, rbd(a + reg.x));
			break;
		}
		case addrmode::zery:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], a + reg.y, rbd(a + reg.y));
			break;
		}
		case addrmode::abso:
		{
			u16 a = rw(pc + 1);
			bool isjump = b[0] == 0x4c || b[0] == 0x20;

			if (!isjump)
				snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2], name, a, rbd(a));
			else
				snprintf(data, TEXTSIZE, "%04X: %02X %02X %-3.02X %-3s $%04X", pc, b[0], b[1], b[2], name, a);
			break;
		}
		case addrmode::absx:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2],
				name, a, a + reg.x, rbd(a + reg.x));
			break;
		}
		case addrmode::absy:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2],
				name, a, a + reg.y, rbd(a + reg.y));
			break;
		}
		case addrmode::indx:
		{
			u16 a = get_indx(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], a, rbd(a));
			break;
		}
		case addrmode::indy:
		{
			u16 a = get_indy(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], a, rbd(a));
			break;
		}
		case addrmode::indi:
		{
			u16 a = rw(pc + 1);
			break;
		}
		case addrmode::rela:
		{
			u16 a = get_rela(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], name, a);
			break;
		}
		default:
			snprintf(data, TEXTSIZE, "%04X: FF %16s ", pc, "UNDEFINED");
			break;
	}

	vector<disasmentry> entry;
	disasmentry e;

	if (get_registers)
	{
		char flags[9] = { "........" };
		char temp[TEXTSIZE] = { 0 };

		flags[7] = reg.ps & FC ? 'C' : 'c'; flags[6] = reg.ps & FZ ? 'Z' : 'z';
		flags[5] = reg.ps & FI ? 'I' : 'i'; flags[4] = reg.ps & FD ? 'D' : 'd';
		flags[3] = reg.ps & FB ? 'B' : 'b'; flags[2] = reg.ps & FU ? 'U' : 'u';
		flags[1] = reg.ps & FV ? 'V' : 'v'; flags[0] = reg.ps & FN ? 'N' : 'n';

		snprintf(temp, TEXTSIZE, "A:%02X X:%02X Y:%02X S:%02X P:%s",
			reg.a, reg.x, reg.y, reg.sp, flags);

		e.line = temp;
		e.line += "  $";
	}

	e.line += data;
	e.size = size;
	entry.push_back(e);

	return entry;
}

stringstream format_string(int width, u8* bytes)
{
	stringstream ss;

	ss << setw(2) << hex << setfill('0') << uppercase;
	return ss;
}