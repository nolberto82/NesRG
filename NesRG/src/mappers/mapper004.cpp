#include "mappers.h"
#include "mem.h"
#include  "ppu.h"

MMC4 mmc4;

void mapper004_update(u16 addr, u8 v)
{
	if (addr >= 0x8000 && addr <= 0x9fff)
	{
		if ((addr % 2) == 0)
		{
			mmc4.chrreg = v & 0x7;
			mmc4.chrmode = (v >> 7) & 1;
			mmc4.prgmode = (v >> 6) & 1;;
			mmc4.prgbank = v & 0x7;
			mmc4.bankreg[mmc4.chrreg];
		}
		else
		{
			mmc4.bankreg[mmc4.chrreg] = v;
		}

		if (mmc4.chrmode == 0)
		{
			mmc4.chr[0] = mmc4.bankreg[0] & 0xfe;
			mmc4.chr[1] = mmc4.bankreg[0] + 1;
			mmc4.chr[2] = mmc4.bankreg[1] & 0xfe;
			mmc4.chr[3] = mmc4.bankreg[1] + 1;
			mmc4.chr[4] = mmc4.bankreg[2];
			mmc4.chr[5] = mmc4.bankreg[3];
			mmc4.chr[6] = mmc4.bankreg[4];
			mmc4.chr[7] = mmc4.bankreg[5];
		}
		else
		{
			mmc4.chr[0] = mmc4.bankreg[2];
			mmc4.chr[1] = mmc4.bankreg[3];
			mmc4.chr[2] = mmc4.bankreg[4];
			mmc4.chr[3] = mmc4.bankreg[5];
			mmc4.chr[4] = mmc4.bankreg[0] & 0xfe;
			mmc4.chr[5] = mmc4.bankreg[0] + 1;
			mmc4.chr[6] = mmc4.bankreg[1] & 0xfe;
			mmc4.chr[7] = mmc4.bankreg[1] + 1;
		}

			for (int i = 0; i < 8; i++)
				mem_vrom(vram, i * 0x400, mmc4.chr[i] * 0x400, 0x0400);

		if (mmc4.prgmode == 0)
		{
			mem_rom(ram, 0x8000, 0x10 + mmc4.bankreg[6] * 0x2000, 0x2000);
			mem_rom(ram, 0xa000, 0x10 + mmc4.bankreg[7] * 0x2000, 0x2000);
			mem_rom(ram, 0xc000, rom.size() - 0x4000, 0x2000);
			mem_rom(ram, 0xe000, rom.size() - 0x2000, 0x2000);
		}
		else
		{
			mem_rom(ram, 0x8000, rom.size() - 0x4000, 0x2000);
			mem_rom(ram, 0xa000, 0x10 + mmc4.bankreg[7] * 0x2000, 0x2000);
			mem_rom(ram, 0xc000, 0x10 + mmc4.bankreg[6] * 0x2000, 0x2000);
			mem_rom(ram, 0xe000, rom.size() - 0x2000, 0x2000);
		}
	}
	else if (addr >= 0xa000 && addr <= 0xbfff)
	{
		if ((addr % 2) == 0)
		{
			header.mirror = (v & 1) + 2;
		}
		else
		{
			mmc4.write_prot = (v >> 6) & 1;
			mmc4.prg_ram = (v >> 7) & 1;
		}
	}
	else if (addr >= 0xc000 && addr <= 0xdfff)
	{
		if ((addr % 2) == 0)
		{
			mmc4.rvalue = v;
		}
		else
		{
			mmc4.counter = 0;
			mmc4.reload = 1;
		}
	}
	else if (addr >= 0xe000 && addr <= 0xffff)
	{
		if ((addr % 2) == 0)
		{
			mmc4.irq = 0;
		}
		else
		{
			mmc4.irq = 1;
		}
	}
}

void mapper004_scanline()
{
	if (mmc4.counter == 0)
	{
		mmc4.counter = mmc4.rvalue;
		mmc4.reload = 0;
	}
	else
	{
		mmc4.counter--;
	}

	if (mmc4.counter == 0 && mmc4.irq)
	{
		if ((reg.ps & FI) == 0)
			mmc4.fire = 1;
	}
}

void mapper004_reset()
{
	memset(&mmc4, 0x00, sizeof(mmc4));
}