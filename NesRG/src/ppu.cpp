#include "ppu.h"
#include "mem.h"
#include <sdlgfx.h>

void Ppu::step(int num)
{
	u8 tile = 0;

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
			if (pixel % 8 == 0)
			{

			}

			do_scanline();
		}
		else if (scanline == 241 && pixel == 1)
		{
			mem.ram[0x2002] = ppu2002 |= 0x80;
			nmi = true;


		}
		else if (scanline == 261 && pixel == 1)
		{
			mem.ram[0x2002] = ppu2002 &= 0x7f;
			nmi = false;

			gfx.render_frame();
		}
	}
}

void Ppu::ppuctrl(u8 v) //2000
{
	nametableaddr = 0x2000 | (v & 3) << 10;

	if (v & 0x10)
	{
		ppu2002 |= v;
		mem.ram[0x2002] |= v;
	}

	vramaddrincrease = (ppu2000 & 0x04) > 0;
	spritesize = (ppu2000 & 0x20) > 0;
}

void Ppu::ppumask(u8 v)
{
	ppu2001 = v;

	background_render = (ppu2001 & 0x08) > 0;
	sprite_render = (ppu2001 & 0x10) > 0;
}

u8 Ppu::ppustatus()
{
	if (ppu2002 & 0x80)
	{
		u8 v = ppu2002;
		mem.ram[0x2002] = ppu2002 &= 0x7f;
		return v;
	}

	return ppu2002;
}

void Ppu::oamaddrwrite(u8 v)
{
	ppu2003 = v;
}

void Ppu::oamdatawrite(u8 v)
{
	ppu2004 = v;
}

void Ppu::scrollwrite(u8 v)
{
	if (preg.w)
	{
		preg.t = preg.t & 0x7fe0 | (v & 0xf8) >> 3;
	}
	else
	{
		preg.t = (preg.t & 0xc1f) | (v & 7) << 12 | (v & 0xf8);
		preg.x = (u8)(v & 0x07);
	}

	preg.w = !preg.w;
}

void Ppu::addrwrite(u8 v)
{
	if (!preg.w)
	{
		preg.t = (u16)((preg.t & 0x80ff) | (v & 0x3f) << 8);
	}
	else
	{
		ppu2000 &= 0xfe;
		preg.t = preg.t & 0xff00 | v;
		preg.v = preg.t;
	}

	preg.w = !preg.w;
}

void Ppu::datawrite(u8 v)
{
	mem.ppuwb(preg.v, v);

	if (ppu2000 & 0x04)
		preg.v += 32;
	else
		preg.v++;
}

void Ppu::reset()
{
	cycles = 0;
	scanline = 0;
}

void Ppu::do_scanline()
{
	u16 ntaddr = 0x2000 | (preg.v & 0xfff);
	u16 ataddr = 0x23c0 | (preg.v & 0xc00) | ((preg.v >> 4) & 0x38) | ((preg.v >> 2) & 0x07);

	if (background_render)
		render_background(ntaddr, ataddr);
}

void Ppu::render_background(u16 ntaddr, u16 ataddr)
{
	u16 patternaddr = (ppu2000 & 0x10) ? 0x1000 : 0x0000;
	int y = scanline;
	int row = y % 8;
	u8 id = mem.ppurb(ntaddr);
	u8 b1 = mem.ppurb(patternaddr + id * 16 + row + 0);
	u8 b2 = mem.ppurb(patternaddr + id * 16 + row + 8);
	int shift = 0x80;

	for (int x = 0; x < 1; x++)
	{
		int bit0 = (b1 & shift) > 0 ? 1 : 0;
		int bit1 = (b2 & shift) > 0 ? 1 : 0;

		shift >>= 1;

		u8 bit2 = mem.vram[ataddr + (y / 4) * 8 + (x / 4)];
		
		int palindex = bit0 | bit1 * 2;

		int colorindex = bit2 * 4 + palindex;

		int color = palettes[mem.vram[0x3f00 | colorindex]];

		gfx.disp_pixels[x + y * 256] = color;
	}
}

void Ppu::x_increment()
{
	if ((preg.v & 0x1f) == 31)
	{
		preg.v &= ~0x1f;
		preg.v ^= 0x400;
	}
	else
		preg.v++;
}

void Ppu::y_increment()
{
	if ((preg.v & 0x7000) != 0x7000)
	{
		preg.v += 0x1000;
	}
	else
	{
		preg.v &= ~0x7000;
		int y = (preg.v & 0x3e0) >> 5;

		if (y == 29)
		{
			y = 0;
			preg.v ^= 0x800;
		}
		else if (y == 31)
		{
			y = 0;
		}
		else
			y++;

		preg.v = (preg.v & ~0x3e0) | (y << 5);
	}
}
