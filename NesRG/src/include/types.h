﻿#pragma once

#define SDL_MAIN_HANDLED


#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define TEXTSIZE 128

#include <cstdint>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <filesystem>

#include "opcodes.h"

#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL2_framerate.h>
#include <SDL_opengl.h>

using namespace std;
namespace fs = std::filesystem;

typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;

typedef __int8 s8;
typedef __int16 s16;
typedef __int32 s32;

#if DEBUG
const int APP_WIDTH = 1250;
const int APP_HEIGHT = 975;
#else
const int APP_WIDTH = 256 * 3;
const int APP_HEIGHT = 240 * 3;
#endif

const int NES_WIDTH = 256;
const int NES_HEIGHT = 240;

const int PATT_WIDTH = 128;
const int PATT_HEIGHT = 128;

const int FRAME_CYCLES = 29780;

enum cstate
{
	running,
	debugging,
	stepping,
	scanlines,
	cycles,
	frame,
	crashed
};

struct Registers
{
	u8 a, x, y, ps, sp;
	u16 pc, npc;
};

struct Cpu
{
	bool pagecrossed = false;
	int state = 0;
	int cpucycles = 0;
	u8 state_loaded = 0;
	u32 cycles = 0;
	s16 stepoveraddr = -1;
};

struct RAddr
{
	u8 cx;
	u8 cy;
	u8 nt;
	u8 fy;
};

struct PpuRegisters
{
	u16 v;
	u16 t;
	u8 fx;
	u8 w;
};

struct SpriteData
{
	u8 x;
	u8 tile;
	u8 attrib;
	u8 y;
	u8 num;
	u8 lo;
	u8 hi;
};

struct PpuCtrl
{
	u8 nametable;
	u8 vaddr;
	u8 spraddr;
	u8 bgaddr;
	u8 spritesize;
	u8 master;
	u8 nmi;
};

struct PpuMask
{
	u8 greyscale;
	u8 backgroundleft;
	u8 spriteleft;
	u8 background;
	u8 sprite;
	u8 red;
	u8 green;
	u8 blue;
};

struct PpuStatus
{
	u8 lsb;
	u8 sproverflow;
	u8 sprite0hit;
	u8 vblank;
};

struct CheatLine
{
	u16 addr;
	int romaddr;
	s8 compare;
	u8 value;
	u8 size;
};

struct Cheats
{
	int id;
	string name;
	vector<CheatLine> lines;
	vector<string> gglines;
	bool enabled;
};

extern Registers reg;
extern Cpu cpu;
extern PpuCtrl pctrl;
extern PpuMask pmask;
extern PpuStatus pstatus;
extern Cheats cheat;
