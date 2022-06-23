#include "mappers.h"
#include "mem.h"

void Mapper007::setup()
{
	int prgsize = header.prgnum * 0x4000;
	int chrsize = header.chrnum * 0x2000;

	memcpy(&MEM::ram[0x8000], MEM::rom.data() + 0x10, prgsize / header.prgnum * 2);
	//memcpy(&MEM::ram[0xc000], MEM::rom.data() + 0x10 + prgsize - (prgsize / header.prgnum), prgsize / header.prgnum);
	if (header.chrnum > 0)
	{
		memcpy(&MEM::vram[0x0000], MEM::vrom.data(), chrsize / header.chrnum);
	}
}

void Mapper007::update()
{
}

void Mapper007::wb(u16 addr, u8 v)
{
	prg[0] = v & 7;
	MEM::mem_rom(MEM::ram, 0x8000,  prg[0] * prgbank * 2 + 0x10, prgbank * 2);
	header.mirror = ((v >> 4) & 1);
}

u8 Mapper007::rb(u16 addr)
{
	return u8();
}

void Mapper007::reset()
{
	prgbank = 0x4000;
	chrbank = 0x2000;
	prg.resize(1);
	chr.resize(1);
}

void Mapper007::scanline()
{
}

vector<u8> Mapper007::get_prg()
{
	return prg;
}

vector<u8> Mapper007::get_chr()
{
	return chr;
}
