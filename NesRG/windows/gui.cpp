#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"

Breakpoint bpk;
ImGui::FileBrowser fileDialog;

bool Gui::init_gui()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("Couldn't initialize SDL: %s\n", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow("Nes RG", 10, SDL_WINDOWPOS_CENTERED, APP_WIDTH, APP_HEIGHT, 0);

	if (!window)
	{
		printf("Failed to open %d x %d window: %s\n", APP_WIDTH, APP_HEIGHT, SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (!renderer)
	{
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return false;
	}

	//create textures
	display.w = 256; gui.display.h = 240;

	display.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, display.w, display.h);
	//tile.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, display.w, display.h);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(gui.window);
	ImGui_ImplSDLRenderer_Init(gui.renderer);

	ImGui::StyleColorsLight();

	ImGuiStyle* style = &ImGui::GetStyle();

	ImVec4 framebgcol = ImVec4(230 / 255.0f, 230 / 255.0f, 230 / 255.0f, 1.0f);

	style->Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style->Colors[ImGuiCol_FrameBg] = framebgcol;
	style->ItemSpacing = ImVec2(8, 1);

	gui.running = true;
	cpu.state = cstate::debugging;

	return true;
}

void Gui::update_gui()
{
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	while (gui.running)
	{
		step_gui(false);
		input(io);

		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame(gui.window);
		ImGui::NewFrame();

		//ImGui::ShowDemoWindow();
		//ImGui::ShowStyleEditor();
		//ImGui::ShowStyleSelector("stylesel");

		//Menu
		show_menu();

		//Debugger UI
		ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		ImGui::SetWindowSize(ImVec2(900, 900), ImGuiCond_Once);

		ImGui::BeginChild("Buttons", ImVec2(-1, 45), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		show_buttons(io);
		ImGui::EndChild();

		ImGui::Columns(3);

		ImGui::SetColumnWidth(0, 390);

		ImGui::Separator();

		//Show Debugger
		ImGui::BeginChild("Disassembly", ImVec2(0, 500), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		show_disassembly(io);
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::NextColumn();

		if (ImGui::BeginChild("Display"))
		{
			ImGui::SetWindowSize(ImVec2(256, 240));
			ImVec2 tsize = ImGui::GetContentRegionAvail();

			if (cpu.state == cstate::scanline || cpu.state == cstate::cycle)
			{
				gui.render_frame();
			}

			ImGui::Image((void*)gui.display.texture, ImVec2(256, 240));
		}
		ImGui::EndChild();

		ImGui::SameLine();
		ImGui::NextColumn();

		ImGui::BeginChild("Breakpoints", ImVec2(-1, 250), false, ImGuiWindowFlags_NoScrollbar);
		show_breakpoints();
		ImGui::EndChild();

		ImGui::BeginChild("Registers", ImVec2(0, 250), false, ImGuiWindowFlags_NoScrollbar);
		show_registers(io);
		ImGui::EndChild();

		ImGui::Columns(1);

		ImGui::BeginChild("Memory Editor", ImVec2(-1, 310));
		show_memory();
		ImGui::EndChild();

		ImGui::End();

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(gui.renderer, 114, 144, 154, 255);
		SDL_RenderClear(gui.renderer);
		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		SDL_RenderPresent(gui.renderer);
	}
}

void Gui::show_disassembly(ImGuiIO io)
{
	char text[TEXTSIZE] = { 0 };
	u16 pc = is_jump ? inputaddr : reg.pc;

	vector<string> vdasm;
	ImU32 tablerowcolor = 0xff00ffff;
	ImU32 tablecolcolor = 0xffe0e0e0;
	ImVec4 bpsetcolor = { 0x00, 0xff, 0x00, 0xff };
	ImVec4 bpunsetcolor = { 0xe0 / 255.0f, 0xe0 / 255.0f, 0xe0 / 255.0f ,0xff };

	static int item_num = 0;

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

	for (int i = 0; i < 23; i++)
	{
		int pcaddr = pc + lineoffset;
		vector<disasmentry> vdentry = get_trace_line(text, pcaddr);

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

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(bpcheck ? bpsetcolor : bpunsetcolor));
		ImGui::PushID(pcaddr);

		if (ImGui::Button(pctext, ImVec2(40, 19)))
		{
			if (!bpcheck)
			{
				bpk.add(pcaddr, bp_exec);
			}
			else
			{
				bpk.remove(pcaddr);
			}
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

void Gui::show_buttons(ImGuiIO io)
{
	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		ppu.reset();
		mem.load_rom(fileDialog.GetSelected().u8string().c_str());
		cpu.reset();
		cpu.state = cstate::debugging;
		fileDialog.ClearSelected();
	}

	if (ImGui::Button("Load Rom", ImVec2(BUTTONSIZE_X, 0)))
	{
		fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Mapper0Games\\";
		fileDialog.SetTitle("Load Nes Rom");
		fileDialog.SetTypeFilters({ ".nes" });
		fileDialog.SetPwd(game_dir);
		fileDialog.Open();
	}

	ImGui::SameLine();

	if (ImGui::Button("Run", ImVec2(BUTTONSIZE_X, 0)))
	{
		if (mem.rom_loaded)
		{
			step_gui(true);
			is_jump = false;
			cpu.state = cstate::running;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(BUTTONSIZE_X, 0)))
	{
		if (mem.rom_loaded)
		{
			create_close_log(false);
			cpu.state = cstate::debugging;
			lineoffset = 0;
			ppu.reset();
			mem.set_mapper();
			cpu.reset();
			is_jump = false;
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Step Into", ImVec2(BUTTONSIZE_X, 0)))
	{
		if (mem.rom_loaded)
		{
			cpu.state = cstate::debugging;
			lineoffset = 0;
			step_gui(true);
			is_jump = false;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Step Over", ImVec2(BUTTONSIZE_X, 0)))
	{
		if (mem.rom_loaded)
		{
			cpu.state = cstate::debugging;
			lineoffset = 0;

			u8 op = mem.rb(reg.pc);
			u16 ret_pc = reg.pc + 3;
			//cpu.state = cstate::running;

			if (op == 0x20)
			{
				while (reg.pc != ret_pc)
				{
					step_gui(true);
				}
			}
			else
				step_gui(true);

			is_jump = false;
		}
	}

	ImGui::Spacing();
	ImGui::Spacing();
}

void Gui::show_memory()
{
	static MemoryEditor mem_edit;

	//for (int i = 0; i < 10; i++)
	ImGui::Spacing();

	ImGui::Separator();

	ImGui::Text("Memory Editor");

	if (ImGui::BeginTabBar("##mem_tabs", ImGuiTabBarFlags_None))
	{
		if (ImGui::BeginTabItem("RAM"))
		{
			mem_edit.DrawContents(mem.ram.data(), mem.ram.size());
			ImGui::EndTabItem();
		}

		if ((ImGui::BeginTabItem("VRAM")))
		{
			mem_edit.DrawContents(mem.vram.data(), mem.vram.size());
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
}

void Gui::show_breakpoints()
{
	static u16 bplistaddr = 0;
	static bool bpaddchkbox[3] = { false };
	static char bpaddrtext[5] = { 0 };

	ImGui::Text("Breakpoints");

	ImGui::Text("Address:");
	ImGui::SameLine();
	ImGui::PushItemWidth(55);
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

	if (strlen(bpaddrtext) == 0)
		ImGui::BeginDisabled(true);

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

		bplistaddr = stoi(bpaddrtext, nullptr, 16);
		bpk.add(bplistaddr, bptype);
	}

	if (strlen(bpaddrtext) == 0)
		ImGui::EndDisabled();

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
		s16 newaddr = -1;

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

void Gui::show_registers(ImGuiIO io)
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

		ImGui::TableNextColumn(); ImGui::Text("tot cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.totalcycles);

		ImGui::TableNextColumn(); ImGui::Text("ppu cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.cycle);

		ImGui::TableNextColumn(); ImGui::Text("scanline");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.scanline);

		ImGui::TableNextColumn(); ImGui::Text("ppuaddr");
		ImGui::TableNextColumn(); ImGui::Text("%04X", preg.v);

		ImGui::TableNextColumn(); ImGui::Text("t-addr");
		ImGui::TableNextColumn(); ImGui::Text("%04X", preg.v);

		ImGui::TableNextColumn(); ImGui::Text("flags");
		ImGui::TableNextColumn(); ImGui::Text("%s", flags);

		ImGui::TableNextColumn(); ImGui::Text("------------");
		ImGui::TableNextColumn(); ImGui::Text("-----------");

		ImGui::TableNextColumn(); ImGui::Text("%11s", "PC");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%04X", reg.pc);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "A");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.a);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "X");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.x);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Y");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.y);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "P");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", reg.ps);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "SP");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%04X", reg.sp | 0x100);

		ImGui::EndTable();
	}
}

void Gui::show_menu()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{


			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Debug"))
		{
			if (ImGui::MenuItem("Trace To File"))
			{
				create_close_log(!logging);
			}

			if (ImGui::MenuItem("Dump VRAM"))
			{
				std::ofstream outFile("vram.bin", std::ios::binary);
				outFile.write((char*)mem.vram.data(), VRAMSIZE);
				outFile.close();
			}

			if (ImGui::MenuItem("Run 1 Cycle (F6)"))
			{
				cpu.state = cstate::cycle;
				ppu.step(1);
			}

			if (ImGui::MenuItem("Run 1 Scanline (F7)"))
			{
				cpu.state = cstate::scanline;
				step_gui(true);
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void Gui::step_gui(bool stepping, bool over)
{
	ppu.frame_ready = false;
	if (cpu.state == cstate::running)
	{
		while (!ppu.frame_ready)
		{
			u16 pc = reg.pc;

			for (auto& it : bpk.breakpoints)
			{
				if (it.type == bp_exec && cpu.state == cstate::running)
				{
					if (bpk.check(pc, bp_exec, it.enabled))
					{
						cpu.state = cstate::debugging;
						return;
					}
				}

				if (bpk.check_access(cpu.write_addr, bp_write, it.enabled))
				{
					cpu.state = cstate::debugging;
					cpu.write_addr = 0;
					lineoffset = -6;
					return;
				}

				if (bpk.check_access(cpu.read_addr, bp_read, it.enabled))
				{
					cpu.state = cstate::debugging;
					cpu.read_addr = 0;
					lineoffset = -6;
					return;
				}
			}

			int cyc = cpu.step();
			ppu.step(cyc);
			ppu.totalcycles += cyc / 3;

			if (logging)
				log_to_file(reg.pc);

			if (cpu.state == cstate::crashed)
				return;
		}

		ppu.cycles -= CYCLES_PER_LINE;
	}
	else if (cpu.state == cstate::debugging && stepping)
	{
		u16 pc = reg.pc;

		int cyc = cpu.step();
		ppu.step(cyc);
		ppu.totalcycles += cyc / 3;

		if (logging)
			log_to_file(reg.pc);

	}
	else if (cpu.state == cstate::scanline && stepping)
	{
		u16 pc = reg.pc;

		if (logging)
			log_to_file(pc);

		int old_scanline = ppu.scanline;

		while (old_scanline == ppu.scanline)
		{
			int cyc = cpu.step();
			ppu.step(cyc);
			ppu.totalcycles += cyc / 3;
		}
	}
}

void Gui::input(ImGuiIO io)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		ImGui_ImplSDL2_ProcessEvent(&event);
		if (event.type == SDL_QUIT)
			gui.running = 0;
		if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
			gui.running = 0;
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F6))
	{
		cpu.state = cstate::cycle;
		ppu.step(1);
	}

	if (ImGui::IsKeyPressed(SDL_SCANCODE_F7))
	{
		cpu.state = cstate::scanline;
		step_gui(true);
	}
}

void Gui::render_frame()
{
	SDL_UpdateTexture(display.texture, NULL, disp_pixels, display.w * sizeof(unsigned int));
	SDL_RenderCopy(renderer, display.texture, NULL, NULL);
}

void Gui::log_to_file(u16 pc)
{
	char text[TEXTSIZE] = { 0 };
	vdentry = get_trace_line(text, pc, true);

	//ofstream outFile("cpu_trace.log", ios_base::app);
	for (const auto& e : vdentry)
	{
		outFile
			<< uppercase << hex << setw(4) << setfill('0') << e.offset
			<< " "
			<< e.bytetext
			<< " "
			<< left << setfill(' ') << setw(24)
			<< e.dtext
			<< e.regtext
			<< "\n";
	}
}

void Gui::create_close_log(bool status)
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

vector<disasmentry> Gui::get_trace_line(const char* text, u16 pc, bool get_registers, bool memory_access)
{
	u8 op = mem.rb(pc);

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
			snprintf(bytes, TEXTSIZE, "$%-10.02X", mem.rb(pc));
			break;
		}
		case addrmode::imme:
		{
			u16 b = cpu.get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);
			snprintf(data + strlen(data), TEXTSIZE, "#$%02X", b);
			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::zerp:
		{
			u16 b = cpu.get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);
			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X = #$%02X", b, mem.rb(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X", b);
			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::zerx:
		{
			u16 b = cpu.get_zerx(pc + 1);
			u8 d1 = mem.rb(pc);
			u8 d2 = mem.rb(pc + 1);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,X @ $%04X = #$%02X", d2, (u8)(d2 + reg.x), mem.rb((u8)(d2 + reg.x)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,X", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", d1, d2);
			break;
		}
		case addrmode::zery:
		{
			u16 b = cpu.get_zery(pc + 1);
			u8 d1 = mem.rb(pc);
			u8 d2 = mem.rb(pc + 1);
			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,Y @ $%04X = #$%02X", d2, (u8)(d2 + reg.y), mem.rb((u8)(d2 + reg.y)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%02X,Y", d2);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", d1, d2);
			break;
		}
		case addrmode::abso:
		{
			u16 b = mem.rw(pc + 1);
			bool isjump = op == 0x4c || op == 0x20;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (isjump)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X", b);
			else
			{
				if (memory_access)
					snprintf(data + strlen(data), TEXTSIZE, "$%04X = #$%02X", b, mem.ram[b]);
				else
					snprintf(data + strlen(data), TEXTSIZE, "$%04X", b);
			}

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", mem.ram[pc], mem.ram[pc + 1], mem.ram[pc + 2]);
			break;
		}
		case addrmode::absx:
		{
			u16 b = cpu.get_absx(pc + 1);
			u8 d1 = mem.rb(pc);
			u8 d2 = mem.rb(pc + 1);
			u8 d3 = mem.rb(pc + 2);
			u16 a = d3 << 8 | d2;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,X @ $%04X = #$%02X", a, (u16)a + reg.x, mem.rb((u16)(a + reg.x)));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,X", a);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", d1, d2, d3);
			break;
		}
		case addrmode::absy:
		{
			u16 b = cpu.get_absy(pc + 1);
			u8 d1 = mem.rb(pc);
			u8 d2 = mem.rb(pc + 1);
			u8 d3 = mem.rb(pc + 2);
			u16 a = d3 << 8 | d2;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,Y @ $%04X = #$%02X", a, (u16)(a + reg.y), mem.rb(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "$%04X,Y", a);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", d1, d2, d3);
			break;
		}
		case addrmode::indx:
		{
			u16 b = cpu.get_indx(pc + 1, true);
			u8 d1 = mem.rb((u8)(b + reg.x));
			u8 d2 = mem.rb((u8)(b + 1 + reg.x));
			u16 a = d2 << 8 | d1;

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%02X,X) @ $%04X = #$%02X", b, a, mem.rb(a));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%02X,X)", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::indy:
		{
			u16 b = cpu.get_indy(pc + 1, true);
			u8 d1 = mem.rb((u8)b);
			u8 d2 = mem.rb((u8)(b + 1));
			u16 a = (u16)((d2 << 8 | d1) + reg.y);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%02X),Y @ $%04X = #$%02X", (u8)b, a, mem.rb(a));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%02X),Y", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::indi:
		{
			u16 b = mem.rw(pc + 1);
			u8 d1 = mem.rb(pc);
			u8 d2 = mem.rb(pc + 1);
			u8 d3 = mem.rb(pc + 2);

			snprintf(data, TEXTSIZE, "%-4s", name);

			if (memory_access)
				snprintf(data + strlen(data), TEXTSIZE, "($%04X) = $%04X", b, mem.rw(b));
			else
				snprintf(data + strlen(data), TEXTSIZE, "($%04X)", b);

			snprintf(bytes, TEXTSIZE, "$%02X $%02X $%-2.02X", d1, d2, d3);
			break;
		}
		case addrmode::rela:
		{
			u8 b1 = mem.rb(pc + 1);
			u16 b = pc + (s8)(b1)+2;
			snprintf(data, TEXTSIZE, "%-4s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04X", b);
			snprintf(bytes, TEXTSIZE, "$%02X $%-6.02X", mem.rb(pc), mem.rb(pc + 1));
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

		//snprintf(temp, TEXTSIZE, "c%-12d", ppu.totalcycles);
		//e.regtext += temp;

		snprintf(temp, TEXTSIZE, "A:%02X X:%02X Y:%02X P:%02X SP:%02X CYC:%-3d SL:%-3d",
			reg.a, reg.x, reg.y, reg.ps, reg.sp, ppu.cycle, ppu.scanline);

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
