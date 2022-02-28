#pragma once

#include "types.h"

void mapper001_update(u16 addr, u8 v);
void mapper001_reset();

struct MMC1
{
	u8 reg;
	u8 writes;
	u8 control;
	u8 prgmode;
	u8 chrmode;
	u16 prgbank;
	u16 chrbank;
	int prgrom;
	int chrrom;
};

inline u8 sram_disabled;

extern MMC1 mmc1;