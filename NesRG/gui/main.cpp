#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "gui.h"
#include "sdlcc.h"
#include "tracer.h"
#include "breakpoints.h"
#include "main.h"
#include "mappers.h"
#include "mem.h"
#include "types.h"

const int fps = 60;

Cpu cpu;
Registers reg;
PpuRegisters lp;
Header header;
PpuCtrl pctrl;
PpuMask pmask;
PpuStatus pstatus;
Keys newkeys, oldkeys;

int main(int argc, char* argv[])
{
	u32 start_time = 0;
	GUIGL::running = 1;

	if (SDL::init())
	{
		if (GUIGL::init())
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
							GUIGL::running = 0;
						}
					}
				}

#if DEBUG
				GUIGL::debug_enable = true;
#endif

				MEM::init();
				CPU::init();
				PPU::init();
			}

			ImGuiIO& io = ImGui::GetIO(); (void)io;
			//io.IniFilename = "assets\\imgui.ini";

			while (GUIGL::running)
			{
				start_time = SDL_GetTicks();

				main_update();
				GUIGL::update();

				if (cpu.state == cstate::running)
					main_step();

				//limit fps
				if (SDL::frame_limit)
				{
					if ((1000 / fps) > (SDL_GetTicks() - start_time))
						SDL_Delay((1000 / fps) - (SDL_GetTicks() - start_time));
				}
			}

			//Save imgui.ini
			//fs::path assets(fs::current_path().parent_path().parent_path()
			//	.parent_path().parent_path() / io.IniFilename);
			//ImGui::SaveIniSettingsToDisk(assets.generic_u8string().c_str());

			MEM::save_sram();
		}

		APU::clean();
		MEM::clean();
		//SDL::clean();
	}

	if (logging)
		create_close_log(false);

	return 0;
}

void main_update()
{
	SDL_Event ev;
	while (SDL_PollEvent(&ev))
	{
		ImGui_ImplSDL2_ProcessEvent(&ev);
		if (ev.type == SDL_QUIT)
			GUIGL::running = 0;
		if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_CLOSE)
		{
			if (ev.window.windowID == SDL_GetWindowID(SDL::window))
				GUIGL::running = 0;
			if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				SDL_SetWindowSize(SDL::window, ev.window.data1, ev.window.data2);
			}

		}
	}

	SDL::input_new();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_TAB)) //frame limit
	{
		SDL::frame_limit = false;
	}
	else if (ImGui::IsKeyReleased(SDL_SCANCODE_TAB))
	{
		SDL::frame_limit = true;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F5)) //run
	{
		if (logging)
			log_to_file(reg.pc);

		CPU::step();
		cpu.state = cstate::running;
	}

	if (newkeys.lshift && newkeys.f1 && !oldkeys.f1)
	{
		main_save_state(0);
	}
	else if (newkeys.f1 && !oldkeys.f1)
	{
		main_load_state(0);
	}
//	else if (newkeys.f2 && !oldkeys.f2) //take screenshot
//	{
//#if SDL_BYTEORDER == SDL_BIG_ENDIAN
//		Uint32 rmask = 0xff000000;
//		Uint32 gmask = 0x00ff0000;
//		Uint32 bmask = 0x0000ff00;
//		Uint32 amask = 0x000000ff;
//#else
//		Uint32 rmask = 0x000000ff;
//		Uint32 gmask = 0x0000ff00;
//		Uint32 bmask = 0x00ff0000;
//		Uint32 amask = 0xff000000;
//#endif
//
//		string screenshot(header.name + ".bmp");
//		fs::path screenshots(fs::current_path().parent_path().parent_path()
//			.parent_path() / screenshot);
//		time_t now = time(0);
//		char* tmp = ctime(&now);
//		int w, h;
//
//		SDL_GetRendererOutputSize(SDL::renderer, &w, &h);
//		w = 512;
//		h = 480;
//		SDL_Surface* screen = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, rmask, gmask, bmask, amask);
//		glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, screen->pixels);
//
//		vector<u32> bytes(screen->w * screen->h * 4);
//		u32* data = (u32*)screen->pixels;
//
//		//flip screenshot vertically
//		int j = 0;
//		for (int y = h; y > 0; y--)
//		{
//			for (int x = 0; x < w; x++)
//			{
//				bytes[j++] = data[y * w + x];
//			}
//		}
//
//		memcpy(screen->pixels, bytes.data(), bytes.size());
//
//		SDL_SaveBMP(screen, screenshot.c_str());
//		SDL_FreeSurface(screen);
//	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F6)) //run one ppu cycle
	{
		PPU::step();
		cpu.state = cstate::cycles;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7)) //run one scanline
		main_step_scanline(1);

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F8)) //run one frame
		main_step_frame();

	if (newkeys.f9 && !oldkeys.f9) //run 240 scanlines
		main_step_scanline(240);

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F10)) //step over
		main_step_over();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F11)) //step into
	{
		if (logging)
			log_to_file(reg.pc);

		CPU::step();
		GUIGL::follow_pc = true;
		cpu.state = cstate::debugging;
	}

	SDL::input_old();
}

void main_step()
{
	if (!MEM::rom_loaded)
		return;

	GUIGL::follow_pc = true;
	PPU::frame_ready = false;
	cpu.cpucycles = FRAME_CYCLES;
	while (!PPU::frame_ready)
	{
		u16 pc = reg.pc;
		if (cpu.stepoveraddr == -1)
		{
			for (auto& it : breakpoints)
			{
				if (!it.enabled)
					continue;
				if (bp_exec_access(reg.pc))
				{
					cpu.state = cstate::debugging;
					return;
				}
				else if (bp_read_access(MEM::read_addr))
				{
					MEM::read_addr = -1;
					cpu.state = cstate::debugging;
					return;
				}
				else if (bp_write_access(MEM::write_addr))
				{
					MEM::write_addr = -1;
					cpu.state = cstate::debugging;
					return;
				}
				else if (bp_ppu_write_access(MEM::ppu_write_addr))
				{
					MEM::ppu_write_addr = -1;
					cpu.state = cstate::debugging;
					return;
				}
			}
		}

		if (cpu.state == cstate::crashed)
			return;

		if (logging)
			log_to_file(pc);

		CPU::step();

		if ((u16)cpu.stepoveraddr == reg.pc)
		{
			cpu.state = cstate::debugging;
			cpu.stepoveraddr = -1;
			return;
		}
	}

	SDL_GL_SetSwapInterval(0);
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
		GUIGL::follow_pc = true;
		cpu.state = cstate::debugging;
	}
}

void main_step_frame()
{
	if (!MEM::rom_loaded)
		return;

	u16 pc = reg.pc;
	cpu.state = cstate::frame;
	if (logging)
		log_to_file(pc);

	int old_frame = PPU::frame;
	while (old_frame == PPU::frame)
	{
		CPU::step();
		//ppu_step(cyc);
		if (cpu.state == cstate::crashed)
			return;
	}
}

void main_step_scanline(int lines)
{
	if (!MEM::rom_loaded)
		return;

	u16 pc = reg.pc;
	cpu.state = cstate::scanlines;

	if (lines == 1)
	{
		int old_scanline = PPU::scanline;
		while (old_scanline == PPU::scanline)
		{
			if (logging)
				log_to_file(pc);

			CPU::step();
			if (cpu.state == cstate::crashed)
				return;
		}
	}
	else if (lines != 1)
	{
		int old_scanline = PPU::scanline;
		while (old_scanline == PPU::scanline)
		{
			if (logging)
				log_to_file(pc);

			CPU::step();
			if (cpu.state == cstate::crashed)
				return;
		}

		while (PPU::scanline != lines)
		{
			if (logging)
				log_to_file(pc);

			CPU::step();
			if (cpu.state == cstate::crashed)
				return;
		}
	}
	else
	{
		int old_scanline = PPU::scanline;
		while (old_scanline == PPU::scanline)
		{
			if (logging)
				log_to_file(pc);

			CPU::step();
			if (cpu.state == cstate::crashed)
				return;
		}
	}
	//SDL::draw_frame(PPU::screen_pixels, cstate::scanlines);
}

void main_save_state(u8 slot)
{
	if (MEM::rom_loaded)
	{
		ofstream state(header.name + "." + to_string(slot), ios::binary);
		state.write((char*)MEM::ram.data(), MEM::ram.size());
		state.write((char*)MEM::vram.data(), MEM::vram.size());
		//if (header.mappernum == 4)
		//	state.write((char*)&mmc3, sizeof(mmc3));
		state.close();
	}
}

void main_load_state(u8 slot)
{
	if (MEM::rom_loaded)
	{
		string file = header.name + "." + to_string(slot);
		if (fs::exists(file))
		{
			ifstream state(file, ios::binary);
			state.read((char*)MEM::ram.data(), MEM::ram.size());
			state.read((char*)MEM::vram.data(), MEM::vram.size());
			//if (header.mappernum == 4)
			//	state.read((char*)&mmc3, sizeof(mmc3));
			state.close();
			cpu.state_loaded = 1;
		}
	}
}

void set_spacing(int count)
{
	for (int i = 0; i < count; i++)
		ImGui::Spacing();
}

