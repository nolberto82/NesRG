#include "ppu.h"
#include "mem.h"

void Ppu::step()
{

}

void Ppu::ppu_2000_wb(u8 v)
{
	u8 r0 = mem.rb(0x2000);
	u8 r2 = mem.rb(0x2002);

	if ((v & 0x80) && !(r0 & 0x80) && (r2 & 0x80))
		nmi = true;

	if ((scanline == 241) && !(v & 0x80) && cycle < 3)
		nmi = false;

	mem.wb(0x2000, v);
}

u8 Ppu::ppu_2002_rb(u8 v)
{
	if (v & 0x80)
	{
		v &= 0x60;
	}

	if (scanline == 241)
	{
		if (ppu.cycle == 0)
			v &= ~0x80;

		v = 0x80;
	}

	return v;
}
