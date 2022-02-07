#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "renderer.h"
#include "tracer.h"

ImGui::FileBrowser fileDialog;

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
		gui_step(false);
		gui_input(io);

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame(window);
		ImGui::NewFrame();

		//Menu
		gui_show_menu();

		//ImGuiCond_Once

		//Debugger UI
		if (ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImGui::SetWindowSize(ImVec2(DEBUG_W, DEBUG_H));
			ImGui::SetWindowPos(ImVec2(DEBUG_X, DEBUG_Y));

			ImGui::BeginChild("Buttons", ImVec2(-1, 45), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			gui_show_buttons(io);
			ImGui::EndChild();

			ImGui::Separator();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Columns(2, "##disasm", false);

			ImGui::SetColumnWidth(0, 300);

			//Show Debugger
			ImGui::BeginChild("Disassembly", ImVec2(0, 400), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			gui_show_disassembly(io);
			ImGui::EndChild();

			ImGui::SameLine();
			ImGui::NextColumn();

			//Show Registers
			ImGui::BeginChild("Registers");
			gui_show_registers(io);
			ImGui::EndChild();

			ImGui::Columns(1);
		}
		ImGui::End();

		if (ImGui::Begin("Breakpoints", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetWindowSize(ImVec2(DEBUG_W, MEM_H));
			ImGui::SetWindowPos(ImVec2(DEBUG_X, DEBUG_H + DEBUG_Y + 5));

			gui_show_breakpoints();
		}
		ImGui::End();

		if (ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetWindowSize(ImVec2(MEM_W, DEBUG_H));
			ImGui::SetWindowPos(ImVec2(DEBUG_X + DEBUG_W + 5, DEBUG_Y));

			if (cpu_state == cstate::scanlines || cpu_state == cstate::cycles)
				render_frame(screen_pixels);

			ImGui::Image((void*)screen, ImGui::GetContentRegionAvail());
		}
		ImGui::End();

		if (ImGui::Begin("Memory Editor", nullptr))
		{
			ImGui::SetWindowSize(ImVec2(MEM_W, MEM_H));
			ImGui::SetWindowPos(ImVec2(DEBUG_X + DEBUG_W + 5, DEBUG_Y + DEBUG_H + 5));

			gui_show_memory();
		}
		ImGui::End();

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255), (u8)(clear_color.z * 255), (u8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_framerateDelay(&fpsman);
		SDL_RenderPresent(renderer);
	}
}

void gui_show_disassembly(ImGuiIO io)
{
	char text[TEXTSIZE] = { 0 };
	static int item_num = 0;
	u16 pc = is_jump ? inputaddr : reg.pc;
	vector<string> vdasm;

	if (io.MouseWheel != 0 && (pc + lineoffset < 0x10000))
	{
		if (ImGui::IsWindowHovered())
		{
			if (io.MouseWheel > 0)
				lineoffset -= 3;
			else
				lineoffset += 3;
		}
	}

	if (lineoffset == 0)
		ImGui::SetScrollHereY(0.5f);

	//ImGui::SameLine(-5);

	for (int i = 0; i < 19; i++)
	{
		int pcaddr = pc + lineoffset;
		vector<disasmentry> vdentry = get_trace_line(text, pcaddr, false, true);

		bool bpcheck = false;

		for (auto& it : breakpoints)
		{
			if (it.addr == pcaddr)
			{
				bpcheck = true;
				break;
			}
		}

		char pctext[5] = "";
		snprintf(pctext, sizeof(pctext), "%04X", vdentry[0].offset);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(bpcheck ? GREEN : LIGHTGRAY));
		ImGui::PushID(pcaddr);

		if (ImGui::Button(pctext, ImVec2(40, 19)))
		{
			//bp_edit(pcaddr, bp_exec);
		}

		ImGui::PopID();
		ImGui::PopStyleColor(1);

		ImGui::SameLine();

		ImVec4 currlinecol = ImVec4(0, 0, 0, 1);

		if (pcaddr == reg.pc)
			currlinecol = ImVec4(0, 0, 1, 1);

		ImGui::TextColored(currlinecol, vdentry[0].bytetext.c_str());

		ImGui::SameLine();

		ImGui::TextColored(currlinecol, vdentry[0].dtext.c_str());

		pc += vdentry[0].size;

		if (stepping)
		{
			ImGui::SetScrollHereY(0);
			stepping = false;
		}
	}
}

void gui_show_buttons(ImGuiIO io)
{
	fileDialog.Display();
	fileDialog.SetWindowSize(500, 800);

	if (fileDialog.HasSelected())
	{
		ppu_reset();
		clear_pixels();
		load_rom(fileDialog.GetSelected().u8string().c_str());
		cpu_reset();
		cpu_state = cstate::debugging;
		fileDialog.ClearSelected();
	}

	if (ImGui::Button("Load Rom", ImVec2(BUTTON_W, 0)))
	{
		cpu_state = cstate::debugging;
		fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Mapper0Games\\";
		fileDialog.SetTitle("Load Nes Rom");
		fileDialog.SetTypeFilters({ ".nes" });
		fileDialog.SetPwd(game_dir);
		fileDialog.Open();
	}

	ImGui::SameLine();

	if (ImGui::Button("Run", ImVec2(BUTTON_W, 0)))
	{
		if (rom_loaded)
		{
			gui_step(true);
			cpu_state = cstate::running;
			is_jump = false;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(BUTTON_W, 0)))
	{
		if (rom_loaded)
		{
			create_close_log(false);
			cpu_state = cstate::debugging;
			lineoffset = 0;
			ppu_reset();
			set_mapper();
			cpu_reset();
			is_jump = false;
			clear_pixels();
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Step Into", ImVec2(BUTTON_W, 0)))
	{
		if (rom_loaded)
		{
			cpu_state = cstate::debugging;
			lineoffset = 0;
			gui_step(true);
			is_jump = false;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Step Over", ImVec2(BUTTON_W, 0)))
	{
		if (rom_loaded)
		{
			lineoffset = 0;

			gui_step_over();

			is_jump = false;
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(logging ? GREEN : RED));
	if (ImGui::Button("Trace Log", ImVec2(BUTTON_W, 0)))
	{
		create_close_log(!logging);
	}
	ImGui::PopStyleColor(1);

	ImGui::Spacing();
	ImGui::Spacing();
}

void gui_show_memory()
{
	static MemoryEditor mem_edit;
	//mem_edit.OptGreyOutZeroes = false;

	//for (int i = 0; i < 10; i++)
	ImGui::Spacing();

	ImGui::Separator();

	ImGui::Text("Memory Editor");

	if (ImGui::BeginTabBar("##mem_tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("RAM"))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			mem_edit.DrawContents(ram.data(), ram.size());
			ImGui::PopStyleColor(1);
			ImGui::EndTabItem();
		}

		if ((ImGui::BeginTabItem("VRAM")))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			mem_edit.DrawContents(vram.data(), vram.size());
			ImGui::PopStyleColor(1);
			ImGui::EndTabItem();
		}

		if ((ImGui::BeginTabItem("OAM")))
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 1.0f, 1.0f));
			mem_edit.DrawContents(oam.data(), oam.size());
			ImGui::PopStyleColor(1);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}
}

void gui_show_breakpoints()
{
	static u16 bplistaddr = { 0 };
	static bool bpaddchk[5] = { false };
	static char bpaddrtext[BREAKPOINT_MAX][5] = { 0 };
	static int item_id = 0;

	ImGui::BeginGroup();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::SameLine();

	ImGui::EndGroup();

	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Separator();

	if (ImGui::ListBoxHeader("##bps", ImVec2(-1, -1)))
	{
		int n = 0;

		for (auto& it : breakpoints)
		{
			u8 bptype = it.type;

			ImGui::PushID(n);

			ImGui::PushItemWidth(40);

			if (ImGui::InputText("##bpadd", (char*)bpaddrtext[n], IM_ARRAYSIZE((char*)bpaddrtext[n]), INPUT_FLAGS))
			{
				std::istringstream ss(bpaddrtext[n]);
				ss >> std::hex >> inputaddr;
				//is_jump = true;
				lineoffset = 0;
			}
			ImGui::PopItemWidth();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_read ? GREEN : RED);
			if (ImGui::Button("CR", ImVec2(BUTTON_W - 90, 0)))
			{
				bp_edit(inputaddr, it.type ^= bp_read, n, it.enabled ^= 1);
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_write ? GREEN : RED);
			if (ImGui::Button("CW", ImVec2(BUTTON_W - 90, 0)))
			{
				bp_edit(inputaddr, it.type ^= bp_write, n, it.enabled ^= 1);
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_exec ? GREEN : RED);
			if (ImGui::Button("CX", ImVec2(BUTTON_W - 90, 0)))
			{
				bp_edit(inputaddr, it.type ^= bp_exec, n, it.enabled ^= 1);
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_vread ? GREEN : RED);
			if (ImGui::Button("VR", ImVec2(BUTTON_W - 90, 0)))
			{
				bp_edit(inputaddr, it.type ^= bp_vread, n, it.enabled ^= 1);
			}
			ImGui::PopStyleColor();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_vwrite ? GREEN : RED);
			if (ImGui::Button("VW", ImVec2(BUTTON_W - 90, 0)))
			{
				bp_edit(inputaddr, it.type ^= bp_vwrite, n, it.enabled ^= 1);
			}
			ImGui::PopStyleColor();

			ImGui::PopID();

			n++;
		}

		ImGui::ListBoxFooter();
	}
}

void gui_show_registers(ImGuiIO io)
{
	ImU32 tablecolcolor = 0xffe0e0e0;

	if (ImGui::BeginTable("Regs", 2, ImGuiTableFlags_None, ImVec2(-1, -1)))
	{
		char flags[7] = { "......" };
		char text[32] = "";

		flags[5] = reg.ps & FC ? 'C' : '.';
		flags[4] = reg.ps & FZ ? 'Z' : '.';
		flags[3] = reg.ps & FI ? 'I' : '.';
		flags[2] = reg.ps & FD ? 'D' : '.';
		flags[1] = reg.ps & FV ? 'V' : '.';
		flags[0] = reg.ps & FN ? 'N' : '.';

		ImGui::TableSetupColumn("regnames", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("regvalues", ImGuiTableColumnFlags_WidthFixed, 80);

		ImGui::TableNextColumn(); ImGui::Text("FPS");
		ImGui::TableNextColumn(); ImGui::Text("%f", io.Framerate);

		ImGui::TableNextColumn(); ImGui::Text("cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", totalcycles);

		ImGui::TableNextColumn(); ImGui::Text("ppu cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", cycle);

		ImGui::TableNextColumn(); ImGui::Text("scanline");
		ImGui::TableNextColumn(); ImGui::Text("%d", scanline);

		ImGui::TableNextColumn(); ImGui::Text("ppuaddr");
		ImGui::TableNextColumn(); ImGui::Text("%04X", lp.v);

		ImGui::TableNextColumn(); ImGui::Text("t-addr");
		ImGui::TableNextColumn(); ImGui::Text("%04X", lp.t);

		ImGui::TableNextColumn(); ImGui::Text("flags");
		ImGui::TableNextColumn(); ImGui::Text("%s", flags);

		ImGui::TableNextColumn(); ImGui::Text("------------");
		ImGui::TableNextColumn(); ImGui::Text("-----------");

		ImGui::TableNextColumn(); ImGui::Text("%11s", "PC");
		ImGui::TableNextColumn(); ImGui::Text("%04X", reg.pc);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "A");
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.a);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "X");
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.x);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Y");
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.y);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "P");
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.ps);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "SP");
		ImGui::TableNextColumn(); ImGui::Text("%04X", reg.sp | 0x100);

		ImGui::EndTable();
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

			if (ImGui::MenuItem("Run 1 Cycle (F6)"))
			{
				cpu_state = cstate::cycles;
				ppu_step(1);
			}

			if (ImGui::MenuItem("Run 1 Scanline (F7)"))
			{
				cpu_state = cstate::scanlines;
				gui_step(true);
			}

			if (ImGui::MenuItem("Step Over (F10)"))
			{
				cpu_state = cstate::debugging;
				gui_step_over();
			}

			if (ImGui::MenuItem("Step Into (F11)"))
			{
				cpu_state = cstate::debugging;
				gui_step(true);
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void gui_step(bool stepping, bool over)
{
	frame_ready = false;

	if (cpu_state == cstate::running)
	{
		while (!frame_ready)
		{
			u16 pc = reg.pc;

			for (auto& it : breakpoints)
			{
				if (it.type == bp_exec && it.enabled)
				{
					if (bp_check(pc, bp_exec, it.enabled))
					{
						cpu_state = cstate::debugging;
						return;
					}
				}

				if (it.enabled && it.type > 0)
				{
					if (bp_check_access(read_addr, bp_read, it.enabled))
					{
						cpu_state = cstate::debugging;
						read_addr = -1;
						lineoffset = -9;
						return;
					}

					if (bp_check_access(write_addr, bp_write, it.enabled))
					{
						cpu_state = cstate::debugging;
						write_addr = -1;
						lineoffset = -9;
						return;
					}

					if (bp_check_access(ppu_read_addr, bp_vread, it.enabled))
					{
						cpu_state = cstate::debugging;
						ppu_read_addr = -1;
						lineoffset = -9;
						return;
					}

					if (bp_check_access(ppu_write_addr, bp_vwrite, it.enabled))
					{
						cpu_state = cstate::debugging;
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
			totalcycles += cyc / 3;

			if (cpu_state == cstate::crashed)
				return;
		}
	}
	else if (cpu_state == cstate::debugging && stepping)
	{
		if (logging)
			log_to_file(reg.pc);

		int cyc = cpu_step();
		ppu_step(cyc);
		totalcycles += cyc / 3;
	}
	else if (cpu_state == cstate::scanlines && stepping)
	{
		u16 pc = reg.pc;

		if (logging)
			log_to_file(pc);

		int old_scanline = scanline;

		while (old_scanline == scanline)
		{
			int cyc = cpu_step();
			ppu_step(cyc);
			totalcycles += cyc / 3;
		}
	}
}

void gui_step_over()
{
	u8 op = rbd(reg.pc);
	u16 ret_pc = reg.pc + 3;

	cpu_state = cstate::debugging;

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

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7)) //run one scanline
	{
		cpu_state = cstate::scanlines;
		gui_step(true);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F8)) //run one ppu cycle
	{
		cpu_state = cstate::cycles;
		ppu_step(1);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F10)) //step over
	{
		cpu_state = cstate::debugging;
		gui_step_over();
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F11)) //step into
	{
		cpu_state = cstate::debugging;
		gui_step(true);
	}
}

void gui_clean()
{
	if (logging)
		create_close_log(false);

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}