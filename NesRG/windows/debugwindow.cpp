#include "debugwindow.h"
#include "cpu.h"
#include "ppu.h"
#include "tracer.h"
#include "breakpoints.h"

ImGui::FileBrowser fileDialog;

void gui_create_button(bplist it, const char* text, u8 n, u16 inputaddr, u8 bptype);

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

		if (pcaddr == reg.pc)
			currlinecol = ImVec4(0, 0, 1, 1);

		//char pctext[5] = "";
		//snprintf(pctext, sizeof(pctext), "%04X", vdentry[0].offset);

		//ImGui::TextColored(currlinecol, pctext);
		//ImGui::SameLine();
		ImGui::TextColored(currlinecol, entry[0].line.c_str());
		//ImGui::SameLine();
		//ImGui::TextColored(currlinecol, vdentry[0].dtext.c_str());

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


