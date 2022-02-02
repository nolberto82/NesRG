
//#include "include/types.h"
#include "sdlgfx.h"
#include "cpu.h"
#include "ppu.h"
#include "gui.h"

SDLGfx gfx;
Cpu cpu;
Memory mem;
Ppu ppu;
Registers reg;
PpuRegisters preg;

int main(int argc, char* argv[])
{
	printf("Starting NesRG");
	Gui gui;

	if (gfx.init())
	{
		cpu.init();
		gui.init();

		if (mem.load_rom("D:\\Emulators+Hacking\\NES\\Mapper0Games\\nestest.nes"))
		{
			cpu.reset();
			gui.update();
		}

		gui.clean();
		gfx.clean();
	}

	return 0;
}
