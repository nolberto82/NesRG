#include "ppu.h"
#include "mem.h"
#include <sdlgfx.h>

void Ppu::step(int num)
{
	pixel += num * 3;

	if (scanline == -1)
		frame_ready = false;

	if (pixel > 340)
	{
		pixel -= CYCLES_PER_LINE;
		scanline++;

		if (scanline > -1 && scanline < 240)
		{

			if (background_render || sprite_render)
			{
				render_background_new();
				render_sprites(0x20);
			}
		}
		else if (scanline == 241)
		{
			mem.ram[0x2002] = ppu2002 |= 0x80;
			if (ppu2000 & 0x80)
				nmi = true;

			mem.ram[0x2002] = ppu2002 &= 0xbf;
			//pixel = 1;
			//frame_ready = true;
		}
		else if (scanline > 260)
		{
			if (background_render || sprite_render)
			{
				gfx.render_frame();
			}


			nmi = false;
			frame_ready = true;
			//pixel = 0;
			scanline = -1;
		}
	}
}

void Ppu::ppuctrl(u8 v) //2000
{
	nametableaddr = 0x2000 | (v & 3) << 10;

	if (v & 0x10)
	{
		ppu2002 |= v;
		mem.ram[0x2002] = ppu2002;
	}

	vramaddrincrease = (ppu2000 & 0x04) > 0;
	spritesize = (ppu2000 & 0x20) > 0;

	ppu2000 = v;
}

void Ppu::ppumask(u8 v)
{
	ppu2001 = v;

	background_render = (ppu2001 & 0x08) > 0;
	sprite_render = (ppu2001 & 0x10) > 0;
}

u8 Ppu::ppustatus()
{
	//if (ppu2002 & 0x80)
	//{
	u8 v = ppu2002;
	mem.ram[0x2002] = ppu2002 &= 0x7f;
	//}

	preg.w = 0;

	return v;
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

u8 Ppu::dataread()
{
	u8 v = 0;

	if (preg.v < 0x3f00)
	{
		v = ppu_dummy2007;
		ppu_dummy2007 = mem.vram[preg.v];
	}

	if (ppu2000 & 0x04)
		preg.v += 32;
	else
		preg.v++;

	return v;
}

void Ppu::reset()
{
	cycles = 0;
	scanline = 0;
	pixel = 0;
	totalcycles = 0;
	ppu.frame_ready = false;

	for (int i = 0; i < sizeof(palbuffer); i += 3)
	{
		palettes[i / 3] = palbuffer[i] | palbuffer[i + 1] << 8 | palbuffer[i + 2] << 16 | 0xff000000;
	}

	memset(gfx.disp_pixels, 0x00, sizeof(gfx.disp_pixels));
	gfx.render_frame();
}

void Ppu::render_background_new()
{
	int patternaddr = (ppu2000 & 0x10) > 0 ? 0x1000 : 0x0000;
	int paladdr = 0x3f00;
	int left8 = (ppu2000 & 0x02) > 0 ? 1 : 0;
	int y = (scanline / 8);// +scroll_y;
	int x = pixel;

	int sx = scroll_x;// +(ppuctrl & 1 ? 256 : 0);
	int sy = scroll_y;
	int xMin = (sx / 8);// + left8;
	int xMax = (sx + 256) / 8;
	int yMin = (sy / 8);// + left8;
	int yMax = (sy + 240) / 8;

	for (int x = xMin; x <= xMax; x++)
	{
		int addr = 0;
		int natx = 0;

		if (x < 32)
		{
			addr = 0x2000 + 32 * y + x;
		}
		else if (x < 64)
		{
			addr = 0x2400 + 32 * y + (x - 32);
			natx = 32;
		}
		else
		{
			addr = 0x2800 + 32 * y + (x - 64);
		}

		if ((ppu2000 & 3) > 0)
			addr ^= 0x400;

		if (sy > 0xef)
			sy = 0;

		int offx = x * 8 - sx;
		int offy = y * 8 - sy;

		int baseaddr = addr & 0x2c00;

		int tileid = mem.vram[addr];

		int bit2 = get_attr_index(addr & 0x1f, ((addr & 0x3e0) >> 5), mem.vram[baseaddr + 0x3c0 + (y / 4) * 8 + ((x - natx) / 4)]);

		int row = scanline % 8;

		u8 byte1 = mem.vram[patternaddr + tileid * 16 + row + 0];
		u8 byte2 = mem.vram[patternaddr + tileid * 16 + row + 8];

		int shift = 0x80;

		for (int col = 0; col < 8; col++)
		{
			int bit0 = (byte1 & shift) > 0 ? 1 : 0;
			int bit1 = (byte2 & shift) > 0 ? 1 : 0;

			shift >>= 1;

			int palindex = bit0 | bit1 * 2;

			int colorindex = bit2 * 4 + palindex;

			int xp = offx + col;
			int yp = offy + row;

			if (xp < 0 || xp >= 256 || yp < 0 || yp >= 240)
				continue;

			int color = palettes[mem.vram[paladdr | colorindex]];
			gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 3] = (u8)(color >> 0);
			gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 2] = (u8)(color >> 8);
			gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 1] = (u8)(color >> 16);
			gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 0] = (u8)(color >> 24);

			sp0data[(yp * 256 * 4) + (xp * 4) + 0] = (u8)palindex;

			//DrawFrame();
		}
		//DrawFrame();
	}
}

void Ppu::render_sprites(u8 frontback)
{
	int oamaddr = 0x0100 * ppuoamdma;
	int nametableaddr = 0x2000 | ppu2000 & 3 * 0x400;
	int patternaddr = (ppu2000 & 0x08) > 0 ? 0x1000 : 0x0000;
	int paladdr = 0x3f10;
	int left8 = (ppu2000 & 0x04) > 0 ? 1 : 0;

	u8 x, y;
	int tileid, att, i;

	for (int j = 64; j > 0; j--)
	{
		i = j % 64;

		y = (u8)(mem.oam[i * 4 + 0] + 1);
		tileid = mem.oam[i * 4 + 1];
		att = mem.oam[i * 4 + 2];
		x = mem.oam[i * 4 + 3];// & 0xff + left8;

		int size = 8;
		if (spritesize)
		{
			size = 16;
		}

		bool flipH = (att & 0x40) > 0;
		bool flipV = (att & 0x80) > 0;

		int byte1 = 0;
		int byte2 = 0;

		if (size == 16)
		{
			if (tileid == 1)
			{
				if (y < 0xf1)
				{
					int yu = 0;
				}
			}

			if ((tileid & 1) == 0)
				patternaddr = 0x0000;
			else
				patternaddr = 0x1000;

			tileid &= 0xfe;

			for (int r = 0; r < 16; r++)
			{
				int rr = r % 8;

				if (r < 8)
				{
					byte1 = mem.vram[patternaddr + tileid * 16 + rr + 0];
					byte2 = mem.vram[patternaddr + tileid * 16 + rr + 8];
				}
				else
				{
					byte1 = mem.vram[patternaddr + (tileid + 1) * 16 + rr + 0];
					byte2 = mem.vram[patternaddr + (tileid + 1) * 16 + rr + 8];
				}

				for (int cl = 0; cl < 8; cl++)
				{
					int col = 7 - cl;
					int row = r;

					if (flipH && flipV)
					{
						col = cl;
						row = 7 - r;
					}
					else if (flipV)
					{
						row = 7 - r;
					}
					else if (flipH)
					{
						col = cl;
					}

					int bit0 = (byte1 & 1) > 0 ? 1 : 0;
					int bit1 = (byte2 & 1) > 0 ? 1 : 0;

					byte1 >>= 1;
					byte2 >>= 1;

					int palindex = bit0 | bit1 * 2;

					int colorindex = palindex + (att & 3) * 4;

					int xp = x + col;
					int yp = y + row;
					if (x < 0 || x >= 255 || y < 0 || y >= 240)
						break;

					if (palindex != 0)
					{
						u8 bgpalindex = sp0data[256 * (y + row) * 4 + (x + col) * 4 + 0];
						if (bgpalindex != 0 && i == 0 && yp == scanline && x < 255)
							set_sprite_zero();

						if ((att & frontback) == 0 || bgpalindex == 0)
						{
							int color = palettes[mem.vram[paladdr | colorindex]];
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 0] = (u8)(color >> 0);
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 1] = (u8)(color >> 8);
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 2] = (u8)(color >> 16);
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 3] = 255;
						}
					}
					//DrawFrame();
				}
			}
		}
		else
		{
			for (int r = 0; r < 8; r++)
			{
				byte1 = mem.vram[patternaddr + tileid * 16 + r + 0];
				byte2 = mem.vram[patternaddr + tileid * 16 + r + 8];

				//u8 byte1 = mem.PpuRead(patternaddr + tileid * 16 + r + 0);
				//u8 byte2 = mem.PpuRead(patternaddr + tileid * 16 + r + 8);

				for (int cl = 0; cl < 8; cl++)
				{
					int col = 7 - cl;
					int row = r;

					if (flipH && flipV)
					{
						col = cl;
						row = 7 - r;
					}
					else if (flipV)
					{
						row = 7 - r;
					}
					else if (flipH)
					{
						col = cl;
					}

					int bit0 = (byte1 & 1) > 0 ? 1 : 0;
					int bit1 = (byte2 & 1) > 0 ? 1 : 0;

					byte1 >>= 1;
					byte2 >>= 1;

					int palindex = bit0 | bit1 * 2;

					int colorindex = palindex + (att & 3) * 4;

					int xp = x + col;
					int yp = y + row;
					if (xp < 0 || xp >= 255 || yp < 0 || yp >= 240)
						break;

					if (palindex != 0)
					{
						u8 bgpalindex = sp0data[256 * (y + row) * 4 + (x + col) * 4 + 0];
						if (bgpalindex != 0 && i == 0 && yp == scanline && x < 255)
							set_sprite_zero();

						if ((att & frontback) == 0 || bgpalindex == 0)
						{
							int color = palettes[mem.vram[paladdr | colorindex]];
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 3] = (u8)(color >> 0);
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 2] = (u8)(color >> 8);
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 1] = (u8)(color >> 16);
							gfx.disp_pixels[256 * (y + row) * 4 + (x + col) * 4 + 0] = (u8)(color >> 24);
						}
					}
				}
			}
		}
	}
}

void Ppu::set_sprite_zero()
{
	ppu2002 |= 0x40;
	mem.ram[0x2002] = ppu2002;
	//sprite0 = true;
}

void Ppu::clear_sprite_zero()
{
	ppu2002 &= 0xbf;
	mem.ram[0x2002] = ppu2002;
	//sprite0 = true;
}

int Ppu::get_attr_index(int x, int y, int attrib)
{
	//get the right attribute
	if ((y & 2) > 0)
	{
		if ((x & 2) > 0)
			return (attrib & 0xc0) >> 6;
		else
			return (attrib & 0x30) >> 4;
	}
	else
	{
		if ((x & 2) > 0)
			return (attrib & 0x0c) >> 2;
		else
			return (attrib & 0x03) >> 0;
	}
}

void Ppu::render_background()
{
	u16 patternaddr = (ppu2000 & 0x10) ? 0x1000 : 0x0000;
	int y = scanline;
	int x = pixel;
	int row = y % 8;
	int shift = 0x80;

	y = 0;

	//for (int x = 0; x < 32; x++)
	//{
	u8 fx = (preg.x + x) & 7;
	u16 ntaddr = 0x2000 | (preg.v & 0xfff);
	u16 ataddr = 0x23c0 | (preg.v & 0xc00) | ((preg.v >> 4) & 0x38) | ((preg.v >> 2) & 0x07);

	if (ntaddr == 0x2000)
	{
		int yu = 0;
	}

	u8 id = mem.ppurb(ntaddr);
	u8 b1 = mem.ppurb(patternaddr + id * 16 + row + 0);
	u8 b2 = mem.ppurb(patternaddr + id * 16 + row + 8);

	int bit0 = b1 >> (8 - fx) & 1;
	int bit1 = b2 >> (8 - fx) & 1;

	//shift = ((preg.v >> 4) & 4) | preg.v & 2;

	u8 bit2 = mem.vram[ataddr + (y / 4) + (x / 4)];

	int palindex = bit0 | bit1 * 2;

	int colorindex = bit2 * 4 + palindex;
	//colorindex = 3;
	int color = palettes[mem.vram[0x3f00 | colorindex]];

	if (ntaddr < 0x2400)
		gfx.disp_pixels[y * 256 + x] = color;

	if ((x + 1) % 8 == 0)
	{
		x_increment();
		//shift = 0x80;
		//gfx.render_frame();
	}

	//}

	if (x == 257)
	{
		y_increment();

	}

}

void Ppu::render_tile()
{
	for (int y = 0; y < 8; y++)
	{
		for (int x = 0; x < 8; x++)
		{

		}
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
