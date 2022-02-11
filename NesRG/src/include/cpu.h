#pragma once

#include "types.h"
#include "mem.h"

#define FC 0x01
#define FZ 0x02
#define FI 0x04
#define FD 0x08
#define FB 0x10
#define FU 0x20
#define FV 0x40
#define FN 0x80

#define INT_NMI	  0xfffa
#define INT_RESET 0xfffc
#define INT_BRK   0xfffe

const int CYCLES_PER_FRAME = 262 * 341;
const int CYCLES_PER_LINE = 341;

enum cstate
{
	running,
	debugging,
	scanlines,
	cycles,
	crashed
};

struct Registers
{
	u8 a, x, y, ps, sp;
	u16 pc;
};

struct Cpu
{
	bool pagecrossed = false;
	u8 branchtaken = 0;
	int state;


};

void op_nmi();
u8 op_pop();
void op_push(u16 addr, u8 v);
void op_bra(u16 addr, u8 op, bool flag);
void set_flag(bool flag, u8 v);
int cpu_step();
void cpu_init();
void cpu_reset();

u16 get_imme(u16 pc, bool trace = false);
u16 get_zerp(u16 pc, bool trace = false);
u16 get_zerx(u16 pc, bool trace = false);
u16 get_zery(u16 pc, bool trace = false);
u16 get_abso(u16 pc, bool trace = false);
u16 get_absx(u16 pc, bool trace = false);
u16 get_absy(u16 pc, bool trace = false);
u16 get_indx(u16 pc, bool trace = false);
u16 get_indy(u16 pc, bool trace = false);
u16 get_indi(u16 pc, bool trace = false);
u16 get_rela(u16 pc, bool trace = false);
u16 get_impl(u16 pc, bool trace = false);
u16 get_accu(u16 pc, bool trace = false);
u16 get_erro(u16 pc, bool trace = false);



struct amodefuncs
{
	u16(*modefuncs)(u16 pc, bool trace);
};

extern Registers reg;
extern Cpu cpu;
extern vector<amodefuncs> func;