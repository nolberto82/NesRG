#pragma once

#include "types.h"

struct MMC2
{
	u8 write_prot;
	u8 prg_ram;
	u8 prgreg;
	u8 chrreg[4];

	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v);
	void reset();

	u8 latch1, latch2;
	u8 updatechr;
	u8 prgmode;
	u8 chrmode;
	u16 prgbank;
	u16 chrbank;
	vector<u8> prg;
	vector<u8> chr1;
	vector<u8> chr2;
};

extern MMC2 mmc2;