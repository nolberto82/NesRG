#include "mappers.h"
#include "mem.h"

UxROM uxrom;

void UxROM::update(u16 addr, u8 v)
{
	if (addr >= 0xc000 && addr <= 0xffff)
	{
		int prg = 0x10 + 0x4000 * (v & 7);
		MEM::mem_rom(MEM::ram, 0x8000, prg, 0x4000);
	}
}

void UxROM::reset()
{

}
