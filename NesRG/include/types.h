﻿#pragma once

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <iostream>
#include <stdio.h>
#include <stdbool.h>
#include <algorithm>
#include <array>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <iomanip>
#include <memory>
#include <filesystem>
#include <functional>

#include "opcodes.h"

using namespace std;
namespace fs = std::filesystem;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long u64;

typedef char s8;
typedef short s16;
typedef int s32;

const int APP_WIDTH = 1300;
const int APP_HEIGHT = 900;
//const int APP_WIDTH = 512;
//const int APP_HEIGHT = 480;