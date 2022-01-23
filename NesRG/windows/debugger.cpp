#include "debugger.h"
#include "breakpoints.h"
#include "sdlgfx.h"
#include "cpu.h"
#include "ppu.h"

Breakpoint bpk;

bool Debugger::init()
{
	ImGui::CreateContext();
	ImGuiSDL::Initialize(gfx.get_renderer(), 1440, 1000);
	ImGui_ImplSDL2_InitForSDLRenderer(gfx.get_window());

	ImGui::StyleColorsLight();

	ImGuiStyle* style = &ImGui::GetStyle();

	style->Colors[ImGuiCol_WindowBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style->Colors[ImGuiCol_ChildBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	//ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.25f);
	//style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0 / 255.0f, 95 / 255.0f, 184 / 255.0f, 1.0f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0, 0, 0, 1);
	style->FrameBorderSize = 1.20f;
	//style->ScaleAllSizes(0.9f);

	return false;
}

void Debugger::update()
{
	gfx.running = true;
	cpu.state = cstate::debugging;

	while (gfx.running)
	{
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		gfx.begin_frame();

		step();
		input(io);

		//Debugger UI
		ImGui::Begin("Debugger", nullptr, 0);

		ImGui::SetWindowSize(ImVec2(500, 900));

		ImGui::Columns(2);

		ImGui::SetColumnWidth(0, 186);

		show_registers();
		show_breakpoints();

		ImGui::SameLine();
		ImGui::NextColumn();

		show_disassembly();

		ImGui::Columns(1);

		ImGui::End();

		//Render
		ImGui::Render();
		ImGuiSDL::Render(ImGui::GetDrawData());

		gfx.end_frame();
	}
}

void Debugger::show_disassembly()
{
	char text[TEXTSIZE] = { 0 };
	u16 pc = r.pc;

	std::vector<std::string> vdasm;
	ImU32 tablerowcolor = 0xff00ffff;
	ImU32 tablecolcolor = 0xffe0e0e0;
	ImU32 bpsetcolor = 0xff00ff00;

	ImGuiIO& io = ImGui::GetIO();

	//Show Debugger
	ImGui::BeginChild("Disassembly", ImVec2(0, ImGui::GetWindowHeight()));

	if (io.MouseWheel != 0 && (pc + lineoffset < 0x10000))
	{
		if (ImGui::IsWindowHovered())
		{
			if (io.MouseWheel > 0)
				lineoffset -= 6;
			else
				lineoffset += 6;
		}
	}

	if (lineoffset == 0)
		ImGui::SetScrollHereY(0.5f);

	ImGui::SameLine(1, 0);

	if (ImGui::Button("Run", ImVec2(80, 0)))
	{
		cpu.state = cstate::running;
		cpu.step();
	}

	ImGui::SameLine();

	if (ImGui::Button("Step Into", ImVec2(80, 0)))
	{
		cpu.state = cstate::debugging;
		lineoffset = 0;
		step(true);
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(80, 0)))
	{
		cpu.state = cstate::debugging;
		lineoffset = 0;
		mem.set_mapper();
		cpu.reset();
	}

	if (ImGui::BeginTable("", 3, 0, ImVec2(500, 400)))
	{
		ImGui::TableSetupColumn("offset", ImGuiTableColumnFlags_WidthFixed, 40.0f);
		ImGui::TableSetupColumn("disasmtext", ImGuiTableColumnFlags_WidthFixed, 120);
		ImGui::TableSetupColumn("bytes", ImGuiTableColumnFlags_WidthFixed, 110);

		for (int i = 0; i < 37; i++)
		{
			int pcaddr = pc + lineoffset;
			std::vector<disasmentry> vdentry = get_trace_line(text, pcaddr);

			ImGui::TableNextRow();

			//ImGui::TableNextColumn();
			ImGui::SameLine(5, 0);

			bool bpcheck = false;
			for (auto& it : bpk.get_breakpoints())
			{
				if (it.addr == pcaddr)
				{
					bpcheck = true;
					break;
				}
			}

			ImGui::TableNextColumn(); ImGui::Text("%04X", vdentry[0].offset);
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, bpcheck ? bpsetcolor : pcaddr == r.pc ? tablerowcolor : tablecolcolor);
			ImGui::TableNextColumn(); ImGui::Text(vdentry[0].dtext.c_str());
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, pcaddr == r.pc ? tablerowcolor : 0xffffffff);
			ImGui::TableNextColumn(); ImGui::Text(vdentry[0].bytetext.c_str());
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, pcaddr == r.pc ? tablerowcolor : tablecolcolor);

			pc += vdentry[0].size;

			if (stepping)
			{
				ImGui::SetScrollHereY(0);
				stepping = false;
			}
		}
		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void Debugger::show_breakpoints()
{
	static u16 bplistaddr = 0;
	static bool bpaddchkbox[3] = { false };
	static char bpaddrtext[5] = { 0 };
	static ImVec4 checkcolor[3] = { };
	static bool openpopup = false;
	static bool edit_breakpoint = false;

	//ImGui::BeginChild("Breakpoints");
	//{
		//ImGui::SetWindowPos(ImVec2(15, 250));

	checkcolor[0] = ImVec4(255, 255, 255, 255);
	checkcolor[1] = ImVec4(255, 255, 255, 255);
	checkcolor[2] = ImVec4(255, 255, 255, 255);

	if (bpaddchkbox[0])
		checkcolor[0] = ImVec4(0, 95 / 255.0f, 184 / 255.0f, 1);
	if (bpaddchkbox[1])
		checkcolor[1] = ImVec4(0, 95 / 255.0f, 184 / 255.0f, 1);
	if (bpaddchkbox[2])
		checkcolor[2] = ImVec4(0, 95 / 255.0f, 184 / 255.0f, 1);

	//std::ostringstream ss;
	//ss << std::setw(4) << std::setfill('0') << bplistaddr;
	//bpaddrtext = ss.str();

	ImGui::Text("Address:");
	ImGui::SameLine();
	ImGui::PushItemWidth(55);
	ImGui::InputText("##bpadd", bpaddrtext, IM_ARRAYSIZE(bpaddrtext));
	ImGui::PopItemWidth();

	ImGui::Checkbox("R", &bpaddchkbox[0]);

	ImGui::SameLine(0, 15);

	ImGui::Checkbox("W", &bpaddchkbox[1]);

	ImGui::SameLine(0, 15);

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

		bplistaddr = std::stoi(bpaddrtext, nullptr, 16);
		bpk.add(bplistaddr, bptype);
	}

	if (strlen(bpaddrtext) == 0)
		ImGui::EndDisabled();

	ImGui::SameLine();

	bool disable_buttons = bpk.get_breakpoints().size() == 0;

	if (disable_buttons)
		ImGui::BeginDisabled(true);

	if (ImGui::Button("Delete", ImVec2(50, 0)))
	{
		for (auto& it : bpk.get_breakpoints())
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

		std::stringstream str(bpaddrtext);
		str >> std::hex >> newaddr;

		bpk.edit(bplistaddr, bptype, newaddr);
	}

	if (disable_buttons)
		ImGui::EndDisabled();

	if (ImGui::ListBoxHeader("##bps", ImVec2(-1, ImGui::GetWindowHeight() - 90)))
	{
		int n = 0;

		for (auto it : bpk.get_breakpoints())
		{
			char temp[16];
			char ctype[5] = { 0 };
			ctype[0] = it.enabled ? 'E' : '-';
			ctype[1] = it.type & bp_read ? 'R' : '-';
			ctype[2] = it.type & bp_write ? 'W' : '-';
			ctype[3] = it.type & bp_exec ? 'X' : '-';
			snprintf(temp, sizeof(temp), "%04X %s", it.addr, ctype);

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

	//ImGui::EndChild();
//}
}

void Debugger::show_registers()
{
	ImU32 tablecolcolor = 0xffe0e0e0;

	ImGui::BeginChild("Registers", ImVec2(0, 200));

	if (ImGui::BeginTable("Regs", 2))
	{
		ImGui::SetWindowPos(ImVec2(0, 400));

		char flags[7] = { "......" };
		char text[32] = "";

		flags[5] = r.ps & FC ? 'C' : '.';
		flags[4] = r.ps & FZ ? 'Z' : '.';
		flags[3] = r.ps & FI ? 'I' : '.';
		flags[2] = r.ps & FD ? 'D' : '.';
		flags[1] = r.ps & FV ? 'V' : '.';
		flags[0] = r.ps & FN ? 'N' : '.';

		ImGui::TableSetupColumn("regnames", ImGuiTableColumnFlags_WidthFixed, 80.0f);
		ImGui::TableSetupColumn("regvalues", ImGuiTableColumnFlags_WidthFixed, 80);

		ImGui::TableNextColumn(); ImGui::Text("cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", cpu.cycles);

		ImGui::TableNextColumn(); ImGui::Text("scanline");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.scanline);

		ImGui::TableNextColumn(); ImGui::Text("flags");
		ImGui::TableNextColumn(); ImGui::Text("%s", flags);

		ImGui::TableNextColumn(); ImGui::Text("------------");
		ImGui::TableNextColumn(); ImGui::Text("-----------");

		//ImGui::TableNextColumn(); ImGui::Text("%11s", "PC");
		//ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);


		ImGui::TableNextColumn(); ImGui::Text("%11s", "PC");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%04X", r.pc);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "A");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", r.a);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "X");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", r.x);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Y");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", r.y);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "P");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%02X", r.ps);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "SP");
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, tablecolcolor);
		ImGui::TableNextColumn(); ImGui::Text("%04X", r.sp | 0x100);

		ImGui::EndTable();
	}
	ImGui::EndChild();
}

void Debugger::show_buttons(u16& inputaddr, bool& is_jump, ImGuiIO io)
{
	static char inputtext[5] = "";
	static char testtext[2] = "";
	u16 pc = r.pc;

	if (ImGui::Begin("Buttons", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove))
	{
		ImGui::SetWindowPos(ImVec2(0, 0));
		ImGui::SetWindowSize(ImVec2(APP_WIDTH, 60));



		ImGui::SameLine();


		if (ImGui::Button("Step Over", ImVec2(80, 0)))
		{
			u8 op = mem.rb(pc);
			u8 b1 = mem.rb(pc + 1);
			disasmdata dasm = get_disasm_entry(op, pc);

			if (strstr(dasm.name, "djnz") || strstr(dasm.name, "call") || strstr(dasm.name, "rst"))
			{
				u16 prevpc = pc;
				if (op == 0xc3)
					cpu.step();
				else
				{
					u16 retpc = pc + dasm.size;
					cpu.state = cstate::running;

					while (pc != retpc)
					{
						cpu.step();

						//gui(io);

						if (cpu.state != cstate::running)
							break;
					}
				}
			}
			else
				cpu.step();

			cpu.state = cstate::debugging;
			stepping = true;
			lineoffset = 0;
			is_jump = false;
		}

		ImGui::SameLine();

		std::string log = logging ? "Logging: On" : "Logging: Off";

		if (ImGui::Button(log.c_str(), ImVec2(100, 0)))
		{
			logging = !logging;

			//if (logging)
			//	outFile.open("cpu_trace.log");
			//else
			//	outFile.close();
		}

		ImGui::SameLine();

		if (ImGui::Button("Dump Memory", ImVec2(100, 0)))
		{
			std::ofstream outFile("ram.bin", std::ios::binary);
			outFile.write((char*)mem.ram, sizeof(mem.ram));
			outFile.close();
		}

		ImGui::SameLine();

		ImGui::PushItemWidth(40);

		if (ImGui::InputText("Jump To Address", inputtext, IM_ARRAYSIZE(inputtext), ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			std::istringstream ss(inputtext);
			ss >> std::hex >> inputaddr;
			is_jump = true;
			lineoffset = 0;
		}

		ImGui::PopItemWidth();

		ImGui::SameLine(0, 5);

		//ImGui::Checkbox("Show Tiles/Sprites", &showtiles);

		ImGui::SameLine(0, 50);

		ImGui::Text("%s: %f", "FPS", io.Framerate);

		ImGui::End();
	}
}

void Debugger::input(ImGuiIO io)
{
	SDL_Event e;
	int wheel = 0;

	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL2_ProcessEvent(&e);
		if (e.type == SDL_QUIT)
			gfx.running = false;
		else if (e.type == SDL_WINDOWEVENT)
		{
			if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				io.DisplaySize.x = static_cast<float>(e.window.data1);
				io.DisplaySize.y = static_cast<float>(e.window.data2);
			}
		}
		else if (e.type == SDL_MOUSEWHEEL)
		{
			wheel = e.wheel.y;
		}
		else if (e.type == SDL_KEYDOWN && e.key.repeat == 0)
		{
			switch (e.key.keysym.sym)
			{
				case SDLK_UP:
					//keystick1[0] = 1;
					break;
				case SDLK_LEFT:
					//keystick1[1] = 1;
					break;
				case SDLK_RIGHT:
					//keystick1[2] = 1;
					break;
				case SDLK_DOWN:
					//keystick1[3] = 1;
					break;
				case SDLK_RETURN:
					//keystick1[5] = 1;

					break;

				case SDLK_SPACE:
					//keystick1[7] = 1;
					break;
			}
		}
		else if (e.type == SDL_KEYUP && e.key.repeat == 0)
		{
			switch (e.key.keysym.sym)
			{
				case SDLK_UP:
					//keystick1[0] = 0;
					break;
				case SDLK_LEFT:
					//keystick1[1] = 0;
					break;
				case SDLK_RIGHT:
					//keystick1[2] = 0;
					break;
				case SDLK_DOWN:
					//keystick1[3] = 0;
					break;
				case SDLK_RETURN:
					//keystick1[5] = 0;
					//keystick2[5] = 0;
					break;

				case SDLK_SPACE:
					//keystick1[7] = 0;
					break;
			}
		}
	}

	int mouseX, mouseY;
	const int buttons = SDL_GetMouseState(&mouseX, &mouseY);

	// Setup low-level inputs (e.g. on Win32, GetKeyboardState(), or write to those fields from your Windows message loop handlers, etc.)

	io.DeltaTime = 1.0f / 60.0f;
	io.MousePos = ImVec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
	io.MouseDown[0] = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
	io.MouseDown[1] = buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);
	io.MouseWheel = static_cast<float>(wheel);
}

void Debugger::step(bool stepping)
{
	if (cpu.state == cstate::running)
	{
		while (cpu.cycles < CYCLES_PER_FRAME)
		{
			u16 pc = r.pc;

			for (auto it : bpk.get_breakpoints())
			{
				if (it.type == bp_exec)
				{
					if (bpk.check(pc, bp_exec, it.enabled))
					{
						cpu.state = cstate::debugging;
						lineoffset = -6;
						return;
					}
				}

				//if (bpk->check(pc, it.enabled))
				//{
				//	*state = cstate::debugging;
				//	return;
				//}

				//if (bpk.check_access(cpu.get_write_addr(), bp_write, it.enabled))
				//{
				//	//cpu.state = cstate::debugging;
				//	//cpu.set_write_addr(0);
				//	//lineoffset = -6;
				//	return;
				//}

				//if (bpk.check_access(cpu.get_read_addr(), bp_read, it.enabled))
				//{
				//	cpu.state = cstate::debugging;
				//	cpu.set_read_addr(0);
				//	return;
				//}
			}

			cpu.step();

			if (cpu.state == cstate::crashed)
				return;
		}

		cpu.cycles -= CYCLES_PER_FRAME;

		ppu.set_scanline();
	}
	else if (stepping)
	{
		cpu.step();

		if (cpu.cycles >= CYCLES_PER_FRAME)
		{
			cpu.cycles -= CYCLES_PER_FRAME;
			ppu.set_scanline();
		}
	}
}

std::vector<disasmentry> Debugger::get_trace_line(const char* text, u16 pc, bool get_registers)
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
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(bytes, TEXTSIZE, "%02X", mem.rb(pc));
			break;
		}
		case addrmode::imme:
		{
			u16 b = cpu.get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "#$%02x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::zerp:
		{
			u16 b = cpu.get_zerp(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%02x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::zerx:
		{
			u16 b = cpu.get_zerx(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%02x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::zery:
		{
			u16 b = cpu.get_zery(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%02x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::abso:
		{
			u16 b = cpu.get_abso(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X %02X", mem.rb(pc), mem.rb(pc + 1), mem.rb(pc + 2));
			break;
		}
		case addrmode::absx:
		{
			u16 b = cpu.get_abso(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04x, x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X %02X", mem.rb(pc), mem.rb(pc + 1), mem.rb(pc + 2));
			break;
		}
		case addrmode::absy:
		{
			u16 b = cpu.get_abso(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X %02X", mem.rb(pc), mem.rb(pc + 1), mem.rb(pc + 2));
			break;
		}
		case addrmode::indx:
		{
			u16 b = cpu.get_indx(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%02x, x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::indy:
		{
			u16 b = cpu.get_indy(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "($%02x), y", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		case addrmode::rela:
		{
			u16 b = cpu.get_rela(pc + 1);
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(data + strlen(data), TEXTSIZE, "$%04x", b);
			snprintf(bytes, TEXTSIZE, "%02X %02X", mem.rb(pc), mem.rb(pc + 1));
			break;
		}
		default:
			snprintf(data, TEXTSIZE, "%-5s", name);
			snprintf(bytes, TEXTSIZE, "%s", "nn");
			break;
	}

	e.name = name;
	e.offset = pc;
	e.size = size;
	e.dtext = data;
	e.bytetext = bytes;
	entry.push_back(e);

	return entry;
}

disasmdata Debugger::get_disasm_entry(u8 op, u16 pc)
{
	return disasmdata();
}

void Debugger::clean()
{
	ImGuiSDL::Deinitialize();

	ImGui::DestroyContext();
}
