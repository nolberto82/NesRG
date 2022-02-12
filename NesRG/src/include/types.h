#pragma once

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#define TEXTSIZE 256

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

const int APP_WIDTH = 1200;
const int APP_HEIGHT = 950;

const int NES_SCREEN_WIDTH = 256;
const int NES_SCREEN_HEIGHT = 240;