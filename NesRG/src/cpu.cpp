#include "cpu.h"
#include "ppu.h"
#include "mappers.h"

u8 op = 0;

int cpu_step()
{
	op = rb(reg.pc);
	u8 b1 = 0;
	u16 b2 = 0;
	cpu.branchtaken = 0;

	int mode = disasm[op].mode;
	u16 addr = 0;
	u16 pc = reg.pc;
	switch (mode)
	{
		case impl:
		case accu:
			break;
		case imme:
			addr = pc + 1; break;
		case zerp:
			addr = rbd(pc + 1); break;
		case zerx:
			addr = (rbd(pc + 1) + reg.x) & 0xff; break;
		case zery:
			addr = rbd(pc + 1) + reg.y & 0xff; break;
		case abso:
			addr = rw(pc + 1); break;
		case absx:
		{
			u16 oldaddr = rw(pc + 1);
			u16 newaddr = oldaddr + reg.x;
			if (disasm[op].extracycle)
				cpu.pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
			addr = newaddr & 0xffff;
			break;
		}
		case absy:
		{
			u16 oldaddr = rw(pc + 1);
			u16 newaddr = oldaddr + reg.y;
			cpu.pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
			addr = newaddr & 0xffff;
			break;
		}
		case indx:
		{
			u8 b1 = rbd(pc + 1);
			u8 lo = rbd(b1 + reg.x & 0xff);
			u8 hi = rbd(b1 + 1 + reg.x & 0xff);
			addr = (hi << 8) | lo;
			break;
		}
		case indy:
		{
			u8 b1 = rbd(pc + 1);
			u8 lo = rbd((b1 & 0xff));
			u8 hi = rbd(b1 + 1 & 0xff);
			u16 oldaddr = (hi << 8 | lo);
			addr = oldaddr + reg.y;
			cpu.pagecrossed = (addr & 0xff00) != (oldaddr & 0xff00) ? true : false;
			break;
		}
		case indi:
		{
			addr = rw(pc + 1);
			if ((addr & 0xff) == 0xff)
				++addr;
			else
				addr = rw(addr);
			break;
		}
		case rela:
		{
			u8 b1 = rbd(pc + 1);
			addr = pc + (s8)(b1)+2;
			if (disasm[op].extracycle)
				cpu.pagecrossed = (addr & 0xff00) != (pc & 0xff00) ? true : false;
			break;
		}
		case erro: 
			cpu.state = cstate::crashed;
			break;
	}

	if (cpu.state == cstate::crashed)
		return 0;

	switch (disasm[op].id)
	{
		case opcid::ADC:
		{
			u8 v = rb(addr);
			u16 b = reg.a + v + (reg.ps & FC);

			set_flag((b & 0xff) == 0, FZ);
			set_flag(b & 0x80, FN);
			set_flag(~(reg.a ^ v) & (reg.a ^ b) & 0x80, FV);
			set_flag(b > 0xff, FC);

			reg.a = (u8)b;
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::AND:
		{
			reg.a &= rb(addr);

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::ASL:
		{
			u8 b = 0;
			if (mode == addrmode::accu)
			{
				set_flag(reg.a & 0x80, FC);
				reg.a = b = (reg.a << 1) & 0xfe;
			}
			else
			{
				b = rb(addr);
				set_flag((b & 0x80) >> 7, FC);
				b = (b << 1) & 0xfe;
				wb(addr, b);
			}
			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::BCC:
		{
			op_bra(addr, op, !(reg.ps & FC));
			break;
		}
		case opcid::BCS:
		{
			op_bra(addr, op, (reg.ps & FC));
			break;
		}
		case opcid::BEQ:
		{
			op_bra(addr, op, (reg.ps & FZ));
			break;
		}
		case opcid::BIT:
		{
			u8 v = rb(addr);
			u8 b = reg.a & v;
			set_flag(b == 0, FZ);
			set_flag(v & 0x80, FN);
			set_flag(v & 0x40, FV);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::BMI:
		{
			op_bra(addr, op, (reg.ps & FN));
			break;
		}
		case opcid::BNE:
		{
			op_bra(addr, op, !(reg.ps & FZ));
			break;
		}
		case opcid::BPL:
		{
			op_bra(addr, op, !(reg.ps & FN));
			break;
		}
		case opcid::BRK:
		{
			u8 pb = rb(reg.pc);
			pb = rb(reg.pc);
			op_push(reg.sp--, (reg.pc + 2) >> 8);
			op_push(reg.sp--, (reg.pc + 2) & 0xff);
			op_push(reg.sp--, reg.ps | FB | FU);
			reg.pc = rw(INT_BRK);
			reg.ps |= FI;
			break;
		}
		case opcid::BVC:
		{
			op_bra(addr, op, !(reg.ps & FV));
			break;
		}
		case opcid::BVS:
		{
			op_bra(addr, op, (reg.ps & FV));
			break;
		}
		case opcid::CLC:
		{
			set_flag(0, FC);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::CLD:
		{
			set_flag(0, FD);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::CLI:
		{
			set_flag(0, FI);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::CLV:
		{
			set_flag(0, FV);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::CMP:
		{
			u8 v = rb(addr);
			s8 t = reg.a - v;
			set_flag(reg.a >= v, FC);
			set_flag(reg.a == v, FZ);
			set_flag(t & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::CPX:
		{
			u8 v = rb(addr);
			s8 t = reg.x - v;
			set_flag(reg.x >= v, FC);
			set_flag(reg.x == v, FZ);
			set_flag(t & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::CPY:
		{
			u8 v = rb(addr);
			s8 t = reg.y - v;
			set_flag(reg.y >= v, FC);
			set_flag(reg.y == v, FZ);
			set_flag(t & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::DEC:
		{
			u8 b = rb(addr) - 1;
			wb(addr, b);
			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::DEX:
		{
			reg.x--;
			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::DEY:
		{
			reg.y--;
			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::EOR:
		{
			reg.a ^= rb(addr);
			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::INC:
		{
			u8 b = rb(addr) + 1;
			wb(addr, b);
			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::INX:
		{
			reg.x++;
			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::INY:
		{
			reg.y++;
			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::JMP:
		{
			reg.pc = addr;
			break;
		}
		case opcid::JSR:
		{
			u16 pc = reg.pc + 2;
			op_push(reg.sp--, pc >> 8);
			op_push(reg.sp--, pc & 0xff);
			reg.pc = addr;
			break;
		}
		case opcid::LDA:
		{
			reg.a = rb(addr);
			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::LDX:
		{
			reg.x = rb(addr);
			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::LDY:
		{
			reg.y = rb(addr);
			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::LSR:
		{
			u8 b = 0;
			if (mode == addrmode::accu)
			{
				set_flag(reg.a & 0x01, FC);
				reg.a = b = (reg.a >> 1) & 0x7f;
			}
			else
			{
				b = rb(addr);
				set_flag(b & 0x01, FC);
				b = (b >> 1) & 0x7f;
				wb(addr, b);
			}
			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::NOP:
		{
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::ORA:
		{
			reg.a |= rb(addr);
			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::PHA:
		{
			op_push(reg.sp--, reg.a);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::PHP:
		{
			op_push(reg.sp--, reg.ps | 0x30);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::PLA:
		{
			reg.a = op_pop();
			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::PLP:
		{
			reg.ps = op_pop() & ~0x30;
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::ROL:
		{
			u8 b = 0;
			u8 bit7 = 0;

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
				b = rb(addr);
				bit7 = b & 0x80 ? 1 : 0;
				b = b << 1;
				if (reg.ps & FC)
					b |= 0x01;

				wb(addr, b);
			}
			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			set_flag(bit7, FC);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::ROR:
		{
			u8 b = 0;
			u8 bit0 = 0;

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
				b = rb(addr);
				bit0 = b & 0x01 ? 1 : 0;
				b = b >> 1;
				if (reg.ps & FC)
					b |= 0x80;

				wb(addr, b);
			}
			set_flag(b == 0, FZ);
			set_flag(b & FN, FN);
			set_flag(bit0, FC);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::RTI:
		{
			reg.ps = op_pop() & ~0x30;
			reg.pc = op_pop();
			reg.pc |= op_pop() << 8;
			reg.pc;
			break;
		}
		case opcid::RTS:
		{
			reg.pc = op_pop();
			reg.pc |= op_pop() << 8;
			reg.pc++;
			break;
		}
		case opcid::SBC:
		{
			u8 b = rb(addr);
			u16 t = reg.a + ~b + (reg.ps & FC ? 1 : 0);
			set_flag((t & 0xff) == 0, FZ);
			set_flag(t & 0x80, FN);
			set_flag((reg.a ^ b) & (reg.a ^ t) & 0x80, FV);
			set_flag((t & 0xff00) == 0, FC);
			reg.a = (u8)t;
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::SEC:
		{
			set_flag(1, FC);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::SED:
		{
			set_flag(1, FD);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::SEI:
		{
			set_flag(1, FI);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::STA:
		{
			cpu.pagecrossed = false;
			wb(addr, reg.a);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::STX:
		{
			cpu.pagecrossed = false;
			wb(addr, reg.x);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::STY:
		{
			cpu.pagecrossed = false;
			wb(addr, reg.y);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::TAX:
		{
			reg.x = reg.a;
			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::TAY:
		{
			reg.y = reg.a;
			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::TSX:
		{
			reg.x = reg.sp;
			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::TXA:
		{
			reg.a = reg.x;
			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::TXS:
		{
			reg.sp = reg.x;
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::TYA:
		{
			reg.a = reg.y;
			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			reg.pc += disasm[op].size;
			break;
		}
		case opcid::ERR:
		{
			//cpu.state = cstate::crashed;
			reg.pc++;
			return 3;
		}
	}

	ppu.totalcycles += disasm[op].cycles + cpu.branchtaken + cpu.pagecrossed;
	return ((disasm[op].cycles + cpu.branchtaken) * 3) + cpu.pagecrossed;
}

void cpu_init()
{
	cpu.state = cstate::debugging;
}

void cpu_reset()
{
	reg.pc = rw(INT_RESET);
	//if (header.name == "nestest.nes")
	//	reg.pc = 0xc000;
	reg.sp = 0xfd;
	reg.ps = 0x04;
	reg.x = 0x00;
	reg.a = 0x00;
	reg.y = 0x00;
	memset(ram.data(), 0x00, 0x8000);
	cpu.state = cstate::debugging;
}

void op_nmi()
{
	op_push(reg.sp--, reg.pc >> 8);
	op_push(reg.sp--, reg.pc & 0xff);
	op_push(reg.sp--, reg.ps);
	reg.pc = rw(INT_NMI);
	//pctrl.nmi = false;
}

u8 op_pop()
{
	return rb(++reg.sp | 0x100);
}

void op_push(u16 addr, u8 v)
{
	wb(addr | 0x100, v);
}

void op_bra(u16 addr, u8 op, bool flag)
{
	if (flag)
	{
		reg.pc = addr;
		cpu.branchtaken = 1;
	}
	else
	{
		cpu.branchtaken = 0;
		reg.pc += disasm[op].size;
	}
}

void set_flag(bool flag, u8 v)
{
	if (flag)
		reg.ps |= v;
	else
		reg.ps &= ~v;
}
