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
	void step();
	void init();
	void reset();

	//int* get_cycles() { return &cycles; }
	//int* set_cycles(int v) { cycles = v; }
	//int* get_state() { return &cycles; }
	//int* set_state(int v) { cycles = v; }

	u16 get_imme(u16 pc);
	u16 get_zerp(u16 pc);
	u16 get_zerx(u16 pc);
	u16 get_zery(u16 pc);
	u16 get_abso(u16 pc);
	u16 get_absx(u16 pc);
	u16 get_absy(u16 pc);
	u16 get_indx(u16 pc);
	u16 get_indy(u16 pc);
	u16 get_indi(u16 pc);
	u16 get_rela(u16 pc);
	u16 get_impl(u16 pc);
	u16 get_accu(u16 pc);

	u16 get_erro(u16 pc);

	int cycles = 0;
	int state = cstate::running;

private:
	bool pagecrossed = false;

private:
	u8 op_pop();
	void op_push(u16 addr, u8 v);
	void op_bra(u16 addr, bool flag);
	void op_bit(u16 addr);
	u8 op_and(u16 addr);
	void op_dec(u16 addr);
	void op_inc(u16 addr);
	void op_sty(u16 addr);
	void op_stx(u16 addr);
	void op_sta(u16 addr);
	void op_ldy(u16 addr);
	void op_ldx(u16 addr);
	void op_lda(u16 addr);
	void op_cpy(u16 addr);
	void op_cpx(u16 addr);
	void op_cmp(u16 addr);
	void op_eor(u16 addr);
	void op_ora(u16 addr);
	void op_sbc(u16 addr);
	void op_adc(u16 addr);
	void op_dey();
	void op_dex();
	void op_iny();
	void op_inx();
	void op_tay();
	void op_tax();
	void op_tya();
	void op_txa();
	void op_rol(u16 addr);
	void op_asl(u16 addr);
	void op_ror(u16 addr);
	void op_lsr(u16 addr);

	void set_flag(bool flag, u8 v);
};

extern Cpu cpu;
extern Registers r;

struct amodefuncs
{
	u16(Cpu::* modefuncs)(u16 pc);
};

extern vector<amodefuncs> func;