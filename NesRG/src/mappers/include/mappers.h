#pragma once

#include "mapper001.h"
#include "mapper002.h"
#include "mapper004.h"
#include "mapper009.h"

inline vector<u8> mappers_get_prg(u8 id)
{
	switch (id)
	{
		case 1: return mmc1.prg;
		case 2: return uxrom.prg;
		case 4: return mmc3.prg;
		case 9: return mmc2.prg;
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
		case 9:
		{
			vector<u8> chr;
			chr.push_back(mmc2.chr1[mmc2.latch1]);
			chr.push_back(mmc2.chr2[mmc2.latch2]);
			return chr;
		}
		default:
			return vector<u8>();
	}
}