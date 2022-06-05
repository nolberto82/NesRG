#include "ppu.h"
#include "cpu.h"
#include "mem.h"
#include "sdlcc.h"
#include "mappers.h"

SpriteData sprites[8];
namespace PPU
{
	void step()
	{
		if (scanline < 240 && cycle == 260 && RENDERING)
		{
			MEM::mapper->scanline();
		}

		if (scanline >= SCAN_START && scanline <= SCAN_END && RENDERING)
		{
			if ((cycle >= CYCLE_START && cycle < CYCLE_END) || (cycle >= CYCLE_PRE1 && cycle <= CYCLE_PRE2))
			{
				render_pixels();
				get_tiles();
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
				eval_sprites();
			}

			//copy horizontal bits
			if (cycle == 256)
				lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);
		}
		else if (scanline == SCAN_IDLE)
		{
			if (cycle == 1)
			{
				frame++;
				frame_ready = true;
			}
		}
		else if (scanline == VBLANK)
		{
			if (cycle == 1)
			{
				pstatus.vblank = 1;
				//MEM::ram[0x2002] |= 0x80;
				if (pctrl.nmi && !no_nmi)
				{
					nmi_triggered = 1;
				}

			}
		}
		else if (scanline == SCAN_PRE)
		{
			if (RENDERING)
			{
				if ((cycle >= CYCLE_START && cycle <= CYCLE_END) || (cycle >= CYCLE_PRE1 && cycle <= CYCLE_PRE2))
					get_tiles();

				//copy horizontal bits
				if (cycle == 257)
				{
					//y_inc();
					lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);
					load_registers();
					eval_sprites();
				}

				if ((cycle > 280 && cycle <= 304))
					lp.v = (lp.v & ~0x7be0) | (lp.t & 0x7be0);

				//if (cycle == 1)
				//	SDL::draw_frame(screen_pixels, cpu.state);

				frame_ready = false;
			}
		}

		cycle++;
		if (cycle >= 340)
		{
			if (odd_frame() && RENDERING && scanline == 0)
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

		if (pctrl.nmi && pstatus.vblank && !nmi_vblank)
			nmi_triggered = 1;

		nmi_vblank = pctrl.nmi;
		MEM::ram[0x2000 & 0x2007] = v;
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

		u8 v = 0;
		if (scanline == 241)
		{
			if (cycle == 1)
			{
				no_nmi = no_vbl = 1;
				pstatus.vblank = 0;

			}
			else if (cycle == 2 || cycle == 3)
			{
				no_nmi = 0;
				//pstatus.vblank = 0;
				//MEM::ram[0x2002] |= 0x80;
				return v;
			}
		}

		v = (pstatus.vblank << 7 | pstatus.sprite0hit << 6 | pstatus.sprite0hit << 5) & 0xe0
			| (dummy2007 & 0x1f);

		MEM::ram[0x2000] |= v;
		MEM::ram[0x2002] &= v;
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
		//MEM::ram[0x2002] |= 0x1f;
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

		//clear ppu pixels
		memset(PPU::screen_pix.data(), 0, PPU::screen_pix.size());
	}

	void init()
	{
		screen_pix.resize(NES_WIDTH * NES_HEIGHT * 4);
		ntable_pix.resize((NES_WIDTH * 2 * NES_HEIGHT * 2));
		for (int i = 0; i < 2; i++)
			patt_pix[i].resize(PATT_WIDTH * 2 * PATT_HEIGHT * 2);
	}

	void render_pixels()
	{
		if (RENDERING)
		{
			u16 x = cycle - 2;
			u16 y = scanline;
			u8 bg_pixel = 0;
			u8 bg_pal = 0;
			u8 spr_pixel = 0;
			u8 spr_pal = 0;
			u8 attrib = 0;
			u8 spx = 0;

			if (BACKGROUND_LEFT)
			{
				bg_pixel = (((bgshiftlo >> (15 - lp.fx)) & 1) | (bgshifthi >> (15 - lp.fx) & 1) * 2);
				bg_pal = ((atshiftlo >> (7 - lp.fx) & 1) | (atshifthi >> (7 - lp.fx) & 1) * 2) & 3;
			}

			if (SPRITE_LEFT)
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

					if (spr.spritenum == 0)
					{
						if (bg_pixel)
						{
							if (sx != 255 && x != 255 && x < 255 && !pstatus.sprite0hit)
							{
								MEM::ram[0x2002] |= 0x40;
								pstatus.sprite0hit = 1;
							}
						}
					}
					break;
				}
			}

			screen_pix[y * 256 + x] = 0;
			u8 offset = 0;

			if (bg_pixel && spr_pixel)
			{
				if ((attrib & 0x20) == 0 && spx <= 249)
					offset = spr_pixel + spr_pal * 4 + 0x10;
				else
					offset = bg_pixel + bg_pal * 4;
			}
			else if (bg_pixel)
				offset = bg_pixel + bg_pal * 4;
			else if (spr_pixel && spx <= 249)
				offset = spr_pixel + spr_pal * 4 + 0x10;

			screen_pix[y * 256 + x] = palettes[MEM::vram[0x3f00 + offset]];

			update_registers();
		}
	}

	void render_nametable()
	{

		memset(PPU::ntable_pix.data(), 0, PPU::ntable_pix.size());
		for (int y = 0; y < 480; y++)
		{
			u16 a = 0;
			for (int x = 0; x < 512; x++)
			{
				if (header.mirror == mirrortype::vertical && x >= 256)
				{
					a = 0x400;
				}

				if (header.mirror == mirrortype::horizontal && y >= 240)
				{
					a = 0x800;
				}

				u16 ppuaddr = 0x2000 + a + ((x % 256) / 8) + ((y % 240) / 8) * 32;
				u8 ntid = (ppuaddr >> 10) & 3;

				u16 attaddr = 0x23c0 | (ppuaddr & 0xc00) | ((ppuaddr >> 4) & 0x38) | ((ppuaddr >> 2) & 0x07);
				u16 patternaddr = pctrl.bgaddr ? 0x1000 : 0x0000;

				s16 fy = y & 7;
				s16 fx = x & 7;
				u16 bgaddr = patternaddr + MEM::vram[ppuaddr] * 16 + fy;
				u8 attr_shift = (ppuaddr >> 4) & 4 | (ppuaddr & 2);
				u8 bit2 = (MEM::vram[attaddr] >> attr_shift) & 3;

				u8 color = MEM::vram[bgaddr] >> (7 - fx) & 1 |
					(MEM::vram[bgaddr + 8] >> (7 - fx) & 1) * 2;
				ntable_pix[y * 512 + x] = palettes[MEM::vram[0x3f00 | bit2 * 4 + color]];
			}
		}
		glBindTexture(GL_TEXTURE_2D, SDL::nametable);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 480, 0, GL_RGBA, GL_UNSIGNED_BYTE, PPU::ntable_pix.data());
	}

	void render_sprites()
	{
		u16 bgaddr = pctrl.spraddr & 0x08 ? 0x1000 : 0x0000;
		u16 oamaddr = 0x0100 * oamdma;

		//memset(sprite_pix, 0x00, sizeof(sprite_pixels));

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
							sprite_pix[yp * 256 + xp] = color;
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
							patt_pix[i][yp * PATT_WIDTH + xp] = palettes[MEM::vram[0x3f00 | 0 * 4 + color]];
						}
					}
					tileid++;
				}
			}
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
					MEM::ram[0x2002] |= (pstatus.sproverflow) << 5;
					break;
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
		return (frame % 2) == 0;
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

	void get_tiles()
	{
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
			bgaddr = get_bg_addr(FINE_Y);
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
}