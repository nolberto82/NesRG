#include "mem.h"
#include "ppu.h"

bool Memory::load_rom(const char* filename)
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

	return rom_loaded = true;
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
		mirrornametable = mirrortype::vertical;
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
	cpu.read_addr = addr;

	if (addr == 0x2002)
		return ppu.ppustatus();
	if (addr == 0x2007)
		return ppu.dataread();

	return ram[addr];
}

u16 Memory::rw(u16 addr)
{
	return rb(addr + 0) | rb(addr + 1) << 8;
}

void Memory::wb(u16 addr, u8 val)
{
	cpu.write_addr = addr;

	if (addr == 0x2000)
		ppu.ppuctrl(val);
	else if (addr == 0x2001)
		ppu.ppumask(val);
	else if (addr == 0x2003)
		ppu.oamaddrwrite(val);
	else if (addr == 0x2004)
		ppu.oamdatawrite(val);
	else if (addr == 0x2005)
		ppu.scrollwrite(val);
	else if (addr == 0x2006)
		ppu.addrwrite(val);
	else if (addr == 0x2007)
		ppu.datawrite(val);
	else if (addr == 0x4014)
	{
		ppu.ppuoamdma = val;
		int oamaddr = val << 8;
		for (int i = 0; i < 256; i++)
			oam[i] = ram[oamaddr + i];
		ppu.cycles += 513;
	}

	ram[addr] = val;
}

void Memory::ww(u16 addr, u16 val)
{
	wb(addr, val & 0xff);
	wb(addr, val >> 8);
}

u8 Memory::ppurb(u16 addr)
{
	u8 v = 0;

	if (mem.mirrornametable == mirrortype::horizontal)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			v = vram[addr + 0x400 & 0x3fff];
		}
		else if (addr >= 0x2800 && addr < 0x2c00)
		{
			v = vram[addr + 0xc00 & 0x3fff];
		}
		else
			v = vram[addr & 0x3fff];
	}
	else if (mem.mirrornametable == mirrortype::vertical)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			v = vram[addr + 0x1000 & 0x3fff];
			v = vram[addr + 0x800 & 0x3fff];
		}
		else if (addr >= 0x2400 && addr < 0x2800)
		{
			//vram[addr + 0x1000 & 0x3fff] = val;
			v = vram[addr + 0x800 & 0x3fff];
		}
		else if (addr >= 0x2800 && addr < 0x2c00)
		{
			v = vram[addr + 0xc00 & 0x3fff];
			v = vram[addr & 0x3fff];
		}
		else
			v = vram[addr & 0x3fff];
	}

	return v;
}

void Memory::ppuwb(u16 addr, u8 val)
{
	if (addr > 0x2400 && addr < 0x27ff)
	{
		int yu = 0;
	}

	if (mem.mirrornametable == mirrortype::horizontal)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			vram[addr + 0x400 & 0x3fff] = val;
		}
		if (addr >= 0x2800 && addr < 0x2c00)
		{
			vram[addr + 0xc00 & 0x3fff] = val;
		}
	}
	else if (mem.mirrornametable == mirrortype::vertical)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			vram[addr + 0x1000 & 0x3fff] = val;
			vram[addr + 0x800 & 0x3fff] = val;
		}
		if (addr >= 0x2400 && addr < 0x2800)
		{
			//vram[addr + 0x1000 & 0x3fff] = val;
			vram[addr + 0x800 & 0x3fff] = val;
		}
		if (addr >= 0x2800 && addr < 0x2c00)
		{
			vram[addr + 0xc00 & 0x3fff] = val;
			vram[addr & 0x3fff] = val;
		}
	}

	vram[addr & 0x3fff] = val;
}

void Memory::reset()
{
	memset(ram, 0x00, 0x10000);
	memset(vram, 0x00, 0x4000);
}
