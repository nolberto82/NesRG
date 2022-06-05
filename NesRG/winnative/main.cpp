#define SDL_MAIN_HANDLED

#include <windows.h>
#include "types.h"

#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "guigl.h"
#include "sdlcc.h"
#include "tracer.h"
#include "breakpoints.h"
#include "main.h"
#include "mappers.h"
#include "mem.h"
#include "types.h"
#include <SDL2/SDL_syswm.h>

#define ID_LOADROM 1
#define ID_ABOUT 2
#define ID_EXIT 3

Cpu cpu;
Registers reg;
PpuRegisters lp;
Header header;
PpuCtrl pctrl;
PpuMask pmask;
PpuStatus pstatus;
Keys newkeys, oldkeys;

int isRunning = 0;
HMENU hFile;
HMENU hMenuBar;

const int fps = 60;

HWND getSDLWinHandle(SDL_Window* win)
{
	SDL_SysWMinfo infoWindow;
	SDL_VERSION(&infoWindow.version);
	if (!SDL_GetWindowWMInfo(win, &infoWindow))
	{
		return NULL;
	}
	return (infoWindow.info.win.window);
}

int main(int argc, char* argv[])
{
	u32 start_time = 0;
	int running = 1;

	if (SDL::init())
	{
		if (APU::init())
		{
			for (int i = 0; i < SDL_NumJoysticks(); i++)
			{
				// Load joystick
				if (SDL_IsGameController(i))
				{
					SDL::controller = SDL_GameControllerOpen(i);
					if (SDL::controller)
					{
						SDL_GameControllerAddMapping(SDL_GameControllerMapping(SDL::controller));
						break;
					}
					else
					{
						printf("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
						running = 0;
					}
				}
			}

#if DEBUG
			GUIGL::debug_enable = true;
#endif

			MEM::init();
			CPU::init();
			PPU::init();

			getSDLWinHandle(SDL::window);

			//Enable WinAPI Events Processing
			//SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

			while (running)
			{

				

				//main_update();
				//GUIGL::update();

				//if (cpu.state == cstate::running)
				//	main_step();

				//limit fps
				//if (SDL::frame_limit)
				//{
				//	if ((1000 / fps) > (SDL_GetTicks() - start_time))
				//		SDL_Delay((1000 / fps) - (SDL_GetTicks() - start_time));
				//}
			}
		}

		APU::clean();
		SDL::clean();
	}

	return 0;
}