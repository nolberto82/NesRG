#include "mappers.h"
#include "mem.h"

MMC1 mmc1;

void mapper001_update(u16 addr, u8 v)
{
	int prgbanks = header.prgnum = rom[4];
	int chrbanks = header.chrnum = rom[5];
	int prgsize = prgbanks * 0x4000;
	int chrsize = chrbanks * 0x2000;

	if (addr >= 0x8000 && addr <= 0xffff)
	{
		if (v & 0x80)
			mapper001_reset();
		else
		{
			mmc1.control = mmc1.control >> 1 | (v & 0x01) << 4;
			mmc1.count++;
		}

		if (mmc1.count == 5)
		{
			mmc1.reg = (addr >> 13) & 3;
			if (mmc1.reg == 0)
			{
				mirrornametable = mmc1.control & 3;
				mmc1.prgmode = (mmc1.control >> 2) & 1;
				mmc1.chrmode = (mmc1.control >> 4) & 1;
			}
			else if (mmc1.reg == 1 && chrbanks > 0)
			{
				if (mmc1.chrmode == 0)
				{
					int chr = 0x10 + prgsize / prgbanks * (mmc1.control & 0x1f) + prgsize;
					memcpy(&vram[0x0000], rom.data() + chr, prgsize);
				}
				else
				{
					int chr = 0x10 + prgsize / prgbanks * (mmc1.control & 0x1f);
					memcpy(&vram[0x0000], rom.data() + chr, chrsize);
				}
			}
			else if (mmc1.reg == 2 && chrbanks > 0)
			{
				if (mmc1.chrmode == 0)
					memcpy(&vram[0x1000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.control + prgsize, prgsize);
				else
					memcpy(&vram[0x1000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.control, chrsize);
			}
			else if (mmc1.reg == 3)
			{
				int prg1 = 0x10 + prgsize / prgbanks * (mmc1.control & 0xf);
				int prg2 = 0x10 + prgsize - (prgsize / prgbanks);
				if (mmc1.prgmode == 0)
					mem_copy(0x8000, prg1, prgsize / prgbanks * 2);
				else
				{
					mem_copy(0x8000, prg1, prgsize / prgbanks);
					mem_copy(0xc000, prg2, prgsize / prgbanks);
				}
				sram_disabled = (mmc1.control >> 4) & 1;
			}
			mmc1.count = 0;
		}
	}
}

void mapper001_reset()
{
	memset(&mmc1, 0x00, sizeof(mmc1));
}
