#pragma once

#include "types.h"

class Mapper
{
public:
	Mapper::Mapper()
	{
	}

	virtual Mapper::~Mapper()
	{
		
	}

	vector<u8> prg;
	vector<u8> chr;

	u8 prgmode = 0;
	u8 chrmode = 0;
	u16 prgbank = 0;
	u16 chrbank = 0;
	u8 sram = 0;
	u8 fire = 0;
	u8 counter = 0;

	virtual void setup(struct Header h) = 0;
	virtual void update(u16 addr, u8 v) = 0;
	virtual void set_latch(u16 addr, u8 v) = 0;
	virtual void reset() = 0;
	virtual void scanline() = 0;
	virtual vector<u8> get_prg() = 0;
	virtual vector<u8> get_chr() = 0;
};

struct Mapper000 : public Mapper
{
	Mapper000() {}

	vector<u8> prg;
	vector<u8> chr;

	void setup(struct Header h);
	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v) {};
	void reset();
	void scanline();
	vector<u8> get_prg();
	vector<u8> get_chr();
};

struct Mapper001 : public Mapper
{
	Mapper001() {}

	vector<u8> prg;
	vector<u8> chr;

	u8 writes = 0;
	u8 control = 0;

	void setup(struct Header h);
	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v) {};
	void reset();
	void scanline();
	vector<u8> get_prg();
	vector<u8> get_chr();
};

struct Mapper002 : public Mapper
{
	Mapper002() {}

	vector<u8> prg;
	vector<u8> chr;

	void setup(struct Header h);
	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v) {};
	void reset();
	void scanline();
	vector<u8> get_prg();
	vector<u8> get_chr();
};

struct Mapper003 : public Mapper
{
	Mapper003() {}

	vector<u8> prg;
	vector<u8> chr;

	void setup(struct Header h);
	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v) {};
	void reset();
	void scanline();
	vector<u8> get_prg();
	vector<u8> get_chr();
};

struct Mapper004 : public Mapper
{
	Mapper004() {}

	vector<u8> prg;
	vector<u8> chr;

	u8 irq = 0;
	u8 reload = 0;
	u8 rvalue = 0;
	u8 write_prot = 0;
	u8 prg_ram = 0;
	u8 chrreg = 0;
	u8 bankreg[8] = { 0 };

	void setup(struct Header h);
	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v) {};
	void reset();
	void scanline();
	vector<u8> get_prg();
	vector<u8> get_chr();
};

struct Mapper009 : public Mapper
{
	Mapper009() {}

	vector<u8> prg;
	vector<u8> chr;
	vector<u8> chr1;
	vector<u8> chr2;

	u8 latch1, latch2 = 1;
	u8 updatechr = 0;

	void setup(struct Header h);
	void update(u16 addr, u8 v);
	void set_latch(u16 addr, u8 v);
	void reset();
	void scanline();
	vector<u8> get_prg();
	vector<u8> get_chr();
};