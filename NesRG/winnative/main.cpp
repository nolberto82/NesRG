
#include <windows.h>
#include "types.h"

#include "cpu.h"
#include "ppu.h"
#include "apu.h"
//#include "gui.h"
#include "sdlcc.h"
//#include "tracer.h"
//#include "breakpoints.h"
//#include "main.h"
#include "mappers.h"
#include "mem.h"
#include "types.h"
#include <SDL2/SDL_syswm.h>

#define IDM_LOADROM 1
#define IDM_ABOUT 2
#define IDM_EXIT 3

Cpu cpu;
Registers reg;
PpuRegisters lp;
Header header;
PpuCtrl pctrl;
PpuMask pmask;
PpuStatus pstatus;
Keys newkeys, oldkeys;

int isRunning = 0;

const int fps = 60;

void main_step();
void open_rom();
void create_menu_bar(HWND hWnd);

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

			//#if DEBUG
			//			GUIGL::debug_enable = true;
			//#endif

			MEM::init();
			CPU::init();
			PPU::init();

			HWND handle = getSDLWinHandle(SDL::window);
			create_menu_bar(handle);
			SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

			//Enable WinAPI Events Processing
			//SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
			GLfloat color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			while (running)
			{
				SDL_GL_SetSwapInterval(0);

				glClearColor(color[0], color[1], color[2], color[3]);
				glClear(GL_COLOR_BUFFER_BIT);

				SDL_Event event;
				while (SDL_PollEvent(&event))
				{
					if (event.type == SDL_SYSWMEVENT)
					{
						if (event.syswm.msg->msg.win.msg == WM_COMMAND)
						{
							if (LOWORD(event.syswm.msg->msg.win.wParam) == IDM_LOADROM)
								open_rom();
						}
					}
					else if (event.type == WM_PAINT)
					{

					}
					else if (event.type == SDL_QUIT)
					{
						running = 0;
					}
					else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_SPACE)
					{
						GLfloat red = color[0];
						color[0] = color[1];
						color[1] = red;
					}

					main_step();

					InvalidateRect(handle, nullptr, false);

					int w, h;
					SDL_GL_GetDrawableSize(SDL::window, &w, &h);
					SDL::render_screen(SDL::screen, PPU::screen_pix.data(), w, h, 0);

					//RECT r = { 0,0,w,h };

					SDL_GL_SwapWindow(SDL::window);
				}

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

			APU::clean();
			SDL::clean();
		}
	}

	return 0;
}

void main_step()
{
	if (!MEM::rom_loaded)
		return;

	//GUIGL::follow_pc = true;
	PPU::frame_ready = false;
	cpu.cpucycles = FRAME_CYCLES;
	while (!PPU::frame_ready)
	{
		u16 pc = reg.pc;
		//if (cpu.stepoveraddr == -1)
		//{
		//	for (auto& it : breakpoints)
		//	{
		//		if (!it.enabled)
		//			continue;
		//		if (bp_exec_access(reg.pc))
		//		{
		//			cpu.state = cstate::debugging;
		//			return;
		//		}
		//		else if (bp_read_access(MEM::read_addr))
		//		{
		//			MEM::read_addr = -1;
		//			cpu.state = cstate::debugging;
		//			return;
		//		}
		//		else if (bp_write_access(MEM::write_addr))
		//		{
		//			MEM::write_addr = -1;
		//			cpu.state = cstate::debugging;
		//			return;
		//		}
		//		else if (bp_ppu_write_access(MEM::ppu_write_addr))
		//		{
		//			MEM::ppu_write_addr = -1;
		//			cpu.state = cstate::debugging;
		//			return;
		//		}
		//	}

		if (cpu.state == cstate::crashed)
			return;

		//if (logging)
		//	log_to_file(pc);

		CPU::step();

		if ((u16)cpu.stepoveraddr == reg.pc)
		{
			cpu.state = cstate::debugging;
			cpu.stepoveraddr = -1;
			return;
		}
	}

	APU::step();
}

void main_step_over()
{
	if (!MEM::rom_loaded)
		return;

	u8 op = MEM::ram[reg.pc];
	if (op == 0x00 || op == 0x20)
	{
		cpu.stepoveraddr = reg.pc + disasm[op].size;
		cpu.state = cstate::running;
	}
	else
	{
		CPU::step();
		//GUIGL::follow_pc = true;
		cpu.state = cstate::debugging;
	}
}

void create_menu_bar(HWND hWnd)
{
	HMENU hMenu = CreateMenu();
	HMENU hSubMenu = CreatePopupMenu();

	AppendMenu(hSubMenu, MF_STRING, IDM_LOADROM, "Open Rom");

	AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, "&File");

	SetMenu(hWnd, hMenu);
}

void open_rom()
{
	OPENFILENAMEA ofn;
	char filename[512]{};
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = nullptr;
	ofn.lpstrTitle = "Open Nes Rom";
	ofn.lpstrFilter = "Nes Roms (*.nes)\0*.nes";
	ofn.lpstrFile = filename;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = "nes";
	std::vector<uint8_t> rom;

	if (GetOpenFileNameA(&ofn))
	{
		if (!MEM::load_rom((char*)ofn.lpstrFile))
		{
			cpu.state = cstate::debugging;
			MEM::rom.clear();
		}
		else
		{
			if (MEM::rom_loaded) //&& !debug_enable)
				cpu.state = cstate::running;
		}
	}
}