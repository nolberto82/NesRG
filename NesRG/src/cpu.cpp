#include "cpu.h"
#include "ppu.h"

vector<amodefuncs> func;

int Cpu::step()
{
	u8 op = mem.rb(reg.pc);
	u8 b1 = 0;
	u16 b2 = 0;
	branchtaken = 0;

	int mode = disasm[op].mode;
	u16 addr = (this->*func[mode].modefuncs)(reg.pc + 1, false);

	if (state == cstate::crashed)
		return 0;

	switch (disasm[op].id)
	{
		case opcid::ADC:
		{
			u8 v = mem.rb(addr);
			u16 b = reg.a + v + (reg.ps & FC);

			set_flag((b & 0xff) == 0, FZ);
			set_flag(b & 0x80, FN);
			set_flag(~(reg.a ^ v) & (reg.a ^ b) & 0x80, FV);
			set_flag(b > 0xff, FC);

			reg.a = (u8)b;
			break;
		}
		case opcid::AND:
		{
			reg.a &= mem.rb(addr);

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
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
				b = mem.rb(addr);
				set_flag((b & 0x80) >> 7, FC);
				b = (b << 1) & 0xfe;
				mem.wb(addr, b);
			}

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			break;
		}
		case opcid::BCC:
		{
			op_bra(addr, !(reg.ps & FC));
			break;
		}
		case opcid::BCS:
		{
			op_bra(addr, (reg.ps & FC));
			break;
		}
		case opcid::BEQ:
		{
			op_bra(addr, (reg.ps & FZ));
			break;
		}
		case opcid::BIT:
		{
			u8 v = mem.rb(addr);
			u8 b = reg.a & v;

			set_flag(b == 0, FZ);
			set_flag(v & 0x80, FN);
			set_flag(v & 0x40, FV);
			break;
		}
		case opcid::BMI:
		{
			op_bra(addr, (reg.ps & FN));
			break;
		}
		case opcid::BNE:
		{
			op_bra(addr, !(reg.ps & FZ));
			break;
		}
		case opcid::BPL:
		{
			op_bra(addr, !(reg.ps & FN));
			break;
		}
		case opcid::BRK:
		{
			u8 pb = mem.rb(reg.pc + 1);
			op_push(reg.sp--, reg.pc >> 8);
			op_push(reg.sp--, reg.pc & 0xff);
			op_push(reg.sp--, reg.ps);
			reg.pc = mem.rw(0xfffe);
			break;
		}
		case opcid::BVC:
		{
			op_bra(addr, !(reg.ps & FV));
			break;
		}
		case opcid::BVS:
		{
			op_bra(addr, (reg.ps & FV));
			break;
		}
		case opcid::CLC:
		{
			set_flag(0, FC);
			break;
		}
		case opcid::CLD:
		{
			set_flag(0, FD);
			break;
		}
		case opcid::CLI:
		{
			set_flag(0, FI);
			break;
		}
		case opcid::CLV:
		{
			set_flag(0, FV);
			break;
		}
		case opcid::CMP:
		{
			u8 v = mem.rb(addr);
			s8 t = reg.a - v;

			set_flag(reg.a >= v, FC);
			set_flag(reg.a == v, FZ);
			set_flag(t & 0x80, FN);
			break;
		}
		case opcid::CPX:
		{
			u8 v = mem.rb(addr);
			s8 t = reg.x - v;

			set_flag(reg.x >= v, FC);
			set_flag(reg.x == v, FZ);
			set_flag(t & 0x80, FN);
			break;
		}
		case opcid::CPY:
		{
			u8 v = mem.rb(addr);
			s8 t = reg.y - v;

			set_flag(reg.y >= v, FC);
			set_flag(reg.y == v, FZ);
			set_flag(t & 0x80, FN);
			break;
		}
		case opcid::DEC:
		{
			u8 b = mem.rb(addr) - 1;
			mem.wb(addr, b);

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			break;
		}
		case opcid::DEX:
		{
			reg.x--;

			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			break;
		}
		case opcid::DEY:
		{
			reg.y--;

			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			break;
		}
		case opcid::EOR:
		{
			reg.a ^= mem.rb(addr);

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			break;
		}
		case opcid::INC:
		{
			u8 b = mem.rb(addr) + 1;
			mem.wb(addr, b);

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			break;
		}
		case opcid::INX:
		{
			reg.x++;

			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			break;
		}
		case opcid::INY:
		{
			reg.y++;

			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			break;
		}
		case opcid::JMP:
		{
			reg.pc = addr - 3;
			break;
		}
		case opcid::JSR:
		{
			u16 pc = reg.pc + 2;
			op_push(reg.sp--, pc >> 8);
			op_push(reg.sp--, pc & 0xff);
			reg.pc = addr - 3;
			break;
		}
		case opcid::LDA:
		{
			reg.a = mem.rb(addr);

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			break;
		}
		case opcid::LDX:
		{
			reg.x = mem.rb(addr);

			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			break;
		}
		case opcid::LDY:
		{
			reg.y = mem.rb(addr);

			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
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
				b = mem.rb(addr);
				set_flag(b & 0x01, FC);
				b = (b >> 1) & 0x7f;
				mem.wb(addr, b);
			}

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			break;
		}
		case opcid::NOP:
		{
			break;
		}
		case opcid::ORA:
		{
			reg.a |= mem.rb(addr);

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			break;
		}
		case opcid::PHA:
		{
			op_push(reg.sp--, reg.a);
			break;
		}
		case opcid::PHP:
		{
			op_push(reg.sp--, reg.ps | 0x30);
			break;
		}
		case opcid::PLA:
		{
			reg.a = op_pop();

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			break;
		}
		case opcid::PLP:
		{
			reg.ps = op_pop();
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
				b = mem.rb(addr);
				bit7 = reg.a & 0x80 ? 1 : 0;
				b = b << 1;
				if (reg.ps & FC)
					b |= 0x01;

				mem.wb(addr, b);
			}

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			set_flag(bit7, FC);
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
				b = mem.rb(addr);
				bit0 = reg.a & 0x01 ? 1 : 0;
				b = b >> 1;
				if (reg.ps & FC)
					b |= 0x80;

				mem.wb(addr, b);
			}

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			set_flag(bit0, FC);
			break;
		}
		case opcid::RTI:
		{
			reg.ps = op_pop();
			reg.pc = op_pop();
			reg.pc |= op_pop() << 8;
			reg.pc--;
			break;
		}
		case opcid::RTS:
		{
			reg.pc = op_pop();
			reg.pc |= op_pop() << 8;
			break;
		}
		case opcid::SBC:
		{
			u8 b = mem.rb(addr);
			u16 t = reg.a + ~b + (reg.ps & FC ? 1 : 0);

			set_flag((t & 0xff) == 0, FZ);
			set_flag(t & 0x80, FN);
			set_flag((reg.a ^ b) & (reg.a ^ t) & 0x80, FV);
			set_flag((t & 0xff00) == 0, FC);

			reg.a = (u8)t;
			break;
		}
		case opcid::SEC:
		{
			set_flag(1, FC);
			break;
		}
		case opcid::SED:
		{
			set_flag(1, FD);
			break;
		}
		case opcid::SEI:
		{
			set_flag(1, FI);
			break;
		}
		case opcid::STA:
		{
			pagecrossed = false;
			mem.wb(addr, reg.a);
			break;
		}
		case opcid::STX:
		{
			pagecrossed = false;
			mem.wb(addr, reg.x);
			break;
		}
		case opcid::STY:
		{
			pagecrossed = false;
			mem.wb(addr, reg.y);
			break;
		}
		case opcid::TAX:
		{
			reg.x = reg.a;

			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			break;
		}
		case opcid::TAY:
		{
			reg.y = reg.a;

			set_flag(reg.y == 0, FZ);
			set_flag(reg.y & 0x80, FN);
			break;
		}
		case opcid::TSX:
		{
			reg.x = reg.sp;

			set_flag(reg.x == 0, FZ);
			set_flag(reg.x & 0x80, FN);
			break;
		}
		case opcid::TXA:
		{
			reg.a = reg.x;

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			break;
		}
		case opcid::TXS:
		{
			reg.sp = reg.x;
			break;
		}
		case opcid::TYA:
		{
			reg.a = reg.y;

			set_flag(reg.a == 0, FZ);
			set_flag(reg.a & 0x80, FN);
			break;
		}
		case opcid::ERR:
		{
			state = cstate::crashed;
			break;
		}
	}

	reg.pc += disasm[op].size;

	if (ppu.nmi)
	{
		op_nmi();
		return 7 * 3;
	}

	return (disasm[op].cycles + branchtaken) * 3;
}

void Cpu::init()
{
	using c = Cpu;
	func =
	{
		{ &c::get_imme },
		{ &c::get_zerp },
		{ &c::get_zerx },
		{ &c::get_zery },
		{ &c::get_abso },
		{ &c::get_absx },
		{ &c::get_absy },
		{ &c::get_indx },
		{ &c::get_indy },
		{ &c::get_indi },
		{ &c::get_rela },
		{ &c::get_impl },
		{ &c::get_accu },
		{ &c::get_erro },
	};

	//reg.ps = 0x36;
	//reg.x = 0x80;
}

void Cpu::reset()
{
	reg.pc = mem.rw(0xfffc);
	//reg.pc = 0xc000;
	reg.sp = 0xfd;
	reg.ps = 0x04;
	reg.x = 0x00;
	reg.a = 0x00;
	reg.y = 0x00;
}

void Cpu::op_nmi()
{
	op_push(reg.sp--, reg.pc >> 8);
	op_push(reg.sp--, reg.pc & 0xff);
	op_push(reg.sp--, reg.ps);
	reg.pc = mem.rw(0xfffa);
	ppu.nmi = false;
	ppu.cycles += 7;
}

u8 Cpu::op_pop()
{
	return mem.rb(++reg.sp | 0x100);
}

void Cpu::op_push(u16 addr, u8 v)
{
	mem.wb(addr | 0x100, v);
}

void Cpu::op_bra(u16 addr, bool flag)
{
	if (flag)
	{
		reg.pc = addr - 2;
		branchtaken = 1;
	}
	else
	{
		branchtaken = 0;
	}
}

void Cpu::set_flag(bool flag, u8 v)
{
	if (flag)
		reg.ps |= v;
	else
		reg.ps &= ~v;
}

u16 Cpu::get_imme(u16 pc, bool trace)
{
	return pc;
}

u16 Cpu::get_zerp(u16 pc, bool trace)
{
	return (u8)mem.rb(pc);
}

u16 Cpu::get_zerx(u16 pc, bool trace)
{
	return (u8)(mem.rb(pc) + reg.x);
}

u16 Cpu::get_zery(u16 pc, bool trace)
{
	return (u8)(mem.rb(pc) + reg.y);
}

u16 Cpu::get_abso(u16 pc, bool trace)
{
	return mem.rw(pc);
}

u16 Cpu::get_absx(u16 pc, bool trace)
{
	u16 oldaddr = mem.rw(pc);
	u32 newaddr = oldaddr + reg.x;
	pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
	return newaddr & 0xffff;
}

u16 Cpu::get_absy(u16 pc, bool trace)
{
	u16 oldaddr = mem.rw(pc);
	u32 newaddr = oldaddr + reg.y;
	pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
	return newaddr & 0xffff;
}

u16 Cpu::get_indx(u16 pc, bool trace)
{
	u8 b1 = mem.rb(pc);
	u8 lo = mem.rb(b1 + reg.x & 0xff);
	u8 hi = mem.rb(b1 + 1 + reg.x & 0xff);

	if (trace)
		return b1;
	else
		return hi << 8 | lo;
}

u16 Cpu::get_indy(u16 pc, bool trace)
{
	u8 b1 = mem.rb(pc);
	u8 lo = mem.rb((b1 & 0xff));
	u8 hi = mem.rb(b1 + 1 & 0xff);
	u16 oldaddr = (hi << 8 | lo);
	u32 newaddr = oldaddr + reg.y;
	pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;

	if (trace)
		return b1;
	else
		return (hi << 8 | lo) + reg.y;
}

u16 Cpu::get_indi(u16 pc, bool trace)
{
	u16 addr = mem.rw(pc);

	if ((addr & 0xff) == 0xff)
	{
		return ++addr;
	}
	else
	{
		return mem.rw(addr);
	}
}

u16 Cpu::get_rela(u16 pc, bool trace)
{
	u8 b1 = mem.rb(pc);
	return pc + (s8)(b1)+1;
}

u16 Cpu::get_impl(u16 pc, bool trace)
{
	return 0;
}

u16 Cpu::get_accu(u16 pc, bool trace)
{
	return 0;
}

u16 Cpu::get_erro(u16 pc, bool trace)
{
	state = cstate::crashed;
	return 0;
}
