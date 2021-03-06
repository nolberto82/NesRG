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
bool bp_read_access(u16 addr);
bool bp_write_access(u16 addr);
bool bp_ppu_write_access(u16 addr);
bool bp_exec_access(u16 addr);

inline std::vector<bplist> breakpoints;

enum bptype
{
	bp_read = 1,
	bp_write = 2,
	bp_exec = 4,
	bp_vread = 8,
	bp_vwrite = 16,
};