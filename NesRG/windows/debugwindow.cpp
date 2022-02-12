#include "debugwindow.h"
#include "gui.h"
#include "cpu.h"
#include "ppu.h"
#include "tracer.h"
#include "breakpoints.h"

ImGui::FileBrowser fileDialog;

void debug_update()
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	//Debugger UI
	if (ImGui::Begin("Debugger", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::SetWindowSize(ImVec2(DEBUG_W, DEBUG_H));
		ImGui::SetWindowPos(ImVec2(DEBUG_X, DEBUG_Y));

		debug_show_buttons(io);

		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Columns(2, "##disasm", false);

		ImGui::SetColumnWidth(0, 320);

		debug_show_disassembly(io);

		ImGui::SameLine();
		ImGui::NextColumn();

		debug_show_registers(io);

		ImGui::Columns(1);
	}
	ImGui::End();

	debug_show_breakpoints();
	debug_show_memory();
}

void debug_show_disassembly(ImGuiIO io)
{
	static int item_num = 0;
	static bool ispc = false;
	u16 pc = is_jump ? jumpaddr : reg.pc;
	vector<string> vdasm;

	ImGui::BeginChild("Disassembly", ImVec2(0, -1), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

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

	for (int i = 0; i < 32; i++)
	{
		int pcaddr = pc + lineoffset;

		vector<disasmentry> entry = get_trace_line(pcaddr, false);

		bool bpcheck = false;

		for (auto& it : breakpoints)
		{
			if (it.addr == pcaddr)
			{
				bpcheck = true;
				break;
			}
		}

		ImVec4 currlinecol = ImVec4(0, 0, 0, 1);

		ispc = false;
		if (pcaddr == reg.pc)
			ispc = true;

		if (entry[0].line != "")
			ImGui::Selectable(entry[0].line.c_str(), ispc);

		pc += entry[0].size;

		if (stepping)
		{
			ImGui::SetScrollHereY(0);
			stepping = false;
		}
	}

	ImGui::EndChild();
}

void debug_show_buttons(ImGuiIO io)
{
	fileDialog.Display();
	fileDialog.SetWindowSize(500, 800);

	if (fileDialog.HasSelected())
	{
		ppu_reset();
		clear_pixels();
		load_rom((char*)fileDialog.GetSelected().u8string().c_str());
		cpu_reset();
		//cpu.state = cstate::running;
		fileDialog.ClearSelected();
	}

	ImGui::BeginChild("Buttons", ImVec2(-1, 45), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

	if (ImGui::Button("Load Rom", ImVec2(BUTTON_W, 0)))
	{
		fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Tests\\";
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
			cpu.state = cstate::running;
			is_jump = false;
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Reset", ImVec2(BUTTON_W, 0)))
	{
		if (rom_loaded)
		{
			create_close_log(false);
			cpu.state = cstate::running;
			lineoffset = 0;
			ppu_reset();
			set_mapper();
			cpu_reset();
			is_jump = false;
			clear_pixels();
		}
	}

	ImGui::SameLine();

	ImGui::Text("Jump To");

	ImGui::Spacing();
	ImGui::Spacing();

	if (ImGui::Button("Step Into", ImVec2(BUTTON_W, 0)))
	{
		if (rom_loaded)
		{
			cpu.state = cstate::debugging;
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

	ImGui::SameLine();

	static char jumpaddrtext[5] = { 0 };

	ImGui::PushItemWidth(40);
	if (ImGui::InputText("##jumptoaddr", jumpaddrtext, IM_ARRAYSIZE(jumpaddrtext), INPUT_ENTER))
	{
		std::istringstream ss(jumpaddrtext);
		ss >> std::hex >> jumpaddr;
		is_jump = true;
		lineoffset = 0;
	}
	ImGui::PopItemWidth();

	ImGui::Spacing();
	ImGui::Spacing();

	ImGui::EndChild();
}

void debug_show_registers(ImGuiIO io)
{
	static string flag_names = "NVUBDIZC";
	static bool flag_values[8] = { };

	stringstream ss;

	ImGui::BeginChild("Registers");

	ImGui::Text("FPS=%d", static_cast<int>(round(io.Framerate)));

	ImGui::Spacing();
	ImGui::Separator();


	for (int i = 0; i < 3; i++)
		ImGui::Spacing();

	ImGui::Text("PC:%04X", reg.pc);
	ImGui::Text("SP:%04X", reg.sp);
	ImGui::Text(" A:%02X", reg.a);
	ImGui::Text(" X:%02X", reg.x);
	ImGui::Text(" Y:%02X", reg.y);
	ImGui::Separator();
	ImGui::Spacing();
	ImGui::Spacing();

	u8 shift = 0x80;
	for (int i = 0; i < 8; i++)
	{
		bool checked = reg.ps & shift;
		char c[2] = { 0 };
		c[0] = flag_names[i];
		ImGui::Checkbox(&c[0], &checked);

		if (i != 3)
			ImGui::SameLine();
		shift >>= 1;
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	ImGui::EndChild();
}

void debug_show_breakpoints()
{
	static u16 inputaddr;
	static char bpaddrtext[5] = { 0 };
	static int item_id = 0;
	static u8 bptype = 0;

	if (ImGui::Begin("Breakpoints", nullptr, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::SetWindowSize(ImVec2(DEBUG_W, MEM_H));
		ImGui::SetWindowPos(ImVec2(DEBUG_X, DEBUG_H + DEBUG_Y + 5));

		if (ImGui::Button("Add breakpoint", ImVec2(BUTTON_W, 0)))
		{
			bptype = 0;
			ImGui::OpenPopup("Add breakpoint");
		}

		if (ImGui::BeginPopupModal("Add breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushItemWidth(40);

			if (ImGui::InputText("##bpadd", (char*)bpaddrtext, IM_ARRAYSIZE(bpaddrtext), INPUT_FLAGS))
			{
				std::istringstream ss(bpaddrtext);
				ss >> std::hex >> inputaddr;
				lineoffset = 0;
			}
			ImGui::PopItemWidth();

			debug_bp_buttons("Cpu Read", bp_read, &bptype, BUTTON_W);
			ImGui::SameLine();
			debug_bp_buttons("Cpu Write", bp_write, &bptype, BUTTON_W);
			ImGui::SameLine();
			debug_bp_buttons("Cpu Exec", bp_exec, &bptype, BUTTON_W);

			if (ImGui::Button("Close"))
			{
				bp_add(inputaddr, bptype, true);
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		if (ImGui::ListBoxHeader("##bps", ImVec2(-1, -1)))
		{
			int n = 0;

			for (auto& it : breakpoints)
			{
				u8 bptype = it.type;

				ImGui::PushID(n);

				stringstream ss;
				ss << hex << uppercase << setfill('0') << it.addr;

				if (ImGui::Button(ss.str().c_str()))
				{
					jumpaddr = it.addr;
					lineoffset = 0;
					is_jump = true;
				}

				ImGui::SameLine();

				ImGui::PushStyleColor(ImGuiCol_Button, it.enabled ? GREEN : RED);
				if (ImGui::Button("Enabled"))
					it.enabled = !it.enabled;
				ImGui::PopStyleColor();

				ImGui::SameLine();

				if (ImGui::Button("Delete"))
					breakpoints.erase(breakpoints.begin() + n);

				ImGui::PopID();

				n++;
			}
			ImGui::ListBoxFooter();
		}
	}
	ImGui::End();
}

void debug_show_memory()
{
	static MemoryEditor mem_edit;

	if (ImGui::Begin("Memory Editor", nullptr))
	{
		ImGui::SetWindowSize(ImVec2(MEM_W, MEM_H));
		ImGui::SetWindowPos(ImVec2(DEBUG_X + DEBUG_W + 5, DEBUG_Y + DEBUG_H + 5));

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
	ImGui::End();
}

void debug_create_textbox(string l, string id, u16 regval, int width)
{
	stringstream ss;
	ss << setw(width) << hex << setfill('0') << uppercase << regval;
	ImGui::Text(l.c_str());
	ImGui::SameLine();
	ImGui::Text(ss.str().c_str());
}

void debug_bp_buttons(string l, u8 bp, u8* type, float width)
{
	bool bpenabled = bp & *(type);
	ImGui::PushStyleColor(ImGuiCol_Button, bpenabled ? GREEN : RED);
	if (ImGui::Button(l.c_str(), ImVec2(width, 0)))
	{
		*type ^= bp;
	}
	ImGui::PopStyleColor();
}
