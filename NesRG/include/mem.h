#pragma once

#include "types.h"

#include "cpu.h"

struct Memory
{
public:
	u8* ram;
	u8* vram;
	u8* rom;

	int mirrornametable = 0;

	bool load(const char* filename);
	void set_mapper();
	u8 rb(u16 addr);
	u16 rw(u16 addr);
	void wb(u16 addr, u8 val);
	void ww(u16 addr, u16 val);
	void reset();

	Memory()
	{
		ram = new u8[0x10000];
		vram = new u8[0x4000];
	}

	~Memory()
	{
		delete ram;
		delete vram;
		delete rom;
	}
};

extern Memory mem;