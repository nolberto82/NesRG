#include "tracer.h"
#include "mem.h"
#include "ppu.h"

void log_to_file(u16 pc)
{
	char text[TEXTSIZE] = { 0 };
	vector<disasmentry> entry = get_trace_line(pc, true, false);

	for (const auto& e : entry)
		outFile << e.line << '\n';
}

void create_close_log(bool status)
{
	logging = status;

	if (logging)
		outFile.open("cpu_trace.log"); //outFile << "FCEUX 2.6.1 - Trace Log File\n";
	else
		outFile.close();
}

vector<disasmentry> get_trace_line(u16 pc, bool get_registers, bool get_cycles)
{
	u8 b[3];
	b[0] = rbd(pc);
	b[1] = rbd(pc + 1);
	b[2] = rbd(pc + 2);

	int size = 0;
	const char* name;
	int mode;
	char data[TEXTSIZE] = { 0 };
	bool jump = false;

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
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], name, b[1], rbd(a));
			break;
		}
		case addrmode::zerx:
		{
			u16 a = rbd(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], (u8)(a + reg.x), rbd((u8)(a + reg.x)));
			break;
		}
		case addrmode::zery:
		{
			u16 a = rbd(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], (u8)(a + reg.y), rbd((u8)(a + reg.y)));
			break;
		}
		case addrmode::abso:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2], name, a, rbd(a));
			break;
		}
		case addrmode::absx:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2],
				name, a, (u16)(a + reg.x), rbd((u16)(a + reg.x)));
			break;
		}
		case addrmode::absy:
		{
			u16 a = rw(pc + 1);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2],
				name, a, (u16)(a + reg.y), rbd((u16)(a + reg.y)));
			break;
		}
		case addrmode::indx:
		{
			u16 a = ((b[1] + reg.x) << 8) | (u8)(b[1] + 1 + reg.x);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], a, rbd(a));
			break;
		}
		case addrmode::indy:
		{
			u16 a = rw(b[1]) + reg.y;
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, b[1], a, rbd(a));
			break;
		}
		case addrmode::indi:
		{
			u16 a = rw(pc + 1);
			if ((a & 0xff) == 0xff) ++a;
			else a = rw(a);
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1], b[2],
				name, b[2] << 8 | b[1], a, rbd(a));
			break;
		}
		case addrmode::rela:
		{
			u16 a = pc + (s8)(b[1]) + 2;
			snprintf(data, TEXTSIZE, mode_formats[mode], pc, b[0], b[1],
				name, a, rbd(a));
			jump = true;
			break;
		}
		default:
			snprintf(data, TEXTSIZE, "%04X FF %16s ", pc, "UNDEFINED");
			break;
	}

	vector<disasmentry> entry;
	disasmentry e;
	e.pc = pc;
	e.line += data;
	e.size = size;
	e.isjump = jump;

	if (get_registers)
	{
		char temp[TEXTSIZE] = { 0 };
		char str[TEXTSIZE] = { 0 };
		snprintf(temp, TEXTSIZE, "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%-3d SL:%-3d CPU Cycle:%d",
			reg.a, reg.x, reg.y, reg.ps, reg.sp, ppu.cycle, ppu.scanline, ppu.totalcycles);

		string t(data);
		t.erase(5, 10);
		int ind = t.find('=');
		if (ind > 0)
			t.erase(ind, 5);

		snprintf(str, TEXTSIZE, "%-48s %s", t.data(), temp);
		e.line = str;
	}
	else if (!get_registers)
		e.line.erase(0, 5);

	entry.push_back(e);
	return entry;
}