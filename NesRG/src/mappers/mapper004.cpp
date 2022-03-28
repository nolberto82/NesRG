#include "mappers.h"
#include "mem.h"
#include  "ppu.h"

MMC3 mmc3;

void mapper004_update(u16 addr, u8 v)
{
	if (addr >= 0x8000 && addr <= 0x9fff)
	{
		if ((addr % 2) == 0)
		{
			mmc3.chrreg = v & 0x7;
			mmc3.chrmode = (v >> 7) & 1;
			mmc3.prgmode = (v >> 6) & 1;;
			mmc3.prgbank = v & 0x7;
			mmc3.bankreg[mmc3.chrreg];
		}
		else
		{
			mmc3.bankreg[mmc3.chrreg] = v;
		}

		if (mmc3.chrmode == 0)
		{
			mmc3.chr[0] = mmc3.bankreg[0] & 0xfe;
			mmc3.chr[1] = mmc3.bankreg[0] + 1;
			mmc3.chr[2] = mmc3.bankreg[1] & 0xfe;
			mmc3.chr[3] = mmc3.bankreg[1] + 1;
			mmc3.chr[4] = mmc3.bankreg[2];
			mmc3.chr[5] = mmc3.bankreg[3];
			mmc3.chr[6] = mmc3.bankreg[4];
			mmc3.chr[7] = mmc3.bankreg[5];
		}
		else
		{
			mmc3.chr[0] = mmc3.bankreg[2];
			mmc3.chr[1] = mmc3.bankreg[3];
			mmc3.chr[2] = mmc3.bankreg[4];
			mmc3.chr[3] = mmc3.bankreg[5];
			mmc3.chr[4] = mmc3.bankreg[0] & 0xfe;
			mmc3.chr[5] = mmc3.bankreg[0] + 1;
			mmc3.chr[6] = mmc3.bankreg[1] & 0xfe;
			mmc3.chr[7] = mmc3.bankreg[1] + 1;
		}

		for (int i = 0; i < sizeof(mmc3.chr); i++)
			mem_vrom(vram, i * 0x400, mmc3.chr[i] * 0x400, 0x0400);

		if (mmc3.prgmode == 0)
		{
			mmc3.prg[0] = mmc3.bankreg[6];
			mmc3.prg[1] = mmc3.bankreg[7];
			mmc3.prg[2] = (header.prgnum * 2) - 2;
			mmc3.prg[3] = (header.prgnum * 2) - 1;
		}
		else
		{
			mmc3.prg[0] = (header.prgnum * 2) - 2;
			mmc3.prg[1] = mmc3.bankreg[7];
			mmc3.prg[2] = mmc3.bankreg[6];
			mmc3.prg[3] = (header.prgnum * 2) - 1;
		}

		for (int i = 0; i <sizeof(mmc3.prg); i++)
			mem_rom(ram, 0x8000 + i * 0x2000, 0x10 + mmc3.prg[i] * 0x2000, 0x2000);
	}
	else if (addr >= 0xa000 && addr <= 0xbfff)
	{
		if ((addr % 2) == 0)
		{
			header.mirror = (v & 1) + 2;
		}
		else
		{
			mmc3.write_prot = (v >> 6) & 1;
			mmc3.prg_ram = (v >> 7) & 1;
		}
	}
	else if (addr >= 0xc000 && addr <= 0xdfff)
	{
		if ((addr % 2) == 0)
		{
			mmc3.rvalue = v;
		}
		else
		{
			mmc3.counter = 0;
			mmc3.reload = 1;
		}
	}
	else if (addr >= 0xe000 && addr <= 0xffff)
	{
		if ((addr % 2) == 0)
		{
			mmc3.irq = 0;
		}
		else
		{
			mmc3.irq = 1;
		}
	}
}

void mapper004_scanline()
{
	if (mmc3.counter == 0)
	{
		mmc3.counter = mmc3.rvalue;
		mmc3.reload = 0;
	}
	else
	{
		mmc3.counter--;
	}

	if (mmc3.counter == 0 && mmc3.irq)
	{
		if ((reg.ps & FI) == 0)
			mmc3.fire = 1;
	}
}

void mapper004_reset()
{
	memset(&mmc3, 0x00, sizeof(mmc3));
}