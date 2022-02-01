#include "ppu.h"
#include "mem.h"
#include <sdlgfx.h>

void Ppu::step(int num)
{
	//pixel += num * 3;

	if (scanline == -1)
	{
		frame_ready = false;
		memset(gfx.disp_pixels, 0x00, sizeof(gfx.disp_pixels));
	}

	while (num-- > 0)
	{
		pixel++;
		if (pixel > 340)
		{
			pixel -= CYCLES_PER_LINE;
			scanline++;

			if (scanline > -1 && scanline < 240)
			{
				render_pixels();
			}
			else if (scanline == 241)
			{
				set_vblank();

				if (ppu2000 & 0x80)
					nmi = true;
			}
			else if (scanline > 260)
			{
				if (background_on || sprite_render)
				{
					gfx.render_frame();
				}

				clear_vblank();
				clear_sprite_zero();

				nmi = false;
				frame_ready = true;
				scanline = -1;
			}
		}
	}
}

void Ppu::ppuctrl(u8 v) //2000
{
	//preg.t = (preg.t & 0x73ff) | (v & 3) << 10;
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
	sprite_render = (ppu2001 & 0x10) > 0;
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
	pixel = 27;
	totalcycles = 8;
	ppu.frame_ready = true;
	ppu2000 = 0;
	preg.v = 0;
	preg.t = 0;
	background_on = false;

	for (int i = 0; i < sizeof(palbuffer); i += 3)
	{
		palettes[i / 3] = palbuffer[i] | palbuffer[i + 1] << 8 | palbuffer[i + 2] << 16 | 0xff000000;
	}

	memset(gfx.disp_pixels, 0x00, sizeof(gfx.disp_pixels));
	//gfx.render_frame();
}

void Ppu::render_pixels()
{
	int patternaddr = (ppu2000 & 0x10) > 0 ? 0x1000 : 0x0000;
	int paladdr = 0x3f00;
	int left8 = (ppu2000 & 0x02) > 0 ? 1 : 0;
	int y = (scanline / 8);
	int x = pixel;

	if (background_on)
	{
		int ppuaddr = 0x2000 | (preg.v & 0xfff);
		u16 attaddr = 0x23c0 | (preg.v & 0xc00) | ((preg.v >> 4) & 0x38) | ((preg.v >> 2) & 0x07);

		if (ppuaddr == 0x2080 || ppuaddr == 0x2480)
		{
			int yu = 0;
		}

		if (scanline == 241)
		{
			int yu = 0;
		}

		for (int x = 0; x < 32; x++)
		{

			u8 fx = preg.x;
			u8 fy = (preg.v & 0x7000) >> 12;
			u8 cx = (preg.v & 0x1f);
			u8 cy = (preg.v & 0x3e0);
			u8 nametable = (preg.v & 0xc00) >> 10;

			int offx = x * 8;
			int offy = y * 8;

			int baseaddr = ppuaddr & 0x2c00;

			int tileid = mem.ppurb(ppuaddr + x);

			u8 bit2 = get_attr_index(x, y, mem.ppurb(attaddr));
			//int row = scanline % 8;

			u8 byte1 = mem.ppurb(patternaddr + tileid * 16 + fy + 0);
			u8 byte2 = mem.ppurb(patternaddr + tileid * 16 + fy + 8);

			int shift = 0x80;

			for (int col = 0; col < 8; col++)
			{
				int bit0 = (byte1 & shift) > 0 ? 1 : 0;
				int bit1 = (byte2 & shift) > 0 ? 1 : 0;

				shift >>= 1;

				int palindex = bit0 | bit1 * 2;

				int colorindex = bit2 * 4 + palindex;

				int xp = offx + col;
				int yp = offy + fy;

				if (xp < 0 || xp >= 256 || yp < 0 || yp >= 240)
					continue;

				int color = palettes[mem.vram[paladdr | colorindex]];
				gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 3] = (u8)(color >> 0);
				gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 2] = (u8)(color >> 8);
				gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 1] = (u8)(color >> 16);
				gfx.disp_pixels[(yp * 256 * 4) + (xp * 4) + 0] = (u8)(color >> 24);

				sp0data[(yp * 256 * 4) + (xp * 4) + 0] = (u8)palindex;
			}
			x_increment();
		}
		y_increment();
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
						//u8 bgpalindex = sp0data[256 * (y + row) * 4 + (x + col) * 4 + 0];
						if (i == 0 && yp == scanline && x < 255)
							set_sprite_zero();

						if ((att & frontback) == 0)
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
						if (i == 0 && yp == scanline && x < 255)
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
		preg.v ^= 0x41f;
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
