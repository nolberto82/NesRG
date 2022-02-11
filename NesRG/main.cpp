
#include "cpu.h"
#include "ppu.h"
#include "gui.h"
#include "renderer.h"

Cpu cpu;
Ppu ppu;
Registers reg;
PpuRegisters lp;
Header header;

int main(int argc, char* argv[])
{
	if (render_init() && gui_init())
	{
		mem_init();
		cpu_init();
		gui_update();
	}

	gui_clean();
	render_clean();

	return 0;
}
