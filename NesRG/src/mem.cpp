#include "mem.h"
#include "ppu.h"

bool Memory::load_rom(const char* filename)
{
	if (filename == NULL)
		return rom_loaded = false;

	load_file(filename, rom, 0);

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
				//copy(rom.begin() + 0x10, rom.begin() + 0x10 + prgsize, ram.begin() + 0xc000);
				//copy(rom.begin() + 0x10 + prgsize, rom.begin() + 0x10 + prgsize + chrsize, vram.begin());
				memcpy(&ram[0xc000], rom.data() + 0x10, prgsize);
				memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize, chrsize);
			}
			else
			{
				memcpy(&ram[0x8000], rom.data() + 0x10, prgsize);
				memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize, chrsize);
				//copy(rom.begin() + 0x10, rom.begin() + 0x10 + prgsize, ram.begin() + 0x8000);
				//copy(rom.begin() + 0x10 + prgsize, rom.begin() + 0x10 + prgsize + chrsize, vram.begin());
			}
			break;
	}
}

bool Memory::load_file(const char* filename, std::vector<u8>& rom, int offset)
{
	int size = 0;
	bool res = true;

	FILE* fp;
	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Couldn't open file");
		return false;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (rom.size())
		vector<u8>().swap(rom);

	rom.resize(size);

	fread(&rom[offset], 1, size, fp);
	printf("%08X\n", rom.size());

	fclose(fp);

	return res;
}

u8 Memory::rb(u16 addr)
{
	read_addr = addr;

	if (addr == 0x2002)
		return ppu->ppustatus();
	if (addr == 0x2007)
		return ppu->dataread();

	return ram[addr];
}

u16 Memory::rw(u16 addr)
{
	return rb(addr + 0) | rb(addr + 1) << 8;
}

void Memory::wb(u16 addr, u8 val)
{
	write_addr = addr;

	if (addr >= 0x0000 && addr <= 0x1fff)
		ram[addr & 0x7ff] = val;
	else if (addr == 0x2000)
		ppu->ppuctrl(val);
	else if (addr == 0x2001)
		ppu->ppumask(val);
	else if (addr == 0x2003)
		ppu->oamaddrwrite(val);
	else if (addr == 0x2004)
		ppu->oamdatawrite(val);
	else if (addr == 0x2005)
		ppu->scrollwrite(val);
	else if (addr == 0x2006)
		ppu->addrwrite(val);
	else if (addr == 0x2007)
		ppu->datawrite(val);
	else if (addr == 0x4014)
	{
		ppu->ppuoamdma = val;
		int oamaddr = val << 8;
		for (int i = 0; i < 256; i++)
			oam[i] = ram[oamaddr + i];
		ppu->cycles += 513;
	}
}

void Memory::ww(u16 addr, u16 val)
{
	wb(addr, val & 0xff);
	wb(addr, val >> 8);
}

u8 Memory::ppurb(u16 addr)
{
	u8 v = 0;

	if (mirrornametable == mirrortype::horizontal)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			v = vram[addr & 0x3fff];
		}
		else if (addr >= 0x2400 && addr < 0x2800)
		{
			v = vram[addr - 0x400 & 0x3fff];
		}
		else
		{
			v = vram[addr & 0x3fff];
		}
	}
	else if (mirrornametable == mirrortype::vertical)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			v = vram[addr & 0x3fff];
		}
		else if (addr >= 0x2400 && addr < 0x2800)
		{
			v = vram[addr & 0x3fff];
		}
		else if (addr >= 0x2800 && addr < 0x2c00)
		{
			v = vram[addr - 0x800 & 0x3fff];
		}
		else if (addr >= 0x2c00 && addr < 0x3000)
		{
			v = vram[addr - 0x800 & 0x3fff];
		}
		else
			v = vram[addr & 0x3fff];
	}

	return v;
}

void Memory::ppuwb(u16 addr, u8 val)
{
	ppu_write_addr = addr;

	if (mirrornametable == mirrortype::horizontal)
	{
		if (addr >= 0x2000 && addr < 0x2400)
		{
			vram[addr + 0x400 & 0x3fff] = val;
		}
		if (addr >= 0x2800 && addr < 0x2c00)
		{
			vram[addr + 0xc00 & 0x3fff] = val;
		}
		if (addr >= 0x3000 && addr < 0x3400)
		{
			vram[addr - 0x1000 & 0x3fff] = val;
		}
	}
	else if (mirrornametable == mirrortype::vertical)
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
	//fill(ram.begin)
	//memset(ram, 0x00, 0x10000);
	//memset(vram, 0x00, 0x4000);
}
