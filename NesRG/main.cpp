
#include "cpu.h"
#include "ppu.h"
#include "gui.h"

Cpu cpu;
Memory mem;
Ppu ppu;
Registers reg;
PpuRegisters preg;
Gui gui;

int main(int argc, char* argv[])
{
	printf("Starting NesRG");

	//gfx.init_sdl();

	if (gui.init_gui())
	{
		cpu.init();

		if (mem.load_rom("D:\\Emulators+Hacking\\NES\\Mapper0Games\\nestest.nes"))
		{
			cpu.reset();
			gui.update_gui();
		}
	}

	return 0;
}
