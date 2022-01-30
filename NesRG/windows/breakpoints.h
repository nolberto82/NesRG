#pragma once

#include "types.h"

struct bplist
{
	u16 addr;
	u8 enabled;
	u8 type;
};

struct Breakpoint
{
public:
	Breakpoint() {}
	~Breakpoint() {}

	void add(u16 addr, u8 type);
	void edit(u16 addr, u8 type, s16 newaddr = -1, bool enabled = true);
	void remove(u16 addr);
	bool check(u16 addr, u8 type, bool enabled);
	bool check_access(u16 addr, u8 type, bool enabled = false);
	std::vector<bplist>::iterator find(u16 addr);

	u16 addr = 0;
	bool enabled = false;
	u8 type = 0;

	std::vector<bplist> breakpoints;
};

enum bpaccesstype
{
	read = 1,
	write = 2,
	exec = 4
};

extern Breakpoint bpk;