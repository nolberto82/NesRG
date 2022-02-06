#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "renderer.h"

Breakpoint bpk;
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

			//Show Debugger
			ImGui::BeginChild("Disassembly", ImVec2(0, 400), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			gui_show_disassembly(io);
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Registers", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetWindowSize(ImVec2((DEBUG_W / 2), 300));
			ImGui::SetWindowPos(ImVec2(DEBUG_X, DEBUG_H + DEBUG_Y + 5));

			gui_show_registers(io);
		}
		ImGui::End();

		if (ImGui::Begin("Breakpoints", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetWindowSize(ImVec2((DEBUG_W / 2), 300));
			ImGui::SetWindowPos(ImVec2(DEBUG_X + 200, DEBUG_H + DEBUG_Y + 5));

			gui_show_breakpoints();
		}
		ImGui::End();

		if (ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::SetWindowSize(ImVec2(DEBUG_W + 150, DEBUG_H));
			ImGui::SetWindowPos(ImVec2(DEBUG_X + 400 + 5, DEBUG_Y));

			if (cpu_state == cstate::scanlines || cpu_state == cstate::cycles)
				render_frame(screen_pixels);

			ImGui::Image((void*)screen, ImGui::GetContentRegionAvail());
		}
		ImGui::End();

		if (ImGui::Begin("Memory Editor", nullptr))
		{
			ImGui::SetWindowSize(ImVec2(550, 300));
			ImGui::SetWindowPos(ImVec2(DEBUG_X + 400 + 5, DEBUG_Y + DEBUG_H + 5));

			gui_show_memory();
		}
		ImGui::End();

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255), (u8)(clear_color.z * 255), (u8)(clear_color.w * 255));
		SDL_RenderClear(renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
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

		for (auto& it : bpk.breakpoints)
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
			if (!bpcheck)
				bpk.add(pcaddr, bp_exec);
			else
				bpk.remove(pcaddr);
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

	if (fileDialog.HasSelected())
	{
		ppu_reset();
		clear_pixels();
		load_rom(fileDialog.GetSelected().u8string().c_str());
		cpu_reset();
		cpu_state = cstate::debugging;
		fileDialog.ClearSelected();
	}

	if (ImGui::Button("Load Rom", ImVec2(BUTTONSIZE_X, 0)))
	{
		cpu_state = cstate::debugging;
		fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Mapper0Games\\";
		fileDialog.SetTitle("Load Nes Rom");
		fileDialog.SetTypeFilters({ ".nes" });
		fileDialog.SetPwd(game_dir);
		fileDialog.Open();
	}

	ImGui::SameLine();

	if (ImGui::Button("Run", ImVec2(BUTTONSIZE_X, 0)))
	{
		if (rom_loaded)
		{
			gui_step(true);
			cpu_state = cstate::running;
			is_jump = false;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(BUTTONSIZE_X, 0)))
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

	if (ImGui::Button("Step Into", ImVec2(BUTTONSIZE_X, 0)))
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

	if (ImGui::Button("Step Over", ImVec2(BUTTONSIZE_X, 0)))
	{
		if (rom_loaded)
		{
			cpu_state = cstate::debugging;
			lineoffset = 0;

			u8 op = rbd(reg.pc);
			u16 ret_pc = reg.pc + 3;
			//cpu_state = cstate::running;

			if (op == 0x20)
			{
				while (reg.pc != ret_pc)
				{
					gui_step(true);
				}
			}
			else
				gui_step(true);

			is_jump = false;
		}
	}

	ImGui::SameLine();

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(logging ? GREEN : RED));
	if (ImGui::Button("Trace Log", ImVec2(BUTTONSIZE_X, 0)))
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
		ImGui::EndTabBar();
	}
}

void gui_show_breakpoints()
{
	static u16 bplistaddr = 0;
	static bool bpaddchkbox[3] = { false };
	static char bpaddrtext[5] = { 0 };

	ImGui::Text("Address/GoTo");
	ImGui::SameLine(124);
	ImGui::PushItemWidth(75);
	if (ImGui::InputText("##bpadd", bpaddrtext, IM_ARRAYSIZE(bpaddrtext), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue))
	{
		std::istringstream ss(bpaddrtext);
		ss >> std::hex >> inputaddr;
		is_jump = true;
		lineoffset = 0;
	}

	ImGui::PopItemWidth();

	ImGui::Checkbox("R", &bpaddchkbox[0]);

	ImGui::SameLine(0, 28);

	ImGui::Checkbox("W", &bpaddchkbox[1]);

	ImGui::SameLine(0, 28);

	ImGui::Checkbox("X", &bpaddchkbox[2]);

	if (ImGui::Button("Add", ImVec2(50, 0)))
	{
		u8 bptype = 0;
		for (int i = 0; i < 3; i++)
		{
			if (bpaddchkbox[i])
			{
				bptype |= 1 << i;
			}
		}

		if (strcmp(bpaddrtext, "") != 0)
		{
			bplistaddr = stoi(bpaddrtext, nullptr, 16);
			bpk.add(bplistaddr, bptype);
		}
	}

	ImGui::SameLine();

	bool disable_buttons = bpk.breakpoints.size() == 0;

	if (disable_buttons)
		ImGui::BeginDisabled(true);

	if (ImGui::Button("Delete", ImVec2(50, 0)))
	{
		for (auto& it : bpk.breakpoints)
		{
			if (it.addr == bplistaddr)
			{
				bpk.remove(bplistaddr);
			}
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Edit", ImVec2(50, 0)))
	{
		u8 bptype = 0;
		u16 newaddr = 0;

		for (int i = 0; i < 3; i++)
		{
			if (bpaddchkbox[i])
				bptype |= 1 << i;
		}

		stringstream str(bpaddrtext);
		str >> hex >> newaddr;

		bpk.edit(bplistaddr, bptype, newaddr);
	}

	if (disable_buttons)
		ImGui::EndDisabled();

	if (ImGui::ListBoxHeader("##bps", ImVec2(-1, -1)))
	{
		int n = 0;

		for (auto& it : bpk.breakpoints)
		{
			char temp[16];
			char ctype[5] = { 0 };
			ctype[0] = it.enabled ? 'E' : '-';
			ctype[1] = it.type & bp_read ? 'R' : '-';
			ctype[2] = it.type & bp_write ? 'W' : '-';
			ctype[3] = it.type & bp_exec ? 'X' : '-';
			snprintf(temp, sizeof(temp), "$%04X:%s", it.addr, ctype);

			ImGui::PushID(n);

			bool selected = (item_id == n);

			if (ImGui::Selectable(temp, selected, ImGuiSelectableFlags_AllowDoubleClick))
			{
				bplistaddr = it.addr;
				item_id = n;

				for (int i = 0; i < 3; i++)
				{
					if (it.type & (1 << i))
						bpaddchkbox[i] = true;
					else
						bpaddchkbox[i] = false;
				}

				snprintf(bpaddrtext, 5, "%04X", bplistaddr);

				if (ImGui::IsMouseDoubleClicked(0))
				{
					bpk.remove(it.addr);
				}
			}

			if (selected)
				ImGui::SetItemDefaultFocus();

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
		//ImGui::SetWindowPos(ImVec2(0, 420));

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
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%04X", reg.pc);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "A");
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.a);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "X");
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.x);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Y");
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.y);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "P");
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.ps);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "SP");
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
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

			for (auto& it : bpk.breakpoints)
			{
				if (it.type == bp_exec)
				{
					if (bpk.check(pc, bp_exec, it.enabled))
					{
						cpu_state = cstate::debugging;
						return;
					}
				}

				if (bpk.check_access(write_addr, bp_write, it.enabled))
				{
					cpu_state = cstate::debugging;
					write_addr = 0;
					lineoffset = -6;
					return;
				}

				if (bpk.check_access(ppu_write_addr, bp_write, it.enabled))
				{
					cpu_state = cstate::debugging;
					ppu_write_addr = 0;
					lineoffset = -6;
					return;
				}

				if (bpk.check_access(read_addr, bp_read, it.enabled))
				{
					cpu_state = cstate::debugging;
					read_addr = 0;
					lineoffset = -6;
					return;
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

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F8))
	{
		cpu_state = cstate::cycles;
		ppu_step(1);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7))
	{
		cpu_state = cstate::scanlines;
		gui_step(true);
	}
}

void log_to_file(u16 pc)
{
	char text[TEXTSIZE] = { 0 };
	vdentry = get_trace_line(text, pc, true, true);

	//ofstream outFile("cpu_trace.log", ios_base::app);
	for (const auto& e : vdentry)
	{
		outFile
			<< uppercase << hex << setw(4) << setfill('0') << e.offset
			<< " "
			<< e.bytetext
			<< " "
			<< left << setfill(' ') << setw(40)
			<< e.dtext
			<< e.regtext
			<< "\n";
	}
}

void create_close_log(bool status)
{
	logging = status;

	if (logging)
	{
		outFile.open("cpu_trace.log");
		//outFile << "FCEUX 2.6.1 - Trace Log File\n";
	}
	else
	{
		outFile.close();
	}
}

vector<disasmentry> get_trace_line(const char* text, u16 pc, bool get_registers, bool memory_access)
{
	u8 op = rbd(pc);

	int size = 0;
	int asize = 0;
	const char* name;
	int mode;
	char line[TEXTSIZE] = { 0 };
	char bytes[TEXTSIZE] = { 0 };
	char* data = (char*)text;

	vector<disasmentry> entry;

	name = disasm[op].name;
	size = disasm[op].size;
	mode = disasm[op].mode;

	disasmentry e;

	switch (mode)
	{
		case addrmode::impl:
		case addrmode::accu:
		{
			snprintf(data, TEXTSIZE, "%s", name);
			snprintf(bytes, TEXTSIZE, "$%-10.02X", rbd(pc));
			break;
		}
		case addrmode::imme:
		{
			u16 b = get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);
			snprintf(data + strlen(data), TEXTSIZE, "#$%02X", b);
			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::zerp:
		{
			u16 b = get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);
			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X = $%02X", b, rbd(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X", b);
			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::zerx:
		{
			u16 b = get_zerx(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,X @ $%04X = $%02X", d2, (u8)(d2 + reg.x), rbd((u8)(d2 + reg.x)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,X", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", d1, d2);
			break;
		}
		case addrmode::zery:
		{
			u16 b = get_zery(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,Y @ $%04X = $%02X", d2, (u8)(d2 + reg.y), rbd((u8)(d2 + reg.y)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,Y", d2);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", d1, d2);
			break;
		}
		case addrmode::abso:
		{
			u16 b = rw(pc + 1);
			//bool isjump = op == 0x4c || op == 0x20;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X = $%02X", b, ram[b]);
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", ram[pc], ram[pc + 1], ram[pc + 2]);
			break;
		}
		case addrmode::absx:
		{
			u16 b = get_absx(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			u8 d3 = rbd(pc + 2);
			u16 a = d3 << 8 | d2;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,X @ $%04X = $%02X", a, (u16)a + reg.x, rbd((u16)(a + reg.x)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,X", a);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", d1, d2, d3);
			break;
		}
		case addrmode::absy:
		{
			u16 b = get_absy(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			u8 d3 = rbd(pc + 2);
			u16 a = d3 << 8 | d2;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,Y @ $%04X = $%02X", a, (u16)(a + reg.y), rbd(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,Y", a);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", d1, d2, d3);
			break;
		}
		case addrmode::indx:
		{
			u16 b = get_indx(pc + 1, true);
			u8 d1 = rbd((u8)(b + reg.x));
			u8 d2 = rbd((u8)(b + 1 + reg.x));
			u16 a = d2 << 8 | d1;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%02X,X) @ $%04X = $%02X", b, a, rbd(a));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%02X,X)", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::indy:
		{
			u16 b = get_indy(pc + 1, true);
			u8 d1 = rbd((u8)b);
			u8 d2 = rbd((u8)(b + 1));
			u16 a = (u16)((d2 << 8 | d1) + reg.y);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%02X),Y @ $%04X = $%02X", (u8)b, a, rbd(a));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%02X),Y", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		case addrmode::indi:
		{
			u16 b = rw(pc + 1);
			u8 d1 = rbd(pc);
			u8 d2 = rbd(pc + 1);
			u8 d3 = rbd(pc + 2);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%04X) = $%04X", b, rw(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%04X)", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", d1, d2, d3);
			break;
		}
		case addrmode::rela:
		{
			u8 b1 = rbd(pc + 1);
			u16 b = pc + (s8)(b1)+2;
			snprintf(data, TEXTSIZE, "%-4s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04X = $%02X", b, rbd(pc));
			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", rbd(pc), rbd(pc + 1));
			break;
		}
		default:
			snprintf(data, TEXTSIZE, "%-4s", "UNDEFINED");
			snprintf(bytes, TEXTSIZE, "%-11s", "$nn");
			break;
	}



	if (get_registers)
	{
		char align[42] = { 0 };
		char temp[TEXTSIZE] = { 0 };

		char flags[9] = { "........" };
		char text[32] = "";

		flags[7] = reg.ps & FC ? 'C' : 'c';
		flags[6] = reg.ps & FZ ? 'Z' : 'z';
		flags[5] = reg.ps & FI ? 'I' : 'i';
		flags[4] = reg.ps & FD ? 'D' : 'd';
		flags[3] = reg.ps & FB ? 'B' : 'b';
		flags[2] = reg.ps & FU ? 'U' : 'u';
		flags[1] = reg.ps & FV ? 'V' : 'v';
		flags[0] = reg.ps & FN ? 'N' : 'n';

		//snprintf(temp, TEXTSIZE, "c%-12d", ppu_totalcycles);
		//e.regtext += temp;

		snprintf(temp, TEXTSIZE, "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%-3d SL:%-3d",
			reg.a, reg.x, reg.y, reg.ps, reg.sp, cycle, scanline);

		e.regtext += temp;
	}

	e.name = name;
	e.offset = pc;
	e.size = size;
	e.dtext = data;
	e.bytetext = bytes;
	//e.cycles = cycles;
	entry.push_back(e);

	return entry;
}

void gui_clean()
{
	if (logging)
		create_close_log(false);

	ImGui_ImplSDLRenderer_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}