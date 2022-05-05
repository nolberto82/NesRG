#include "types.h"

//#include "nes_apu/Nes_Apu.h"
#include "Simple_Apu.h"
#include "Sound_Queue.h"

struct APU
{
public:
	APU();
	~APU();
	bool init();
	void wb(u16 addr, u8 v);
	u8 rb();
	void play(const blip_sample_t* samples, int count);
	void step();
	static int read_dmc(void* _, cpu_addr_t addr);
	void reset();
};

extern APU apu;