#include "mappers.h"
#include "mem.h"

MMC1 mmc1;

void MMC1::update(u16 addr, u8 v)
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
				mem_vrom(vram, 0x0000, chr[0] * chrbank, chrbank);
			}
			else if (addr >= 0xc000 && addr <= 0xdfff)
			{
				if (chrmode == 0)
					chr[1] = (control & 0x1e) >> 1;
				else
					chr[1] = control & 0x1f;
				mem_vrom(vram, 0x1000, chr[1] * 0x1000,0x1000);
			}
			else if (addr >= 0xe000 && addr <= 0xffff)
			{
				if (prgmode == 0 || prgmode == 1)
				{
					prg[0] = control & 0xe;
					prg[1] = header.prgnum - 1;
					int prg = 0x10 + prgbank * (control & 0xe);
					mem_rom(ram, 0x8000, prg, prgbank);
				}
				else if (prgmode == 2)
				{
					prg[0] = header.prgnum - 1;
					prg[1] = control & 0xf;
					int prg = 0x10 + prgbank * (control & 0xf);
					mem_rom(ram, 0xc000, prg, prgbank);
				}
				else if (prgmode == 3)
				{
					prg[0] = control & 0xf;
					prg[1] = header.prgnum - 1;
					int prg = 0x10 + prgbank * (control & 0xf);
					mem_rom(ram, 0x8000, prg, prgbank);
				}
				sram_disabled = (control >> 4) & 1;
			}
			writes = 0;
		}
	}
}

void MMC1::reset()
{
	memset(&mmc1, 0x00, sizeof(mmc1));
	prgmode = 3;
	prgbank = 0x4000;
	chrbank = 0x2000;
	prg.resize(2);
	chr.resize(2);
}

vector<u8> MMC1::get_prg()
{
	return vector<u8>();
}
