#include "mappers.h"
#include "mem.h"

void Mapper000::setup(struct Header h)
{
	int prgsize = h.prgnum * 0x4000;
	int chrsize = h.chrnum * 0x2000;

	if (h.prgnum == 1)
	{
		memcpy(&MEM::ram[0xc000], MEM::rom.data() + 0x10, prgsize);
		memcpy(&MEM::vram[0x0000], MEM::vrom.data(), chrsize);
	}
	else
	{
		memcpy(&MEM::ram[0x8000], MEM::rom.data() + 0x10, prgsize);
		memcpy(&MEM::vram[0x0000], MEM::vrom.data(), chrsize);
	}
}

void Mapper000::wb(u16 addr, u8 v)
{

}

u8 Mapper000::rb(u16 addr)
{
	return u8();
}

void Mapper000::reset()
{
}

void Mapper000::scanline()
{
}

vector<u8> Mapper000::get_prg()
{
	return vector<u8>();
}

vector<u8> Mapper000::get_chr()
{
	return vector<u8>();
}
