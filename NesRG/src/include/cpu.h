#pragma once

#include "types.h"

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



namespace CPU
{
	inline u8 nmi_triggered = 0;
	inline u8 nmi_vblank = 0;
	inline u64 instructions = 0;

	void op_nmi();
	u8 op_pop();
	void op_push(u16 addr, u8 v);
	void op_brk(u16 pc);
	void op_irq(u16 pc);
	void set_flag(bool flag, u8 v);
	void step();
	void op_adc(int mode);
	void op_and(int mode);
	void op_asl(int mode);
	void op_bra(int mode, bool flag);
	void op_bit(int mode);
	void op_cmp(int mode);
	void op_cpx(int mode);
	void op_cpy(int mode);
	void op_dec(int mode);
	void op_dex(int mode);
	void op_dey(int mode);
	void op_eor(int mode);
	void op_inc(int mode);
	void op_inx(int mode);
	void op_iny(int mode);
	void op_jsr(int mode);
	void op_lda(int mode);
	void op_ldx(int mode);
	void op_ldy(int mode);
	void op_lsr(int mode);
	void op_ora(int mode);
	void op_pha(int mode);
	void op_php(int mode);
	void op_pla(int mode);
	void op_plp(int mode);
	void op_rol(int mode);
	void op_ror(int mode);
	void op_rti(int mode);
	void op_rts(int mode);
	void op_sbc(int mode);
	void op_sta(int mode);
	void op_stx(int mode);
	void op_sty(int mode);
	void op_tax(int mode);
	void op_tay(int mode);
	void op_tsx(int mode);
	void op_txa(int mode);
	void op_txs(int mode);
	void op_tya(int mode);
	void op_set(int mode, u8 v, u8 f);
	u16 addr_mode_r(int mode);
	u16 addr_mode_w(int mode);
	void init();
	void reset();
}