#include "mappers.h"
#include "mem.h"
#include "cpu.h"

u8 chr5id[2][8] = { {0,1,2,3,4,5,6,7}, {8,9,10,11,8,9,10,11} };

void Mapper005::setup(struct Header h)
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

void Mapper005::wb(u16 addr, u8 v)
{
	if (addr == 0x5100)
	{
		prgmode = v & 3;
	}
	if (addr == 0x5101)
	{
		chrmode = v & 3;
	}
	if (addr == 0x5102)
	{
		write_prot1 = v & 3;
	}
	if (addr == 0x5103)
	{
		write_prot2 = v & 3;
	}
	if (addr == 0x5104)
	{
		extended_ram = v & 3;
	}
	if (addr == 0x5105)
	{
		//extended_ram = v & 3;
	}
	else if (addr == 0x5113)
	{

	}
	else if (addr == 0x5114)
	{

	}
	else if (addr == 0x5115)
	{
		if (prgmode == 1 || prgmode == 2)
		{
			prg[0] = v & 0x7f;
			prg[1] = prg[0] + 1;
			MEM::mem_rom(MEM::ram, 0x8000 + 0 * 0x4000, 0x10 + prg[0] * 0x2000, 0x4000);
			//MEM::mem_rom(MEM::ram, 0x8000 + 0 * 0x2000, 0x10 + prg[1] * 0x2000, 0x2000);
		}
	}
	else if (addr == 0x5115)
	{

	}
	else if (addr >= 0x5120 && addr <= 0x512b)
	{
		u8 r = addr - 0x5120;
		if (r > 7)
		{
			chr[r - 8] = v;
			chr[r] = v;
		}
		else
		{
			chr[r] = v;
		}

		if (pctrl.spritesize == 0)
		{

			switch (chrmode)
			{
			case 3:

				//for (int i = 0; i < 4; i++)
				//	MEM::mem_vrom(MEM::vram, r * 0x400, chr[r] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 0 * 0x400, chr[0] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 1 * 0x400, chr[1] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 2 * 0x400, chr[2] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 3 * 0x400, chr[3] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 4 * 0x400, chr[4] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 5 * 0x400, chr[5] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 6 * 0x400, chr[6] * 0x400, 0x0400);
				MEM::mem_vrom(MEM::vram, 7 * 0x400, chr[7] * 0x400, 0x0400);
				break;
			}
		}
		else
		{
			switch (chrmode)
			{
			case 3:
				//for (int i = 0; i < 8; i++)
				//{
				if (r < 8)
				{
					MEM::mem_vrom(MEM::vram, 0 * 0x400, chr[0] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 1 * 0x400, chr[1] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 2 * 0x400, chr[2] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 3 * 0x400, chr[3] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 4 * 0x400, chr[4] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 5 * 0x400, chr[5] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 6 * 0x400, chr[6] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 7 * 0x400, chr[7] * 0x400, 0x0400);;
					//MEM::mem_vrom(MEM::vram, 8 * 0x400 + 0x1000, chr[8] * 0x400, 0x0400);
					//MEM::mem_vrom(MEM::vram, 9 * 0x400 + 0x1000, chr[9] * 0x400, 0x0400);
					//MEM::mem_vrom(MEM::vram, 10 * 0x400 + 0x1000, chr[10] * 0x400, 0x0400);
					//MEM::mem_vrom(MEM::vram, 11 * 0x400 + 0x1000, chr[11] * 0x400, 0x0400);
				}
				else
				{
					MEM::mem_vrom(MEM::vram, 0 * 0x400, chr[0] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 1 * 0x400, chr[1] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 2 * 0x400, chr[2] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 3 * 0x400, chr[3] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 0 * 0x400 + 0x1000, chr[0] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 1 * 0x400 + 0x1000, chr[1] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 2 * 0x400 + 0x1000, chr[2] * 0x400, 0x0400);
					MEM::mem_vrom(MEM::vram, 3 * 0x400 + 0x1000, chr[3] * 0x400, 0x0400);
				}

				//}
				break;
			}
		}
	}
}

u8 Mapper005::rb(u16 addr)
{
	return u8();
}

void Mapper005::switchchr(u16 addr, u8 v)
{
	if (pctrl.spritesize == 0)
	{
		chr[0] = 0; chr[0] = 0;

	}

	//MEM::mem_vrom(MEM::vram, r * 0x400, chr[r] * 0x400, 0x0400);
}

void Mapper005::reset()
{
	prg.resize(4);
	chr.resize(12);
}

void Mapper005::scanline()
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

vector<u8> Mapper005::get_prg()
{
	return prg;
}

vector<u8> Mapper005::get_chr()
{
	return chr;
}
