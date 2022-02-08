#include "mem.h"
#include "ppu.h"
#include "controls.h"

void mem_init()
{
	ram.resize(RAMSIZE);
	vram.resize(VRAMSIZE);
	oam.resize(OAMSIZE);
	oamalt.resize(OAMSIZE - 0x80);
}

bool load_rom(const char* filename)
{
	if (filename == NULL)
		return rom_loaded = false;

	load_file(filename, rom, 0);

	set_mapper();

	return rom_loaded = true;
}

void set_mapper()
{
	int prgbanks = rom[4];
	int chrbanks = rom[5];
	int prgsize = prgbanks * 0x4000;
	int chrsize = chrbanks * 0x2000;
	int mappernum = (rom[6] & 0xf0) >> 4;
	mirrornametable = 0;

	int control = 0;
	int shift = 0;

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
		{
			if (prgbanks == 1)
			{
				memcpy(&ram[0xc000], rom.data() + 0x10, prgsize);
				memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize, chrsize);
			}
			else
			{
				memcpy(&ram[0x8000], rom.data() + 0x10, prgsize);
				memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize, chrsize);
			}
			break;
		}
		case 1:
		{
			memcpy(&ram[0x8000], rom.data() + 0x10, prgsize / prgbanks);
			memcpy(&ram[0xc000], rom.data() + 0x10 + prgsize - (prgsize / prgbanks), prgsize / prgbanks);
			memcpy(&vram[0x0000], rom.data() + 0x10 + prgsize, chrsize);
			break;
		}
		default:
			printf("Mapper not supported");
			break;
	}
}

bool load_file(const char* filename, std::vector<u8>& rom, int offset)
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

	fclose(fp);

	return res;
}

u8 rb(u16 addr)
{
	read_addr = addr;

	if (addr == 0x2002)
		return ppu_status();
	else if (addr == 0x2007)
		return ppu_data_rb();
	else if (addr == 0x4016)
		return controls_read();

	return ram[addr];
}

u8 rbd(u16 addr)
{
	return ram[addr];
}

u16 rw(u16 addr)
{
	return rb(addr + 0) | rb(addr + 1) << 8;
}

void wb(u16 addr, u8 val)
{
	write_addr = addr;

	if (addr >= 0x0000 && addr <= 0x1fff)
		ram[addr & 0x7ff] = val;
	else if (addr == 0x2000)
		ppu_ctrl(val);
	else if (addr == 0x2001)
		ppu_mask(val);
	else if (addr == 0x2003)
		ppu_oam_addr(val);
	else if (addr == 0x2004)
		ppu_oam_data(val);
	else if (addr == 0x2005)
		ppu_scroll(val);
	else if (addr == 0x2006)
		ppu_addr(val);
	else if (addr == 0x2007)
		ppu_data_wb(val);
	else if (addr == 0x4014)
	{
		ppuoamdma = val;
		int oamaddr = val << 8;
		for (int i = 0; i < 256; i++)
			oam[i] = ram[oamaddr + i];
		cycle += 513;
	}
	else if (addr == 0x4016)
		return controls_write(val);
}

void ww(u16 addr, u16 val)
{
	wb(addr, val & 0xff);
	wb(addr, val >> 8);
}

u8 ppurb(u16 addr)
{
	u8 v = 0;
	ppu_read_addr = addr;

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
		else if (addr >= 0x2800 && addr < 0x2c00)
		{
			v = vram[addr - 0x800 & 0x3fff];
		}
		else if (addr >= 0x2c00 && addr < 0x3000)
		{
			v = vram[addr - 0xc00 & 0x3fff];
		}
		else if (addr >= 0x3000 && addr < 0x3400)
		{
			v = vram[addr - 0x1000 & 0x3fff];
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

void ppuwb(u16 addr, u8 val)
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

	for (int i = 0; i < 7; i++)
		memcpy(&vram[0x3f20 + i * 32], &vram[0x3f00], 0x20);

	for (int i = 0; i < 4; i++)
		memcpy(&vram[0x3f04 + i * 0x4], &vram[0x3f10], 0x01);

	if (addr == 0x3f10)
		vram[0x3f00] = val;
	else if (addr == 0x3f00)
		vram[0x3f10] = val;
	//else if (addr == 0x3f10)
	//	vram[addr & 0x3fff] = val;
}

void reset()
{
	//fill(ram.begin)
	//memset(ram, 0x00, 0x10000);
	//memset(vram, 0x00, 0x4000);
}
