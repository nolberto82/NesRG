#include "mappers.h"
#include "mem.h"
#include "ppu.h"

MMC2 mmc2;

void MMC2::update(u16 addr, u8 v)
{
	//for (int i = 0; i < prg.size(); i++)
		//mem_rom(ram, 0x8000 + i * 0x2000, 0x10 + prg[i] * 0x2000, 0x2000);
	if (addr >= 0xa000 && addr <= 0xafff)
	{
		prg[0] = v & 0xf;
		prg[1] = (header.prgnum * 2) - 3;
		prg[2] = (header.prgnum * 2) - 2;
		prg[3] = (header.prgnum * 2) - 1;
		for (int i = 0; i < prg.size(); i++)
			mem_rom(ram, 0x8000 + i * 0x2000, 0x10 + prg[i] * 0x2000, 0x2000);
	}
	else if (addr >= 0xb000 && addr <= 0xbfff)
	{
		chr1[0] = v & 0x1f;
		mem_vrom(vram, 0x0000, chr1[latch1] * 0x1000, 0x1000);
	}
	else if (addr >= 0xc000 && addr <= 0xcfff)
	{
		chr1[1] = v & 0x1f;
		mem_vrom(vram, 0x0000, chr1[latch1] * 0x1000, 0x1000);
	}
	else if (addr >= 0xd000 && addr <= 0xdfff)
	{
		chr2[0] = v & 0x1f;
		mem_vrom(vram, 0x1000, chr2[latch2] * 0x1000, 0x1000);
	}
	else if (addr >= 0xe000 && addr <= 0xefff)
	{
		chr2[1] = v & 0x1f;
		mem_vrom(vram, 0x1000, chr2[latch2] * 0x1000, 0x1000);
	}
	else if (addr >= 0xf000 && addr <= 0xffff)
	{
		header.mirror = (v & 1) + 2;
	}
}

void MMC2::set_latch(u16 addr, u8 v)
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
		mem_vrom(vram, 0x0000, chr1[latch1] * 0x1000, 0x1000);
		mem_vrom(vram, 0x1000, chr2[latch2] * 0x1000, 0x1000);
		updatechr = 0;
	}

}

void MMC2::reset()
{
	memset(&mmc2, 0x00, sizeof(mmc2));
	prg.resize(4);
	chr1.resize(2);
	chr2.resize(2);
	latch1 = latch2 = 1;
	updatechr = 0;
}