#pragma once

#include "types.h"

#include "cpu.h"

const int RAMSIZE = 0x10000;
const int VRAMSIZE = 0x4000;
const int PATTSIZE = 0x1000;
const int NTABSIZE = 0x400;
const int OAMSIZE = 0x100;
const int PALSIZE = 0x20;

enum mirrortype
{
	single_nt0,
	single_nt1,
	vertical,
	horizontal,
	fourscreen
};

struct Header
{
	string id;
	u8 prgnum;
	u8 chrnum;
	u16 mappernum;
	u8 mirror;
	u8 battery;
	u8 trainer;
	string name;
};

inline u16 mirrorhor[] = { 0, 0, 1, 1 };
inline u16 mirrorver[] = { 0, 1, 0, 1 };
inline u16 mirrornt0[] = { 0, 0, 0, 0 };
inline u16 mirrornt1[] = { 1, 1, 1, 1 };
inline u16 mirror4sc[] = { 0, 1, 2, 3 };

inline string mirrornames[] =
{
	{ "one screen 0"},
	{ "one screen 1"},
	{ "vertical"},
	{ "horizontal"},
	{ "fourscreen"}
};

extern Header header;

inline vector<u8> ram;
inline vector<u8> vram;
inline vector<u8> ntram[2];
inline vector<u8> oam;
inline vector<u8> rom;
inline vector<u8> vrom;

inline int mirrornametable = 0;

inline bool rom_loaded = false;

inline s16 write_addr = -1;
inline s16 read_addr = -1;
inline s16 ppu_write_addr = -1;
inline s16 ppu_read_addr = -1;

void mem_init();
bool load_rom(const char* filename);
bool set_mapper();
bool load_file(const char* filename, std::vector<u8>& rom, int offset, int size);
u8 rb(u16 addr, u8 cycles = 0);
u8 rbd(u16 addr);
u16 rw(u16 addr);
void wb(u16 addr, u8 val);
void ww(u16 addr, u16 val);
u8 ppurb(u16 addr);
void ppuwb(u16 addr, u8 val);
void mem_rom(vector<u8>& dst, u16 addr, int offset, int size);
void mem_vrom(vector<u8>& dst, u16 addr, int offset, int size);
