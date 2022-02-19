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

bool bp_check(u16 addr, u8 type)
{
	if (type & bp_read || type & bp_vread)
	{
		auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
			{
				return (obj.addr == addr && obj.enabled);
			});
		return it != breakpoints.end();
	}
	else if (type & bp_write || type & bp_vwrite)
	{
		auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
			{
				return (obj.addr == addr && obj.enabled);
			});
		return it != breakpoints.end();
	}
	else
	{
		auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
			{
				return (obj.addr == addr && obj.enabled);
			});
		return it != breakpoints.end();
	}
}