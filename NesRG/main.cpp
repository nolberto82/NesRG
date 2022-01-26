
//#include "include/types.h"
#include "sdlgfx.h"
#include "debugger.h"
#include "cpu.h"
#include "ppu.h"

SDLGfx gfx;
Cpu cpu;
Memory mem;
Ppu ppu;
Registers reg;
PpuRegisters preg;

//std::shared_ptr<sdlgfx> gfx(new sdlgfx);

int main(int argc, char* argv[])
{
	printf("Starting NesRG");

	//if (mem.load("tests/nestest.nes"))
	//{
		//cpu.init();
		//cpu.reset();
		gfx.init();

#if _WIN64
#include "debugger.h"
		Debugger dbg;

		dbg.init();
		dbg.update();
		dbg.clean();
#else

#endif // _WIN64
	//}

	gfx.clean();

	return 0;
}
