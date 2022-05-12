#include "mappers.h"
#include "mem.h"

void Mapper009::setup(struct Header h)
{
	int prgsize = h.prgnum * 0x4000;
	int chrsize = h.chrnum * 0x2000;

	memcpy(&MEM::ram[0xa000], MEM::rom.data() + 0x10 + prgsize - 0x6000, prgsize / h.prgnum / 2);
	memcpy(&MEM::ram[0xc000], MEM::rom.data() + 0x10 + prgsize - (prgsize / h.prgnum), prgsize / h.prgnum);
	if (chrbank > 0)
	{
		memcpy(&MEM::vram[0x0000], MEM::vrom.data() + 0x8000, chrsize / h.chrnum / 2);
		memcpy(&MEM::vram[0x1000], MEM::vrom.data() + 0x7000, chrsize / h.chrnum / 2);
	}
}

void Mapper009::update(u16 addr, u8 v)
{
	if (addr >= 0xa000 && addr <= 0xafff)
	{
		prg[0] = v & 0xf;
		prg[1] = (header.prgnum * 2) - 3;
		prg[2] = (header.prgnum * 2) - 2;
		prg[3] = (header.prgnum * 2) - 1;
		for (int i = 0; i < prg.size(); i++)
			MEM::mem_rom(MEM::ram, 0x8000 + i * 0x2000, 0x10 + prg[i] * 0x2000, 0x2000);
	}
	else if (addr >= 0xb000 && addr <= 0xbfff)
	{
		chr1[0] = v & 0x1f;
		MEM::mem_vrom(MEM::vram, 0x0000, chr1[latch1] * 0x1000, 0x1000);
	}
	else if (addr >= 0xc000 && addr <= 0xcfff)
	{
		chr1[1] = v & 0x1f;
		MEM::mem_vrom(MEM::vram, 0x0000, chr1[latch1] * 0x1000, 0x1000);
	}
	else if (addr >= 0xd000 && addr <= 0xdfff)
	{
		chr2[0] = v & 0x1f;
		MEM::mem_vrom(MEM::vram, 0x1000, chr2[latch2+2] * 0x1000, 0x1000);
	}
	else if (addr >= 0xe000 && addr <= 0xefff)
	{
		chr2[1] = v & 0x1f;
		MEM::mem_vrom(MEM::vram, 0x1000, chr2[latch2+2] * 0x1000, 0x1000);
	}
	else if (addr >= 0xf000 && addr <= 0xffff)
	{
		header.mirror = (v & 1) + 2;
	}
}

void Mapper009::set_latch(u16 addr, u8 v)
{

	if (addr == 0x0fd8)
	{
		latch1 = 0;
		updatechr = 1;
	}
	else if (addr == 0x0fe8)
	{
		latch1 = 1;
		updatechr = 1;
	}
	else if (addr >= 0x1fd8 && addr <= 0x1fdf)
	{
		latch2 = 0;
		updatechr = 1;
	}
	else if (addr >= 0x1fe8 && addr <= 0x1fef)
	{
		latch2 = 1;
		updatechr = 1;
	}

	if (updatechr)
	{
		MEM::mem_vrom(MEM::vram, 0x0000, chr1[latch1] * 0x1000, 0x1000);
		MEM::mem_vrom(MEM::vram, 0x1000, chr2[latch2] * 0x1000, 0x1000);
		updatechr = 0;
	}
}

void Mapper009::reset()
{
	prg.resize(4);
	chr.resize(2);
	chr1.resize(2);
	chr2.resize(2);
}

void Mapper009::scanline()
{
}

vector<u8> Mapper009::get_prg()
{
	return prg;
}

vector<u8> Mapper009::get_chr()
{
	chr[0] = chr1[latch1];
	chr[1] = chr2[latch2];
	return chr;
}
