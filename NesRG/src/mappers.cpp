#include "mappers.h"
#include "mem.h"

MMC1 mmc1;

void mapper_update(u16 addr, u8 v)
{
	switch (header.mappernum)
	{
		case 1:
			mapper001(addr, v);
			break;
		default:
			break;
	}
}

void mapper001(u16 addr, u8 v)
{
	int prgbanks = header.prgnum = rom[4];
	int chrbanks = header.chrnum = rom[5];
	int prgsize = prgbanks * 0x4000;
	int chrsize = chrbanks * 0x2000;

	if (mmc1.count >= 5)
		mmc1.count = 0;

	if (addr >= 0x8000 && addr <= 0x9fff)
	{
		mirrornametable = v & 3;
		mmc1.prgmode = (v >> 2) & 3;
		mmc1.chrmode = (v >> 4) & 1;
	}
	else if (addr >= 0xa000 && addr <= 0xbfff)
	{
		if (chrbanks == 0)
			return;
		if (++mmc1.count == 5) {
			if (mmc1.chrmode == 0)
				memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.bank + prgsize, prgsize);
			else
				memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.bank, chrsize);
		}
	}
	else if (addr >= 0xc000 && addr <= 0xdfff)
	{
		if (chrbanks == 0)
			return;
		if (++mmc1.count == 5) {
			if (mmc1.chrmode == 0)
				memcpy(&vram[0x1000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.bank + prgsize, prgsize);
			else
				memcpy(&vram[0x1000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.bank, chrsize);
		}
	}
	else if (addr >= 0xe000 && addr <= 0xffff)
	{
		if (!mmc1.first)
		{
			mmc1.shift = v;
			mmc1.first = 1;
		}
		else if (mmc1.count < 5)
		{
			//if (mmc1.bank == 0)
			mmc1.bank = v;
			mmc1.shift >>= 1;
			if (++mmc1.count == 5)
			{
				memcpy(&ram[0x8000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.bank, prgsize / prgbanks);
				memcpy(&ram[0xc000], rom.data() + 0x10 + prgsize - (prgsize / prgbanks), prgsize / prgbanks);
			}
		}
	}
}

void mapper_reset()
{
	memset(&mmc1, 0x00, sizeof(mmc1));
}
