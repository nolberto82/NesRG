
#include "cpu.h"
#include "ppu.h"
#include "gui.h"
#include "renderer.h"

Registers reg;
PpuRegisters preg;

int main(int argc, char* argv[])
{
	Cpu* cpu = new Cpu();
	Memory* mem = new Memory();
	Ppu* ppu = new Ppu();
	Gui* gui = new Gui();
	Gfx* gfx = new Gfx();

	cpu->set_obj(mem, ppu);
	ppu->set_obj(mem, gfx);
	mem->set_obj(cpu, ppu);
	gui->set_obj(cpu, mem, ppu, gfx);

	if (gfx->init() && gui->init())
	{
		cpu->init();
		gui->update();
	}

	delete cpu;
	delete mem;
	delete ppu;
	delete gui;
	delete gfx;

	return 0;
}
