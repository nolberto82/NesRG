#include "breakpoints.h"

void bp_add(u16 addr, u8 type, bool enabled)
{
	bplist bp = { addr, enabled, type };
	breakpoints.push_back(bp);
}

void bp_edit(u16 addr, u8 type, u8 id, bool enabled)
{
	breakpoints[id].addr = addr;
	breakpoints[id].type = type;
	breakpoints[id].enabled = enabled;
}

bool bp_check(u16 addr, u8 type, bool enabled = false)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled && obj.type & type); //breakpoint on execute
		});
	return it != breakpoints.end();
}

bool bp_check_access(u16 addr, u8 type, bool enabled)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled && obj.type & type); //breakpoint on access
		});
	return it != breakpoints.end();
}