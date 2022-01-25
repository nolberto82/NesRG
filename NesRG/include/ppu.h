#pragma once

#include "types.h"

struct Ppu
{
	int scanline = 0;
	int cycles = 0;

	bool nmi = false;

	void step(int num);
	void ppu_2000_wb(u8 v);
	u8 ppu_2002_rb(u8 v);
	void reset();

private:
	int pixel = 0;
};

extern Ppu ppu;