#pragma once

#include "types.h"

//Debug defines
#define BREAKPOINT_MAX 5

struct bplist
{
	u16 addr;
	u8 enabled;
	u8 type;
};

void bp_add(u16 addr, u8 type, bool enabled);
void bp_edit(u16 addr, u8 type, u8 id, bool enabled);
bool bp_check(u16 addr, u8 type, bool enabled);
bool bp_check_access(u16 addr, u8 type, bool enabled = false);

inline std::vector<bplist> breakpoints;

enum bpaccesstype
{
	read = 1,
	write = 2,
	exec = 4
};