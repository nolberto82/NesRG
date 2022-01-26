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

const int CYCLES_PER_FRAME = 341;

enum cstate
{
	running = 1,
	debugging = 2,
	crashed = 4
};

struct Registers
{
	u8 a, x, y, ps, sp;
	u16 pc;
};

struct Cpu
{
public:
	void execute();
	int step();
	void init();
	void reset();

	//int* get_cycles() { return &cycles; }
	//int* set_cycles(int v) { cycles = v; }
	//int* get_state() { return &cycles; }
	//int* set_state(int v) { cycles = v; }

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

	int cycles = 0;
	int state = cstate::running;

private:
	bool pagecrossed = false;

private:
	void op_nmi();
	u8 op_pop();
	void op_push(u16 addr, u8 v);
	void op_bra(u16 addr, bool flag);
	void op_dey();
	void op_dex();
	void op_iny();
	void op_inx();
	void op_tay();
	void op_tax();
	void op_tya();
	void op_txa();

	void set_flag(bool flag, u8 v);
};

extern Cpu cpu;
extern Registers reg;

struct amodefuncs
{
	u16(Cpu::* modefuncs)(u16 pc, bool trace);
};

extern vector<amodefuncs> func;