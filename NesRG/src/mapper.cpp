#include "mappers.h"
#include "mappers.h"

void Mapper::setup(Header h)
{
}

u8 Mapper::rb(u16 addr)
{
	return u8();
}

void Mapper::set_latch(u16 addr, u8 v)
{
}

void Mapper::reset(Header h)
{
}

void Mapper::scanline()
{
}

vector<u8> Mapper::get_prg()
{
	return vector<u8>();
}

vector<u8> Mapper::get_chr()
{
	return vector<u8>();
}
