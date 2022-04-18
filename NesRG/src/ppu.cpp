#include "ppu.h"
#include "mem.h"
#include "sdlcore.h"
#include "mappers.h"

SpriteData sprites[8];

u8 ppu_step(int num, u16 addr)
{
	u8 v = 0;
	for (int n = 0; n < num; n++)
	{
		rendering = pmask.background || pmask.sprite;
		u8 viewcycles = ppu.cycle > 0 && ppu.cycle < 258;
		u8 precycles = ppu.cycle >= 321 && ppu.cycle <= 336;
		u8 viewscanlines = ppu.scanline >= 0 && ppu.scanline < 240;
		u8 emptyscanline = ppu.scanline == 240;
		u8 vblankscanline = ppu.scanline == 241;
		u8 prescanline = ppu.scanline == 261;

		if ((ppu.scanline >= -1 && ppu.scanline < 240) && ppu.cycle == 260 && rendering)
		{
			if (header.mappernum == 4)
			{
				mmc3.scanline();
			}
		}

		if (viewscanlines && rendering)
		{
			render_pixels();

			u8 fx = (lp.x + (ppu.cycle)) & 7;

			//increment x scroll
			if ((ppu.cycle > 0 && ppu.cycle < 255) && (fx == 0))
				x_inc();

			//increment y scroll
			if (ppu.cycle == 257)
				y_inc();

			//if (ppu.cycle == 328 || ppu.cycle == 336)
			//	x_inc();

			//copy horizontal bits
			if (ppu.cycle == 256)
				lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);

			ppu_eval_sprites();
		}
		else if (ppu.scanline == 240)
		{
			if (ppu.cycle == 1)
			{
				ppu.frame++;
				ppu.frame_ready = true;
			}
		}
		else if (ppu.scanline == 241)
		{
			if (ppu.cycle == 1)
			{
				//if ()
				//{
				pstatus.vblank = 1;

				ram[0x2002] |= 0x80;
				if (pctrl.nmi && !ppu.no_nmi)
					nmi_triggered = 1;
				//}
			}
		}
		else if (ppu.scanline == -1)
		{
			if (rendering)
			{
				//copy horizontal bits
				if (ppu.cycle == 257)
					lp.v = (lp.v & ~0x41f) | (lp.t & 0x41f);

				if ((ppu.cycle > 280 && ppu.cycle <= 304))
					lp.v = (lp.v & ~0x7be0) | (lp.t & 0x7be0);

				if (ppu.cycle == 1)
					sdl_frame(ppu.screen_pixels, cpu.state);

				ppu.frame_ready = false;

				ppu_eval_sprites();
			}
		}

		ppu.cycle++;
		if (ppu.cycle > 340)
		{
			if (ppu_odd_frame() && rendering && ppu.scanline == 0)
				ppu.cycle = 1;
			else
				ppu.cycle = 0;
			ppu.scanline++;

			if (ppu.scanline == 261)
			{
				ppu.scanline = -1;
				ppu.sprite_count = 0;
				pstatus.vblank = 0;
				pstatus.sprite0hit = 0;
				pstatus.sproverflow = 0;
				ppu.no_nmi = ppu.no_vbl = 0;
				ram[0x2002] &= 0x1f;
			}
		}
	}
	return v;
}

void ppu_ctrl(u8 v) //2000
{
	lp.t = (lp.t & ~0xc00) | (v & 3) << 10;
	pctrl.nametable = (v >> 1) & 3;
	pctrl.vaddr = (v >> 2) & 1;
	pctrl.spraddr = (v >> 3) & 1;
	pctrl.bgaddr = (v >> 4) & 1;
	pctrl.spritesize = (v >> 5) & 1;
	pctrl.nmi = (v >> 7) & 1;

	ram[0x2000] = v;
}

void ppu_mask(u8 v)
{
	ram[0x2001] = v;
	ram[0x2002] = v;
	pmask.backgroundleft = (v >> 1) & 1;
	pmask.spriteleft = (v >> 2) & 1;
	pmask.background = (v >> 3) & 1;
	pmask.sprite = (v >> 4) & 1;
}

u8 ppu_status(u8 cycles)
{
	u8 v = (pstatus.vblank << 7 | pstatus.sprite0hit << 6 | pstatus.sprite0hit << 5) & 0xe0
		| (ppu.dummy2007 & 0x1f);

	//if (ppu.scanline == 241)
	//{
	//	if (ppu.cycle == 0)
	//	{
	//		ppu.no_nmi = ppu.no_vbl = 1;
	//		pstatus.vblank = 1;
	//	}
	//	else if (ppu.cycle <= 2)
	//		ppu.no_nmi = 1;
	//	else if (ppu.cycle == 3)
	//		v |= 0x80;
	//}

	ram[0x2000] |= v;
	ram[0x2002] &= 0x7f;
	pstatus.vblank = 0;

	lp.w = 0;
	return v;
}

void ppu_oam_addr(u8 v)
{
	ppu.p2003 = v;
	ram[0x2003] = v;
}

void ppu_oam_data(u8 v)
{
	ppu.p2004 = v;
	ram[0x2004] = v;
	ram[0x2002] |= 0x1f;
	ram[0x2003] = ppu.p2003++;
}

void ppu_scroll(u8 v)
{
	if (!lp.w)
	{
		lp.t &= 0x7fe0;
		lp.t |= ((v & 0xf8) >> 3);
		lp.x = (u8)(v & 0x07);
		ppu.scroll = v;
	}
	else
	{
		lp.t = (lp.t & 0xc1f) | (v & 0xf8) << 2 | (v & 7) << 12;
		ppu.scroll |= v << 8;
	}
	lp.w ^= 1;
}

void ppu_addr(u8 v)
{
	if (!lp.w)
		lp.t = (u16)((lp.t & 0x80ff) | (v & 0x3f) << 8);
	else
	{
		//ppu.p2000 &= 0xfc;
		lp.t = (lp.t & 0xff00) | v;
		lp.v = lp.t;
	}
	lp.w ^= 1;
}

void ppu_data_wb(u8 v)
{
	ppuwb(lp.v, v);
	lp.v += pctrl.vaddr ? 32 : 1;
}

u8 ppu_data_rb()
{
	u8 v = 0;
	if (lp.v <= 0x3eff)
	{
		v = ppu.dummy2007;
		ppu.dummy2007 = ppurb(lp.v);
	}
	else
		v = ppu.dummy2007 = ppurb(lp.v);

	lp.v += pctrl.vaddr ? 32 : 1;
	return v;
}

void ppu_reset()
{
	ppu.scanline = ppu.tile_shift = 0;
	lp.v = lp.t = pmask.background = ppu.dummy2007 = 0;
	ppu.cycle = 27;
	ppu.totalcycles = 8;
	ppu.frame = 1;
	ppu.frame_ready = true;

	for (int i = 0; i < sizeof(ppu.palbuffer); i += 3)
		ppu.palettes[i / 3] = ppu.palbuffer[i] | ppu.palbuffer[i + 1] << 8 | ppu.palbuffer[i + 2] << 16 | 0xff000000;

	memset(vram.data(), 0x00, vram.size());
	memset(&pctrl, 0x00, sizeof(pctrl));
	memset(&pmask, 0x00, sizeof(pmask));
	memset(&pstatus, 0x00, sizeof(pstatus));
	clear_pixels();
}

void clear_pixels()
{
	memset(ppu.screen_pixels, 0x00, sizeof(ppu.screen_pixels));
	sdl_frame(ppu.screen_pixels, cpu.state);
}

void render_pixels()
{
	int patternaddr = pctrl.bgaddr ? 0x1000 : 0x0000;
	int y = ppu.scanline;
	int x = ppu.cycle - 1;
	u8 bkg_pixel = 0; u8 spr_pixel = 0;
	u8 bg_pal = 0; u8 sp_pal = 0;
	u8 attrib = 0; u8 spritenum = 0;
	u8 spx = 0;
	u8 sprleftclip = !(x < 8 && !pmask.spriteleft);

	if (ppu.cycle > 0 && ppu.cycle < 258)
	{
		if (pmask.background && !(x < 8 && !pmask.backgroundleft))
		{
			u16 ppuaddr = 0x2000 | (lp.v & 0xfff);
			u16 attaddr = 0x23c0 | (lp.v & 0xc00) | ((lp.v >> 4) & 0x38) | ((lp.v >> 2) & 0x07);
			u8 fx = (lp.x + x) & 7;
			u16 bgaddr = patternaddr + ppurb(ppuaddr) * 16 + ((lp.v & 0x7000) >> 12);//fy
			u8 attr_shift = (lp.v >> 4) & 4 | (lp.v & 2);
			bg_pal = (ppurb(attaddr) >> attr_shift) & 3;
			bkg_pixel = ((ppurb(bgaddr) >> (7 - fx)) & 1) | ((ppurb(bgaddr + 8) >> (7 - fx)) & 1) * 2;
		}

		if (pmask.sprite && !(x < 8 && !pmask.spriteleft))
		{
			u16 bgaddr = pctrl.spraddr ? 0x1000 : 0x0000;
			u16 oamaddr = 0x0100 * ppu.oamdma;
			//ram[0x0787] = 9;

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

				spr_pixel = (ppurb(spraddr) >> fx & 1) | (ppurb(spraddr + 8) >> fx & 1) * 2;
				if (!spr_pixel) continue;
				sp_pal = attrib & 3;

				if (spr.spritenum == 0 && bkg_pixel && sx != 255 && x != 255 && x < 255 && !pstatus.sprite0hit)
				{
					ram[0x2002] |= 0x40;
					pstatus.sprite0hit = 1;
				}
				break;
			}
		}

		ppu.screen_pixels[y * 256 + x] = 0;
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


		ppu.screen_pixels[y * 256 + x] = ppu.palettes[vram[0x3f00 + offset]];
	}
}

void process_nametables(u16 addrnt, int i, u32* pixels)
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
			u16 bgaddr = patternaddr + vram[ppuaddr] * 16 + r;
			for (int cl = 0; cl < 8; cl++)
			{
				int attr_shift = (ppuaddr >> 4) & 4 | (ppuaddr & 2);
				u8 bit2 = (vram[attaddr] >> attr_shift) & 3;

				int color = vram[bgaddr] >> (7 - cl) & 1 |
					(vram[bgaddr + 8] >> (7 - cl) & 1) * 2;

				int xp = x * 8 + cl;
				int yp = y * 8 + r;

				pixels[yp * 256 + xp] = ppu.palettes[vram[0x3f00 | bit2 * 4 + color]];
			}
		}
	}
}

void process_sprites()
{
	u16 bgaddr = pctrl.spraddr & 0x08 ? 0x1000 : 0x0000;
	u16 oamaddr = 0x0100 * ppu.oamdma;

	memset(ppu.sprite_pixels, 0x00, sizeof(ppu.sprite_pixels));

	for (int j = 0; j >= 0; j--)
	{
		u8 i = j % 64;

		u8 sy = oam[i + 0];
		u8 tileid = oam[i + 1];
		u8 attrib = oam[i + 2];
		u8 sx = oam[i + 3];

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
			u8 b1 = ppurb(bgaddr + tileid * 16 + fy + 0);
			u8 b2 = ppurb(bgaddr + tileid * 16 + fy + 8);
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
						int color = ppu.palettes[vram[0x3f10 | colorindex]];
						ppu.sprite_pixels[yp * 256 + xp] = color;
					}
				}
			}
		}
	}
}

void process_pattern()
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
					u8 b1 = vram[bgaddr + tileid * 16 + yy + 0];
					u8 b2 = vram[bgaddr + tileid * 16 + yy + 8];
					for (int xx = 0; xx < 8; xx++)
					{
						int color = b1 >> (7 - xx) & 1 |
							(b2 >> (7 - xx) & 1) * 2;
						int xp = x * 8 + xx;
						int yp = y * 8 + yy;
						ppu.pattern_pixels[i][yp * PATTERN_WIDTH + xp] = ppu.palettes[vram[0x3f00 | 0 * 4 + color]];
					}
				}
				tileid++;
			}
		}
		SDL_UpdateTexture(sdl.patscreen[i], NULL, ppu.pattern_pixels[i], PATTERN_WIDTH * sizeof(unsigned int));
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

void ppu_eval_sprites()
{
	if (ppu.cycle == 257)
	{
		memset(sprites, 0xff, sizeof(sprites));
		int c = 0;

		for (int i = 0; i < 64; i++)
		{
			if (c >= 8)
			{
				ram[0x2002] |= (pstatus.sproverflow) << 5; break;
			}

			int yp = ppu.scanline - oam[i * 4 + 0];
			u8 size = pctrl.spritesize ? 16 : 8;
			if (yp > -1 && yp < size)
			{
				sprites[c].y = oam[i * 4 + 0];
				sprites[c].tile = oam[i * 4 + 1];
				sprites[c].attrib = oam[i * 4 + 2];
				sprites[c].x = oam[i * 4 + 3];
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

bool ppu_odd_frame()
{
	return ppu.frame % 2;
}