#include "ppu.h"
#include "mem.h"
#include "gui.h"

void Ppu::step(int num)
{
	//pixel += num * 3;

	if (scanline == -1)
	{
		frame_ready = false;
		memset(gui.disp_pixels, 0x00, sizeof(gui.disp_pixels));
	}

	while (num-- > 0)
	{
		if (scanline > -1 && scanline < 240)
		{
			if (cycle >= 2 && cycle < 258)
				render_pixels();
		}
		else if (scanline == 241 && cycle == 1)
		{
			set_vblank();

			if (ppu2000 & 0x80)
				nmi = true;
		}

		if ((background_on || sprite_on) && cycle == 256)
			y_increment();

		if ((background_on || sprite_on) && cycle == 257)
			preg.v = (preg.v & 0xfbe0) | (preg.t & 0x41f);

		if ((background_on || sprite_on) && (cycle == 328 || cycle == 336))
		{
			x_increment();
		}

		if (scanline == 260)
		{
			if ((background_on || sprite_on) && (cycle >= 280 && cycle <= 304))
				preg.v = (preg.v & ~0x7be0) | (preg.t & 0x7be0);

			if ((background_on || sprite_on) && cycle == 1)
			{
				gui.render_frame();
			}

			clear_vblank();
			clear_sprite_zero();
		}

		cycle++;
		if (cycle > 340)
		{
			cycle -= CYCLES_PER_LINE;
			cycle = 0;
			scanline++;
			if (scanline > 260)
			{
				nmi = false;
				frame_ready = true;
				scanline = -1;
			}
		}
	}
}

void Ppu::ppuctrl(u8 v) //2000
{
	preg.t = (preg.t & 0x73ff) | (v & 3) << 10;
	//nametableaddr = 0x2000 | (v & 3) << 10;

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

	background_on = (ppu2001 & 0x08) > 0;
	sprite_on = (ppu2001 & 0x10) > 0;
}

u8 Ppu::ppustatus()
{
	u8 v = ppu2002 & 0xe0;
	clear_vblank();
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
		preg.t = (preg.t & 0x73e0) | (v & 0xf8) >> 3;
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
	//mem.ppuwb(preg.v, v);
	mem.vram[preg.v] = v;

	if (ppu2000 & 0x04)
		preg.v += 32;
	else
		preg.v++;
}

u8 Ppu::dataread()
{
	u8 v = 0;

	v = mem.ppurb(preg.v);

	if (preg.v < 0x2000)
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
	scanline = 0;
	cycle = 27;
	totalcycles = 8;
	tile_shift = 0;
	ppu.frame_ready = true;
	ppu2000 = 0;
	preg.v = 0;
	preg.t = 0;
	background_on = false;

	for (int i = 0; i < sizeof(palbuffer); i += 3)
	{
		palettes[i / 3] = palbuffer[i] | palbuffer[i + 1] << 8 | palbuffer[i + 2] << 16 | 0xff000000;
	}

	memset(gui.disp_pixels, 0x00, sizeof(gui.disp_pixels));
	gui.render_frame();
	fill(mem.vram.begin(), mem.vram.end(), 0x00);
}

void Ppu::render_pixels()
{
	int patternaddr = ppu2000 & 0x10 ? 0x1000 : 0x0000;
	int y = scanline;
	int x = cycle - 2;

	if (background_on)
	{
		int ppuaddr = 0x2000 | (preg.v & 0xfff);
		u16 attaddr = 0x23c0 | (preg.v & 0xc00) | ((preg.v >> 4) & 0x38) | ((preg.v >> 2) & 0x07);

		u8 fx = (preg.x + x) & 7;
		u8 fy = (preg.v & 0x7000) >> 12;
		u8 cx = (preg.v & 0x1f);
		u8 cy = (preg.v & 0x3e0) >> 5;
		u8 nametable = (preg.v & 0xc00) >> 10;

		int tileid = mem.ppurb(ppuaddr);

		u8 byte1 = mem.ppurb(patternaddr + tileid * 16 + fy + 0);
		u8 byte2 = mem.ppurb(patternaddr + tileid * 16 + fy + 8);

		int attr_shift = (preg.v >> 4) & 4 | (preg.v & 2);
		u8 attr = mem.ppurb(attaddr);
		u8 bit2 = (attr >> attr_shift) & 3;

		int bit0 = (byte1 >> (7 - fx)) & 1;
		int bit1 = (byte2 >> (7 - fx)) & 1;

		int palindex = bit0 | bit1 * 2;
		int colorindex = bit2 * 4 + palindex;

		int color = palettes[mem.vram[0x3f00 | colorindex]];
		gui.disp_pixels[y * 256 + x] = color;

		if (fx == 7)
			x_increment();
	}

	patternaddr = ppu2000 & 0x08 ? 0x1000 : 0x0000;
	u16 oamaddr = 0x0100 * ppuoamdma;
	u8 tileid, sx, sy, i, attr;
	u8 bgpixel = 0;
	u8 sppixel = 0;

	if (sprite_on)
	{
		for (int j = 8; j > 0; j--)
		{
			i = j % 8;

			sy = (u8)(mem.oam[i * 4 + 0] + 1);
			tileid = mem.oam[i * 4 + 1];
			attr = mem.oam[i * 4 + 2];
			sx = mem.oam[i * 4 + 3];;

			int size = 8;
			if (spritesize)
				size = 16;

			bool flipH = (attr >> 6) & 1;
			bool flipV = (attr >> 7) & 1;

			int byte1 = 0;
			int byte2 = 0;


		}
	}
	//int color = 
	//gui.disp_pixels[y * 256 + x] = color;
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
						//u8 bgpalindex = sp0data[256 * (y + row) * 4 + (x + col) * 4 + 0];
						if (i == 0 && yp == scanline && x < 255)
							set_sprite_zero();

						if ((att & frontback) == 0)
						{
							int color = palettes[mem.vram[paladdr | colorindex]];

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
						if (i == 0 && yp == scanline && x < 255)
							set_sprite_zero();

						if ((att & frontback) == 0 || bgpalindex == 0)
						{
							int color = palettes[mem.vram[paladdr | colorindex]];

						}
					}
				}
			}
		}
	}
}

void Ppu::set_vblank()
{
	ppu2002 |= 0x80;
	mem.ram[0x2002] = ppu2002;
}

void Ppu::clear_vblank()
{
	ppu2002 &= 0x7f;
	mem.ram[0x2002] = ppu2002;
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

void Ppu::x_increment()
{
	if ((preg.v & 0x1f) == 0x1f)
		preg.v = (preg.v & ~0x1f) | 0x400;
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
