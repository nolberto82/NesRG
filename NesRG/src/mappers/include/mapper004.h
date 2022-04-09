#pragma once

#include "types.h"

struct MMC3
{
	u8 fire;
	u8 irq;
	u8 reload;
	u8 rvalue;
	u8 counter;
	u8 write_prot;
	u8 prg_ram;
	u8 chrreg;
	u8 bankreg[8];

	void update(u16 addr, u8 v);
	void reset();
	void scanline();

	u8 prgmode;
	u8 chrmode;
	u16 prgbank;
	u16 chrbank;
	vector<u8> prg;
	vector<u8> chr;
};

extern MMC3 mmc3;