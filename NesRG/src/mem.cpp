#include "mem.h"
#include "ppu.h"
#include "apu.h"
#include "controls.h"
#include "mappers.h"

namespace MEM
{
	void init()
	{
		ram.resize(RAMSIZE);
		vram.resize(VRAMSIZE);
		oam.resize(OAMSIZE);
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
		int mappernum = header.mappernum = (rom[6] & 0xf0) >> 4 | (rom[7] & 0xf0);
		header.mirror = 0;

		int control = 0;
		int shift = 0;

		if (rom[6] & 0x01)
			header.mirror = mirrortype::vertical;
		else
			header.mirror = mirrortype::horizontal;

		header.battery = (rom[6] >> 1) & 1;
		header.trainer = (rom[6] >> 2) & 1;

		PPU::reset();
		mmc3.counter = 0;

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
		case 1: case 2: case 4:
		{
			memcpy(&ram[0x8000], rom.data() + 0x10, prgsize / prgbanks);
			memcpy(&ram[0xc000], rom.data() + 0x10 + prgsize - (prgsize / prgbanks), prgsize / prgbanks);
			if (chrbanks > 0)
			{
				memcpy(&vram[0x0000], vrom.data(), chrsize / chrbanks);
			}

			if (mappernum == 4)
				mmc3.reset();
			break;
		}
		case 9:
		{
			memcpy(&ram[0xa000], rom.data() + 0x10 + prgsize - 0x6000, prgsize / prgbanks / 2);
			memcpy(&ram[0xc000], rom.data() + 0x10 + prgsize - (prgsize / prgbanks), prgsize / prgbanks);
			if (chrbanks > 0)
			{
				memcpy(&vram[0x0000], vrom.data() + 0x8000, chrsize / chrbanks / 2);
				memcpy(&vram[0x1000], vrom.data() + 0x7000, chrsize / chrbanks / 2);
			}
			mmc2.reset();
			break;
		}
		default:
			printf("Mapper not supported");
			return false;
		}

		CPU::reset();

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

		fseek(fp, offset, SEEK_SET);

		if (data.size())
			vector<u8>().swap(data);

		data.resize(size);

		fread(&data[0], 1, size, fp);

		fclose(fp);

		return res;
	}

	u8 rb(u16 addr, u8 cycles)
	{
		read_addr = addr;

		PPU_STEP;
		cpu.cpucycles++;
		PPU::totalcycles++;

		if (addr >= 0x2000 && addr <= 0x2fff)
		{
			if ((addr & 0x7) == 0x02)
				return PPU::status(cycles);
			if ((addr & 0x7) == 0x07)
				return PPU::data_rb();
		}
		else if (addr == 0x4015)
			return apu.rb();
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

		PPU_STEP;
		cpu.cpucycles++;
		PPU::totalcycles++;

		if (addr >= 0x0000 && addr <= 0x1fff)
			ram[addr & 0x7ff] = v;
		else if (addr == 0x2000)
			PPU::ctrl(v);
		else if (addr == 0x2001)
			PPU::mask(v);
		else if (addr == 0x2003)
			PPU::oam_addr(v);
		else if (addr == 0x2004)
			PPU::oam_data(v);
		else if (addr == 0x2005)
			PPU::ppuscroll(v);
		else if (addr == 0x2006)
			PPU::ppuaddr(v);
		else if (addr == 0x2007)
			PPU::data_wb(v);
		else if (addr == 0x4014)
		{
			u16 oamaddr = v << 8; PPU::oamdma = v;
			for (int i = 0; i < 256; i++)
			{
				oam[i] = ram[oamaddr + i];
				PPU_STEP;
				PPU_STEP;
				PPU::totalcycles += 2;
			}
			PPU_STEP;
			if (PPU::cycle & 1)
				PPU::totalcycles++;
		}
		else if ((addr >= 0x4000 && addr <= 0x4013) || (addr == 0x4015))
			apu.wb(addr, v);
		else if (addr == 0x4016)
			controls_write(v);
		else if (addr == 0x4017)
			apu.wb(addr, v & 0xc0);
		else if (addr >= 0x6000 && addr <= 0x7fff && !sram_disabled)
			ram[addr] = v;
		else if (addr >= 0x8000)
		{
			switch (header.mappernum)
			{
			case 1:
				mmc1.update(addr, v);
				break;
			case 2:
				uxrom.update(addr, v);
				break;
			case 4:
				mmc3.update(addr, v);
				break;
			case 9:
				mmc2.update(addr, v);
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
		addr &= 0x3fff;
		u16 a = addr;
		u8 v = 0;
		ppu_read_addr = addr;

		v = vram[addr];

		if (header.mappernum == 9 && addr < 0x2000)
			mmc2.set_latch(addr, v);

		if (header.mirror == mirrortype::horizontal)
		{
			if (a >= 0x2000 && a < 0x3000)
				v = vram[0x2000 + (a % 0x400) + mirrorhor[(a >> 10) - 8] * 0x400];
		}
		if (header.mirror == mirrortype::vertical)
		{
			if (a >= 0x2000 && a < 0x3000)
				v = vram[0x2000 + (a % 0x400) + mirrorver[(a >> 10) - 8] * 0x400];
		}
		else if (header.mirror == mirrortype::single_nt0)
		{
			if (a >= 0x2000 && a < 0x3000)
				v = vram[0x2000 + (a % 0x400) + mirrornt0[(a >> 10) - 8] * 0x400];
		}
		else if (header.mirror == mirrortype::single_nt1)
		{
			if (a >= 0x2000 && a < 0x3000)
				v = vram[0x2000 + (a % 0x400) + mirrornt1[(a >> 10) - 8] * 0x400];
		}
		return v;
	}

	void ppuwb(u16 addr, u8 v)
	{
		addr &= 0x3fff;
		ppu_write_addr = addr; //ppu breakpoint address
		u16 a = addr;

		if (a < 0x2000)
		{
			int yu = 0;
		}

		vram[a] = v;

		if (header.mirror == mirrortype::horizontal)
		{
			if (a >= 0x2000 && a < 0x3f00)
				vram[0x2000 + (a % 0x400) + mirrorhor[(a >> 10) - 8] * 0x400] = v;
		}
		if (header.mirror == mirrortype::vertical)
		{
			if (a >= 0x2000 && a < 0x3000)
			{
				vram[0x2000 + (a % 0x400) + mirrorver[(a >> 10) - 8] * 0x400] = v;
			}

		}
		if (header.mirror == mirrortype::single_nt0)
		{
			if (a >= 0x2000 && a < 0x3000)
				vram[0x2000 + (a % 0x400) + mirrornt0[(a >> 10) - 8] * 0x400] = v;
		}
		if (header.mirror == mirrortype::single_nt1)
		{
			if (a >= 0x2000 && a < 0x3000)
				vram[0x2000 + (a % 0x400) + mirrornt1[(a >> 10) - 8] * 0x400] = v;
		}

		for (int i = 0; i < 7; i++)
			memcpy(&vram[0x3f20 + i * 32], &vram[0x3f00], 0x20);

		for (int i = 0; i < 4; i++)
			memcpy(&vram[0x3f04 + i * 0x4], &vram[0x3f10], 0x01);

		if (addr == 0x3f10)
			vram[0x3f00] = v;
		else if (addr == 0x3f00)
			vram[0x3f10] = v;
		//else if (addr == 0x3f10)
		//	vram[addr & 0x3fff] = v;
	}

	void mem_rom(vector<u8>& dst, u16 addr, int offset, int size)
	{
		if (offset >= rom.size())
			return;
		memcpy(dst.data() + addr, rom.data() + offset, size);
	}

	void mem_vrom(vector<u8>& dst, u16 addr, int offset, int size)
	{
		if (!header.chrnum)
			return;
		if (offset >= vrom.size())
			return;
		memcpy(dst.data() + addr, vrom.data() + offset, size);
	}

	int cyclecount()
	{
		return FRAME_CYCLES + 1;
	}
}