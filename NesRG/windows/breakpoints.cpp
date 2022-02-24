#include "breakpoints.h"

void bp_add(u16 addr, u8 type, bool enabled)
{
	auto it = remove_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr);
		});

	if (it != breakpoints.end())
	{
		breakpoints.erase(it, breakpoints.end());
		return;
	}

	bplist bp = { addr, enabled, type };
	breakpoints.push_back(bp);
}

void bp_edit(u16 addr, u8 type, u8 id, bool enabled)
{
	breakpoints[id].addr = addr;
	breakpoints[id].type = type;
	breakpoints[id].enabled = enabled;
}

bool bp_read_access(u16 addr)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled && obj.type & bp_read);
		});
	return it != breakpoints.end();
}

bool bp_write_access(u16 addr)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled && obj.type & bp_write);
		});
	return it != breakpoints.end();
}

bool bp_exec_access(u16 addr)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled);
		});
	return it != breakpoints.end();
}