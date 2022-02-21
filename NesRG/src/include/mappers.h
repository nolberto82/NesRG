#pragma once

#include "types.h"

void mapper001(u8 v);
void mapper_reset();

struct MMC1
{
	u8 shift;
	u8 first;
	u8 bank;
	u8 count;
};

extern MMC1 mmc1;