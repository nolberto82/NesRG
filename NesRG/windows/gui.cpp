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
	ImGui_ImplSDL2_InitForSDLRenderer(window);
	ImGui_ImplSDLRenderer_Init(renderer);

	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	ImGuiStyle* style = &ImGui::GetStyle();

	ImVec4 windowbgcol = ImVec4(0, 0, 0, 180 / 255.0f);

	//style->Colors[ImGuiCol_WindowBg] = windowbgcol;
	style->ItemSpacing = ImVec2(8, 1);
	style->FrameBorderSize = 1.0f;

	breakpoints.resize(BREAKPOINT_MAX);

	return true;
}

void gui_update()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	gui_running = 1;

	while (gui_running)
	{
		ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		if (cpu.state == cstate::running)
			gui_step(false);

		gui_input(io);

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
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
					ImGui::Image((void*)screen, ImGui::GetContentRegionAvail());
					ImGui::EndTabItem();
				}

				if ((ImGui::BeginTabItem("Nametables")))
				{
					int x = (lp.t & 0x1f);
					int y = (lp.t & 0x3e0) >> 5;

					for (int i = 0; i < 4; i++)
					{
						for (int a = 0; a < 0x400; a++)
							process_nametables(a, i, ppu.ntable_pixels[i]);

						render_nttable(ppu.ntable_pixels[i], i, x, y);

						if (i % 2 == 1)
						{
							ImGui::SameLine();
						}

						ImGui::Image((void*)ntscreen[i], ImVec2(256, 240));
					}

					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
		}
		ImGui::End();

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255), (u8)(clear_color.z * 255), (u8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		if (frame_limit)
			SDL_framerateDelay(&fpsman);
		SDL_RenderPresent(renderer);
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
			{
				show_style_editor = !show_style_editor;
			}

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
			{
				gui_step(true);
				cpu.state = cstate::running;
			}

			if (ImGui::MenuItem("Run 1 Cycle (F6)"))
			{
				cpu.state = cstate::cycles;
				ppu_step(1);
			}

			if (ImGui::MenuItem("Run 1 Scanline (F7)"))
			{
				cpu.state = cstate::scanlines;
				gui_step(true);
			}

			if (ImGui::MenuItem("Step Over (F10)"))
			{
				cpu.state = cstate::debugging;
				gui_step_over();
			}

			if (ImGui::MenuItem("Step Into (F11)"))
			{
				cpu.state = cstate::debugging;
				gui_step(true);
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void gui_step(bool stepping, bool over)
{
	ppu.frame_ready = false;

	if (cpu.state == cstate::running)
	{
		while (!ppu.frame_ready)
		{
			u16 pc = reg.pc;

			for (auto& it : breakpoints)
			{
				if (!it.enabled)
					continue;

				if (it.type == bp_exec && it.enabled)
				{
					if (bp_check(pc, bp_exec, it.enabled))
					{
						cpu.state = cstate::debugging;
						return;
					}
				}

				if (it.enabled && it.type > 0)
				{
					if (bp_check_access(read_addr, bp_read, it.enabled))
					{
						cpu.state = cstate::debugging;
						read_addr = -1;
						lineoffset = -9;
						return;
					}

					if (bp_check_access(write_addr, bp_write, it.enabled))
					{
						cpu.state = cstate::debugging;
						write_addr = -1;
						lineoffset = -9;
						return;
					}

					if (ppu_read_addr == 0x27dc)
					{
						int yu = 0;
					}

					if (bp_check_access(ppu_read_addr, bp_vread, it.enabled))
					{
						cpu.state = cstate::debugging;
						ppu_read_addr = -1;
						lineoffset = -9;
						return;
					}

					if (bp_check_access(ppu_write_addr, bp_vwrite, it.enabled))
					{
						cpu.state = cstate::debugging;
						ppu_write_addr = -1;
						lineoffset = -9;
						return;
					}
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
}

void gui_step_over()
{
	u8 op = rbd(reg.pc);
	u16 ret_pc = reg.pc + 3;

	lineoffset = 0;

	cpu.state = cstate::debugging;

	if (op == 0x20)
	{
		while (reg.pc != ret_pc)
		{
			u16 oldpc = reg.pc;
			gui_step(true);
			if (oldpc == reg.pc)
				break;
		}
	}
	else
		gui_step(true);
}

void gui_input(ImGuiIO io)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT)
			gui_running = 0;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
			gui_running = 0;
	}

	render_input();

	if (ImGui::IsKeyPressed(SDL_SCANCODE_TAB)) //frame limit
	{
		frame_limit = false;
	}
	else if (ImGui::IsKeyReleased(SDL_SCANCODE_TAB))
	{
		frame_limit = true;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F5)) //run one scanline
	{
		gui_step(true);
		cpu.state = cstate::running;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7)) //run one scanline
	{
		cpu.state = cstate::scanlines;
		gui_step(true);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F8)) //run one ppu cycle
	{
		cpu.state = cstate::cycles;
		ppu_step(1);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F10)) //step over
	{
		cpu.state = cstate::debugging;
		gui_step_over();
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F11)) //step into
	{
		cpu.state = cstate::debugging;
		gui_step(true);
	}
}

void gui_create_button(bplist it, const char* text, u8 n, u16 inputaddr, u8 bptype)
{
	ImGui::PushStyleColor(ImGuiCol_Button, it.type & bptype ? GREEN : RED);
	if (ImGui::Button(text, ImVec2(BUTTON_W - 62, 0)))
	{
		bp_edit(inputaddr, it.type ^= bptype, n, it.enabled ^= 1);
	}
	ImGui::PopStyleColor();
}

void gui_show_table_text(const char* format, const char* text)
{
	ImGui::TableNextColumn(); ImGui::Text(format, text);
}

void gui_show_table_number(const char* format, int value)
{
	ImGui::TableNextColumn(); ImGui::Text(format, value);
}

void gui_clean()
{
	if (logging)
		create_close_log(false);

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}