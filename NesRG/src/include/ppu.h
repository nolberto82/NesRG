#pragma once

#include "types.h"

struct RAddr
{
	u8 cx;
	u8 cy;
	u8 nt;
	u8 fy;
};

struct PpuRegisters
{
	u16 v;
	u16 t;
	u8 x;
	u8 w;
};

struct SpriteData
{
	u8 x;
	u8 tile;
	u8 attrib;
	u8 y;
	u8 spritenum;
};

void ppu_step(int num);
void ppu_ctrl(u8 v);
void ppu_mask(u8 v);
u8 ppu_status();
void ppu_oam_addr(u8 v);
void ppu_oam_data(u8 v);
void ppu_scroll(u8 v);
void ppu_addr(u8 v);
void ppu_data_wb(u8 v);
u8 ppu_data_rb();
void ppu_reset();
void clear_pixels();
void render_pixels();
void process_nametables(u16 addrnt, int i, u32* pixels);
void set_vblank();
void clear_vblank();
void set_sprite_zero();
void clear_sprite_zero();
void x_inc();
void y_inc();

struct Ppu
{
	u8 dummy2007;
	u8 oamdma;
	u8 tile_shift;
	u8 scroll_x;
	u8 scroll_y;
	u8 p2000;
	u8 p2001;
	u8 p2002;
	u8 p2003;
	u8 p2004;
	u8 p2005;
	u8 p2006;
	u8 p2007;
	u8 tile_id;
	u8 tile_att;
	u8 tile_lo;
	u8 tile_hi;
	u8 bits_lo[8];
	u8 bits_hi[8];

	u16 scroll = 0;

	u16 nametableaddr;

	u32 frame = 0;
	u32 tempcolor[8];

	int scanline;
	int cycle;
	u32 totalcycles;

	bool spritesize;
	bool nmi_flag;
	bool frame_ready;
	bool background_on;
	bool sprite_on;
	bool tabkey;
	bool oldtabkey;

	u32 screen_pixels[NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT] = {};
	u32 ntable_pixels[4][NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT] = {};
	u32 palettes[192 / 3] = {};

	u8 palbuffer[192] =
	{
		0x61,0x61,0x61,0x00,0x00,0x88,0x1F,0x0D,0x99,0x37,0x13,0x79,0x56,0x12,0x60,0x5D,
		0x00,0x10,0x52,0x0E,0x00,0x3A,0x23,0x08,0x21,0x35,0x0C,0x0D,0x41,0x0E,0x17,0x44,
		0x17,0x00,0x3A,0x1F,0x00,0x2F,0x57,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0xAA,0xAA,0xAA,0x0D,0x4D,0xC4,0x4B,0x24,0xDE,0x69,0x12,0xCF,0x90,0x14,0xAD,0x9D,
		0x1C,0x48,0x92,0x34,0x04,0x73,0x50,0x05,0x5D,0x69,0x13,0x16,0x7A,0x11,0x13,0x80,
		0x08,0x12,0x76,0x49,0x1C,0x66,0x91,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
		0xFC,0xFC,0xFC,0x63,0x9A,0xFC,0x8A,0x7E,0xFC,0xB0,0x6A,0xFC,0xDD,0x6D,0xF2,0xE7,
		0x71,0xAB,0xE3,0x86,0x58,0xCC,0x9E,0x22,0xA8,0xB1,0x00,0x72,0xC1,0x00,0x5A,0xCD,
		0x4E,0x34,0xC2,0x8E,0x4F,0xBE,0xCE,0x42,0x42,0x42,0x00,0x00,0x00,0x00,0x00,0x00,
		0xFC,0xFC,0xFC,0xBE,0xD4,0xFC,0xCA,0xCA,0xFC,0xD9,0xC4,0xFC,0xEC,0xC1,0xFC,0xFA,
		0xC3,0xE7,0xF7,0xCE,0xC3,0xE2,0xCD,0xA7,0xDA,0xDB,0x9C,0xC8,0xE3,0x9E,0xBF,0xE5,
		0xB8,0xB2,0xEB,0xC8,0xB7,0xE5,0xEB,0xAC,0xAC,0xAC,0x00,0x00,0x00,0x00,0x00,0x00
	};
};



extern PpuRegisters lp;
extern Ppu ppu;
extern SpriteData sprites[8];