#include "cpu.h"
#include "ppu.h"

vector<amodefuncs> func;

void Cpu::execute()
{
	while (cycles < CYCLES_PER_FRAME)
	{
		cpu.step();
	}

	cycles -= CYCLES_PER_FRAME;

	ppu.set_scanline();
}

void Cpu::step()
{
	u8 op = mem.rb(r.pc);
	u8 b1 = 0;
	u16 b2 = 0;

	int mode = disasm[op].mode;
	u16 addr = (this->*func[disasm[op].mode].modefuncs)(r.pc + 1);

	if (state == cstate::crashed)
		return;

	switch (disasm[op].id)
	{
		case opcid::ADC:
		{
			u8 v = mem.rb(addr);
			u16 t = r.a + v + (r.ps & FC ? 1 : 0);

			set_flag((t & 0xff) == 0, FZ);
			set_flag(t & 0x80, FN);
			set_flag((r.a ^ v) & (r.a ^ t) & 0x80, FV);
			set_flag((t & 0xff00) == 0, FC);

			r.a = t;
			break;
		}
		case opcid::AND:
		{
			r.a &= mem.rb(addr);

			set_flag(r.a == 0, FZ);
			set_flag(r.a & 0x80, FN);
			break;
		}
		case opcid::ASL:
		{
			u8 b = 0;
			if (mode == addrmode::accu)
			{
				set_flag(r.a & 0x80, FC);
				r.a = b = (r.a << 1) & 0xfe;
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
			op_bra(addr, !(r.ps & FC));
			break;
		}
		case opcid::BCS:
		{
			op_bra(addr, (r.ps & FC));
			break;
		}
		case opcid::BEQ:
		{
			op_bra(addr, (r.ps & FZ));
			break;
		}
		case opcid::BIT:
		{
			break;
		}
		case opcid::BMI:
		{
			op_bra(addr, (r.ps & FN));
			break;
		}
		case opcid::BNE:
		{
			op_bra(addr, !(r.ps & FZ));
			break;
		}
		case opcid::BPL:
		{
			op_bra(addr, !(r.ps & FN));
			break;
		}
		case opcid::BRK:
		{
			break;
		}
		case opcid::BVC:
		{
			op_bra(addr, !(r.ps & FV));
			break;
		}
		case opcid::BVS:
		{
			op_bra(addr, (r.ps & FV));
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
			s8 t = r.a - v;

			set_flag(r.a >= v, FC);
			set_flag(r.a == v, FZ);
			set_flag(t & 0x80, FN);
			break;
		}
		case opcid::CPX:
		{
			u8 v = mem.rb(addr);
			s8 t = r.x - v;

			set_flag(r.x >= v, FC);
			set_flag(r.x == v, FZ);
			set_flag(t & 0x80, FN);
			break;
		}
		case opcid::CPY:
		{
			u8 v = mem.rb(addr);
			s8 t = r.y - v;

			set_flag(r.y >= v, FC);
			set_flag(r.y == v, FZ);
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
			r.x--;

			set_flag(r.x == 0, FZ);
			set_flag(r.x & 0x80, FN);
			break;
		}
		case opcid::DEY:
		{
			r.y--;

			set_flag(r.y == 0, FZ);
			set_flag(r.y & 0x80, FN);
			break;
		}
		case opcid::EOR:
		{
			r.a ^= mem.rb(addr);

			set_flag(r.a == 0, FZ);
			set_flag(r.a & 0x80, FN);
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
			r.x++;

			set_flag(r.x == 0, FZ);
			set_flag(r.x & 0x80, FN);
			break;
		}
		case opcid::INY:
		{
			r.y++;

			set_flag(r.y == 0, FZ);
			set_flag(r.y & 0x80, FN);
			break;
		}
		case opcid::JMP:
		{
			r.pc = addr - 3;
			break;
		}
		case opcid::JSR:
		{
			u16 pc = r.pc + 2;
			op_push(r.sp-- | 0x100, pc & 0xff);
			op_push(r.sp-- | 0x100, pc >> 8);
			r.pc = addr - 3;
			break;
		}
		case opcid::LDA:
		{
			r.a = mem.rb(addr);

			set_flag(r.a == 0, FZ);
			set_flag(r.a & 0x80, FN);
			break;
		}
		case opcid::LDX:
		{
			r.x = mem.rb(addr);

			set_flag(r.x == 0, FZ);
			set_flag(r.x & 0x80, FN);
			break;
		}
		case opcid::LDY:
		{
			r.y = mem.rb(addr);

			set_flag(r.y == 0, FZ);
			set_flag(r.y & 0x80, FN);
			break;
		}
		case opcid::LSR:
		{
			u8 b = 0;
			if (mode == addrmode::accu)
			{
				set_flag(r.a & 0x01, FC);
				r.a = b = (r.a >> 1) & 0x7f;
			}
			else
			{
				b = mem.rb(addr);
				set_flag(b & 0x01, FC);
				b = (b << 1) & 0x7f;
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
			r.a |= mem.rb(addr);

			set_flag(r.a == 0, FZ);
			set_flag(r.a & 0x80, FN);
			break;
		}
		case opcid::PHA:
		{
			op_push(r.sp--, r.a);
			break;
		}
		case opcid::PHP:
		{
			op_push(r.sp--, r.ps);
			break;
		}
		case opcid::PLA:
		{
			r.a = op_pop();
			break;
		}
		case opcid::PLP:
		{
			r.ps = op_pop();
			break;
		}
		case opcid::ROL:
		{
			u8 b = 0;
			u8 bit7 = 0;

			if (mode == addrmode::accu)
			{
				bit7 = r.a & 0x80 ? 1 : 0;
				r.a = b = (r.a << 1) & 0xff;
				if (r.ps & FC)
					r.a |= 0x01;
			}
			else
			{
				b = mem.rb(addr);
				bit7 = r.a & 0x80 ? 1 : 0;
				b = (b << 1) & 0x7f;
				if (r.ps & FC)
					r.a |= 0x01;

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
				bit0 = r.a & 0x80 ? 1 : 0;
				r.a = b = (r.a >> 1) & 0xff;
				if (r.ps & FC)
					r.a |= 0x80;
			}
			else
			{
				b = mem.rb(addr);
				bit0 = r.a & 0x80 ? 1 : 0;
				b = (b >> 1) & 0x7f;
				if (r.ps & FC)
					r.a |= 0x80;

				mem.wb(addr, b);
			}

			set_flag(b == 0, FZ);
			set_flag(b & 0x80, FN);
			set_flag(bit0, FC);
			break;
		}
		case opcid::RTI:
		{
			r.ps = op_pop();
			r.pc = op_pop() << 8 | op_pop();
			break;
		}
		case opcid::RTS:
		{
			r.pc = op_pop() << 8 | op_pop();
			break;
		}
		case opcid::SBC:
		{
			u8 b = mem.rb(addr);
			u16 t = r.a + ~b + (r.ps & FC ? 1 : 0);

			set_flag((t & 0xff) == 0, FZ);
			set_flag(t & 0x80, FN);
			set_flag((r.a ^ b) & (r.a ^ t) & 0x80, FV);
			set_flag((t & 0xff00) == 0, FC);

			r.a = t;
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
			mem.wb(addr, r.a);
			break;
		}
		case opcid::STX:
		{
			pagecrossed = false;
			mem.wb(addr, r.x);
			break;
		}
		case opcid::STY:
		{
			pagecrossed = false;
			mem.wb(addr, r.y);
			break;
		}
		case opcid::TAX:
		{
			r.x = r.a;

			set_flag(r.x == 0, FZ);
			set_flag(r.x & 0x80, FN);
			break;
		}
		case opcid::TAY:
		{
			r.y = r.a;

			set_flag(r.y == 0, FZ);
			set_flag(r.y & 0x80, FN);
			break;
		}
		case opcid::TSX:
		{
			r.x = r.sp;

			set_flag(r.x == 0, FZ);
			set_flag(r.x & 0x80, FN);
			break;
		}
		case opcid::TXA:
		{
			r.a = r.x;

			set_flag(r.a == 0, FZ);
			set_flag(r.a & 0x80, FN);
			break;
		}
		case opcid::TXS:
		{
			r.sp = r.x;
			break;
		}
		case opcid::TYA:
		{
			r.a = r.y;

			set_flag(r.a == 0, FZ);
			set_flag(r.a & 0x80, FN);
			break;
		}
		case opcid::ERR:
		{
			state = cstate::crashed;
			break;
		}
	}

	cycles += disasm[op].cycles;
	r.pc += disasm[op].size;
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
}

void Cpu::reset()
{
	//r.pc = mem.rw(0xfffc);
	r.pc = 0xc000;
}

//void Cpu::op_brk()
//{
//}
//
//void Cpu::op_nmi()
//{
//	mem.wb(r.sp | 0x100, r.pc >> 8);
//	r.sp--;
//	mem.wb(r.sp | 0x100, r.pc & 0xff);
//	r.sp--;
//	mem.wb(r.sp | 0x100, r.ps);
//	r.sp--;
//	r.pc = mem.rw(0xfffa);
//	//ppu_nmi = false;
//}
//
//void Cpu::op_rti()
//{
//	u8 hi, lo;
//	op_plp();
//	r.sp++;
//	lo = mem.rb(r.sp | 0x100);
//	r.sp++;
//	hi = mem.rb(r.sp | 0x100);
//	r.pc = (hi << 8 | lo);
//}
//
//void Cpu::op_rts()
//{
//	u8 hi, lo;
//	r.sp++;
//	lo = mem.rb(r.sp | 0x100);
//	r.sp++;
//	hi = mem.rb(r.sp | 0x100);
//	r.pc = (hi << 8 | lo) + 1;
//}
//
//void Cpu::op_jsr(u16 addr)
//{
//	r.pc++;
//	mem.wb(r.sp | 0x100, r.pc >> 8);
//	r.sp--;
//	mem.wb(r.sp | 0x100, r.pc & 0xff);
//	r.sp--;
//	r.pc = addr;
//}
//
//void Cpu::op_tsx()
//{
//	r.x = r.sp;
//
//	set_flag(r.x == 0, FZ);
//	set_flag(r.x & 0x80, FN);
//}
//
//void Cpu::op_txs()
//{
//	r.sp = r.x;
//}
//
//void Cpu::op_plp()
//{
//	r.sp++;
//	r.ps = mem.rb(r.sp | 0x100);
//}
//
//void Cpu::op_pla()
//{
//	r.sp++;
//	r.a = mem.rb(r.sp | 0x100);
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//void Cpu::op_php()
//{
//	mem.wb(r.sp | 0x100, r.ps | 0x30);
//	r.sp--;
//}
//
//void Cpu::op_pha()
//{
//	mem.wb(r.sp | 0x100, r.a);
//	r.sp--;
//}
//
u8 Cpu::op_pop()
{
	return mem.rb(++r.sp | 0x100);
}

void Cpu::op_push(u16 addr, u8 v)
{
	mem.wb(addr, v);
}

void Cpu::op_bra(u16 addr, bool flag)
{
	if (flag)
	{
		r.pc = addr - 2;
	}
	//else
	//	r.pc += 2;
}
//
//void Cpu::op_bit(u16 addr)
//{
//	u8 b = 0;
//
//	//if (mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else
//	//	b = mem.rb(get_abso());
//
//	u8 t = r.a & b;
//
//	set_flag(t == 0, FZ);
//	set_flag(b & 0x80, FN);
//	set_flag(b & 0x40, FV);
//}
//
//void Cpu::op_and(u16 addr)
//{
//	r.a &= mem.rb(r.pc);
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//void Cpu::op_dec(u16 addr)
//{
//	u16 b = 0;
//
//	//if (mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else
//	//	b = get_abso();
//
//	mem.wb(b, mem.rb(b) - 1);
//
//	set_flag(b == 0, FZ);
//	set_flag(b & 0x80, FN);
//}
//
//void Cpu::op_inc(u16 addr)
//{
//	u16 b = 0;
//
//	//if (mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else
//	//	b = get_abso();
//
//	mem.wb(b, mem.rb(b) + 1);
//
//	set_flag(b == 0, FZ);
//	set_flag(b & 0x80, FN);
//}
//
//void Cpu::op_sty(u16 addr)
//{
//	u16 b = 0;
//
//	//if (mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else if (mode == addrmode::zery)
//	//	b = get_zerx();
//	//else
//	//	b = get_absy();
//
//	mem.wb(b, r.y);
//
//	pagecrossed = false;
//}
//
//void Cpu::op_stx(u16 addr)
//{
//	u16 b = 0;
//
//	//if (mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else if (mode == addrmode::zery)
//	//	b = get_zery();
//	//else
//	//	b = get_absx();
//
//	mem.wb(b, r.x);
//
//	pagecrossed = false;
//}
//
//void Cpu::op_sta(u16 addr)
//{
//	u16 b = 0;
//
//	//if (mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else if (mode == addrmode::zerx)
//	//	b = get_zerx();
//	//else if (mode == addrmode::indx)
//	//	b = get_indx();
//	//else if (mode == addrmode::indy)
//	//	b = get_indy();
//	//else if (mode == addrmode::absx)
//	//	b = get_absx();
//	//else if (mode == addrmode::absy)
//	//	b = get_absy();
//	//else
//	//	b = get_abso();
//
//	mem.wb(b, r.a);
//
//	pagecrossed = false;
//}
//
//void Cpu::op_ldy(u16 addr)
//{
//	u16 b = 0;
//
//	//if (mode == addrmode::imme || mode == addrmode::zerp)
//	//	b = get_zerp();
//	//else if (mode == addrmode::zerx)
//	//	b = get_zerx();
//	//else if (mode == addrmode::absx)
//	//	b = get_absx();
//	//else
//	//	b = get_abso();
//
//	r.y = mem.rb(b);
//
//	set_flag(r.y == 0, FZ);
//	set_flag(r.y & 0x80, FN);
//}
//
//void Cpu::op_ldx(u16 addr)
//{
//	u16 b = 0;
//
//	r.x = (this->*func[mode].modefuncs)(r.pc + 1);
//
//	//r.x = mem.rb(b);
//
//	set_flag(r.x == 0, FZ);
//	set_flag(r.x & 0x80, FN);
//}
//
//void Cpu::op_lda(u16 addr)
//{
//	u16 b = 0;
//
//	r.a = (this->*func[mode].modefuncs)(r.pc + 1);
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//void Cpu::op_cpy(u16 addr)
//{
//	u8 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	u8 v = mem.rb(b);
//	s8 t = r.y - v;
//
//	set_flag(r.y >= v, FC);
//	set_flag(r.y == v, FZ);
//	set_flag(t & 0x80, FN);
//}
//
//void Cpu::op_cpx(u16 addr)
//{
//	u8 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	u8 v = mem.rb(b);
//	s8 t = r.x - v;
//
//	set_flag(r.x >= v, FC);
//	set_flag(r.x == v, FZ);
//	set_flag(t & 0x80, FN);
//}
//
//void Cpu::op_cmp(u16 addr)
//{
//	u16 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	u8 v = mem.rb(b);
//	s8 t = r.a - v;
//
//	set_flag(r.a >= v, FC);
//	set_flag(r.a == v, FZ);
//	set_flag(t & 0x80, FN);
//}
//
//void Cpu::op_eor(u16 addr)
//{
//	u16 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	r.a ^= mem.rb(b);
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//void Cpu::op_ora(u16 addr)
//{
//	u16 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	r.a |= mem.rb(b);
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//void Cpu::op_sbc(u16 addr)
//{
//	u16 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	u8 v = mem.rb(b);
//	u16 t = r.a + ~v + (r.ps & FC ? 1 : 0);
//
//	set_flag((t & 0xff) == 0, FZ);
//	set_flag(t & 0x80, FN);
//	set_flag((r.a ^ v) & (r.a ^ t) & 0x80, FV);
//	set_flag((t & 0xff00) == 0, FC);
//
//	r.a = t;
//}
//
//void Cpu::op_adc(u16 addr)
//{
//	u16 b = 0;
//
//	b = (this->*func[mode].modefuncs)(r.pc);
//
//	u8 v = mem.rb(b);
//	u16 t = r.a + v + (r.ps & FC ? 1 : 0);
//
//	set_flag((t & 0xff) == 0, FZ);
//	set_flag(t & 0x80, FN);
//	set_flag((r.a ^ v) & (r.a ^ t) & 0x80, FV);
//	set_flag((t & 0xff00) == 0, FC);
//
//	r.a = t;
//}
//
//void Cpu::op_dey()
//{
//	r.y--;
//
//	set_flag(r.y == 0, FZ);
//	set_flag(r.y & 0x80, FN);
//}
//
//void Cpu::op_dex()
//{
//	r.x--;
//
//	set_flag(r.x == 0, FZ);
//	set_flag(r.x & 0x80, FN);
//}
//
//void Cpu::op_iny()
//{
//	r.y++;
//
//	set_flag(r.y == 0, FZ);
//	set_flag(r.y & 0x80, FN);
//}
//
//void Cpu::op_inx()
//{
//	r.x++;
//
//	set_flag(r.x == 0, FZ);
//	set_flag(r.x & 0x80, FN);
//}
//
//void Cpu::op_tay()
//{
//	r.y = r.a;
//
//	set_flag(r.y == 0, FZ);
//	set_flag(r.y & 0x80, FN);
//}
//
//void Cpu::op_tax()
//{
//	r.x = r.a;
//
//	set_flag(r.x == 0, FZ);
//	set_flag(r.x & 0x80, FN);
//}
//
//void Cpu::op_tya()
//{
//	r.a = r.y;
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//void Cpu::op_txa()
//{
//	r.a = r.x;
//
//	set_flag(r.a == 0, FZ);
//	set_flag(r.a & 0x80, FN);
//}
//
//u8 Cpu::op_rol(u16 addr)
//{
//	u16 b = 0;
//	u8 v = 0;
//	bool bit7 = false;
//
//	if (mode == addrmode::accu)
//	{
//		bit7 = r.a & (1 << 7) ? true : false;
//		r.a <<= 1 & 0xff;
//		if (r.ps & FC)
//			r.a |= (1 << 0);
//		v = r.a;
//	}
//	else
//	{
//		b = (this->*func[mode].modefuncs)(r.pc);
//
//		v = mem.rb(b);
//		bit7 = v & (1 << 7) ? true : false;
//		v <<= 1 & 0xff;
//		if (r.ps & FC)
//			v |= (1 << 0);
//	}
//
//	set_flag(v == 0, FZ);
//	set_flag(v & 0x80, FN);
//	set_flag(bit7, FC);
//
//	return v;
//}
//
//void Cpu::op_asl(u16 addr)
//{
//	u16 b = 0;
//	u8 v = 0;
//	bool bit7 = false;
//
//	set_flag(bit7, FC);
//
//	if (mode == addrmode::accu)
//	{
//		bit7 = r.a & (1 << 7) ? true : false;
//		r.a <<= 1 & 0xff;
//		if (r.ps & FC)
//			r.a |= (1 << 0);
//		v = r.a;
//	}
//	else
//	{
//		b = (this->*func[mode].modefuncs)(r.pc);
//
//		v = mem.rb(b);
//		bit7 = v & (1 << 7) ? true : false;
//		v <<= 1 & 0xff;
//		if (r.ps & FC)
//			v |= (1 << 0);
//	}
//
//	set_flag(v == 0, FZ);
//	set_flag(v & 0x80, FN);
//}
//
//void Cpu::op_ror(u16 addr)
//{
//}
//
//void Cpu::op_lsr(u16 addr)
//{
//}

void Cpu::set_flag(bool flag, u8 v)
{
	if (flag)
		r.ps |= v;
	else
		r.ps &= ~v;
}

u16 Cpu::get_imme(u16 pc)
{
	return pc;
}

u16 Cpu::get_zerp(u16 pc)
{
	return (u8)mem.rb(pc);
}

u16 Cpu::get_zerx(u16 pc)
{
	return (u8)(mem.rb(pc) + r.x);
}

u16 Cpu::get_zery(u16 pc)
{
	return (u8)(mem.rb(pc) + r.y);
}

u16 Cpu::get_abso(u16 pc)
{
	return mem.rw(pc);
}

u16 Cpu::get_absx(u16 pc)
{
	u16 oldaddr = mem.rw(pc);
	u32 newaddr = oldaddr + r.x;
	pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
	return newaddr & 0xffff;
}

u16 Cpu::get_absy(u16 pc)
{
	u16 oldaddr = mem.rw(pc);
	u32 newaddr = oldaddr + r.y;
	pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
	return newaddr & 0xffff;
}

u16 Cpu::get_indx(u16 pc)
{
	u8 b1 = mem.rb(pc);
	u8 lo = mem.rb(b1 + r.x & 0xff);
	u8 hi = mem.rb(b1 + 1 + r.x & 0xff);
	//return hi << 8 | lo;
	return b1;
}

u16 Cpu::get_indy(u16 pc)
{
	u8 b1 = mem.rb(pc);
	u8 lo = mem.rb(b1 & 0xff);
	u8 hi = mem.rb(b1 + 1 & 0xff);
	u16 oldaddr = (hi << 8 | lo);
	u32 newaddr = oldaddr + r.y;
	pagecrossed = (newaddr & 0xff00) != (oldaddr & 0xff00) ? true : false;
	return b1;
}

u16 Cpu::get_indi(u16 pc)
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

u16 Cpu::get_rela(u16 pc)
{
	u8 b1 = mem.rb(pc);
	return pc + (s8)(b1 + 1);
}

u16 Cpu::get_impl(u16 pc)
{
	return 0;
}

u16 Cpu::get_accu(u16 pc)
{
	return 0;
}

u16 Cpu::get_erro(u16 pc)
{
	state = cstate::crashed;
	return 0;
}
