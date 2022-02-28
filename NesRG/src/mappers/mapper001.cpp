#include "mappers.h"
#include "mem.h"

MMC1 mmc1;

void mapper001_update(u16 addr, u8 v)
{
	if (addr >= 0x8000 && addr <= 0xffff)
	{
		if (v & 0x80)
			mapper001_reset();
		else
		{
			mmc1.control = mmc1.control >> 1 | (v & 0x01) << 4;
			mmc1.writes++;
		}

		if (mmc1.writes == 5)
		{
			mmc1.reg = (addr >> 13) & 3;
			if (mmc1.reg == 0)
			{
				mirrornametable = mmc1.control & 3;
				mmc1.prgmode = (mmc1.control >> 2) & 3;
				mmc1.chrmode = (mmc1.control >> 4) & 1;
				mmc1.prgbank = (mmc1.control >> 3) & 1 ? 0x4000 : 0x8000;
				mmc1.chrbank = (mmc1.control >> 4) & 1 ? 0x1000 : 0x2000;
				mmc1.prgrom = mmc1.prgbank * header.prgnum;
				mmc1.chrrom = mmc1.chrbank * header.chrnum;
			}
			else if (mmc1.reg == 1)
			{
				if (mmc1.chrmode == 0)
				{
					int chr = mmc1.chrbank * ((mmc1.control & 0x1e) >> 1);
					mem_vrom(vram, 0x0000, chr, 0x2000);
				}
				else
				{
					int chr = mmc1.chrbank * (mmc1.control & 0x1f);
					mem_vrom(vram, 0x0000, chr, mmc1.chrbank);
				}
			}
			else if (mmc1.reg == 2)
			{
				if (mmc1.chrmode == 0)
				{
					int chr = mmc1.chrbank * ((mmc1.control & 0x1e) >> 1);
					mem_vrom(vram, 0x1000, chr, 0x2000);
				}
				else
				{
					int chr = mmc1.chrbank * (mmc1.control & 0x1f);
					mem_vrom(vram, 0x1000, chr, mmc1.chrbank);
				}
			}
			else if (mmc1.reg == 3)
			{
				if (mmc1.prgmode == 0 || mmc1.prgmode == 1)
				{
					int prg = 0x10 + mmc1.prgbank * (mmc1.control & 0xe);
					mem_rom(ram, 0x8000, prg, mmc1.prgbank);
				}
				else if (mmc1.prgmode == 2)
				{
					int prg = 0x10 + mmc1.prgbank * (mmc1.control & 0xf);
					mem_rom(ram, 0xc000, prg, mmc1.prgbank);
				}
				else if (mmc1.prgmode == 3)
				{
					int prg = 0x10 + mmc1.prgbank * (mmc1.control & 0xf);
					mem_rom(ram, 0x8000, prg, mmc1.prgbank);
				}
				sram_disabled = (mmc1.control >> 4) & 1;
			}
			mmc1.writes = 0;
		}
	}
}

void mapper001_reset()
{
	memset(&mmc1, 0x00, sizeof(mmc1));
	mmc1.prgmode = 3;
	mmc1.prgbank = 0x4000;
	mmc1.chrbank = 0x2000;
}
