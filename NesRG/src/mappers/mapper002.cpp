#include "mappers.h"
#include "mem.h"

void Mapper002::setup()
{
	int prgsize = header.prgnum * 0x4000;
	int chrsize = header.chrnum * 0x2000;

	memcpy(&MEM::ram[0x8000], MEM::rom.data() + 0x10, prgsize / header.prgnum);
	memcpy(&MEM::ram[0xc000], MEM::rom.data() + 0x10 + prgsize - (prgsize / header.prgnum), prgsize / header.prgnum);
	if (header.chrnum > 0)
	{
		memcpy(&MEM::vram[0x0000], MEM::vrom.data(), chrsize / header.chrnum);
	}
}

void Mapper002::update()
{
}

void Mapper002::wb(u16 addr, u8 v)
{
	if (addr >= 0xc000 && addr <= 0xffff)
	{
		int prg = 0x10 + 0x4000 * (v & 7);
		MEM::mem_rom(MEM::ram, 0x8000, prg, 0x4000);
	}
}

u8 Mapper002::rb(u16 addr)
{
	return u8();
}

void Mapper002::reset()
{
	prg.resize(2);
	chr.resize(2);
}

void Mapper002::scanline()
{
}

vector<u8> Mapper002::get_prg()
{
	return prg;
}

vector<u8> Mapper002::get_chr()
{
	return chr;
}
