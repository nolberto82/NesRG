﻿#include "cpu.h"
#include "ppu.h"
#include "gui.h"
#include "sdlcore.h"
#include "tracer.h"
#include "breakpoints.h"
#include "main.h"
#include "mappers.h"

Cpu cpu;
Ppu ppu;
SdlCore sdl;
Registers reg;
PpuRegisters lp;
Header header;
PpuCtrl pctrl;
PpuMask pmask;
PpuStatus pstatus;
Keys newkeys, oldkeys;

int main(int argc, char* argv[])
{
	if (sdl_init())
	{
		IMGUI_CHECKVERSION();
		if (ImGui::CreateContext())
		{
			ImGui_ImplSDL2_InitForSDLRenderer(sdl.window, sdl.renderer);
			ImGui_ImplSDLRenderer_Init(sdl.renderer);

			ImGui::StyleColorsLight();
			ImGui::StyleColorsDark();
			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4 windowbgcol = ImVec4(0, 0, 0, 180 / 255.0f);
			style->ItemSpacing = ImVec2(8, 1);
			style->FrameBorderSize = 1.0f;

			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags = ImGuiConfigFlags_DockingEnable;

			io.IniFilename = "assets\\imgui.ini";

			mem_init();
			cpu_init();

			gui_running = 1;

			for (int i = 0; i < SDL_NumJoysticks(); i++)
			{
				// Load joystick
				if (SDL_IsGameController(i))
				{
					sdl.controller = SDL_GameControllerOpen(i);
					if (sdl.controller)
					{
						string cmapping = SDL_GameControllerMapping(sdl.controller);
						SDL_GameControllerAddMapping(cmapping.c_str());
						break;
					}
					else
					{
						printf("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
						gui_running = 0;
					}
				}
			}

			while (gui_running)
			{
				main_update();
				gui_update();
				if (cpu.state == cstate::running)
					main_step();
			}

			//Save imgui.ini
			fs::path assets(fs::current_path().parent_path().parent_path()
				.parent_path().parent_path() / io.IniFilename);
			ImGui::SaveIniSettingsToDisk(assets.generic_u8string().c_str());

			ImGui_ImplSDLRenderer_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
		}
		sdl_clean();
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
			gui_running = 0;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
			event.window.windowID == SDL_GetWindowID(sdl.window))
			gui_running = 0;
	}

	sdl_input_new();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_TAB)) //frame limit
		sdl.frame_limit = false;
	else if (ImGui::IsKeyReleased(SDL_SCANCODE_TAB))
		sdl.frame_limit = true;

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F5)) //run
	{
		if (logging)
			log_to_file(reg.pc);

		int cyc = cpu_step();
		ppu_step(cyc);
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
		ppu_step(1);
		cpu.state = cstate::cycles;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7)) //run one scanline
		main_step_scanline();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F8)) //run one frame
		main_step_frame();

	//if (ImGui::IsKeyDown(SDL_SCANCODE_F9)) //step into fast
	//	gui_step(true, cstate::debugging);

	//if (ImGui::IsKeyPressed(SDL_SCANCODE_F10)) //step over
	//	debug_step_over();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F11)) //step into
	{
		if (logging)
			log_to_file(reg.pc);

		int cyc = cpu_step();
		ppu_step(cyc);
		follow_pc = true;
		cpu.state = cstate::debugging;
	}

	sdl_input_old();
}

void main_step()
{
	follow_pc = true;
	ppu.frame_ready = false;
	int cyc = 0;
	while (!ppu.frame_ready)
	{
		u16 pc = reg.pc;
		for (auto& it : breakpoints)
		{
			if (!it.enabled)
				continue;
			if (bp_exec_access(reg.pc))
			{
				cpu.state = cstate::debugging;
				return;
			}
			else if (bp_read_access(read_addr))
			{
				read_addr = -1;
				cpu.state = cstate::debugging;
				return;
			}
			else if (bp_write_access(write_addr))
			{
				write_addr = -1;
				cpu.state = cstate::debugging;
				return;
			}
		}
		if (logging)
			log_to_file(pc);

		if (cpu.state == cstate::crashed)
			return;

		ppu_step(cpu_step());
	}
}

void main_step_frame()
{
	u16 pc = reg.pc;
	cpu.state = cstate::frame;
	if (logging)
		log_to_file(pc);

	int old_frame = ppu.frame;
	while (old_frame == ppu.frame)
	{
		int cyc = cpu_step();
		ppu_step(cyc);
		if (cpu.state == cstate::crashed)
			return;
	}
}

void main_step_scanline()
{
	u16 pc = reg.pc;
	cpu.state = cstate::scanlines;
	if (logging)
		log_to_file(pc);

	int old_scanline = ppu.scanline;
	while (old_scanline == ppu.scanline)
	{
		int cyc = cpu_step();
		ppu_step(cyc);
		if (cpu.state == cstate::crashed)
			return;
	}
}


void main_save_state(u8 slot)
{
	if (rom_loaded)
	{
		ofstream state(header.name + "." + to_string(slot), ios::binary);
		state.write((char*)ram.data(), ram.size());
		state.write((char*)vram.data(), vram.size());
		if (header.mappernum==4)
			state.write((char*)&mmc4, sizeof(mmc4));
		state.close();
	}
}

void main_load_state(u8 slot)
{
	if (rom_loaded)
	{
		string file = header.name + "." + to_string(slot);
		if (fs::exists(file))
		{
			ifstream state(file, ios::binary);
			state.read((char*)ram.data(), ram.size());
			state.read((char*)vram.data(), vram.size());
			if (header.mappernum == 4)
				state.read((char*)&mmc4, sizeof(mmc4));
			state.close();
		}
	}
}

void set_spacing(int count)
{
	for (int i = 0; i < count; i++)
		ImGui::Spacing();
}

