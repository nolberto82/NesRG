#include "apu.h"
#include "mem.h"
#include "sdlcore.h"

const long sample_rate = 96000;
const size_t OUT_SIZE = 4096;
blip_sample_t buf[OUT_SIZE];
Simple_Apu* sapu;
Sound_Queue* sound_queue;

APU::APU()
{
}

APU::~APU()
{
	delete sound_queue;
	delete sapu;
}

bool APU::init()
{
	sapu = new Simple_Apu();
	sound_queue = new Sound_Queue();
	sapu->dmc_reader(read_dmc, NULL);
	sound_queue->init(sample_rate);

	if (sapu->sample_rate(sample_rate))
		return false;
	return true;
}

void APU::wb(u16 addr, u8 v)
{
	sapu->write_register(addr, v);
}

u8 APU::rb()
{
	return sapu->read_status();
}

void APU::play(const blip_sample_t* samples, int count)
{
	sound_queue->write(samples, count);
}

void APU::step()
{
	if (!sdl.frame_limit)
		return;
	sapu->end_frame();

	if (sapu->samples_avail() >= OUT_SIZE)
	{
		long count = sapu->read_samples(buf, OUT_SIZE);
		play(buf, count);
	}
}

int APU::read_dmc(void* _, cpu_addr_t addr)
{
	return MEM::rb(addr);
}

void APU::reset()
{
	
}

