#pragma once

#include "types.h"

#include "cpu.h"

const int RAMSIZE = 0x10000;
const int VRAMSIZE = 0x4000;
const int OAMSIZE = 0x100;

enum mirrortype
{
	horizontal,
	vertical
};

class Cpu;
class Ppu;

class Memory
{
private:
	Cpu* cpu = nullptr;
	Ppu* ppu = nullptr;

public:
	Memory() 
	{
		ram.resize(RAMSIZE);
		vram.resize(VRAMSIZE);
		oam.resize(OAMSIZE);
		oamalt.resize(OAMSIZE - 0x80);
	}
	~Memory() {}

	void set_obj(Cpu* cpu, Ppu* ppu)
	{
		this->cpu = cpu; this->ppu = ppu;
	}

	vector<u8> ram;
	vector<u8> vram;
	vector<u8> oam;
	vector<u8> oamalt;
	vector<u8> rom;

	int mirrornametable = 0;

	bool rom_loaded = false;

	u16 write_addr = 0;
	u16 read_addr = 0;
	u16 ppu_write_addr = 0;
	u16 ppu_read_addr = 0;

	bool load_rom(const char* filename);
	void set_mapper();
	bool load_file(const char* filename, std::vector<u8>& rom, int offset);
	u8 rb(u16 addr);
	u16 rw(u16 addr);
	void wb(u16 addr, u8 val);
	void ww(u16 addr, u16 val);
	u8 ppurb(u16 addr);
	void ppuwb(u16 addr, u8 val);
	void reset();
};