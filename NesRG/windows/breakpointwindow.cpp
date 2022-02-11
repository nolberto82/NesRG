#include "debugwindow.h"
#include "breakpoints.h"

void gui_create_button(bplist it, const char* text, u8 n, u16 inputaddr, u8 bptype);

void debug_show_breakpoints()
{
	static u16 inputaddr[BREAKPOINT_MAX];
	static bool bpaddchk[5] = { false };
	static char bpaddrtext[BREAKPOINT_MAX][5] = { 0 };
	static int item_id = 0;

	if (ImGui::Begin("Breakpoints", nullptr, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::SetWindowSize(ImVec2(DEBUG_W, MEM_H));
		ImGui::SetWindowPos(ImVec2(DEBUG_X, DEBUG_H + DEBUG_Y + 5));

		if (ImGui::ListBoxHeader("##bps", ImVec2(-1, -1)))
		{
			int n = 0;

			for (auto& it : breakpoints)
			{
				u8 bptype = it.type;

				ImGui::Text("Breakpoint %2d", n + 1);

				ImGui::SameLine();

				ImGui::PushID(n);

				ImGui::PushItemWidth(40);

				if (ImGui::InputText("##bpadd", (char*)bpaddrtext[n], IM_ARRAYSIZE(bpaddrtext[n]), INPUT_FLAGS))
				{
					std::istringstream ss(bpaddrtext[n]);
					ss >> std::hex >> inputaddr[n];
					lineoffset = 0;
				}
				ImGui::PopItemWidth();

				ImGui::SameLine();

				gui_create_button(it, "CR", n, inputaddr[n], bp_read);
				ImGui::SameLine();
				gui_create_button(it, "CW", n, inputaddr[n], bp_write);
				ImGui::SameLine();
				gui_create_button(it, "CX", n, inputaddr[n], bp_exec);
				ImGui::SameLine();
				gui_create_button(it, "VR", n, inputaddr[n], bp_vread);
				ImGui::SameLine();
				gui_create_button(it, "VW", n, inputaddr[n], bp_vwrite);

				ImGui::PopID();

				n++;
			}
			ImGui::ListBoxFooter();
		}
	}
	ImGui::End();
}