#pragma once

#include "types.h"

#include "cpu.h"

const int RAMSIZE = 0x10000;
const int VRAMSIZE = 0x4000;
const int OAMSIZE = 0x100;

enum mirrortype
{
	horizontal,
	vertical
};

struct Memory
{
public:
	u8* ram = nullptr;
	u8* vram = nullptr;
	u8* oam = nullptr;
	u8* rom = nullptr;

	int mirrornametable = 0;

	bool rom_loaded = false;

	bool load_rom(const char* filename);
	void set_mapper();
	u8 rb(u16 addr);
	u16 rw(u16 addr);
	void wb(u16 addr, u8 val);
	void ww(u16 addr, u16 val);
	u8 ppurb(u16 addr);
	void ppuwb(u16 addr, u8 val);
	void reset();

	Memory()
	{
		ram = new u8[RAMSIZE];
		vram = new u8[VRAMSIZE];
		oam = new u8[OAMSIZE];
	}

	~Memory()
	{
		delete ram;
		delete oam;
		delete vram;
		delete rom;
	}
};

extern Memory mem;