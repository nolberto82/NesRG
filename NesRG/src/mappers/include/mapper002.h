#pragma once

#include "types.h"

struct UxROM
{
	void update(u16 addr, u8 v);
	void reset();
	
	u8 prgmode;
	u8 chrmode;
	u16 prgbank;
	u16 chrbank;
	vector<u8> prg;
	vector<u8> chr;
};

extern UxROM uxrom;