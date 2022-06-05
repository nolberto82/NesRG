#include "apu.h"
#include "mem.h"
#include "sdlcc.h"

const long sample_rate = 44000;
const size_t bufsize = 4096;
blip_sample_t buf[bufsize];
Simple_Apu* sapu;
Sound_Queue* sound_queue;

namespace APU
{
	bool init()
	{
		sapu = new Simple_Apu();
		sound_queue = new Sound_Queue();
		sapu->dmc_reader(read_dmc, NULL);
		sound_queue->init(sample_rate);

		if (sapu->sample_rate(sample_rate))
			return false;
		return true;
	}

	void wb(u16 addr, u8 v)
	{
		if (!SDL::frame_limit)
			return;
		sapu->write_register(addr, v);
	}

	u8 rb()
	{
		return sapu->read_status();
	}

	void play(const blip_sample_t* samples, int count)
	{
		sound_queue->write(samples, count);
	}

	void step()
	{
		if (!SDL::frame_limit)
			return;

		if (sapu->samples_avail() >= bufsize)
		{
			long count = sapu->read_samples(buf, bufsize);
			play(buf, count);
		}
		sapu->end_frame();
	}

	int read_dmc(void* _, cpu_addr_t addr)
	{
		return MEM::rb(addr);
	}

	void reset()
	{
		if (sapu && sound_queue)
		{
			clean();
			init();
		}
	}

	void clean()
	{
		if (sound_queue)
			delete sound_queue;
		if (sapu)
			delete sapu;
	}
}

