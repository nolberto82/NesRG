#include "mappers.h"
#include "mem.h"

void Mapper003::setup()
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

void Mapper003::update()
{
}

void Mapper003::wb(u16 addr, u8 v)
{
	chr[0] = v & 3;
	MEM::mem_vrom(MEM::vram, 0x0000, chr[0] * chrbank, chrbank);
}

u8 Mapper003::rb(u16 addr)
{
	return u8();
}

void Mapper003::reset()
{
	chrbank = 0x2000;
	prg.resize(1);
	chr.resize(1);
}

void Mapper003::scanline()
{
}

vector<u8> Mapper003::get_prg()
{
	return prg;
}

vector<u8> Mapper003::get_chr()
{
	return chr;
}
