#include "mappers.h"
#include "mem.h"

MMC1 mmc1;

void mapper001(u8 v)
{
	if (!mmc1.first)
	{
		mmc1.shift = v;
		mmc1.first = 1;

	}
	else if (mmc1.count < 5)
	{
		if (mmc1.bank == 0)
			mmc1.bank = v;
		mmc1.shift >>= 1;
		mmc1.count++;
		if (mmc1.count == 5)
		{
			int prgbanks = header.prgnum = rom[4];
			int chrbanks = header.chrnum = rom[5];
			int prgsize = prgbanks * 0x4000;
			int chrsize = chrbanks * 0x2000;

			memcpy(&ram[0x8000], rom.data() + 0x10 + prgsize / prgbanks * mmc1.bank, prgsize / prgbanks);
			memcpy(&ram[0xc000], rom.data() + 0x10 + prgsize - (prgsize / prgbanks), prgsize / prgbanks);
			//memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize, chrsize);
		}
	}
}

void mapper_reset()
{
	memset(&mmc1, 0x00, sizeof(mmc1));
}
