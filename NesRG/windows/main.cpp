#include "cpu.h"
#include "ppu.h"
#include "gui.h"
#include "renderer.h"
#include "tracer.h"
#include "breakpoints.h"
#include "main.h"

Cpu cpu;
Ppu ppu;
Renderer gfx;
Registers reg;
PpuRegisters lp;
Header header;
PpuCtrl pctrl;
PpuMask pmask;
PpuStatus pstatus;

int main(int argc, char* argv[])
{
	if (render_init())
	{
		IMGUI_CHECKVERSION();
		if (ImGui::CreateContext())
		{
			ImGui_ImplSDL2_InitForSDLRenderer(gfx.window, gfx.renderer);
			ImGui_ImplSDLRenderer_Init(gfx.renderer);

			ImGui::StyleColorsLight();
			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4 windowbgcol = ImVec4(0, 0, 0, 180 / 255.0f);
			style->ItemSpacing = ImVec2(8, 1);
			style->FrameBorderSize = 1.0f;

			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags = ImGuiConfigFlags_DockingEnable;

			io.IniFilename = "./assets/imgui.ini";

			mem_init();
			cpu_init();

			gui_running = 1;

			while (gui_running)
			{
				main_update();
				gui_update();
				if (cpu.state == cstate::running)
					main_step();
			}
		}
	}

	if (logging)
		create_close_log(false);

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	render_clean();

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
			event.window.windowID == SDL_GetWindowID(gfx.window))
			gui_running = 0;
	}

	render_input();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_TAB)) //frame limit
		gfx.frame_limit = false;
	else if (ImGui::IsKeyReleased(SDL_SCANCODE_TAB))
		gfx.frame_limit = true;

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F5)) //run
	{
		if (logging)
			log_to_file(reg.pc);

		int cyc = cpu_step();
		ppu_step(cyc);
		cpu.state = cstate::running;
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
}

void main_step()
{
	follow_pc = true;
	ppu.frame_ready = false;
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

		int cyc = cpu_step();
		if (cpu.state == cstate::crashed)
			return;
		ppu_step(cyc);
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

void set_spacing(int count)
{
	for (int i = 0; i < count; i++)
		ImGui::Spacing();
}
