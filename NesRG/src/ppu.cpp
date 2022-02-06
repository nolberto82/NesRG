#include "ppu.h"
#include "mem.h"
#include "renderer.h"

void ppu_step(int num)
{
	while (num-- > 0)
	{
		if ((scanline >= 0 && scanline < 240) && (background_on || sprite_on))
		{

			if (cycle > 1 && cycle < 258)
			{
				render_pixels();
			}

			if ((cycle % 8 == 0) && (cycle > 1 && cycle < 255))
				x_inc();

			if (cycle == 256)
				y_inc();

			//copy horizontal bits
			if (cycle == 257)
			{
				lp.v = (lp.v & 0xfbe0) | (lp.t & 0xfbe0);
			}

			if ((background_on || sprite_on) && (cycle == 337 || cycle == 338))
			{
				//cycle++;
			}
		}
		else if (scanline == 240)
		{
			if ((background_on || sprite_on) && cycle == 1)
			{
				render_frame(screen_pixels);
			}
		}
		else if (scanline == 241 && cycle == 1)
		{
			set_vblank();

			if (ppu2000 & 0x80)
				nmi_flag = true;
		}

		if (scanline == 260)
		{
			if ((background_on || sprite_on) && cycle == 1)
			{
				clear_vblank();
				clear_sprite_zero();
			}
		}

		if ((scanline == 261) && (background_on || sprite_on))
		{
			if ((cycle % 8 == 0) && (cycle > 1 && cycle < 255) || (cycle >= 321 && cycle <= 336))
				x_inc();

			if ((cycle >= 280 && cycle <= 304))
				lp.v = (lp.v & 0x41f) | (lp.t & 0xfbe0);

			if (cycle == 1)
			{
				frame_ready = false;
				memset(screen_pixels, 0x00, sizeof(screen_pixels));
			}
		}

		cycle++;
		if (cycle > 340)
		{
			cycle -= CYCLES_PER_LINE;
			//cycle = 0;
			scanline++;
			if (scanline > 261)
			{
				nmi_flag = false;
				frame_ready = true;
				scanline = 0;
			}
		}
	}
}

void ppu_ctrl(u8 v) //2000
{
	lp.t = (lp.t & 0x73ff) | (v & 3) << 10;
	//nametableaddr = 0x2000 | (v & 3) << 10;

	if (v & 0x10)
	{
		ppu2002 |= v;
		ram[0x2002] = ppu2002;
	}

	ppu2000 = v;
}

void ppu_mask(u8 v)
{
	ppu2001 = v;

	background_on = (ppu2001 & 0x08) > 0;
	sprite_on = (ppu2001 & 0x10) > 0;
}

u8 ppu_status()
{
	u8 v = ppu2002 & 0xe0;
	clear_vblank();
	lp.w = 0;

	return v;
}

void ppu_oam_addr(u8 v)
{
	ppu2003 = v;
}

void ppu_oam_data(u8 v)
{
	ppu2004 = v;
}

void ppu_scroll(u8 v)
{
	if (!lp.w)
	{
		lp.t = (lp.t & 0xc1f) | (v & 0xf8) >> 3;
	}
	else
	{
		lp.t = (lp.t & ~0x73e0) | (v & 7) << 12 | (v & 0xf8);
		lp.x = (u8)(v & 0x07);
	}

	lp.w = !lp.w;
}

void ppu_addr(u8 v)
{
	if (!lp.w)
	{
		lp.t = (u16)((lp.t & 0x80ff) | (v & 0x3f) << 8);
	}
	else
	{
		ppu2000 &= 0xfe;
		lp.t = lp.t & 0xff00 | v;
		lp.v = lp.t;
	}

	lp.w = !lp.w;
}

void ppu_data_wb(u8 v)
{
	ppuwb(lp.v, v);

	if (ppu2000 & 0x04)
		lp.v += 32;
	else
		lp.v++;
}

u8 ppu_data_rb()
{
	u8 v = 0;

	v = ppurb(lp.v);

	if (lp.v < 0x2000)
	{
		v = ppu_dummy2007;
		ppu_dummy2007 = vram[lp.v];
	}

	if (ppu2000 & 0x04)
		lp.v += 32;
	else
		lp.v++;

	return v;
}

void ppu_reset()
{
	scanline = 0;
	cycle = 0;
	totalcycles = 7;
	tile_shift = 0;
	frame_ready = true;
	ppu2000 = 0;
	lp.v = 0;
	lp.t = 0;
	background_on = false;

	for (int i = 0; i < sizeof(palbuffer); i += 3)
	{
		palettes[i / 3] = palbuffer[i] | palbuffer[i + 1] << 8 | palbuffer[i + 2] << 16 | 0xff000000;
	}
}

void clear_pixels()
{
	memset(screen_pixels, 0x00, sizeof(screen_pixels));
	render_frame(screen_pixels);
}

void render_pixels()
{
	int patternaddr = ppu2000 & 0x10 ? 0x1000 : 0x0000;
	int y = scanline;
	int x = cycle - 1;

	if (background_on)
	{
		int ppuaddr = 0x2000 | (lp.v & 0xfff);
		u16 attaddr = 0x23c0 | (lp.v & 0xc00) | ((lp.v >> 4) & 0x38) | ((lp.v >> 2) & 0x07);

		u8 fx = (lp.x + x) & 7;
		u8 fy = (lp.v & 0x7000) >> 12;
		u8 cx = (lp.v & 0x1f);
		u8 cy = (lp.v & 0x3e0) >> 5;
		u8 nametable = (lp.v & 0xc00) >> 10;

		u8 tileid = ppurb(ppuaddr);

		u8 byte1 = ppurb(patternaddr + tileid * 16 + fy + 0);
		u8 byte2 = ppurb(patternaddr + tileid * 16 + fy + 8);

		int attr_shift = (lp.v >> 4) & 4 | (lp.v & 2);
		u8 attr = ppurb(attaddr);
		u8 bit2 = (attr >> attr_shift) & 3;

		int bit0 = (byte1 >> (7 - fx)) & 1;
		int bit1 = (byte2 >> (7 - fx)) & 1;

		int palindex = bit0 | bit1 * 2;
		int colorindex = bit2 * 4 + palindex;

		int color = palettes[vram[0x3f00 | colorindex]];

		screen_pixels[y * 256 + x] = color;
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

			sy = (u8)(oam[i * 4 + 0] + 1);
			tileid = oam[i * 4 + 1];
			attr = oam[i * 4 + 2];
			sx = oam[i * 4 + 3];;

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

void render_sprites(u8 frontback)
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

		y = (u8)(oam[i * 4 + 0] + 1);
		tileid = oam[i * 4 + 1];
		att = oam[i * 4 + 2];
		x = oam[i * 4 + 3];// & 0xff + left8;

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
					byte1 = vram[patternaddr + tileid * 16 + rr + 0];
					byte2 = vram[patternaddr + tileid * 16 + rr + 8];
				}
				else
				{
					byte1 = vram[patternaddr + (tileid + 1) * 16 + rr + 0];
					byte2 = vram[patternaddr + (tileid + 1) * 16 + rr + 8];
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
							int color = palettes[vram[paladdr | colorindex]];

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
				byte1 = vram[patternaddr + tileid * 16 + r + 0];
				byte2 = vram[patternaddr + tileid * 16 + r + 8];

				//u8 byte1 = PpuRead(patternaddr + tileid * 16 + r + 0);
				//u8 byte2 = PpuRead(patternaddr + tileid * 16 + r + 8);

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
						//u8 bgpalindex = sp0data[256 * (y + row) * 4 + (x + col) * 4 + 0];
						if (i == 0 && yp == scanline && x < 255)
							set_sprite_zero();

						if ((att & frontback) == 0)
						{
							int color = palettes[vram[paladdr | colorindex]];

						}
					}
				}
			}
		}
	}
}

void set_vblank()
{
	ppu2002 |= 0x80;
	ram[0x2002] = ppu2002;
}

void clear_vblank()
{
	ppu2002 &= 0x7f;
	ram[0x2002] = ppu2002;
}

void set_sprite_zero()
{
	ppu2002 |= 0x40;
	ram[0x2002] = ppu2002;
	//sprite0 = true;
}

void clear_sprite_zero()
{
	ppu2002 &= 0xbf;
	wb(0x2002, ppu2002);
	//sprite0 = true;
}

void x_inc()
{
	if ((lp.v & 0x1f) == 0x1f)
		lp.v = (lp.v & ~0x1f) | 0x400;
	else
		lp.v++;
}

void y_inc()
{
	if ((lp.v & 0x7000) != 0x7000)
	{
		lp.v += 0x1000;
	}
	else
	{
		lp.v &= ~0x7000;
		int y = (lp.v & 0x3e0) >> 5;

		if (y == 29)
		{
			y = 0;
			lp.v ^= 0x800;
		}
		else if (y == 31)
		{
			y = 0;
		}
		else
			y++;

		lp.v = (lp.v & ~0x3e0) | (y << 5);
	}
}
