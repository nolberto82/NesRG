#pragma once

#include "types.h"

void mapper001(u16 addr, u8 v);
void mapper001_update(u16 addr, u8 v);
void mapper001_reset();

struct MMC1
{
	u8 reg;
	u8 count;
	u8 control;
	u8 prgmode;
	u8 chrmode;	
};

inline u8 sram_disabled;

extern MMC1 mmc1;