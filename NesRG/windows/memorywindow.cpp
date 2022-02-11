#include "debugwindow.h"
#include "mem.h"

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
