#pragma once

#include "types.h"

void mapper001_update(u16 addr, u8 v);
void mapper001_reset();
void mapper002_update(u16 addr, u8 v);
void mapper002_reset();
void mapper004_update(u16 addr, u8 v);
void mapper004_scanline();
void mapper004_reset();

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

struct UxROM
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

struct MMC4
{
	u8 fire;
	u8 irq;
	u8 reload;
	u8 rvalue;
	u8 counter;
	u8 write_prot;
	u8 prg_ram;
	u8 prgmode;
	u8 prgbank;
	u8 chrmode;
	u8 chrreg;
	u8 bankreg[8];
	u8 prg[4];
	u8 chr[8];
};

inline u8 sram_disabled;

extern MMC1 mmc1;
extern UxROM uxrom;
extern MMC4 mmc4;