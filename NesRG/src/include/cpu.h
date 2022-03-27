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

void op_nmi();
u8 op_pop();
void op_push(u16 addr, u8 v);
void op_bra(u16 addr, u8 op, bool flag);
void op_brk(u16 pc);
void op_irq(u16 pc);
void set_flag(bool flag, u8 v);
int cpu_step();
void cpu_init();
void cpu_reset();

extern Registers reg;
extern Cpu cpu;