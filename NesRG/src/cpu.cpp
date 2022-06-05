#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "mem.h"
#include "mappers.h"

namespace CPU
{
	void step()
	{
		reg.npc = reg.pc;
		u8 op = MEM::rb(reg.npc++);
		int mode = disasm[op].mode;

		if (cpu.state == cstate::crashed)
			return;

		switch (disasm[op].id)
		{
		case opcid::ADC: op_adc(mode); break;
		case opcid::AND: op_and(mode); break;
		case opcid::ASL: op_asl(mode); break;
		case opcid::BCC: op_bra(mode, !(reg.ps & FC)); break;
		case opcid::BCS: op_bra(mode, reg.ps & FC); break;
		case opcid::BEQ: op_bra(mode, reg.ps & FZ); break;
		case opcid::BIT: op_bit(mode); break;
		case opcid::BMI: op_bra(mode, reg.ps & FN); break;
		case opcid::BNE: op_bra(mode, !(reg.ps & FZ)); break;
		case opcid::BPL: op_bra(mode, !(reg.ps & FN)); break;
		case opcid::BRK: op_brk(reg.npc); break;
		case opcid::BVC: op_bra(mode, !(reg.ps & FV)); break;
		case opcid::BVS: op_bra(mode, reg.ps & FV); break;
		case opcid::CLC: op_set(mode, 0, FC); break;
		case opcid::CLD: op_set(mode, 0, FD); break;
		case opcid::CLI: op_set(mode, 0, FI); break;
		case opcid::CLV: op_set(mode, 0, FV); break;
		case opcid::CMP: op_cmp(mode); break;
		case opcid::CPX: op_cpx(mode); break;
		case opcid::CPY: op_cpy(mode); break;
		case opcid::DEC: op_dec(mode); break;
		case opcid::DEX: op_dex(mode);  break;
		case opcid::DEY: op_dey(mode); break;
		case opcid::EOR: op_eor(mode); break;
		case opcid::INC: op_inc(mode); break;
		case opcid::INX: op_inx(mode); break;
		case opcid::INY: op_iny(mode); break;
		case opcid::JMP: reg.npc = addr_mode_r(mode); break;
		case opcid::JSR: op_jsr(mode); break;
		case opcid::LDA: op_lda(mode); break;
		case opcid::LDX: op_ldx(mode); break;
		case opcid::LDY: op_ldy(mode); break;
		case opcid::LSR: op_lsr(mode); break;
		case opcid::NOP: addr_mode_r(mode); break;
		case opcid::ORA: op_ora(mode); break;
		case opcid::PHA: op_pha(mode); break;
		case opcid::PHP: op_php(mode); break;
		case opcid::PLA: op_pla(mode); break;
		case opcid::PLP: op_plp(mode); break;
		case opcid::ROL: op_rol(mode); break;
		case opcid::ROR: op_ror(mode); break;
		case opcid::RTI: op_rti(mode); break;
		case opcid::RTS: op_rts(mode); break;
		case opcid::SBC: op_sbc(mode); break;
		case opcid::SEC: op_set(mode, 1, FC); break;
		case opcid::SED: op_set(mode, 1, FD); break;
		case opcid::SEI: op_set(mode, 1, FI); break;
		case opcid::STA: op_sta(mode); break;
		case opcid::STX: op_stx(mode); break;
		case opcid::STY: op_sty(mode); break;
		case opcid::TAX: op_tax(mode); break;
		case opcid::TAY: op_tay(mode); break;
		case opcid::TSX: op_tsx(mode); break;
		case opcid::TXA: op_txa(mode); break;
		case opcid::TXS: op_txs(mode); break;
		case opcid::TYA: op_tya(mode); break;
		case opcid::ERR:
		{
			if (!cpu.state_loaded)
				cpu.state = cstate::debugging;
			printf("%04X\n", reg.npc);
			cpu.state_loaded = 0;
		}
		}

		if (nmi_triggered == 1)
		{
			nmi_triggered = 2;
		}
		else if (nmi_triggered == 2)
		{
			op_nmi();
			nmi_triggered = 0;
		}

		if (MEM::mapper->fire)
		{
			MEM::mapper->fire = 0;
			op_irq(reg.npc);
		}

		reg.pc = reg.npc;
	}

	void op_adc(int mode)
	{
		u8 v = MEM::rb(addr_mode_r(mode));
		u16 b = reg.a + v + (reg.ps & FC);

		set_flag((b & 0xff) == 0, FZ);
		set_flag(b & 0x80, FN);
		set_flag(~(reg.a ^ v) & (reg.a ^ b) & 0x80, FV);
		set_flag(b > 0xff, FC);

		reg.a = (u8)b;
	}

	void op_and(int mode)
	{
		reg.a &= MEM::rb(addr_mode_r(mode));

		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_asl(int mode)
	{
		u8 b = 0; MEM::rb(reg.npc);
		if (mode == addrmode::accu)
		{
			set_flag(reg.a & 0x80, FC);
			reg.a = b = (reg.a << 1) & 0xfe;
		}
		else
		{
			u16 addr = addr_mode_w(mode);
			b = MEM::rb(addr);
			set_flag((b & 0x80) >> 7, FC);
			b = (b << 1) & 0xfe;
			MEM::wb(addr, b);
		}
		set_flag(b == 0, FZ);
		set_flag(b & 0x80, FN);
	}

	void op_bra(int mode, bool flag)
	{
		u8 l, h = 0;
		s8 v = addr_mode_r(mode);
		if (flag)
		{
			u16 addr = reg.npc + v;
			MEM::rb(reg.npc);
			if ((addr & 0xff00) != (reg.npc & 0xff00))
				MEM::rb(reg.npc++);
			reg.npc = addr;
		}
	}

	void op_bit(int mode)
	{
		u8 v = MEM::rb(addr_mode_r(mode));
		u8 b = reg.a & v;
		set_flag(b == 0, FZ);
		set_flag(v & 0x80, FN);
		set_flag(v & 0x40, FV);
	}

	void op_cmp(int mode)
	{
		u8 v = MEM::rb(addr_mode_r(mode));
		s8 t = reg.a - v;
		set_flag(reg.a >= v, FC);
		set_flag(reg.a == v, FZ);
		set_flag(t & 0x80, FN);
	}

	void op_cpx(int mode)
	{
		u8 v = MEM::rb(addr_mode_r(mode));
		s8 t = reg.x - v;
		set_flag(reg.x >= v, FC);
		set_flag(reg.x == v, FZ);
		set_flag(t & 0x80, FN);
	}

	void op_cpy(int mode)
	{
		u8 v = MEM::rb(addr_mode_r(mode));
		s8 t = reg.y - v;
		set_flag(reg.y >= v, FC);
		set_flag(reg.y == v, FZ);
		set_flag(t & 0x80, FN);
	}

	void op_dec(int mode)
	{
		u16 addr = addr_mode_w(mode); MEM::rb(reg.npc);
		u8 b = MEM::rb(addr) - 1;
		MEM::wb(addr, b);
		set_flag(b == 0, FZ);
		set_flag(b & 0x80, FN);
	}

	void op_dex(int mode)
	{
		reg.x--; MEM::rb(reg.npc);
		set_flag(reg.x == 0, FZ);
		set_flag(reg.x & 0x80, FN);
	}

	void op_dey(int mode)
	{
		reg.y--; MEM::rb(reg.npc);
		set_flag(reg.y == 0, FZ);
		set_flag(reg.y & 0x80, FN);
	}

	void op_eor(int mode)
	{
		reg.a ^= MEM::rb(addr_mode_r(mode));
		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_inc(int mode)
	{
		u16 a = addr_mode_w(mode); MEM::rb(reg.npc);
		u8 b = MEM::rb(a) + 1;
		MEM::wb(a, b);
		set_flag(b == 0, FZ);
		set_flag(b & 0x80, FN);
	}

	void op_inx(int mode)
	{
		reg.x++; MEM::rb(reg.npc);
		set_flag(reg.x == 0, FZ);
		set_flag(reg.x & 0x80, FN);
	}

	void op_iny(int mode)
	{
		reg.y++; MEM::rb(reg.npc);
		set_flag(reg.y == 0, FZ);
		set_flag(reg.y & 0x80, FN);
	}

	void op_jsr(int mode)
	{
		u16 addr = addr_mode_r(mode);
		reg.npc--;
		op_push(reg.sp--, reg.npc >> 8);
		op_push(reg.sp--, reg.npc & 0xff);
		MEM::rb(reg.npc);
		reg.npc = addr;
	}

	void op_lda(int mode)
	{
		reg.a = MEM::rb(addr_mode_r(mode));
		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_ldx(int mode)
	{
		reg.x = MEM::rb(addr_mode_r(mode));
		set_flag(reg.x == 0, FZ);
		set_flag(reg.x & 0x80, FN);
	}

	void op_ldy(int mode)
	{
		reg.y = MEM::rb(addr_mode_r(mode));
		set_flag(reg.y == 0, FZ);
		set_flag(reg.y & 0x80, FN);
	}

	void op_lsr(int mode)
	{
		u8 b = 0; MEM::rb(reg.npc);
		if (mode == addrmode::accu)
		{
			set_flag(reg.a & 0x01, FC);
			reg.a = b = (reg.a >> 1) & 0x7f;
		}
		else
		{
			u16 a = addr_mode_w(mode);
			b = MEM::rb(a);
			set_flag(b & 0x01, FC);
			b = (b >> 1) & 0x7f;
			MEM::wb(a, b);
		}
		set_flag(b == 0, FZ);
		set_flag(b & 0x80, FN);
	}

	void op_ora(int mode)
	{
		reg.a |= MEM::rb(addr_mode_r(mode));
		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_pha(int mode)
	{
		MEM::rb(reg.npc);
		op_push(reg.sp--, reg.a);
	}

	void op_php(int mode)
	{
		MEM::rb(reg.npc);
		op_push(reg.sp--, reg.ps | 0x30);
	}

	void op_pla(int mode)
	{
		MEM::rb(reg.npc); MEM::rb(reg.npc);
		reg.a = op_pop();
		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_plp(int mode)
	{
		MEM::rb(reg.npc); MEM::rb(reg.npc);
		reg.ps = op_pop() & ~0x30;
	}

	void op_rol(int mode)
	{
		u8 b = 0; u8 bit7 = 0; MEM::rb(reg.npc);
		if (mode == addrmode::accu)
		{
			bit7 = reg.a & 0x80 ? 1 : 0;
			reg.a = reg.a << 1;
			if (reg.ps & FC)
				reg.a |= 0x01;
			b = reg.a;
		}
		else
		{
			u16 a = addr_mode_w(mode);
			b = MEM::rb(a);
			bit7 = b & 0x80 ? 1 : 0;
			b = b << 1;
			if (reg.ps & FC)
				b |= 0x01;

			MEM::wb(a, b);
		}
		set_flag(b == 0, FZ);
		set_flag(b & 0x80, FN);
		set_flag(bit7, FC);
	}

	void op_ror(int mode)
	{
		u8 b = 0; u8 bit0 = 0; MEM::rb(reg.npc);
		if (mode == addrmode::accu)
		{
			bit0 = reg.a & 0x01 ? 1 : 0;
			reg.a = reg.a >> 1;
			if (reg.ps & FC)
				reg.a |= 0x80;
			b = reg.a;
		}
		else
		{
			u16 a = addr_mode_w(mode);
			b = MEM::rb(a);
			bit0 = b & 0x01 ? 1 : 0;
			b = b >> 1;
			if (reg.ps & FC)
				b |= 0x80;

			MEM::wb(a, b);
		}
		set_flag(b == 0, FZ);
		set_flag(b & FN, FN);
		set_flag(bit0, FC);
	}

	void op_rti(int mode)
	{
		MEM::rb(reg.npc); MEM::rb(reg.npc);
		reg.ps = op_pop() & ~0x30;
		reg.npc = op_pop();
		reg.npc |= op_pop() << 8;
	}

	void op_rts(int mode)
	{
		MEM::rb(reg.npc); MEM::rb(reg.npc); MEM::rb(reg.npc);
		reg.npc = op_pop();
		reg.npc |= op_pop() << 8;
		reg.npc++;
	}

	void op_sbc(int mode)
	{
		u8 b = MEM::rb(addr_mode_r(mode));
		u16 t = reg.a + ~b + (reg.ps & FC ? 1 : 0);
		set_flag((t & 0xff) == 0, FZ);
		set_flag(t & 0x80, FN);
		set_flag((reg.a ^ b) & (reg.a ^ t) & 0x80, FV);
		set_flag((t & 0xff00) == 0, FC);
		reg.a = (u8)t;
	}

	void op_sta(int mode)
	{
		MEM::wb(addr_mode_w(mode), reg.a);
	}

	void op_stx(int mode)
	{
		MEM::wb(addr_mode_w(mode), reg.x);
	}

	void op_sty(int mode)
	{
		MEM::wb(addr_mode_w(mode), reg.y);
	}

	void op_tax(int mode)
	{
		reg.x = reg.a; MEM::rb(reg.npc);
		set_flag(reg.x == 0, FZ);
		set_flag(reg.x & 0x80, FN);
	}

	void op_tay(int mode)
	{
		reg.y = reg.a;	MEM::rb(reg.npc);
		set_flag(reg.y == 0, FZ);
		set_flag(reg.y & 0x80, FN);
	}

	void op_tsx(int mode)
	{
		reg.x = reg.sp; MEM::rb(reg.npc);
		set_flag(reg.x == 0, FZ);
		set_flag(reg.x & 0x80, FN);
	}

	void op_txa(int mode)
	{
		reg.a = reg.x; MEM::rb(reg.npc);
		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_txs(int mode)
	{
		reg.sp = reg.x; MEM::rb(reg.npc);
	}

	void op_tya(int mode)
	{
		reg.a = reg.y; MEM::rb(reg.npc);
		set_flag(reg.a == 0, FZ);
		set_flag(reg.a & 0x80, FN);
	}

	void op_set(int mode, u8 v, u8 f)
	{
		addr_mode_r(mode);
		set_flag(v, f);
	}

	u16 addr_mode_r(int mode)
	{
		switch (mode)
		{
		case impl:
		case accu:
			MEM::rb(reg.npc);
			break;
		case imme:
			return reg.npc++; break;
		case zerp:
			return MEM::rb(reg.npc++); break;
		case zerx:
			MEM::rb(reg.npc); return (MEM::rb(reg.npc++) + reg.x) & 0xff; break;
		case zery:
			MEM::rb(reg.npc); return MEM::rb(reg.npc++) + reg.y & 0xff; break;
		case abso:
			return MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8; break;
		case absx:
		{
			u16 oldaddr = MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8;
			u16 newaddr = oldaddr + reg.x;
			if ((newaddr & 0xff00) != (oldaddr & 0xff00)) MEM::rb(reg.npc);
			return newaddr & 0xffff;
		}
		case absy:
		{
			u16 oldaddr = MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8;
			u16 newaddr = oldaddr + reg.y;
			if ((newaddr & 0xff00) != (oldaddr & 0xff00)) MEM::rb(reg.npc);
			return newaddr & 0xffff;
		}
		case indx:
		{
			u8 b1 = MEM::rb(reg.npc++);
			u8 lo = MEM::rb(b1 + reg.x & 0xff);
			u8 hi = MEM::rb(b1 + 1 + reg.x & 0xff);
			MEM::rb(reg.npc);
			return (hi << 8) | lo;
		}
		case indy:
		{
			u8 b1 = MEM::rb(reg.npc++);
			u8 lo = MEM::rb((b1 & 0xff));
			u8 hi = MEM::rb(b1 + 1 & 0xff);
			u16 oldaddr = (hi << 8 | lo);
			u16 addr = oldaddr + reg.y;
			if ((addr & 0xff00) != (oldaddr & 0xff00)) MEM::rb(reg.npc);
			return addr;
		}
		case indi:
		{
			u16 addr = MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8;
			u8 h, l = 0;
			if ((addr & 0xff) == 0xff)
			{
				l = MEM::rb(addr++);
				h = MEM::rb(addr - 0x100 & 0xff00);
				addr = (h << 8) | l;
			}
			else
			{
				l = MEM::rb(addr++);
				h = MEM::rb(addr);
				addr = (h << 8) | l;
			}


			return addr;
		}
		case rela:
		{
			return MEM::rb(reg.npc++);
		}
		//case erro:
		//	//cpu.state = cstate::crashed;
		//	printf("%04X\n", reg.pc);
		//	break;
		}
		return 0;
	}

	u16 addr_mode_w(int mode)
	{
		switch (mode)
		{
		case zerp:
			return MEM::rb(reg.npc++); break;
		case zerx:
			MEM::rb(reg.npc); return (MEM::rb(reg.npc++) + reg.x) & 0xff; break;
		case zery:
			MEM::rb(reg.npc); return MEM::rb(reg.npc++) + reg.y & 0xff; break;
		case abso:
			return MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8; break;
		case absx:
		{
			u16 addr = (MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8) + reg.x;
			MEM::rb(reg.npc);
			return addr & 0xffff;
		}
		case absy:
		{
			u16 addr = (MEM::rb(reg.npc++) | MEM::rb(reg.npc++) << 8) + reg.y;
			MEM::rb(reg.npc);
			return addr & 0xffff;
		}
		case indx:
		{
			u8 b1 = MEM::rb(reg.npc++);
			u8 lo = MEM::rb(b1 + reg.x & 0xff); u8 hi = MEM::rb(b1 + 1 + reg.x & 0xff);
			MEM::rb(reg.npc);
			return (hi << 8) | lo;
		}
		case indy:
		{
			u8 b1 = MEM::rb(reg.npc++);
			u8 lo = MEM::rb((b1 & 0xff)); u8 hi = MEM::rb(b1 + 1 & 0xff);
			MEM::rb(reg.npc);
			return ((hi << 8) | lo) + reg.y;
		}
		}
		return 0;
	}

	void init()
	{
		cpu.state = cstate::debugging;
	}

	void reset()
	{
		reg.pc = MEM::rwd(INT_RESET);
		reg.sp = 0xfd;
		reg.ps = 0x04;
		reg.x = 0x00;
		reg.a = 0x00;
		reg.y = 0x00;
		memset(MEM::ram.data(), 0x00, 0x8000);
		cpu.state = cstate::debugging;
		cpu.stepoveraddr = -1;
		APU::reset();
	}

	void op_nmi()
	{
		MEM::rb(reg.npc);
		MEM::rb(reg.npc);
		op_push(reg.sp--, reg.npc >> 8);
		op_push(reg.sp--, reg.npc & 0xff);
		op_push(reg.sp--, reg.ps);
		reg.npc = MEM::rw(INT_NMI);
	}

	u8 op_pop()
	{
		return MEM::rb(++reg.sp | 0x100);
	}

	void op_push(u16 addr, u8 v)
	{
		MEM::wb(addr | 0x100, v);
	}

	void op_brk(u16 pc)
	{
		reg.ps |= FB | FU; MEM::rb(reg.npc++);
		op_push(reg.sp--, pc >> 8);
		op_push(reg.sp--, (pc + 1) & 0xff);
		op_push(reg.sp--, reg.ps);
		reg.ps |= FI;
		reg.npc = MEM::rwd(INT_BRK); MEM::rb(reg.npc); MEM::rb(reg.npc);
	}

	void op_irq(u16 pc)
	{
		op_push(reg.sp--, reg.npc >> 8);
		op_push(reg.sp--, reg.npc & 0xff);
		reg.ps &= ~(FB | FU);
		op_push(reg.sp--, reg.ps | FB | FU);
		reg.npc = MEM::rw(INT_BRK);
		reg.ps |= FI;
	}

	void set_flag(bool flag, u8 v)
	{
		if (flag)
			reg.ps |= v;
		else
			reg.ps &= ~v;
	}
}

