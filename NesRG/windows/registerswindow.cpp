#include "debugwindow.h"
#include "cpu.h"
#include "ppu.h"

void debug_show_registers(ImGuiIO io)
{
	ImGui::BeginChild("Registers");

	if (ImGui::BeginTable("Regs", 2, ImGuiTableFlags_None, ImVec2(-1, 50)))
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

		ImGui::TableNextColumn(); ImGui::Text("%11s", "FPS");
		ImGui::TableNextColumn(); ImGui::Text("%d", static_cast<int>(round(io.Framerate)));

		ImGui::TableNextColumn(); ImGui::Text("%11s", "cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.totalcycles);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "ppu cycles");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.cycle);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "scanline");
		ImGui::TableNextColumn(); ImGui::Text("%d", ppu.scanline);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "ppuaddr");
		ImGui::TableNextColumn(); ImGui::Text("%04X", lp.v);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "t-addr");
		ImGui::TableNextColumn(); ImGui::Text("%04X", lp.t);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "flags");
		ImGui::TableNextColumn(); ImGui::Text("%s", flags);

		ImGui::TableNextColumn(); ImGui::Spacing();
		ImGui::TableNextColumn(); ImGui::Spacing();
		ImGui::TableNextColumn(); ImGui::Separator();
		ImGui::TableNextColumn(); ImGui::Separator();

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

		ImGui::TableNextColumn(); ImGui::Text("%11s", "x");
		ImGui::TableNextColumn(); ImGui::Text("%02X", (lp.t & 7) * 8);

		ImGui::TableNextColumn(); ImGui::Spacing();
		ImGui::TableNextColumn(); ImGui::Spacing();
		ImGui::TableNextColumn(); ImGui::Separator();
		ImGui::TableNextColumn(); ImGui::Separator();

		ImGui::TableNextColumn(); ImGui::Text("%11s", "PRG ROMS");
		ImGui::TableNextColumn(); ImGui::Text("%d", header.prgnum);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "CHR ROMS");
		ImGui::TableNextColumn(); ImGui::Text("%d", header.chrnum);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Mapper");
		ImGui::TableNextColumn(); ImGui::Text("%d", header.mappernum);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Mirroring");
		ImGui::TableNextColumn(); ImGui::Text("%d", header.mirror);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Battery");
		ImGui::TableNextColumn(); ImGui::Text("%d", header.battery);

		ImGui::TableNextColumn(); ImGui::Text("%11s", "Tainer");
		ImGui::TableNextColumn(); ImGui::Text("%d", header.trainer);

		ImGui::EndTable();
	}

	ImGui::EndChild();
}