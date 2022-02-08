#include "tracer.h"
#include "mem.h"
#include "ppu.h"

void log_to_file(u16 pc)
{
	char text[TEXTSIZE] = { 0 };
	vdentry = get_trace_line(text, pc, true, true);

	//ofstream outFile("cpu_trace.log", ios_base::app);
	for (const auto& e : vdentry)
	{
		outFile
			<< uppercase << hex << setw(4) << setfill('0') << e.offset
			<< "  "
			<< e.bytetext
			<< " "
			<< left << setfill(' ') << setw(32)
			<< e.dtext
			<< e.regtext
			<< "\n";
	}
}

void create_close_log(bool status)
{
	logging = status;

	if (logging)
	{
		outFile.open("cpu_trace.log");
		//outFile << "FCEUX 2.6.1 - Trace Log File\n";
	}
	else
	{
		outFile.close();
	}
}

vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers, bool memory_access)
{
	u8 op = rbd(pc);

	int size = 0;
	int asize = 0;
	const char* name;
	int mode;
	char line[TEXTSIZE] = { 0 };
	char bytes[TEXTSIZE] = { 0 };
	char* data = (char*)text;

	vector<disasmentry> entry;

	name = disasm[op].name;
	size = disasm[op].size;
	mode = disasm[op].mode;

	disasmentry e;

	switch (mode)
	{
		case addrmode::impl:
		case addrmode::accu:
		{
			snprintf(data, TEXTSIZE, "%s", name);
			snprintf(bytes, TEXTSIZE, "%-9.02X", rbd(pc));
			break;
		}
		case addrmode::imme:
		{
			u16 b = get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);
			snprintf(data + strlen(data), TEXTSIZE, "#$%02X", b);
			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::zerp:
		{
			u16 b = get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);
			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X = %02X", b, rbd(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X", b);
			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::zerx:
		{
			u16 b = get_zerx(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,X @ %04X = %02X", d2, (u8)(d2 + reg.x), rbd((u8)(d2 + reg.x)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,X", b);

			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", d1, d2);
			break;
		}
		case addrmode::zery:
		{
			u16 b = get_zery(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,Y @ %04X = %02X", d2, (u8)(d2 + reg.y), rbd((u8)(d2 + reg.y)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,Y", d2);

			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", d1, d2);
			break;
		}
		case addrmode::abso:
		{
			u16 b = rw(pc + 1);
			bool isjump = op == 0x4c || op == 0x20;
			bool isspecial = b >= 0x2000 && b < 0x2008;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access && !isjump)
			{
				if (isspecial)
					snprintf(data + strlen(data), TEXTSIZE, "$%04X = FF", b);
				else
					snprintf(data + strlen(data), TEXTSIZE, "$%04X = %02X", b, ram[b]);
			}
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X", b);

			snprintf(bytes, TEXTSIZE, "%02X %02X %-3.02X", ram[pc], ram[pc + 1], ram[pc + 2]);
			break;
		}
		case addrmode::absx:
		{
			u16 b = get_absx(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			u8 d3 = rbd(pc + 2);
			u16 a = d3 << 8 | d2;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,X @ %04X = %02X", a, (u16)a + reg.x, rbd((u16)(a + reg.x)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,X", a);

			snprintf(bytes, TEXTSIZE, "%02X %02X %-3.02X", d1, d2, d3);
			break;
		}
		case addrmode::absy:
		{
			u16 b = get_absy(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			u8 d3 = rbd(pc + 2);
			u16 a = d3 << 8 | d2;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,Y @ %04X = %02X", a, (u16)(a + reg.y), rbd(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,Y", a);

			snprintf(bytes, TEXTSIZE, "%02X %02X %-3.02X", d1, d2, d3);
			break;
		}
		case addrmode::indx:
		{
			u16 b = get_indx(pc + 1, true);
			u8 d1 = rbd((u8)(b + reg.x));
			u8 d2 = rbd((u8)(b + 1 + reg.x));
			u16 a = d2 << 8 | d1;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%02X,X) @ %04X = %02X", b, a, rbd(a));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%02X,X)", b);

			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::indy:
		{
			u16 b = get_indy(pc + 1, true);
			u8 d1 = rbd((u8)b);
			u8 d2 = rbd((u8)(b + 1));
			u16 a = (u16)((d2 << 8 | d1) + reg.y);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%02X),Y @ %04X = %02X", (u8)b, a, rbd(a));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%02X),Y", b);

			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::indi:
		{
			u16 b = rw(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			u8 d3 = rbd(pc + 2);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%04X) = %04X", b, rw(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%04X)", b);

			snprintf(bytes, TEXTSIZE, "%02X %02X %-3.02X", d1, d2, d3);
			break;
		}
		case addrmode::rela:
		{
			u8 b1 = rbd(pc + 1);
			u16 b = pc + (s8)(b1)+2;
			snprintf(data, TEXTSIZE, "%-4s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04X", b);
			snprintf(bytes, TEXTSIZE, "%02X %-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		default:
			snprintf(data, TEXTSIZE, "%-4s", "UNDEFINED");
			snprintf(bytes, TEXTSIZE, "%-9s", "nn");
			break;
	}



	if (get_registers)
	{
		char align[42] = { 0 };
		char temp[TEXTSIZE] = { 0 };

		char flags[9] = { "........" };
		char text[32] = "";

		flags[7] = reg.ps & FC ? 'C' : 'c';
		flags[6] = reg.ps & FZ ? 'Z' : 'z';
		flags[5] = reg.ps & FI ? 'I' : 'i';
		flags[4] = reg.ps & FD ? 'D' : 'd';
		flags[3] = reg.ps & FB ? 'B' : 'b';
		flags[2] = reg.ps & FU ? 'U' : 'u';
		flags[1] = reg.ps & FV ? 'V' : 'v';
		flags[0] = reg.ps & FN ? 'N' : 'n';

		//snprintf(temp, TEXTSIZE, "c%-12d", ppu_totalcycles);
		//e.regtext += temp;

		snprintf(temp, TEXTSIZE, "A:%02X X:%02X Y:%02X P:%02X SP:%02X PPU:%3d,%3d CYC:%d",
			reg.a, reg.x, reg.y, reg.ps, reg.sp, cycle, scanline, totalcycles);

		e.regtext += temp;
	}

	e.name = name;
	e.offset = pc;
	e.size = size;
	e.dtext = data;
	e.bytetext = bytes;
	//e.cycles = cycles;
	entry.push_back(e);

	return entry;
}