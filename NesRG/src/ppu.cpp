#include "ppu.h"
#include "mem.h"
#include "sdlgfx.h"
#include "mappers.h"

SpriteData sprites[8];
namespace PPU
{
	void step()
	{
		rendering = pmask.background || pmask.sprite;
		u8 viewscanlines = scanline >= 0 && scanline < 240;
		u8 emptyscanline = scanline == 240;
		u8 vblankscanline = scanline == 241;
		u8 prescanline = scanline == 261;
		u8 fy = (lp.v & 0x7000) >> 12;

		if ((scanline >= -1 && scanline < 240) && cycle == 260 && rendering)
		{
			if (header.mappernum == 4)
			{
				mmc3.scanline();
			}
		}

		if (viewscanlines && rendering)
		{
			if (FETCH_CYCLES)
			{
				pixels();
				switch (cycle & 7)
				{
					case 1:
						ntaddr = get_nt_addr();
						load_registers();
						break;
					case 2:
						ntbyte = get_nt_byte(ntaddr);
						break;
					case 3:
						ataddr = get_at_addr();
						break;
					case 4:
						atbyte = get_at_byte(ataddr);
						if ((lp.v >> 5) & 2) 
							atbyte >>= 4;
						if (lp.v & 2) 
							atbyte >>= 2;
						break;
					case 5:
						bgaddr = get_bg_addr(fy);
						break;
					case 6:
						lobg = get_bg_lo_byte(bgaddr);
						break;
					case 7:
						bgaddr += 8;
						break;
					case 0:
						hibg = get_bg_hi_byte(bgaddr);
						x_inc();
						break;
				}
			}

			if (cycle == 338 || cycle == 340)
			{
				ntbyte = MEM::ppurb(0x2000 | (lp.v & 0xfff));
			}

			//increment y scroll
			if (cycle == 257)
			{
				y_inc();
				load_registers();
			}

			//copy horizontal bits
			if (cycle == 256)
				lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);

			eval_sprites();
		}
		else if (scanline == 240)
		{
			if (cycle == 1)
			{
				frame++;
				frame_ready = true;
			}
		}
		else if (scanline == 241)
		{
			if (cycle == 1)
			{
				pstatus.vblank = 1;

				MEM::ram[0x2002] |= 0x80;
				if (pctrl.nmi && !no_nmi)
					nmi_triggered = 1;
			}
		}
		else if (scanline == -1)
		{
			if (rendering)
			{
				//copy horizontal bits
				if (cycle == 257)
					lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);

				if ((cycle > 280 && cycle <= 304))
					lp.v = (lp.v & ~0x7be0) | (lp.t & 0x7be0);

				if (cycle == 1)
					SDL::draw_frame(screen_pixels, cpu.state);

				frame_ready = false;

				eval_sprites();
			}
		}

		cycle++;
		if (cycle > 340)
		{
			if (odd_frame() && rendering && scanline == 0)
				cycle = 1;
			else
				cycle = 0;
			scanline++;

			if (scanline == 261)
			{
				scanline = -1;
				sprite_count = 0;
				pstatus.vblank = 0;
				pstatus.sprite0hit = 0;
				pstatus.sproverflow = 0;
				no_nmi = no_vbl = 0;
				MEM::ram[0x2002] &= 0x1f;
			}
		}
	}

	void ctrl(u8 v) //2000
	{
		lp.t = (lp.t & ~0xc00) | (v & 3) << 10;
		pctrl.nametable = (v >> 1) & 3;
		pctrl.vaddr = (v >> 2) & 1;
		pctrl.spraddr = (v >> 3) & 1;
		pctrl.bgaddr = (v >> 4) & 1;
		pctrl.spritesize = (v >> 5) & 1;
		pctrl.nmi = (v >> 7) & 1;

		MEM::ram[0x2000] = v;
	}

	void mask(u8 v)
	{
		MEM::ram[0x2001] = v;
		MEM::ram[0x2002] = v;
		pmask.backgroundleft = (v >> 1) & 1;
		pmask.spriteleft = (v >> 2) & 1;
		pmask.background = (v >> 3) & 1;
		pmask.sprite = (v >> 4) & 1;
	}

	u8 status(u8 cycles)
	{
		u8 v = (pstatus.vblank << 7 | pstatus.sprite0hit << 6 | pstatus.sprite0hit << 5) & 0xe0
			| (dummy2007 & 0x1f);

		//if (scanline == 241)
		//{
		//	if (cycle == 0)
		//	{
		//		no_nmi = no_vbl = 1;
		//		pstatus.vblank = 1;
		//	}
		//	else if (cycle <= 2)
		//		no_nmi = 1;
		//	else if (cycle == 3)
		//		v |= 0x80;
		//}

		MEM::ram[0x2000] |= v;
		MEM::ram[0x2002] &= 0x7f;
		pstatus.vblank = 0;

		lp.w = 0;
		return v;
	}

	void oam_addr(u8 v)
	{
		p2003 = v;
		MEM::ram[0x2003] = v;
	}

	void oam_data(u8 v)
	{
		p2004 = v;
		MEM::ram[0x2004] = v;
		MEM::ram[0x2002] |= 0x1f;
		MEM::ram[0x2003] = p2003++;
	}

	void ppuscroll(u8 v)
	{
		if (!lp.w)
		{
			lp.t &= 0x7fe0;
			lp.t |= ((v & 0xf8) >> 3);
			lp.fx = (u8)(v & 0x07);
			scrolldata = v;
		}
		else
		{
			lp.t = (lp.t & 0xc1f) | (v & 0xf8) << 2 | (v & 7) << 12;
			scrolldata |= v << 8;
		}
		lp.w ^= 1;
	}

	void ppuaddr(u8 v)
	{
		if (!lp.w)
			lp.t = (u16)((lp.t & 0x80ff) | (v & 0x3f) << 8);
		else
		{
			//p2000 &= 0xfc;
			lp.t = (lp.t & 0xff00) | v;
			lp.v = lp.t;
		}
		lp.w ^= 1;
	}

	void data_wb(u8 v)
	{
		MEM::ppuwb(lp.v, v);
		lp.v += pctrl.vaddr ? 32 : 1;
	}

	u8 data_rb()
	{
		u8 v = 0;
		if (lp.v <= 0x3eff)
		{
			v = dummy2007;
			dummy2007 = MEM::ppurb(lp.v);
		}
		else
			v = dummy2007 = MEM::ppurb(lp.v);

		lp.v += pctrl.vaddr ? 32 : 1;
		return v;
	}

	void reset()
	{
		scanline = 0;
		lp.v = lp.t = pmask.background = dummy2007 = 0;
		cycle = 27;
		totalcycles = 8;
		frame = 1;
		frame_ready = true;

		for (int i = 0; i < sizeof(palbuffer); i += 3)
			palettes[i / 3] = palbuffer[i] | palbuffer[i + 1] << 8 | palbuffer[i + 2] << 16 | 0xff000000;

		memset(MEM::vram.data(), 0x00, MEM::vram.size());
		memset(&pctrl, 0x00, sizeof(pctrl));
		memset(&pmask, 0x00, sizeof(pmask));
		memset(&pstatus, 0x00, sizeof(pstatus));
		clear_pixels();
	}

	void clear_pixels()
	{
		memset(screen_pixels, 0x00, sizeof(screen_pixels));
		SDL::draw_frame(screen_pixels, cpu.state);
	}

	void pixels()
	{
		u16 x = cycle - 2;
		u16 y = scanline;

		if (bgshiftlo > 0)
		{
			int yu = 0;
		}

		if (bgshifthi > 0)
		{
			int yu = 0;
		}

		u8 bg_pixel = 0;
		u8 bg_pal = 0;
		u8 spr_pixel = 0;
		u8 spr_pal = 0;
		u8 attrib = 0;
		u8 spx = 0;

		if (pmask.background && BACKGROUND_LEFT)
		{
			bg_pixel = (((bgshiftlo >> (15 - lp.fx)) & 1) | (bgshifthi >> (15 - lp.fx) & 1) * 2);
			bg_pal = ((atshiftlo >> (7 - lp.fx) & 1) | (atshifthi >> (7 - lp.fx) & 1) * 2) & 3;
		}

		if (pmask.sprite && !(x < 8 && !pmask.spriteleft))
		{
			u16 bgaddr = pctrl.spraddr ? 0x1000 : 0x0000;
			u16 oamaddr = 0x0100 * oamdma;

			for (auto& spr : sprites)
			{
				u8 tile = spr.tile;
				attrib = spr.attrib;
				u8 sy = spr.y + 1;
				u8 sx = spx = spr.x;
				u8 fx = (x - sx);
				u8 fy = (y - sy) & (pctrl.spritesize ? 15 : 7);
				if (spr.y >= 239) continue;
				if (fx < 0 || fx > 7) continue;

				u16 spraddr = 0;
				if (!(attrib & 0x40)) fx = 7 - fx;
				if (attrib & 0x80) fy = (pctrl.spritesize ? 15 : 7) - fy;

				if (pctrl.spritesize)
					spraddr = ((tile & 1) * 0x1000) + (tile & 0xfe) * 16 + fy + (fy & 8);
				else
					spraddr = bgaddr + tile * 16 + fy;

				spr_pixel = (MEM::ppurb(spraddr) >> fx & 1) | (MEM::ppurb(spraddr + 8) >> fx & 1) * 2;
				if (!spr_pixel) continue;
				spr_pal = attrib & 3;

				if (spr.spritenum == 0 && bg_pixel && sx != 255 && x != 255 && x < 255 && !pstatus.sprite0hit)
				{
					MEM::ram[0x2002] |= 0x40;
					pstatus.sprite0hit = 1;
				}
				break;
			}
		}

		screen_pixels[y * 256 + x] = 0;
		u8 offset = 0;

		if (bg_pixel && spr_pixel)
		{
			if ((attrib & 0x20) == 0 && spx < 249)
				offset = spr_pixel + spr_pal * 4 + 0x10;
			else
				offset = bg_pixel + bg_pal * 4;
		}
		else if (bg_pixel)
			offset = bg_pixel + bg_pal * 4;
		else if (spr_pixel && spx < 249)
			offset = spr_pixel + spr_pal * 4 + 0x10;

		screen_pixels[y * 256 + x] = palettes[MEM::vram[0x3f00 + offset]];

		update_registers();
	}

	void render_pixels()
	{
		int patternaddr = pctrl.bgaddr ? 0x1000 : 0x0000;
		int y = scanline;
		int x = cycle - 2;
		u8 bkg_pixel = 0; u8 spr_pixel = 0;
		u8 bg_pal = 0; u8 sp_pal = 0;
		u8 attrib = 0; u8 spritenum = 0;
		u8 spx = 0;
		u8 sprleftclip = !(x < 8 && !pmask.spriteleft);

		if (pmask.background && !(x < 8 && !pmask.backgroundleft))
		{
			u16 ppuaddr = 0x2000 | (lp.v & 0xfff);
			u16 attaddr = 0x23c0 | (lp.v & 0xc00) | ((lp.v >> 4) & 0x38) | ((lp.v >> 2) & 0x07);
			u8 fx = (lp.fx + x) & 7;
			u16 bgaddr = patternaddr + MEM::ppurb(ppuaddr) * 16 + ((lp.v & 0x7000) >> 12);//fy
			u8 attr_shift = (lp.v >> 4) & 4 | (lp.v & 2);
			bg_pal = (MEM::ppurb(attaddr) >> attr_shift) & 3;
			bkg_pixel = ((MEM::ppurb(bgaddr) >> (7 - fx)) & 1) | ((MEM::ppurb(bgaddr + 8) >> (7 - fx)) & 1) * 2;
		}

		if (pmask.sprite && !(x < 8 && !pmask.spriteleft))
		{
			u16 bgaddr = pctrl.spraddr ? 0x1000 : 0x0000;
			u16 oamaddr = 0x0100 * oamdma;

			for (auto& spr : sprites)
			{
				u8 tile = spr.tile;
				attrib = spr.attrib;
				u8 sy = spr.y + 1;
				u8 sx = spx = spr.x;
				u8 fx = (x - sx);
				u8 fy = (y - sy) & (pctrl.spritesize ? 15 : 7);
				if (spr.y >= 239) continue;
				if (fx < 0 || fx > 7) continue;

				u16 spraddr = 0;
				if (!(attrib & 0x40)) fx = 7 - fx;
				if (attrib & 0x80) fy = (pctrl.spritesize ? 15 : 7) - fy;

				if (pctrl.spritesize)
					spraddr = ((tile & 1) * 0x1000) + (tile & 0xfe) * 16 + fy + (fy & 8);
				else
					spraddr = bgaddr + tile * 16 + fy;

				spr_pixel = (MEM::ppurb(spraddr) >> fx & 1) | (MEM::ppurb(spraddr + 8) >> fx & 1) * 2;
				if (!spr_pixel) continue;
				sp_pal = attrib & 3;

				if (spr.spritenum == 0 && bkg_pixel && sx != 255 && x != 255 && x < 255 && !pstatus.sprite0hit)
				{
					MEM::ram[0x2002] |= 0x40;
					pstatus.sprite0hit = 1;
				}
				break;
			}
		}

		screen_pixels[y * 256 + x] = 0;
		u8 offset = 0;

		if (bkg_pixel && spr_pixel)
		{
			if ((attrib & 0x20) == 0 && spx < 249)
				offset = spr_pixel + sp_pal * 4 + 0x10;
			else
				offset = bkg_pixel + bg_pal * 4;
		}
		else if (bkg_pixel)
			offset = bkg_pixel + bg_pal * 4;
		else if (spr_pixel && spx < 249)
			offset = spr_pixel + sp_pal * 4 + 0x10;

		screen_pixels[y * 256 + x] = palettes[MEM::vram[0x3f00 + offset]];
	}

	void render_nametables(u16 addrnt, int i, u32* pixels)
	{
		for (int a = addrnt; a < addrnt + 0x3c0; a++)
		{
			int x = (a & 0x1f);
			int y = (a & 0x3e0) >> 5;

			int ppuaddr = 0x2000 | a;
			u16 attaddr = 0x23c0 | (a & 0xc00) | (y / 4) * 8 + (x / 4);
			u16 patternaddr = pctrl.bgaddr ? 0x1000 : 0x0000;

			for (int r = 0; r < 8; r++)
			{
				u16 bgaddr = patternaddr + MEM::vram[ppuaddr] * 16 + r;
				for (int cl = 0; cl < 8; cl++)
				{
					int attr_shift = (ppuaddr >> 4) & 4 | (ppuaddr & 2);
					u8 bit2 = (MEM::vram[attaddr] >> attr_shift) & 3;

					int color = MEM::vram[bgaddr] >> (7 - cl) & 1 |
						(MEM::vram[bgaddr + 8] >> (7 - cl) & 1) * 2;

					int xp = x * 8 + cl;
					int yp = y * 8 + r;

					pixels[yp * 256 + xp] = palettes[MEM::vram[0x3f00 | bit2 * 4 + color]];
				}
			}
		}
	}

	void render_sprites()
	{
		u16 bgaddr = pctrl.spraddr & 0x08 ? 0x1000 : 0x0000;
		u16 oamaddr = 0x0100 * oamdma;

		memset(sprite_pixels, 0x00, sizeof(sprite_pixels));

		for (int j = 0; j >= 0; j--)
		{
			u8 i = j % 64;

			u8 sy = MEM::oam[i + 0];
			u8 tileid = MEM::oam[i + 1];
			u8 attrib = MEM::oam[i + 2];
			u8 sx = MEM::oam[i + 3];

			if (sy > 239)
				continue;

			u8 size = 8;
			if (pctrl.spritesize)
			{
				size = 16;
				if ((tileid & 1))
					bgaddr = ((tileid & 1) * 0x1000) + (tileid & 0xfe);
			}

			bool flipH = attrib & 0x40;
			bool flipV = attrib & 0x80;

			for (int yy = 0; yy < 8; yy++)
			{
				u8 fy = (yy - sy) & 7;
				u8 b1 = MEM::ppurb(bgaddr + tileid * 16 + fy + 0);
				u8 b2 = MEM::ppurb(bgaddr + tileid * 16 + fy + 8);
				for (int xx = 0; xx < 8; xx++)
				{
					u8 fx = (xx - sx) & 7;

					if (!flipH)
						fx = 7 - fx;
					if (flipV)
						fy = 7 - fy;

					int bit0 = (b1 >> fx) & 1;
					int bit1 = (b2 >> fx) & 1;
					u8 palindex = bit0 | bit1 * 2;
					u8 colorindex = palindex + (attrib & 3) * 4;

					int xp = sx + xx;
					int yp = sy + yy;

					if (palindex != 0)
					{
						if ((attrib & 0x20) == 0)
						{
							int color = palettes[MEM::vram[0x3f10 | colorindex]];
							sprite_pixels[yp * 256 + xp] = color;
						}
					}
				}
			}
		}
	}

	void render_pattern()
	{
		u16 bgaddr = 0x0000;
		int tileid = 0;

		for (int i = 0; i < 2; i++)
		{
			for (int y = 0; y < 16; y++)
			{
				for (int x = 0; x < 16; x++)
				{
					for (int yy = 0; yy < 8; yy++)
					{
						u8 b1 = MEM::vram[bgaddr + tileid * 16 + yy + 0];
						u8 b2 = MEM::vram[bgaddr + tileid * 16 + yy + 8];
						for (int xx = 0; xx < 8; xx++)
						{
							int color = b1 >> (7 - xx) & 1 |
								(b2 >> (7 - xx) & 1) * 2;
							int xp = x * 8 + xx;
							int yp = y * 8 + yy;
							pattern_pixels[i][yp * PATTERN_WIDTH + xp] = palettes[MEM::vram[0x3f00 | 0 * 4 + color]];
						}
					}
					tileid++;
				}
			}
			SDL_UpdateTexture(sdl.patscreen[i], NULL, pattern_pixels[i], PATTERN_WIDTH * sizeof(unsigned int));
			SDL_RenderCopy(sdl.renderer, sdl.patscreen[i], NULL, NULL);
			tileid = 0;
			bgaddr = 0x1000;
		}
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

	void eval_sprites()
	{
		if (cycle == 257)
		{
			memset(sprites, 0xff, sizeof(sprites));
			int c = 0;

			for (int i = 0; i < 64; i++)
			{
				if (c >= 8)
				{
					MEM::ram[0x2002] |= (pstatus.sproverflow) << 5; break;
				}

				int yp = scanline - MEM::oam[i * 4 + 0];
				u8 size = pctrl.spritesize ? 16 : 8;
				if (yp > -1 && yp < size)
				{
					sprites[c].y = MEM::oam[i * 4 + 0];
					sprites[c].tile = MEM::oam[i * 4 + 1];
					sprites[c].attrib = MEM::oam[i * 4 + 2];
					sprites[c].x = MEM::oam[i * 4 + 3];
					sprites[c].spritenum = i;
					c++;
				}
			}
		}
	}

	bool ppu_rendering()
	{
		return pmask.background || pmask.sprite;
	}

	bool ppu_clipping()
	{
		return pmask.backgroundleft || pmask.spriteleft;
	}

	bool odd_frame()
	{
		return frame % 2;
	}

	u16 get_nt_addr()
	{
		return 0x2000 | (lp.v & 0xfff);
	}

	u8 get_nt_byte(u16 a)
	{
		return MEM::ppurb(a);
	}

	u16 get_at_addr()
	{
		return 0x23c0 | (lp.v & 0xc00) | ((lp.v >> 4) & 0x38) | ((lp.v >> 2) & 0x07);
	}

	u8 get_at_byte(u16 a)
	{
		return MEM::ppurb(a);
	}

	u16 get_bg_addr(u8 fy)
	{
		return pctrl.bgaddr * 0x1000 + ntbyte * 16 + fy;
	}

	u8 get_bg_lo_byte(u16 addr)
	{
		return MEM::ppurb(addr);
	}

	u8 get_bg_hi_byte(u16 addr)
	{
		return MEM::ppurb(addr);
	}

	void load_registers()
	{
		bgshiftlo = (bgshiftlo & 0xff00) | lobg;
		bgshifthi = (bgshifthi & 0xff00) | hibg;
		atlo = atbyte & 1;
		athi = (atbyte & 2) > 0 ? 1 : 0;
	}

	void update_registers()
	{
		bgshiftlo <<= 1;
		bgshifthi <<= 1;
		atshiftlo = (atshiftlo << 1) | atlo;
		atshifthi = (atshifthi << 1) | athi;
	}
}