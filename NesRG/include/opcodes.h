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
	const char* name;
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
	{"BRK", opcid::BRK, impl, 1, 7}, //0x00
	{"ORA", opcid::ORA, indx, 2, 6}, //0x01
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x02
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x03
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x04
	{"ORA", opcid::ORA, zerp, 2, 3}, //0x05
	{"ASL", opcid::ASL, zerp, 2, 5}, //0x06
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x07
	{"PHP", opcid::PHP, impl, 1, 3}, //0x08
	{"ORA", opcid::ORA, imme, 2, 2}, //0x09
	{"ASL", opcid::ASL, accu, 1, 2}, //0x0A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x0B
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x0C
	{"ORA", opcid::ORA, abso, 3, 4}, //0x0D
	{"ASL", opcid::ASL, abso, 3, 6}, //0x0E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x0F
	{"BPL", opcid::BPL, rela, 2, 2}, //0x10
	{"ORA", opcid::ORA, indy, 2, 5}, //0x11
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x12
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x13
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x14
	{"ORA", opcid::ORA, zerx, 2, 4}, //0x15
	{"ASL", opcid::ASL, zerx, 2, 6}, //0x16
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x17
	{"CLC", opcid::CLC, impl, 1, 2}, //0x18
	{"ORA", opcid::ORA, absy, 3, 4}, //0x19
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x1A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x1B
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x1C
	{"ORA", opcid::ORA, absx, 3, 4}, //0x1D
	{"ASL", opcid::ASL, absx, 3, 7}, //0x1E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x1F
	{"JSR", opcid::JSR, abso, 3, 6}, //0x20
	{"AND", opcid::AND, indx, 2, 6}, //0x21
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x22
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x23
	{"BIT", opcid::BIT, zerp, 2, 3}, //0x24
	{"AND", opcid::AND, zerp, 2, 3}, //0x25
	{"ROL", opcid::ROL, zerp, 2, 5}, //0x26
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x27
	{"PLP", opcid::PLP, impl, 1, 4}, //0x28
	{"AND", opcid::AND, imme, 2, 2}, //0x29
	{"ROL", opcid::ROL, accu, 1, 2}, //0x2A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x2B
	{"BIT", opcid::BIT, abso, 3, 4}, //0x2C
	{"AND", opcid::AND, abso, 3, 4}, //0x2D
	{"ROL", opcid::ROL, abso, 3, 6}, //0x2E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x2F
	{"BMI", opcid::BMI, rela, 2, 2}, //0x30
	{"AND", opcid::AND, indy, 2, 5}, //0x31
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x32
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x33
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x34
	{"AND", opcid::AND, zerx, 2, 4}, //0x35
	{"ROL", opcid::ROL, zerx, 2, 6}, //0x36
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x37
	{"SEC", opcid::SEC, impl, 1, 2}, //0x38
	{"AND", opcid::AND, absy, 3, 4}, //0x39
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x3A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x3B
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x3C
	{"AND", opcid::AND, absx, 3, 4}, //0x3D
	{"ROL", opcid::ROL, absx, 3, 7}, //0x3E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x3F
	{"RTI", opcid::RTI, impl, 1, 6}, //0x40
	{"EOR", opcid::EOR, indx, 2, 6}, //0x41
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x42
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x43
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x44
	{"EOR", opcid::EOR, zerp, 2, 3}, //0x45
	{"LSR", opcid::LSR, zerp, 2, 5}, //0x46
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x47
	{"PHA", opcid::PHA, impl, 1, 3}, //0x48
	{"EOR", opcid::EOR, imme, 2, 2}, //0x49
	{"LSR", opcid::LSR, accu, 1, 2}, //0x4A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x4B
	{"JMP", opcid::JMP, abso, 3, 3}, //0x4C
	{"EOR", opcid::EOR, abso, 3, 4}, //0x4D
	{"LSR", opcid::LSR, abso, 3, 6}, //0x4E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x4F
	{"BVC", opcid::BVC, rela, 2, 2}, //0x50
	{"EOR", opcid::EOR, indy, 2, 5}, //0x51
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x52
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x53
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x54
	{"EOR", opcid::EOR, zerx, 2, 4}, //0x55
	{"LSR", opcid::LSR, zerx, 2, 6}, //0x56
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x57
	{"CLI", opcid::CLI, impl, 1, 2}, //0x58
	{"EOR", opcid::EOR, absy, 3, 4}, //0x59
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x5A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x5B
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x5C
	{"EOR", opcid::EOR, absx, 3, 4}, //0x5D
	{"LSR", opcid::LSR, absx, 3, 7}, //0x5E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x5F
	{"RTS", opcid::RTS, impl, 1, 6}, //0x60
	{"ADC", opcid::ADC, indx, 2, 6}, //0x61
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x62
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x63
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x64
	{"ADC", opcid::ADC, zerp, 2, 3}, //0x65
	{"ROR", opcid::ROR, zerp, 2, 5}, //0x66
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x67
	{"PLA", opcid::PLA, impl, 1, 4}, //0x68
	{"ADC", opcid::ADC, imme, 2, 2}, //0x69
	{"ROR", opcid::ROR, accu, 1, 2}, //0x6A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x6B
	{"JMP", opcid::JMP, indi, 3, 5}, //0x6C
	{"ADC", opcid::ADC, abso, 3, 4}, //0x6D
	{"ROR", opcid::ROR, abso, 3, 7}, //0x6E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x6F
	{"BVS", opcid::BVS, rela, 2, 2}, //0x70
	{"ADC", opcid::ADC, indy, 2, 5}, //0x71
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x72
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x73
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x74
	{"ADC", opcid::ADC, zerx, 2, 4}, //0x75
	{"ROR", opcid::ROR, zerx, 2, 6}, //0x76
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x77
	{"SEI", opcid::SEI, impl, 1, 2}, //0x78
	{"ADC", opcid::ADC, absy, 3, 4}, //0x79
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x7A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x7B
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x7C
	{"ADC", opcid::ADC, absx, 3, 4}, //0x7D
	{"ROR", opcid::ROR, absx, 3, 6}, //0x7E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x7F
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x80
	{"STA", opcid::STA, indx, 2, 6}, //0x81
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x82
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x83
	{"STY", opcid::STY, zerp, 2, 3}, //0x84
	{"STA", opcid::STA, zerp, 2, 3}, //0x85
	{"STX", opcid::STX, zerp, 2, 3}, //0x86
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x87
	{"DEY", opcid::DEY, impl, 1, 2}, //0x88
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x89
	{"TXA", opcid::TXA, impl, 1, 2}, //0x8A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x8B
	{"STY", opcid::STY, abso, 3, 4}, //0x8C
	{"STA", opcid::STA, abso, 3, 4}, //0x8D
	{"STX", opcid::STX, abso, 3, 4}, //0x8E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x8F
	{"BCC", opcid::BCC, rela, 2, 2}, //0x90
	{"STA", opcid::STA, indy, 2, 6}, //0x91
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x92
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x93
	{"STY", opcid::STY, zerx, 2, 3}, //0x94
	{"STA", opcid::STA, zerx, 2, 3}, //0x95
	{"STX", opcid::STX, zery, 2, 3}, //0x96
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x97
	{"TYA", opcid::TYA, impl, 1, 2}, //0x98
	{"STA", opcid::STA, absy, 3, 5}, //0x99
	{"TXS", opcid::TXS, impl, 1, 2}, //0x9A
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x9B
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x9C
	{"STA", opcid::STA, absx, 3, 5}, //0x9D
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x9E
	{"ERR", opcid::ERR ,erro, 1, 0}, //0x9F
	{"LDY", opcid::LDY, imme, 2, 2}, //0xA0
	{"LDA", opcid::LDA, indx, 2, 6}, //0xA1
	{"LDX", opcid::LDX, imme, 2, 2}, //0xA2
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xA3
	{"LDY", opcid::LDY, zerp, 2, 3}, //0xA4
	{"LDA", opcid::LDA, zerp, 2, 3}, //0xA5
	{"LDX", opcid::LDX, zerp, 2, 3}, //0xA6
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xA7
	{"TAY", opcid::TAY, impl, 1, 2}, //0xA8
	{"LDA", opcid::LDA, imme, 2, 2}, //0xA9
	{"TAX", opcid::TAX, impl, 1, 2}, //0xAA
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xAB
	{"LDY", opcid::LDY, abso, 3, 4}, //0xAC
	{"LDA", opcid::LDA, abso, 3, 4}, //0xAD
	{"LDX", opcid::LDX, abso, 3, 4}, //0xAE
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xAF
	{"BCS", opcid::BCS, rela, 2, 2}, //0xB0
	{"LDA", opcid::LDA, indy, 2, 5}, //0xB1
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xB2
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xB3
	{"LDY", opcid::LDY, zerx, 2, 4}, //0xB4
	{"LDA", opcid::LDA, zerx, 2, 4}, //0xB5
	{"LDX", opcid::LDX, zery, 2, 4}, //0xB6
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xB7
	{"CLV", opcid::CLV, impl, 1, 2}, //0xB8
	{"LDA", opcid::LDA, absy, 3, 4}, //0xB9
	{"TSX", opcid::TSX, impl, 1, 2}, //0xBA
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xBB
	{"LDY", opcid::LDY, absx, 3, 4}, //0xBC
	{"LDA", opcid::LDA, absx, 3, 4}, //0xBD
	{"LDX", opcid::LDX, absy, 3, 4}, //0xBE
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xBF
	{"CPY", opcid::CPY, imme, 2, 2}, //0xC0
	{"CMP", opcid::CMP, indx, 2, 6}, //0xC1
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xC2
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xC3
	{"CPY", opcid::CPY, zerp, 2, 3}, //0xC4
	{"CMP", opcid::CMP, zerp, 2, 3}, //0xC5
	{"DEC", opcid::DEC, zerp, 2, 5}, //0xC6
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xC7
	{"INY", opcid::INY, impl, 1, 2}, //0xC8
	{"CMP", opcid::CMP, imme, 2, 2}, //0xC9
	{"DEX", opcid::DEX, impl, 1, 2}, //0xCA
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xCB
	{"CPY", opcid::CPY, abso, 3, 4}, //0xCC
	{"CMP", opcid::CMP, abso, 3, 4}, //0xCD
	{"DEC", opcid::DEC, abso, 3, 6}, //0xCE
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xCF
	{"BNE", opcid::BNE, rela, 2, 2}, //0xD0
	{"CMP", opcid::CMP, indy, 2, 5}, //0xD1
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xD2
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xD3
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xD4
	{"CMP", opcid::CMP, zerx, 2, 4}, //0xD5
	{"DEC", opcid::DEC, zerx, 2, 6}, //0xD6
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xD7
	{"CLD", opcid::CLD, impl, 1, 2}, //0xD8
	{"CMP", opcid::CMP, absy, 3, 4}, //0xD9
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xDA
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xDB
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xDC
	{"CMP", opcid::CMP, absx, 3, 4}, //0xDD
	{"DEC", opcid::DEC, absx, 3, 7}, //0xDE
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xDF
	{"CPX", opcid::CPX, imme, 2, 2}, //0xE0
	{"SBC", opcid::SBC, indx, 2, 6}, //0xE1
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xE2
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xE3
	{"CPX", opcid::CPX, zerp, 2, 3}, //0xE4
	{"SBC", opcid::SBC, zerp, 2, 3}, //0xE5
	{"INC", opcid::INC, zerp, 2, 5}, //0xE6
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xE7
	{"INX", opcid::INX, impl, 1, 2}, //0xE8
	{"SBC", opcid::SBC, imme, 2, 2}, //0xE9
	{"NOP", opcid::NOP, impl, 1, 2}, //0xEA
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xEB
	{"CPX", opcid::CPX, abso, 3, 4}, //0xEC
	{"SBC", opcid::SBC, abso, 3, 4}, //0xED
	{"INC", opcid::INC, abso, 3, 6}, //0xEE
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xEF
	{"BEQ", opcid::BEQ, rela, 2, 2}, //0xF0
	{"SBC", opcid::SBC, indy, 2, 5}, //0xF1
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xF2
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xF3
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xF4
	{"SBC", opcid::SBC, zerx, 2, 4}, //0xF5
	{"INC", opcid::INC, zerx, 2, 6}, //0xF6
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xF7
	{"SED", opcid::SED, impl, 1, 2}, //0xF8
	{"SBC", opcid::SBC, absy, 3, 4}, //0xF9
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xFA
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xFB
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xFC
	{"SBC", opcid::SBC, absx, 3, 4}, //0xFD
	{"INC", opcid::INC, absx, 3, 7}, //0xFE
	{"ERR", opcid::ERR ,erro, 1, 0}, //0xFF
};