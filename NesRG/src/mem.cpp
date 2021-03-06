#include "mem.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "controls.h"
#include "mappers.h"
#include <main.h>

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

		if (mapper)
		{
			delete mapper;
			mapper = nullptr;
		}

		switch (mappernum)
		{
		case 0:
			mapper = new Mapper000();
			break;
		case 1:
			mapper = new Mapper001();
			break;
		case 2:
			mapper = new Mapper002();
			break;
		case 3:
			mapper = new Mapper003();
			break;
		case 4:
			mapper = new Mapper004();
			mapper->counter = 0;
			break;
		case 5:
			mapper = new Mapper005();
			break;
		case 7:
			mapper = new Mapper007();
			mapper->counter = 0;
			break;
		case 9:
			mapper = new Mapper009();
			break;
		default:
			printf("Mapper not supported");
			return false;
		}

		mapper->reset();
		mapper->setup();
		CPU::reset();

		MEM::load_sram()
			;
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

	u8 rb(u16 addr, u8 opbit)
	{
		u8 v = 0;
		read_addr = addr;

		apply_cheats(addr);

		v = ram[addr];

		PPU_STEP;
		//PPU::step();
		//PPU::step();
		cpu.cpucycles++;
		PPU::totalcycles++;

		if (addr >= 0x2000 && addr <= 0x3fff)
		{
			if ((addr & 0x7) == 0x02)
				v = PPU::status(opbit);
			if ((addr & 0x7) == 0x07)
				v = PPU::data_rb();
		}
		else if (addr == 0x4015)
			v = APU::rb();
		else if (addr == 0x4016)
			v = controls_read();
		else if (addr >= 0x6000 && addr <= 0x7fff && mapper->sram)
			v = ram[addr];

		//PPU::step();

		return v;
	}

	void apply_cheats(u16 addr)
	{
		for (auto& it : cheats)
		{
			u16 faddr = 0;
			for (auto& l : it.lines)
			{
				if (faddr == 0)
					faddr = addr;
				if (it.enabled && (u8)l.compare == ram[l.addr])
					MEM::wb_cheats(l.addr, l.value);
				//else if (it.lines.size() > 1 && it.enabled && l.addr == faddr)
				//	MEM::wb_cheats(l.addr, l.value);
			}
		}
	}

	u8 rbd(u16 addr)
	{
		return ram[addr];
	}

	u16 rw(u16 addr)
	{
		return rb(addr + 0) | rb(addr + 1) << 8;
	}

	u16 rwd(u16 addr)
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
		else if (addr >= 0x2000 && addr <= 0x3fff)
		{
			if ((addr & 0x7) == 0)
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
		}

		else if (addr == 0x4014)
		{
			int num = 0;
			u16 oamaddr = v << 8; PPU::oamdma = v;
			for (int i = 0; i < 256; i++)
			{
				oam[i] = ram[oamaddr + i];
				PPU_STEP; PPU_STEP;
				PPU::totalcycles += 2;
				num += 2;

			}

			if (PPU::totalcycles & 1)
			{
				PPU_STEP; PPU_STEP;
				PPU::totalcycles += 2;
			}
			else
			{
				PPU_STEP;
				PPU::totalcycles += 1;
			}

		}
		else if ((addr >= 0x4000 && addr <= 0x4013) || (addr == 0x4015))
			APU::wb(addr, v);
		else if (addr == 0x4016)
			controls_write(v);
		else if (addr == 0x4017)
			APU::wb(addr, v & 0xc0);
		else if (addr >= 0x5000 && addr <= 0x5fff)
			mapper->wb(addr, v);
		else if (addr >= 0x6000 && addr <= 0x7fff && mapper->sram)
			ram[addr] = v;
		else if (addr >= 0x8000)
			mapper->wb(addr, v);
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

		if (addr < 0x2000 || addr >= 0x3f00)
		{
			if (header.mappernum == 9 && addr < 0x2000)
				mapper->set_latch(addr, v);
			return vram[addr];
		}

		if (header.mirror == mirrortype::horizontal)
		{
			v = vram[0x2000 + (a % 0x400) + mirrorhor[(a >> 10) - 8] * 0x400];
		}
		if (header.mirror == mirrortype::vertical)
		{
			v = vram[0x2000 + (a % 0x400) + mirrorver[(a >> 10) - 8] * 0x400];
		}
		else if (header.mirror == mirrortype::single_nt0)
		{
			v = vram[0x2000 + (a % 0x400) + mirrornt0[(a >> 10) - 8] * 0x400];
		}
		else if (header.mirror == mirrortype::single_nt1)
		{
			v = vram[0x2000 + (a % 0x400) + mirrornt1[(a >> 10) - 8] * 0x400];
		}
		return v;
	}

	void ppuwb(u16 addr, u8 v)
	{
		addr &= 0x3fff;
		ppu_write_addr = addr; //ppu breakpoint address
		u16 a = addr;

		if (addr < 0x2000 || addr >= 0x3f00)
			vram[a] = v;
		else
		{
			if (header.mirror == mirrortype::horizontal)
			{
				vram[0x2000 + (a % 0x400) + mirrorhor[(a >> 10) - 8] * 0x400] = v;
			}
			if (header.mirror == mirrortype::vertical)
			{
				vram[0x2000 + (a % 0x400) + mirrorver[(a >> 10) - 8] * 0x400] = v;
			}
			if (header.mirror == mirrortype::single_nt0)
			{
				vram[0x2000 + (a % 0x400) + mirrornt0[(a >> 10) - 8] * 0x400] = v;
			}
			if (header.mirror == mirrortype::single_nt1)
			{
				vram[0x2000 + (a % 0x400) + mirrornt1[(a >> 10) - 8] * 0x400] = v;
			}
		}

		for (int i = 0; i < 7; i++)
			memcpy(&vram[0x3f20 + i * 32], &vram[0x3f00], 0x20);

		for (int i = 0; i < 4; i++)
			memcpy(&vram[0x3f04 + i * 0x4], &vram[0x3f10], 0x01);

		if (addr == 0x3f10)
			vram[0x3f00] = v;
		else if (addr == 0x3f00)
			vram[0x3f10] = v;
	}

	void wb_cheats(int addr, u8 val, s8 cmp)
	{
		ram[addr] = val;
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

	void save_sram()
	{
		if (MEM::mapper)
		{
			string path = get_exec_path();
			ofstream save(path + "\\" + header.name + ".sav", ios::binary);
			save.write((char*)MEM::ram.data() + 0x6000, 0x2000);
			save.close();
		}
	}

	void load_sram()
	{
		string path = get_exec_path();
		if (fs::exists(path + "\\" + header.name + ".sav"))
		{
			ifstream save(path + "\\" + header.name + ".sav", ios::binary);
			save.read((char*)MEM::ram.data() + 0x6000, 0x2000);
			save.close();
		}
	}

	void clean()
	{
		if (mapper)
			delete mapper;
	}
}