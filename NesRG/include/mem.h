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

inline vector<u8> ram;
inline vector<u8> vram;
inline vector<u8> oam;
inline vector<u8> oamalt;
inline vector<u8> rom;

inline int mirrornametable = 0;

inline bool rom_loaded = false;

inline s16 write_addr = -1;
inline s16 read_addr = -1;
inline s16 ppu_write_addr = -1;
inline s16 ppu_read_addr = -1;

void mem_init();
bool load_rom(const char* filename);
void set_mapper();
bool load_file(const char* filename, std::vector<u8>& rom, int offset);
u8 rb(u16 addr);
u8 rbd(u16 addr);
u16 rw(u16 addr);
void wb(u16 addr, u8 val);
void ww(u16 addr, u16 val);
u8 ppurb(u16 addr);
void ppuwb(u16 addr, u8 val);
void reset();