#include "mappers.h"
#include "mem.h"
#include "cpu.h"

u8 chrid[2][8] = { {0,0,1,1,2,3,4,5}, {2,3,4,5,0,0,1,1} };
s8 masid[2][8] = { {-2,1,-2,1,-1,-1,-1,-1}, {-1,-1,-1,-1,-2,1,-2,1} };

void Mapper004::setup(struct Header h)
{
	int prgsize = h.prgnum * 0x4000;
	int chrsize = h.chrnum * 0x2000;

	memcpy(&MEM::ram[0x8000], MEM::rom.data() + 0x10, prgsize / h.prgnum);
	memcpy(&MEM::ram[0xc000], MEM::rom.data() + 0x10 + prgsize - (prgsize / h.prgnum), prgsize / h.prgnum);
	if (h.chrnum > 0)
	{
		memcpy(&MEM::vram[0x0000], MEM::vrom.data(), chrsize / h.chrnum);
	}
}

void Mapper004::update(u16 addr, u8 v)
{
	if (addr >= 0x8000 && addr <= 0x9fff)
	{
		if ((addr % 2) == 0)
		{
			chrreg = v & 0x7;
			chrmode = (v >> 7) & 1;
			prgmode = (v >> 6) & 1;;
			prgbank = v & 0x7;
			bankreg[chrreg];
		}
		else
		{
			bankreg[chrreg] = v;
		}

		u8 m = chrmode;
		for (int i = 0; i < 8; i++)
		{
			if (masid[m][i] != -1)
			{
				chr[i] = bankreg[chrid[m][i]] & masid[m][i];
				chr[i + 1] = bankreg[chrid[m][i]] | masid[m][i + 1];
				i++;
			}
			else
				chr[i] = bankreg[chrid[m][i]] & masid[m][i];
		}

		for (int i = 0; i < chr.size(); i++)
			MEM::mem_vrom(MEM::vram, i * 0x400, chr[i] * 0x400, 0x0400);

		if (prgmode == 0)
		{
			prg[0] = bankreg[6] & (header.prgnum * 2 - 1);
			prg[1] = bankreg[7] & (header.prgnum * 2 - 1);
			prg[2] = (header.prgnum * 2) - 2;
			prg[3] = (header.prgnum * 2) - 1;
		}
		else
		{
			prg[0] = (header.prgnum * 2) - 2;
			prg[1] = bankreg[7] & (header.prgnum * 2 - 1);
			prg[2] = bankreg[6] & (header.prgnum * 2 - 1);
			prg[3] = (header.prgnum * 2) - 1;
		}

		for (int i = 0; i < prg.size(); i++)
			MEM::mem_rom(MEM::ram, 0x8000 + i * 0x2000, 0x10 + prg[i] * 0x2000, 0x2000);
	}
	else if (addr >= 0xa000 && addr <= 0xbfff)
	{
		if ((addr % 2) == 0)
			header.mirror = (v & 1) + 2;
		else
		{
			write_prot = (v >> 6) & 1;
			prg_ram = (v >> 7) & 1;
		}
	}
	else if (addr >= 0xc000 && addr <= 0xdfff)
	{
		if ((addr % 2) == 0)
			rvalue = v;
		else
		{
			counter = 0;
			reload = 1;
		}
	}
	else if (addr >= 0xe000 && addr <= 0xffff)
	{
		if ((addr % 2) == 0)
			irq = 0;
		else
			irq = 1;
	}
}

void Mapper004::reset()
{
	prg.resize(4);
	chr.resize(8);
}

void Mapper004::scanline()
{
	if (counter == 0)
	{
		counter = rvalue;
		reload = 0;
	}
	else
		counter--;

	if (counter == 0 && irq)
	{
		if ((reg.ps & FI) == 0)
			fire = 1;
	}
}

vector<u8> Mapper004::get_prg()
{
	return prg;
}

vector<u8> Mapper004::get_chr()
{
	return chr;
}
