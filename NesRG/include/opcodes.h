#pragma once

enum opcid
{
	ADC,AND,ASL,BCC,BCS,BEQ,BIT,BMI,BNE,BPL,BRK,BVC,BVS,CLC,
	CLD,CLI,CLV,CMP,CPX,CPY,DEC,DEX,DEY,EOR,INC,INX,INY,JMP,
	JSR,LDA,LDX,LDY,LSR,NOP,ORA,PHA,PHP,PLA,PLP,ROL,ROR,RTI,
	RTS,SBC,SEC,SED,SEI,STA,STX,STY,TAX,TAY,TSX,TXA,TXS,TYA,ERR,
};

struct disasmdata
{
	char* name;
	opcid id;
	int mode;
	int size;
	int cycles;
};

enum addrmode
{	
	imme,
	zerp,
	zerx,
	zery,
	abso,
	absx,
	absy,
	indx,
	indy,
	indi,
	rela,
	impl,
	accu,
	erro
};

static struct disasmdata disasm[256] = 
{
	{"brk", opcid::BRK, impl, 1, 7}, //0x00
	{"ora", opcid::ORA, indx, 2, 6}, //0x01
	{"err", opcid::ERR ,erro, 1, 0}, //0x02
	{"err", opcid::ERR ,erro, 1, 0}, //0x03
	{"err", opcid::ERR ,erro, 1, 0}, //0x04
	{"ora", opcid::ORA, zerp, 2, 3}, //0x05
	{"asl", opcid::ASL, zerp, 2, 5}, //0x06
	{"err", opcid::ERR ,erro, 1, 0}, //0x07
	{"php", opcid::PHP, impl, 1, 3}, //0x08
	{"ora", opcid::ORA, imme, 2, 2}, //0x09
	{"asl", opcid::ASL, accu, 1, 2}, //0x0A
	{"err", opcid::ERR ,erro, 1, 0}, //0x0B
	{"err", opcid::ERR ,erro, 1, 0}, //0x0C
	{"ora", opcid::ORA, abso, 3, 4}, //0x0D
	{"asl", opcid::ASL, abso, 3, 6}, //0x0E
	{"err", opcid::ERR ,erro, 1, 0}, //0x0F
	{"bpl", opcid::BPL, rela, 2, 2}, //0x10
	{"ora", opcid::ORA, indy, 2, 5}, //0x11
	{"err", opcid::ERR ,erro, 1, 0}, //0x12
	{"err", opcid::ERR ,erro, 1, 0}, //0x13
	{"err", opcid::ERR ,erro, 1, 0}, //0x14
	{"ora", opcid::ORA, zerx, 2, 4}, //0x15
	{"asl", opcid::ASL, zerx, 2, 6}, //0x16
	{"err", opcid::ERR ,erro, 1, 0}, //0x17
	{"clc", opcid::CLC, impl, 1, 2}, //0x18
	{"ora", opcid::ORA, absy, 3, 4}, //0x19
	{"err", opcid::ERR ,erro, 1, 0}, //0x1A
	{"err", opcid::ERR ,erro, 1, 0}, //0x1B
	{"err", opcid::ERR ,erro, 1, 0}, //0x1C
	{"ora", opcid::ORA, absx, 3, 4}, //0x1D
	{"asl", opcid::ASL, absx, 3, 7}, //0x1E
	{"err", opcid::ERR ,erro, 1, 0}, //0x1F
	{"jsr", opcid::JSR, abso, 3, 6}, //0x20
	{"and", opcid::AND, indx, 2, 6}, //0x21
	{"err", opcid::ERR ,erro, 1, 0}, //0x22
	{"err", opcid::ERR ,erro, 1, 0}, //0x23
	{"bit", opcid::BIT, zerp, 2, 3}, //0x24
	{"and", opcid::AND, zerp, 2, 3}, //0x25
	{"rol", opcid::ROL, zerp, 2, 5}, //0x26
	{"err", opcid::ERR ,erro, 1, 0}, //0x27
	{"plp", opcid::PLP, impl, 1, 4}, //0x28
	{"and", opcid::AND, imme, 2, 2}, //0x29
	{"rol", opcid::ROL, accu, 1, 2}, //0x2A
	{"err", opcid::ERR ,erro, 1, 0}, //0x2B
	{"bit", opcid::BIT, abso, 3, 4}, //0x2C
	{"and", opcid::AND, abso, 3, 4}, //0x2D
	{"rol", opcid::ROL, abso, 3, 6}, //0x2E
	{"err", opcid::ERR ,erro, 1, 0}, //0x2F
	{"bmi", opcid::BMI, rela, 2, 2}, //0x30
	{"and", opcid::AND, indy, 2, 5}, //0x31
	{"err", opcid::ERR ,erro, 1, 0}, //0x32
	{"err", opcid::ERR ,erro, 1, 0}, //0x33
	{"err", opcid::ERR ,erro, 1, 0}, //0x34
	{"and", opcid::AND, zerx, 2, 4}, //0x35
	{"rol", opcid::ROL, zerx, 2, 6}, //0x36
	{"err", opcid::ERR ,erro, 1, 0}, //0x37
	{"sec", opcid::SEC, impl, 1, 2}, //0x38
	{"and", opcid::AND, absy, 3, 4}, //0x39
	{"err", opcid::ERR ,erro, 1, 0}, //0x3A
	{"err", opcid::ERR ,erro, 1, 0}, //0x3B
	{"err", opcid::ERR ,erro, 1, 0}, //0x3C
	{"and", opcid::AND, absx, 3, 4}, //0x3D
	{"rol", opcid::ROL, absx, 3, 7}, //0x3E
	{"err", opcid::ERR ,erro, 1, 0}, //0x3F
	{"rti", opcid::RTI, impl, 1, 6}, //0x40
	{"eor", opcid::EOR, indx, 2, 6}, //0x41
	{"err", opcid::ERR ,erro, 1, 0}, //0x42
	{"err", opcid::ERR ,erro, 1, 0}, //0x43
	{"err", opcid::ERR ,erro, 1, 0}, //0x44
	{"eor", opcid::EOR, zerp, 2, 3}, //0x45
	{"lsr", opcid::LSR, zerp, 2, 5}, //0x46
	{"err", opcid::ERR ,erro, 1, 0}, //0x47
	{"pha", opcid::PHA, impl, 1, 3}, //0x48
	{"eor", opcid::EOR, imme, 2, 2}, //0x49
	{"lsr", opcid::LSR, accu, 1, 2}, //0x4A
	{"err", opcid::ERR ,erro, 1, 0}, //0x4B
	{"jmp", opcid::JMP, abso, 3, 3}, //0x4C
	{"eor", opcid::EOR, abso, 3, 4}, //0x4D
	{"lsr", opcid::LSR, abso, 3, 6}, //0x4E
	{"err", opcid::ERR ,erro, 1, 0}, //0x4F
	{"bvc", opcid::BVC, rela, 2, 2}, //0x50
	{"eor", opcid::EOR, indy, 2, 5}, //0x51
	{"err", opcid::ERR ,erro, 1, 0}, //0x52
	{"err", opcid::ERR ,erro, 1, 0}, //0x53
	{"err", opcid::ERR ,erro, 1, 0}, //0x54
	{"eor", opcid::EOR, zerx, 2, 4}, //0x55
	{"lsr", opcid::LSR, zerx, 2, 6}, //0x56
	{"err", opcid::ERR ,erro, 1, 0}, //0x57
	{"cli", opcid::CLI, impl, 1, 2}, //0x58
	{"eor", opcid::EOR, absy, 3, 4}, //0x59
	{"err", opcid::ERR ,erro, 1, 0}, //0x5A
	{"err", opcid::ERR ,erro, 1, 0}, //0x5B
	{"err", opcid::ERR ,erro, 1, 0}, //0x5C
	{"eor", opcid::EOR, absx, 3, 4}, //0x5D
	{"lsr", opcid::LSR, absx, 3, 7}, //0x5E
	{"err", opcid::ERR ,erro, 1, 0}, //0x5F
	{"rts", opcid::RTS, impl, 1, 6}, //0x60
	{"adc", opcid::ADC, indx, 2, 6}, //0x61
	{"err", opcid::ERR ,erro, 1, 0}, //0x62
	{"err", opcid::ERR ,erro, 1, 0}, //0x63
	{"err", opcid::ERR ,erro, 1, 0}, //0x64
	{"adc", opcid::ADC, zerp, 2, 3}, //0x65
	{"ror", opcid::ROR, zerp, 2, 5}, //0x66
	{"err", opcid::ERR ,erro, 1, 0}, //0x67
	{"pla", opcid::PLA, impl, 1, 4}, //0x68
	{"adc", opcid::ADC, imme, 2, 2}, //0x69
	{"ror", opcid::ROR, accu, 1, 2}, //0x6A
	{"err", opcid::ERR ,erro, 1, 0}, //0x6B
	{"jmp", opcid::JMP, indi, 3, 5}, //0x6C
	{"adc", opcid::ADC, abso, 3, 4}, //0x6D
	{"ror", opcid::ROR, absx, 3, 7}, //0x6E
	{"err", opcid::ERR ,erro, 1, 0}, //0x6F
	{"bvs", opcid::BVS, rela, 2, 2}, //0x70
	{"adc", opcid::ADC, indy, 2, 5}, //0x71
	{"err", opcid::ERR ,erro, 1, 0}, //0x72
	{"err", opcid::ERR ,erro, 1, 0}, //0x73
	{"err", opcid::ERR ,erro, 1, 0}, //0x74
	{"adc", opcid::ADC, zerx, 2, 4}, //0x75
	{"ror", opcid::ROR, zerx, 2, 6}, //0x76
	{"err", opcid::ERR ,erro, 1, 0}, //0x77
	{"sei", opcid::SEI, impl, 1, 2}, //0x78
	{"adc", opcid::ADC, absy, 3, 4}, //0x79
	{"err", opcid::ERR ,erro, 1, 0}, //0x7A
	{"err", opcid::ERR ,erro, 1, 0}, //0x7B
	{"err", opcid::ERR ,erro, 1, 0}, //0x7C
	{"adc", opcid::ADC, absx, 3, 4}, //0x7D
	{"ror", opcid::ROR, abso, 3, 6}, //0x7E
	{"err", opcid::ERR ,erro, 1, 0}, //0x7F
	{"err", opcid::ERR ,erro, 1, 0}, //0x80
	{"sta", opcid::STA, indx, 2, 6}, //0x81
	{"err", opcid::ERR ,erro, 1, 0}, //0x82
	{"err", opcid::ERR ,erro, 1, 0}, //0x83
	{"sty", opcid::STY, zerp, 2, 3}, //0x84
	{"sta", opcid::STA, zerp, 2, 3}, //0x85
	{"stx", opcid::STX, zerp, 2, 3}, //0x86
	{"err", opcid::ERR ,erro, 1, 0}, //0x87
	{"dey", opcid::DEY, impl, 1, 2}, //0x88
	{"err", opcid::ERR ,erro, 1, 0}, //0x89
	{"txa", opcid::TXA, impl, 1, 2}, //0x8A
	{"err", opcid::ERR ,erro, 1, 0}, //0x8B
	{"sty", opcid::STY, abso, 3, 4}, //0x8C
	{"sta", opcid::STA, abso, 3, 4}, //0x8D
	{"stx", opcid::STX, abso, 3, 4}, //0x8E
	{"err", opcid::ERR ,erro, 1, 0}, //0x8F
	{"bcc", opcid::BCC, rela, 2, 2}, //0x90
	{"sta", opcid::STA, indx, 2, 6}, //0x91
	{"err", opcid::ERR ,erro, 1, 0}, //0x92
	{"err", opcid::ERR ,erro, 1, 0}, //0x93
	{"sty", opcid::STY, zerx, 2, 3}, //0x94
	{"sta", opcid::STA, zerx, 2, 3}, //0x95
	{"stx", opcid::STX, zery, 2, 3}, //0x96
	{"err", opcid::ERR ,erro, 1, 0}, //0x97
	{"tya", opcid::TYA, impl, 1, 2}, //0x98
	{"err", opcid::ERR ,erro, 1, 0}, //0x99
	{"txs", opcid::TXS, impl, 1, 2}, //0x9A
	{"err", opcid::ERR ,erro, 1, 0}, //0x9B
	{"err", opcid::ERR ,erro, 1, 0}, //0x9C
	{"sta", opcid::STA, absx, 3, 5}, //0x9D
	{"err", opcid::ERR ,erro, 1, 0}, //0x9E
	{"err", opcid::ERR ,erro, 1, 0}, //0x9F
	{"ldy", opcid::LDY, imme, 2, 2}, //0xA0
	{"lda", opcid::LDA, indx, 2, 6}, //0xA1
	{"ldx", opcid::LDX, imme, 2, 2}, //0xA2
	{"err", opcid::ERR ,erro, 1, 0}, //0xA3
	{"ldy", opcid::LDY, zerp, 2, 3}, //0xA4
	{"lda", opcid::LDA, zerp, 2, 3}, //0xA5
	{"ldx", opcid::LDX, zerp, 2, 3}, //0xA6
	{"err", opcid::ERR ,erro, 1, 0}, //0xA7
	{"tay", opcid::TAY, impl, 1, 2}, //0xA8
	{"lda", opcid::LDA, imme, 2, 2}, //0xA9
	{"tax", opcid::TAX, impl, 1, 2}, //0xAA
	{"err", opcid::ERR ,erro, 1, 0}, //0xAB
	{"ldy", opcid::LDY, abso, 3, 4}, //0xAC
	{"lda", opcid::LDA, abso, 3, 4}, //0xAD
	{"ldx", opcid::LDX, abso, 3, 4}, //0xAE
	{"err", opcid::ERR ,erro, 1, 0}, //0xAF
	{"bcs", opcid::BCS, rela, 2, 2}, //0xB0
	{"lda", opcid::LDA, indy, 2, 5}, //0xB1
	{"err", opcid::ERR ,erro, 1, 0}, //0xB2
	{"err", opcid::ERR ,erro, 1, 0}, //0xB3
	{"ldy", opcid::LDY, zerx, 2, 4}, //0xB4
	{"lda", opcid::LDA, zerx, 2, 4}, //0xB5
	{"ldx", opcid::LDX, zery, 2, 4}, //0xB6
	{"err", opcid::ERR ,erro, 1, 0}, //0xB7
	{"clv", opcid::CLV, impl, 1, 2}, //0xB8
	{"lda", opcid::LDA, absy, 3, 4}, //0xB9
	{"tsx", opcid::TSX, impl, 1, 2}, //0xBA
	{"err", opcid::ERR ,erro, 1, 0}, //0xBB
	{"ldy", opcid::LDY, absx, 3, 4}, //0xBC
	{"lda", opcid::LDA, absx, 3, 4}, //0xBD
	{"ldx", opcid::LDX, absy, 3, 4}, //0xBE
	{"err", opcid::ERR ,erro, 1, 0}, //0xBF
	{"cpy", opcid::CPY, imme, 2, 2}, //0xC0
	{"cmp", opcid::CMP, indx, 2, 6}, //0xC1
	{"err", opcid::ERR ,erro, 1, 0}, //0xC2
	{"err", opcid::ERR ,erro, 1, 0}, //0xC3
	{"cpy", opcid::CPY, zerp, 2, 3}, //0xC4
	{"cmp", opcid::CMP, zerp, 2, 3}, //0xC5
	{"dec", opcid::DEC, zerp, 2, 5}, //0xC6
	{"err", opcid::ERR ,erro, 1, 0}, //0xC7
	{"iny", opcid::INY, impl, 1, 2}, //0xC8
	{"cmp", opcid::CMP, imme, 2, 2}, //0xC9
	{"dex", opcid::DEX, impl, 1, 2}, //0xCA
	{"err", opcid::ERR ,erro, 1, 0}, //0xCB
	{"cpy", opcid::CPY, abso, 3, 4}, //0xCC
	{"cmp", opcid::CMP, abso, 3, 4}, //0xCD
	{"dec", opcid::DEC, abso, 3, 6}, //0xCE
	{"err", opcid::ERR ,erro, 1, 0}, //0xCF
	{"bne", opcid::BNE, rela, 2, 2}, //0xD0
	{"cmp", opcid::CMP, indy, 2, 5}, //0xD1
	{"err", opcid::ERR ,erro, 1, 0}, //0xD2
	{"err", opcid::ERR ,erro, 1, 0}, //0xD3
	{"err", opcid::ERR ,erro, 1, 0}, //0xD4
	{"cmp", opcid::CMP, zerx, 2, 4}, //0xD5
	{"dec", opcid::DEC, zerx, 2, 6}, //0xD6
	{"err", opcid::ERR ,erro, 1, 0}, //0xD7
	{"cld", opcid::CLD, impl, 1, 2}, //0xD8
	{"cmp", opcid::CMP, absy, 3, 4}, //0xD9
	{"err", opcid::ERR ,erro, 1, 0}, //0xDA
	{"err", opcid::ERR ,erro, 1, 0}, //0xDB
	{"err", opcid::ERR ,erro, 1, 0}, //0xDC
	{"cmp", opcid::CMP, absx, 3, 4}, //0xDD
	{"dec", opcid::DEC, absx, 3, 7}, //0xDE
	{"err", opcid::ERR ,erro, 1, 0}, //0xDF
	{"cpx", opcid::CPX, imme, 2, 2}, //0xE0
	{"sbc", opcid::SBC, indx, 2, 6}, //0xE1
	{"err", opcid::ERR ,erro, 1, 0}, //0xE2
	{"err", opcid::ERR ,erro, 1, 0}, //0xE3
	{"cpx", opcid::CPX, zerp, 2, 3}, //0xE4
	{"sbc", opcid::SBC, zerp, 2, 3}, //0xE5
	{"inc", opcid::INC, zerp, 2, 5}, //0xE6
	{"err", opcid::ERR ,erro, 1, 0}, //0xE7
	{"inx", opcid::INX, impl, 1, 2}, //0xE8
	{"sbc", opcid::SBC, imme, 2, 2}, //0xE9
	{"nop", opcid::NOP, impl, 1, 2}, //0xEA
	{"err", opcid::ERR ,erro, 1, 0}, //0xEB
	{"cpx", opcid::CPX, abso, 3, 4}, //0xEC
	{"sbc", opcid::SBC, abso, 3, 4}, //0xED
	{"inc", opcid::INC, abso, 3, 6}, //0xEE
	{"err", opcid::ERR ,erro, 1, 0}, //0xEF
	{"beq", opcid::BEQ, rela, 2, 2}, //0xF0
	{"sbc", opcid::SBC, indy, 2, 5}, //0xF1
	{"err", opcid::ERR ,erro, 1, 0}, //0xF2
	{"err", opcid::ERR ,erro, 1, 0}, //0xF3
	{"err", opcid::ERR ,erro, 1, 0}, //0xF4
	{"sbc", opcid::SBC, zerx, 2, 4}, //0xF5
	{"inc", opcid::INC, zerx, 2, 6}, //0xF6
	{"err", opcid::ERR ,erro, 1, 0}, //0xF7
	{"sed", opcid::SED, impl, 1, 2}, //0xF8
	{"sbc", opcid::SBC, absy, 3, 4}, //0xF9
	{"err", opcid::ERR ,erro, 1, 0}, //0xFA
	{"err", opcid::ERR ,erro, 1, 0}, //0xFB
	{"err", opcid::ERR ,erro, 1, 0}, //0xFC
	{"sbc", opcid::SBC, absx, 3, 4}, //0xFD
	{"inc", opcid::INC, absx, 3, 7}, //0xFE
	{"err", opcid::ERR ,erro, 1, 0}, //0xFF
};