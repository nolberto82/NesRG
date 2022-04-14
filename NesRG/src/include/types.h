#pragma once

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define TEXTSIZE 128

#include <iostream>
#include <stdio.h>
#include <stdbool.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <filesystem>
#include <any>
#include <utility>

#include "opcodes.h"

#include <SDL.h>
#include <SDL2_framerate.h>

using namespace std;
namespace fs = std::filesystem;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef char s8;
typedef short s16;
typedef int s32;

const int APP_WIDTH = 1400;
const int APP_HEIGHT = 975;

const int NES_SCREEN_WIDTH = 256;
const int NES_SCREEN_HEIGHT = 240;

const int PATTERN_WIDTH = 128;
const int PATTERN_HEIGHT = 128;

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
	int state;
	u32 cycles;
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
	u8 x;
	u8 w;
};

struct SpriteData
{
	u8 x;
	u8 tile;
	u8 attrib;
	u8 y;
	u8 spritenum;
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

extern PpuCtrl pctrl;
extern PpuMask pmask;
extern PpuStatus pstatus;
