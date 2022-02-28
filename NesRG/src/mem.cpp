#include "mem.h"
#include "ppu.h"
#include "controls.h"
#include "mappers.h"

void mem_init()
{
	ram.resize(RAMSIZE);
	vram.resize(VRAMSIZE);
	oam.resize(OAMSIZE);
	oamalt.resize(32);
}

bool load_rom(const char* filename)
{
	if (filename == NULL)
		return rom_loaded = false;

	vector<u8> hdr;
	load_file(filename, hdr, 0, 16);
	load_file(filename, rom, 0, hdr[4] * 0x4000 + 0x10);
	load_file(filename, vrom, rom.size(), hdr[5] * 0x2000);

	fs::path p(filename);
	header.name = p.filename().string();

	return rom_loaded = set_mapper();
}

bool set_mapper()
{
	int prgbanks = header.prgnum = rom[4];
	int chrbanks = header.chrnum = rom[5];
	int prgsize = prgbanks * 0x4000;
	int chrsize = chrbanks * 0x2000;
	int mappernum = header.mappernum = (rom[6] & 0xf0) >> 4;
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

	header.mirror = mirrornametable;
	header.battery = (rom[6] >> 1) & 1;
	header.trainer = (rom[6] >> 2) & 1;

	ppu_reset();

	switch (mappernum)
	{
		case 0:
		{
			if (prgbanks == 1)
			{
				memcpy(&ram[0xc000], rom.data() + 0x10, prgsize);
				memcpy(&vram[0x0000], vrom.data(), chrsize);
			}
			else
			{
				memcpy(&ram[0x8000], rom.data() + 0x10, prgsize);
				memcpy(&vram[0x0000], vrom.data(), chrsize);
			}
			break;
		}
		case 1:
		{
			memcpy(&ram[0x8000], rom.data() + 0x10, prgsize / prgbanks);
			memcpy(&ram[0xc000], rom.data() + 0x10 + prgsize - (prgsize / prgbanks), prgsize / prgbanks);
			if (chrbanks > 0)
				memcpy(&vram[0x0000], vrom.data(), chrsize / chrbanks);
			break;
		}
		default:
			printf("Mapper not supported");
			return false;
	}

	cpu_reset();

	return true;
}

bool load_file(const char* filename, std::vector<u8>& data, int offset, int size)
{
	bool res = true;

	FILE* fp;
	fp = fopen(filename, "rb");

	if (fp == NULL)
	{
		printf("Couldn't open file");
		return false;
	}

	//fseek(fp, 0, SEEK_END);
	//size = ftell(fp);
	fseek(fp, offset, SEEK_SET);

	if (data.size())
		vector<u8>().swap(data);

	data.resize(size);

	fread(&data[0], 1, size, fp);

	fclose(fp);

	return res;
}

u8 rb(u16 addr)
{
	read_addr = addr;

	if (addr >= 0x2000 && addr <= 0x2fff)
	{
		if ((addr & 0x7) == 0x02)
			return ppu_status();
		if ((addr & 0x7) == 0x07)
			return ppu_data_rb();
	}
	else if (addr == 0x4016)
		return controls_read();
	else if (addr >= 0x6000 && addr <= 0x7fff && !sram_disabled)
		return ram[addr];

	return ram[addr];
}

u8 rbd(u16 addr)
{
	return ram[addr];
}

u16 rw(u16 addr)
{
	return rbd(addr + 0) | rbd(addr + 1) << 8;
}

void wb(u16 addr, u8 v)
{
	write_addr = addr;

	if (addr >= 0x0000 && addr <= 0x1fff)
		ram[addr & 0x7ff] = v;
	else if (addr == 0x2000)
		ppu_ctrl(v);
	else if (addr == 0x2001)
		ppu_mask(v);
	else if (addr == 0x2003)
		ppu_oam_addr(v);
	else if (addr == 0x2004)
		ppu_oam_data(v);
	else if (addr == 0x2005)
		ppu_scroll(v);
	else if (addr == 0x2006)
		ppu_addr(v);
	else if (addr == 0x2007)
		ppu_data_wb(v);
	else if (addr == 0x4014)
	{
		ppu.oamdma = v;
		int oamaddr = v << 8;
		for (int i = 0; i < 256; i++)
			oam[i] = ram[oamaddr + i];
		ppu.cycle += 513;
	}
	else if (addr == 0x4016)
		controls_write(v);
	else if (addr >= 0x6000 && addr <= 0x7fff && !sram_disabled)
		ram[addr] = v;
	else if (addr >= 0x8000)
	{
		switch (header.mappernum)
		{
			case 1:
				mapper001_update(addr, v);
				break;
			default:
				break;
		}
	}
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
			v = vram[addr & 0x3fff];
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
			vram[addr & 0x3fff] = val;
			vram[addr + 0x400 & 0x3fff] = val;
		}
		if (addr >= 0x2800 && addr < 0x2c00)
		{
			vram[addr & 0x3fff] = val;
			vram[addr + 0x400 & 0x3fff] = val;
		}
		if (addr >= 0x2c00 && addr < 0x3000)
		{
			vram[addr & 0x3fff] = val;
			vram[addr - 0x400 & 0x3fff] = val;
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

void mem_rom(vector<u8>& dst, u16 addr, int offset, int size)
{
	memcpy(dst.data() + addr, rom.data() + offset, size);
}

void mem_vrom(vector<u8>& dst, u16 addr, int offset, int size)
{
	if (!header.chrnum)
		return;
	memcpy(dst.data() + addr, vrom.data() + offset, size);
}
