#include "breakpoints.h"

void Breakpoint::add(u16 addr, u8 type)
{
	bplist o = { addr,true,type };
	breakpoints.push_back(o);
}

void Breakpoint::edit(u16 addr, u8 type, s16 newaddr, bool enabled)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&addr](const bplist& obj) {return obj.addr == addr; });

	if (it != breakpoints.end())
	{
		int id = std::distance(breakpoints.begin(), it);

		if (newaddr > -1)
			breakpoints[id].addr = newaddr;
		breakpoints[id].type = type;
		breakpoints[id].enabled = enabled;
	}

}

void Breakpoint::remove(u16 addr)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&addr](const bplist& obj) {return obj.addr == addr; });

	if (it != breakpoints.end())
		breakpoints.erase(it);
}

bool Breakpoint::check(u16 addr, u8 type, bool enabled = false)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled && obj.type & type); //breakpoint on execute
		});
	return it != breakpoints.end();
}

bool Breakpoint::check_access(u16 addr, u8 type, bool enabled)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&](const bplist& obj)
		{
			return (obj.addr == addr && obj.enabled && obj.type & type); //breakpoint on access
		});
	return it != breakpoints.end();
}


std::vector<bplist>::iterator Breakpoint::find(u16 addr)
{
	auto it = find_if(breakpoints.begin(), breakpoints.end(), [&addr](const bplist& obj) {return obj.addr == addr; });

	return it;
}