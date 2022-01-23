#include "mem.h"
#include "ppu.h"

bool Memory::load(const char* filename)
{
	if (filename == NULL)
		return 0;

	FILE* fp = fopen(filename, "rb");

	if (fp == NULL)
		return false;

	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);
	rewind(fp);

	rom = (u8*)malloc(fsize);

	if (!rom)
	{
		fclose(fp);
		return 0;
	}

	int romsize = fread(rom, sizeof(u8), fsize, fp);

	if (romsize == 0)
	{
		free(rom);
		fclose(fp);
		return 0;
	}

	fclose(fp);

	set_mapper();

	return true;
}

void Memory::set_mapper()
{
	int prgbanks = rom[4];
	int chrbanks = rom[5];
	int prgsize = prgbanks * 0x4000;
	int chrsize = chrbanks * 0x2000;
	int mappernum = rom[8] & 0x0f;
	mirrornametable = 0;

	if (rom[6] & 0x08)
	{
		mirrornametable = 2;
	}
	else if (rom[6] & 0x01)
	{
		mirrornametable = 1;
	}

	reset();

	switch (mappernum)
	{
		case 0:
			if (prgbanks == 1)
			{
				memcpy(&ram[0xc000], rom + 0x10, prgsize);
				memcpy(&vram[0x0000], rom + 0x10 + 0x4000, chrsize);
			}
			else
			{
				memcpy(&ram[0x8000], rom + 0x10, prgsize);
				memcpy(&vram[0x0000], rom + 0x10 + prgsize, chrsize);
			}
			break;
	}
}

u8 Memory::rb(u16 addr)
{
	if (addr == 0x2002)
		return ppu.ppu_2002_rb(ram[0x2002]);

	return ram[addr];
}

u16 Memory::rw(u16 addr)
{
	
	//if (addr == 0x2002)
	//	return ppu.ppu_2002_rb();

	return rb(addr + 0) | rb(addr + 1) << 8;
}

void Memory::wb(u16 addr, u8 val)
{
	if (addr == 0x2000)
		ppu.ppu_2000_wb(val);

	ram[addr] = val;
}

void Memory::ww(u16 addr, u16 val)
{
	wb(addr, val & 0xff);
	wb(addr, val >> 8);
}

void Memory::reset()
{
	memset(ram, 0x00, 0x10000);
	memset(vram, 0x00, 0x4000);
}
