#include "ppu.h"
#include "mem.h"

void Ppu::step(int num)
{

	while (num-- > 0)
	{
		pixel++;
		if (pixel > 340)
		{
			pixel -= 341;
			scanline++;

			if (scanline == 262)
				scanline = 0;
		}

		if (scanline >= 0 && scanline < 240)
		{

		}
		else if (scanline == 241 && pixel == 1)
		{
			mem.ram[0x2002] |= 0x80;
			nmi = true;
		}
		else if (scanline == 261 && pixel == 1)
		{
			mem.ram[0x2002] &= 0x7f;
			nmi = false;
		}
	}
}

void Ppu::ppu_2000_wb(u8 v)
{
	u8 r0 = mem.ram[0x2000];
	u8 r2 = mem.ram[0x2002];

	if ((v & 0x80) && !(r0 & 0x80) && (r2 & 0x80))
		nmi = true;

	if ((scanline == 241) && !(v & 0x80) && cycles < 3)
		nmi = false;

	mem.ram[0x2000] = v;
}

u8 Ppu::ppu_2002_rb(u8 v)
{
	if (v & 0x80)
	{
		v &= 0x7f;
	}

	if (scanline == 241)
	{
		v |= 0x80;
	}

	return v;
}

void Ppu::reset()
{
	cycles = 0;
	scanline = 0;
}
