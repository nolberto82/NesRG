#include "ppu.h"
#include "mem.h"
#include "renderer.h"

void ppu_step(int num)
{
	while (num-- > 0)
	{
		if ((ppu.scanline >= 0 && ppu.scanline < 240) && (ppu.background_on || ppu.sprite_on))
		{
			if (ppu.cycle > 0 && ppu.cycle < 258)
				render_pixels();

			u8 fx = (lp.x + (ppu.cycle)) & 7;

			if ((ppu.cycle > 0 && ppu.cycle < 256) && (fx == 0))// || (cycle == 328 || cycle == 336))
			{
				x_inc();
			}

			if (ppu.cycle == 256)
				y_inc();

			//copy horizontal bits
			if (ppu.cycle == 257)
				lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);

		}
		else if (ppu.scanline == 240)
		{
			if ((ppu.background_on || ppu.sprite_on) && ppu.cycle == 1)
			{
				render_frame(ppu.screen_pixels);
			}
		}
		else if (ppu.scanline == 241 && ppu.cycle == 1)
		{
			set_vblank();

			if (ppu.p2000 & 0x80)
				ppu.nmi_flag = true;
		}

		if (ppu.scanline == 260)
		{
			if ((ppu.background_on || ppu.sprite_on) && ppu.cycle == 1)
			{
				clear_vblank();
				clear_sprite_zero();
			}
		}

		if ((ppu.scanline == 261) && (ppu.background_on || ppu.sprite_on))
		{
			//copy horizontal bits
			if (ppu.cycle == 257)
				lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);

			if ((ppu.cycle >= 280 && ppu.cycle <= 304))
				lp.v = (lp.v & ~0x7be0) | (lp.t & 0x7be0);

			if (ppu.cycle == 1)
			{
				ppu.frame_ready = false;
				memset(ppu.screen_pixels, 0x00, sizeof(ppu.screen_pixels));
			}
		}

		ppu.cycle++;
		int oddeven = ppu.frame % 2 == 0 && ppu.background_on ? 340 : 341;
		if (ppu.cycle > oddeven)
		{
			ppu.cycle -= oddeven;
			//cycle = 0;
			ppu.scanline++;
			if (ppu.scanline > 261)
			{
				ppu.nmi_flag = false;
				ppu.frame_ready = true;
				ppu.scanline = 0;
				ppu.frame++;
			}
		}
	}
}

void ppu_ctrl(u8 v) //2000
{
	lp.t = (lp.t & ~0xc00) | (v & 3) << 10;
	//nametableaddr = 0x2000 | (v & 3) << 10;

	if (v & 0x10)
	{
		ppu.p2002 |= v;
		ram[0x2002] = ppu.p2002;
	}

	ppu.p2000 = v;
}

void ppu_mask(u8 v)
{
	ppu.p2001 = v;

	ppu.background_on = (ppu.p2001 & 0x08) > 0;
	ppu.sprite_on = (ppu.p2001 & 0x10) > 0;
}

u8 ppu_status()
{
	u8 v = ppu.p2002 & 0xe0;
	clear_vblank();
	lp.w = 0;

	return v;
}

void ppu_oam_addr(u8 v)
{
	ppu.p2003 = v;
}

void ppu_oam_data(u8 v)
{
	ppu.p2004 = v;
}

void ppu_scroll(u8 v)
{
	if (!lp.w)
	{
		lp.t &= 0xffe0;
		lp.t |= ((v & 0xf8) >> 3);
		lp.x = (u8)(v & 0x07);
		ppu.scroll = v;
	}
	else
	{
		lp.t = (lp.t & 0xfc1f) | (v & 0xf8) << 2 | (v & 7) << 12;
		ppu.scroll |= v << 8;
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
		ppu.p2000 &= 0xfc;
		lp.t = lp.t & 0xff00 | v;
		lp.v = lp.t;
	}

	lp.w = !lp.w;
}

void ppu_data_wb(u8 v)
{
	ppuwb(lp.v, v);

	if (ppu.p2000 & 0x04)
		lp.v += 32;
	else
		lp.v++;
}

u8 ppu_data_rb()
{
	u8 v = 0;

	v = ppurb(lp.v);

	if (ppu.dummy2007 < 0x2000)
	{
		v = ppu.dummy2007;
		ppu.dummy2007 = vram[lp.v];
	}

	if (ppu.p2000 & 0x04)
		lp.v += 32;
	else
		lp.v++;

	return v;
}

void ppu_reset()
{
	ppu.scanline = 0;
	ppu.cycle = 7;
	ppu.totalcycles = 8;
	ppu.tile_shift = 0;
	ppu.frame_ready = true;
	ppu.p2000 = 0;
	lp.v = 0;
	lp.t = 0;
	ppu.background_on = false;
	ppu.dummy2007 = 0;

	for (int i = 0; i < sizeof(ppu.palbuffer); i += 3)
	{
		ppu.palettes[i / 3] = ppu.palbuffer[i] | ppu.palbuffer[i + 1] << 8 | ppu.palbuffer[i + 2] << 16 | 0xff000000;
	}
}

void clear_pixels()
{
	memset(ppu.screen_pixels, 0x00, sizeof(ppu.screen_pixels));
	render_frame(ppu.screen_pixels);
}

void render_pixels()
{
	int patternaddr = ppu.p2000 & 0x10 ? 0x1000 : 0x0000;
	int y = ppu.scanline;
	int x = ppu.cycle - 1;
	u8 bgindex = 0;

	if (ppu.background_on)
	{
		u16 ppuaddr = 0x2000 | (lp.v & 0xfff);
		u16 attaddr = 0x23c0 | (lp.v & 0xc00) | ((lp.v >> 4) & 0x38) | ((lp.v >> 2) & 0x07);

		u8 fx = (lp.x + x) & 7;
		u8 fy = (lp.v & 0x7000) >> 12;

		u8 tileid = ppurb(ppuaddr);

		u8 b1 = ppurb(patternaddr + tileid * 16 + fy + 0);
		u8 b2 = ppurb(patternaddr + tileid * 16 + fy + 8);

		u8 attr_shift = (lp.v >> 4) & 4 | (lp.v & 2);
		u8 attr = ppurb(attaddr);
		u8 att = (attr >> attr_shift) & 3;

		bgindex = ((b1 >> (7 - fx)) & 1) | ((b2 >> (7 - fx)) & 1) << 1;
		u8 colorindex = att * 4 + bgindex;
		int color = ppu.palettes[vram[0x3f00 | colorindex]];

		ppu.screen_pixels[y * 256 + x] = color;
	}

	u8 bgpixel = 0;
	u8 sppixel = 0;

	if (ppu.sprite_on && (ppu.cycle > 256 && ppu.cycle <= 320))
	{
		u16 patternaddr = ppu.p2000 & 0x08 ? 0x1000 : 0x0000;
		u16 oamaddr = 0x0100 * ppu.oamdma;
		u8 x, y;
		int tileid, att, i;
		int palindex = 0;

		i = (ppu.cycle % 64) - 1;

		y = (u8)(oam[i * 4 + 0] + 1);
		tileid = oam[i * 4 + 1];
		att = oam[i * 4 + 2];
		x = oam[i * 4 + 3];// & 0xff + left8;

		int size = 8;
		if (ppu.spritesize)
			size = 16;

		bool flipH = (att & 0x40) > 0;
		bool flipV = (att & 0x80) > 0;

		int byte1 = 0;
		int byte2 = 0;

		if ((tileid & 1) == 0)
			patternaddr = 0x1000;
		else
			patternaddr = 0x0000;

		tileid &= 0xfe;

		int shift = 0x80;

		for (int r = 0; r < size; r++)
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

				int bit0 = (byte1 >> (7 - shift)) & 1;
				int bit1 = (byte2 >> (7 - shift)) & 1;

				shift >>= 1;

				palindex = bit0 | bit1 * 2;
				int colorindex = palindex + (att & 3) * 4;

				int xp = x + col;
				int yp = y + row;
				if (xp < 0 || xp >= 255 || yp < 0 || yp >= 240)
					break;

				if (palindex != 0)
				{
					if (att == 0)
					{
						int color = ppu.palettes[vram[0x3f10 | colorindex]];
						ppu.screen_pixels[yp * 256 + xp] = color;
					}
				}
			}

			if (i == 0 && ppu.cycle != 255 && (y + 4) == ppu.scanline && !(ppu.p2002 & 0x40) && ppu.background_on)
				set_sprite_zero();
		}
	}
}

void process_nametables(u16 addrnt, int i, u32* pixels)
{
	for (int a = addrnt; a < addrnt + 0x3c0; a++)
	{
		int x = (a & 0x1f);
		int y = (a & 0x3e0) >> 5;

		int ppuaddr = 0x2000 | (a + (i * 0x400) & 0xfff);
		u16 attaddr = 0x23c0 | (a + (i * 0x400) & 0xc00) | (y / 4) * 8 + (x / 4);
		u16 bgaddr = ppu.p2000 & 0x10 ? 0x1000 : 0x0000;

		int offx = x * 8;
		int offy = y * 8;

		for (int r = 0; r < 8; r++)
		{
			int byte1 = vram[bgaddr + (vram[ppuaddr] * 16) + r + 0];
			int byte2 = vram[bgaddr + (vram[ppuaddr] * 16) + r + 8];

			for (int cl = 0; cl < 8; cl++)
			{
				int attr_shift = (ppuaddr >> 4) & 4 | (ppuaddr & 2);
				u8 attr = ppurb(attaddr);
				u8 bit2 = (attr >> attr_shift) & 3;

				int bit0 = (byte1 & 1) > 0 ? 1 : 0;
				int bit1 = (byte2 & 1) > 0 ? 2 : 0;

				byte1 >>= 1;
				byte2 >>= 1;

				int colorindex = bit2 * 4 + (bit0 | bit1);

				int color = ppu.palettes[vram[0x3f00 | colorindex]];

				int xp = offx + (7 - cl);
				int yp = offy + r;

				pixels[yp * 256 + xp] = color;
			}
		}
	}
}

void set_vblank()
{
	ppu.p2002 |= 0x80;
	ram[0x2002] = ppu.p2002;
}

void clear_vblank()
{
	ppu.p2002 &= 0x7f;
	ram[0x2002] = ppu.p2002;
}

void set_sprite_zero()
{
	ppu.p2002 |= 0x40;
	ram[0x2002] = ppu.p2002;
	//sprite0 = true;
}

void clear_sprite_zero()
{
	ppu.p2002 &= 0xbf;
	wb(0x2002, ppu.p2002);
	//sprite0 = true;
}

void x_inc()
{
	if ((lp.v & 0x1f) == 0x1f)
		lp.v = (lp.v & ~0x1f) ^ 0x400;
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
