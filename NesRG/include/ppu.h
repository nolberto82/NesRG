#pragma once

#include "types.h"

struct Ppu
{
	int scanline = 0;
	int cycle = 0;
	bool nmi = false;

	void step();
	void ppu_2000_wb(u8 v);
	u8 ppu_2002_rb(u8 v);

	void set_scanline()
	{
		if (scanline > 260)
			scanline = -1;
		else
			scanline++;
	}
};

extern Ppu ppu;