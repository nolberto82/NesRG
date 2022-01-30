
//#include "include/types.h"
#include "sdlgfx.h"
#include "cpu.h"
#include "ppu.h"
#include "debugger.h"

SDLGfx gfx;
Cpu cpu;
Memory mem;
Ppu ppu;
Registers reg;
PpuRegisters preg;

int main(int argc, char* argv[])
{
	printf("Starting NesRG");
	Debugger dbg;

	if (gfx.init())
	{
		cpu.init();
		dbg.init();
		dbg.update();
		dbg.clean();
		gfx.clean();
	}

	return 0;
}
