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
	vector<u8> ram;
	vector<u8> vram;
	vector<u8> oam;
	vector<u8> oam2;
	vector<u8> rom;

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
		ram.resize(RAMSIZE);
		vram.resize(VRAMSIZE);
		oam.resize(OAMSIZE);
		oam2.resize(OAMSIZE - 0x80);
	}

	~Memory()
	{

	}
};

extern Memory mem;