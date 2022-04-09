#pragma once

#include "mapper001.h"
#include "mapper002.h"
#include "mapper004.h"

inline vector<u8> mappers_get_prg(u8 id)
{
	switch (id)
	{
		case 1: return mmc1.prg;
		case 2: return uxrom.prg;
		case 4: return mmc3.prg;
		default:
			return vector<u8>();
	}
}

inline vector<u8> mappers_get_chr(u8 id)
{
	switch (id)
	{
		case 1: return mmc1.chr;
		case 2: return uxrom.chr;
		case 4: return mmc3.chr;
		default:
			return vector<u8>();
	}
}