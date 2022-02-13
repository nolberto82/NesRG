#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "renderer.h"
#include "tracer.h"
#include "debugwindow.h"

extern void gui_create_button(bplist it, const char* text, u8 n, u16 inputaddr, u8 bptype);

bool gui_init()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(gfx.window);
	ImGui_ImplSDLRenderer_Init(gfx.renderer);

	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	ImGuiStyle* style = &ImGui::GetStyle();

	ImVec4 windowbgcol = ImVec4(0, 0, 0, 180 / 255.0f);

	//style->Colors[ImGuiCol_WindowBg] = windowbgcol;
	style->ItemSpacing = ImVec2(8, 1);
	style->FrameBorderSize = 1.0f;

	//breakpoints.resize(BREAKPOINT_MAX);

	return true;
}

void gui_update()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	gui_running = 1;

	while (gui_running)
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		gui_step(false, cpu.state);

		gui_input(io);

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame(gfx.window);
		ImGui::NewFrame();

		//Debugger Windows
		debug_update();

		//Menu
		gui_show_menu();

		if (ImGui::Begin("Graphics", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetWindowSize(ImVec2(MEM_W, DEBUG_H));
			ImGui::SetWindowPos(ImVec2(DEBUG_X + DEBUG_W + 5, DEBUG_Y));

			if (cpu.state == cstate::scanlines || cpu.state == cstate::cycles)
				render_frame(ppu.screen_pixels);

			if (ImGui::BeginTabBar("##gfx_tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Display"))
				{
					ImGui::Image((void*)gfx.screen, ImGui::GetContentRegionAvail());
					ImGui::EndTabItem();
				}

				if ((ImGui::BeginTabItem("Nametables")))
				{
					int x = 0, y = 0;

					for (int i = 0; i < 4; i++)
					{
						memset(ppu.ntable_pixels[i], 0x00, sizeof(ppu.ntable_pixels[i]));
						process_nametables(i * 0x400, 0, ppu.ntable_pixels[i]);
					}

					render_nttable();

					ImGui::Image((void*)gfx.ntscreen, ImGui::GetContentRegionAvail());
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(gfx.renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255), (u8)(clear_color.z * 255), (u8)(clear_color.w * 255));
		SDL_RenderClear(gfx.renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		if (gfx.frame_limit)
			SDL_framerateDelay(&gfx.fpsman);
		SDL_RenderPresent(gfx.renderer);
	}
}

void gui_show_menu()
{
	static bool show_style_editor = false;

	if (show_style_editor)
		ImGui::ShowStyleEditor();

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Show Styles Editor"))
				show_style_editor = !show_style_editor;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			if (ImGui::MenuItem("Dump VRAM"))
			{
				std::ofstream outFile("vram.bin", std::ios::binary);
				outFile.write((char*)vram.data(), VRAMSIZE);
				outFile.close();
			}

			if (ImGui::MenuItem("Run (F5)"))
				gui_step(true, cstate::running);

			if (ImGui::MenuItem("Run 1 Cycle (F6)"))
				gui_step(true, cstate::cycles);

			if (ImGui::MenuItem("Run 1 Scanline (F7)"))
				gui_step(true, cstate::scanlines);

			if (ImGui::MenuItem("Step Over (F10)"))
				gui_step_over();

			if (ImGui::MenuItem("Step Into (F11)"))
				gui_step(true, cstate::debugging);

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void gui_step(bool stepping, int state)
{
	ppu.frame_ready = false;
	cpu.state = state;

	if (cpu.state == cstate::running)
	{
		while (!ppu.frame_ready)
		{
			u16 pc = reg.pc;

			for (auto& it : breakpoints)
			{
				if (!it.enabled)
					continue;

				if (bp_check(pc, it.type) || bp_check(read_addr, it.type) || bp_check(write_addr, it.type))
				{
					if (it.addr == read_addr)
						read_addr = -1;
					if (it.addr == write_addr)
						write_addr = -1;

					cpu.state = cstate::debugging;
					return;
				}
			}

			if (logging)
				log_to_file(pc);

			int cyc = cpu_step();
			ppu_step(cyc);
			ppu.totalcycles += cyc / 3;

			if (cpu.state == cstate::crashed)
				return;
		}
	}
	else if (cpu.state == cstate::debugging && stepping)
	{
		if (logging)
			log_to_file(reg.pc);

		int cyc = cpu_step();
		ppu_step(cyc);
		ppu.totalcycles += cyc / 3;
	}
	else if (cpu.state == cstate::scanlines && stepping)
	{
		u16 pc = reg.pc;

		if (logging)
			log_to_file(pc);

		int old_scanline = ppu.scanline;

		while (old_scanline == ppu.scanline)
		{
			int cyc = cpu_step();
			ppu_step(cyc);
			ppu.totalcycles += cyc / 3;
		}
	}
	else if (cpu.state == cstate::cycles && stepping)
		ppu_step(1);
}

void gui_step_over()
{
	u8 op = rbd(reg.pc);
	u16 ret_pc = reg.pc + 3;
	lineoffset = 0;

	if (op == 0x20)
	{
		while (reg.pc != ret_pc)
		{
			u16 oldpc = reg.pc;
			gui_step(true, cstate::running);
			if (oldpc == reg.pc)
				break;
		}
	}
	else
		gui_step(true, cstate::debugging);
}

void gui_input(ImGuiIO io)
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

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F5)) //run one scanline
		gui_step(true, cstate::running);

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7)) //run one scanline
		gui_step(true, cstate::scanlines);

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F8)) //run one ppu cycle
		gui_step(true, cstate::cycles);

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F10)) //step over
		gui_step_over();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F11)) //step into
		gui_step(true, cstate::debugging);
}

void gui_create_button(bplist it, const char* text, u8 n, u16 inputaddr, u8 bptype)
{
	ImGui::PushStyleColor(ImGuiCol_Button, it.type & bptype ? GREEN : RED);
	if (ImGui::Button(text, ImVec2(BUTTON_W - 62, 0)))
		bp_edit(inputaddr, it.type ^= bptype, n, it.enabled ^= 1);
	ImGui::PopStyleColor();
}

void gui_clean()
{
	if (logging)
		create_close_log(false);

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}