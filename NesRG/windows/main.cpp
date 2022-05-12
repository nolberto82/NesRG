#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "gui.h"
#include "sdlgfx.h"
#include "tracer.h"
#include "breakpoints.h"
#include "main.h"
#include "mappers.h"
#include "mem.h"
#include "types.h"

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
	GUI::running = 1;

	if (SDL::init())
	{
		if (GUI::init())
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
							string cmapping = SDL_GameControllerMapping(SDL::controller);
							SDL_GameControllerAddMapping(cmapping.c_str());
							break;
						}
						else
						{
							printf("Unable to open game controller! SDL Error: %s\n", SDL_GetError());
							GUI::running = 0;
						}
					}
				}

				MEM::init();
				CPU::init();
			}
		}

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags = ImGuiConfigFlags_DockingEnable;
		io.IniFilename = "assets\\imgui.ini";

		while (GUI::running)
		{
			main_update();
			GUI::update(io);
			if (cpu.state == cstate::running)
				main_step();
		}

		//Save imgui.ini
		fs::path assets(fs::current_path().parent_path().parent_path()
			.parent_path().parent_path() / io.IniFilename);
		ImGui::SaveIniSettingsToDisk(assets.generic_u8string().c_str());

		SDL::clean();
		APU::clean();
	}

	if (logging)
		create_close_log(false);

	return 0;
}

void main_update()
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT)
			GUI::running = 0;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
			event.window.windowID == SDL_GetWindowID(SDL::window))
			GUI::running = 0;
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

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F6)) //run one ppu cycle
	{
		PPU::step();
		cpu.state = cstate::cycles;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7)) //run one scanline
		main_step_scanline(0);

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
		GUI::follow_pc = true;
		cpu.state = cstate::debugging;
	}

	SDL::input_old();
}

void main_step()
{
	GUI::follow_pc = true;
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

	APU::step();
}

void main_step_over()
{
	u8 op = MEM::ram[reg.pc];
	if (op == 0x00 || op == 0x20)
	{
		cpu.stepoveraddr = reg.pc + disasm[op].size;
		cpu.state = cstate::running;
	}
	else
	{
		CPU::step();
		GUI::follow_pc = true;
		cpu.state = cstate::debugging;
	}
}

void main_step_frame()
{
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

void main_step_scanline(u16 lines)
{
	if (!MEM::rom_loaded)
		return;

	u16 pc = reg.pc;
	cpu.state = cstate::scanlines;
	if (logging)
		log_to_file(pc);

	if (lines > 0)
	{
		int old_scanline = PPU::scanline;
		while (old_scanline == PPU::scanline)
		{
			CPU::step();
			if (cpu.state == cstate::crashed)
				return;
		}

		while (PPU::scanline != lines)
		{
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
			CPU::step();
			if (cpu.state == cstate::crashed)
				return;
		}
	}

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

