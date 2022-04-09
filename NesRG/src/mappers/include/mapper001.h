#pragma once

#include "types.h"

#include "mappers.h"

//struct Mapper;

struct MMC1
{
	vector<u8> get_prg();
	void update(u16 addr, u8 v);
	void reset();

	u8 writes;
	u8 control;
	u8 prgmode;
	u8 chrmode;
	u16 prgbank;
	u16 chrbank;
	vector<u8> prg;
	vector<u8> chr;
};

inline u8 sram_disabled = 0;

extern MMC1 mmc1;