#include "mappers.h"
#include "mem.h"

void Mapper001::setup(Header h)
{
	int prgsize = h.prgnum * 0x4000;
	int chrsize = h.chrnum * 0x2000;

	memcpy(&MEM::ram[0x8000], MEM::rom.data() + 0x10, prgsize / h.prgnum);
	memcpy(&MEM::ram[0xc000], MEM::rom.data() + 0x10 + prgsize - (prgsize / h.prgnum), prgsize / h.prgnum);
	if (h.chrnum > 0)
	{
		memcpy(&MEM::vram[0x0000], MEM::vrom.data(), chrsize / h.chrnum);
	}

	sram = 1;
	MEM::load_sram();
}

void Mapper001::wb(u16 addr, u8 v)
{
	if (addr >= 0x8000 && addr <= 0xffff)
	{
		if (v & 0x80)
			reset();
		else
		{
			control = control >> 1 | (v & 0x01) << 4;
			writes++;
		}

		if (writes == 5)
		{
			if (addr >= 0x8000 && addr <= 0x9fff)
			{
				header.mirror = control & 3;
				prgmode = (control >> 2) & 3;
				chrmode = (control >> 4) & 1;
				prgbank = (control >> 3) & 1 ? 0x4000 : 0x8000;
				chrbank = (control >> 4) & 1 ? 0x1000 : 0x2000;
			}
			else if (addr >= 0xa000 && addr <= 0xbfff)
			{
				if (chrmode == 0)
					chr[0] = (control & 0x1e) >> 1;
				else
					chr[0] = (control & 0x1f);
				MEM::mem_vrom(MEM::vram, 0x0000, chr[0] * chrbank, chrbank);
			}
			else if (addr >= 0xc000 && addr <= 0xdfff)
			{
				if (chrmode == 0)
					chr[1] = (control & 0x1e) >> 1;
				else
					chr[1] = control & 0x1f;
				MEM::mem_vrom(MEM::vram, 0x1000, chr[1] * 0x1000, 0x1000);
			}
			else if (addr >= 0xe000 && addr <= 0xffff)
			{
				if (prgmode == 0 || prgmode == 1)
				{
					prg[0] = control & 0xe;
					prg[1] = header.prgnum - 1;
					int prg = 0x10 + prgbank * (control & 0xe);
					MEM::mem_rom(MEM::ram, 0x8000, prg, prgbank);
				}
				else if (prgmode == 2)
				{
					prg[0] = header.prgnum - 1;
					prg[1] = control & 0xf;
					int prg = 0x10 + prgbank * (control & 0xf);
					MEM::mem_rom(MEM::ram, 0xc000, prg, prgbank);
				}
				else if (prgmode == 3)
				{
					prg[0] = control & 0xf;
					prg[1] = header.prgnum - 1;
					int prg = 0x10 + prgbank * (control & 0xf);
					MEM::mem_rom(MEM::ram, 0x8000, prg, prgbank);
				}
				sram = 1;
			}
			writes = 0;
		}
	}
}

u8 Mapper001::rb(u16 addr)
{
	return u8();
}

void Mapper001::reset()
{
	MEM::save_sram();

	prgbank = 0x4000;
	chrbank = 0x2000;
	prgmode = 3;

	prg.resize(2);
	chr.resize(2);
}

void Mapper001::scanline()
{
}

vector<u8> Mapper001::get_prg()
{
	return prg;
}

vector<u8> Mapper001::get_chr()
{
	return chr;
}


