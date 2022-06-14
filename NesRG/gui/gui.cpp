#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "sdlcc.h"
#include "tracer.h"
#include "main.h"
#include "mappers.h"
#include "mem.h"

#include <windows.h>
#include <commdlg.h>

namespace GUIGL
{
	ImGui::FileBrowser fileDialog;
	vector<Cheats> cheats;

	void update()
	{
		//ImGui::StyleColorsLight();
		//glClear(GL_COLOR_BUFFER_BIT);

		if (!debug_enable)
			show_game_view();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		show_menu();

		if (cheat_opened)
			cheat_dialog();

		if (debug_enable)
		{
			show_ppu_debug();
			show_debugger();
			show_memory();

			if (ImGui::Begin("Display", nullptr, NO_SCROLL))
			{
				ImGui::SetWindowPos(ImVec2(5, menubarheight + 5));
				ImGui::SetWindowSize(ImVec2(512, 400));
				ImVec2 tsize = ImGui::GetContentRegionAvail();
				SDL::render_screen_debug(SDL::screen, PPU::screen_pix.data(), 256, 240, menubarheight);
				ImGui::Image((void*)SDL::screen, tsize);
			}
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		SDL_GL_SwapWindow(SDL::window);
	}

	void show_game_view()
	{
		int w, h;
		SDL_GL_GetDrawableSize(SDL::window, &w, &h);
		SDL::render_screen(SDL::screen, PPU::screen_pix.data(), w, h, menubarheight);
	}

	void show_ppu_debug()
	{
		if (ImGui::Begin("PPU Debug", nullptr, NO_SCROLL))
		{
			ImGui::SetWindowPos(ImVec2(5, menubarheight + 5 + 410));
			if (ImGui::BeginTabBar("##gfx_tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Name Tables"))
				{
					if ((PPU::frame % 8) == 0)
						PPU::render_nametable();
					ImVec2 tsize = ImGui::GetContentRegionAvail();
					ImGui::SetWindowSize(ImVec2(512, 540));
					float ratio = tsize.x / tsize.y;
					ImGui::Image((void*)(intptr_t)SDL::nametable, tsize);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Pattern Tables"))
				{
					PPU::render_pattern();
					SDL::update_pattern();
					ImVec2 tsize = ImGui::GetContentRegionAvail();
					ImGui::Image((void*)(intptr_t)SDL::pattern, ImVec2(128 * 2, 128 * 2));
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			//}
		}
		ImGui::End();
	}

	void show_debugger()
	{
		u16 pc = is_jump ? jumpaddr : reg.pc;
		vector<disasmentry> entries;
		int cyc = 0;

		if (ImGui::Begin("Debugger", nullptr, NO_SCROLL))
		{
			ImGui::SetWindowPos(ImVec2(528 + 5, menubarheight + 5));
			ImGui::SetWindowSize(ImVec2(650, menubarheight + 650));
			ImGui::BeginGroup();
			ImGui::Text("%s", header.name.c_str());
			ImGui::Separator();
			set_spacing(1);

			if (ImGui::Button("Run", ImVec2(BUTTON_W, 0)))
				run_emu();

			ImGui::SameLine();

			if (ImGui::Button("Reset", ImVec2(BUTTON_W, 0)))
				reset_emu();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(logging ? BLUE : DEFCOLOR));
			if (ImGui::Button("Trace Log", ImVec2(BUTTON_W, 0)))
				create_close_log(!logging);
			ImGui::PopStyleColor(1);

			set_spacing(1);
			ImGui::Separator();
			set_spacing(1);

			ImGui::Columns(2);

			ImGui::Text("PC:%04X  A:%02X  X:%02X  Y:%02X  P:%02X", reg.pc, reg.a, reg.x, reg.y, reg.ps);

			ImGui::Text("Scanline:%3d  Cycle:%3d  VRAM:%04X  TVRAM:%04X",
				PPU::scanline, PPU::cycle, lp.v, lp.t);
			ImGui::Text("CpuCycles:%d", PPU::totalcycles);

			if (MEM::mapper)
				ImGui::Text("MMC4 Counter:%02X", MEM::mapper->counter);

			ImGui::NextColumn();

			u8 shift = 0x80;
			for (int i = 0; i < 8; i++)
			{
				char* c = (char*)flag_names[i];
				bool f = (reg.ps & (shift >> i));
				ImGui::TextColored(f ? GREEN : RED, "%c", c);
				if (i < 7)
					ImGui::SameLine();
			}

			ImGui::TextColored(pstatus.vblank ? GREEN : RED, "VBlank"); ImGui::SameLine();
			ImGui::TextColored(pstatus.sprite0hit ? GREEN : RED, "Sprite0");; ImGui::SameLine();
			ImGui::TextColored(pmask.background ? GREEN : RED, "BgStatus");; ImGui::SameLine();
			ImGui::TextColored(pmask.sprite ? GREEN : RED, "SprStatus");

			ImGui::Columns(1);

			ImGui::Separator();

			ImGui::Text("FPS:%.1f", ImGui::GetIO().Framerate);
			ImGui::EndGroup();

			ImGui::Separator();
			set_spacing(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 400);

			show_disassembly(pc);

			ImGui::NextColumn();

			show_rom_info();

			set_spacing(1);

			if (ImGui::BeginChild("Breakpoints", ImVec2(0, 0), true, NO_SCROLL))
			{
				if (ImGui::Button("Add breakpoint", ImVec2(-1, 0)))
				{
					bptype = 0;
					ImGui::OpenPopup("Add breakpoint");
				}

				open_dialog();

				if (ImGui::ListBoxHeader("##bps", ImVec2(-1, -1)))
				{
					int n = 0;
					for (auto& it : breakpoints)
					{
						u8 bptype = it.type;
						stringstream ss;
						ss << setw(4) << hex << uppercase << setfill('0') << it.addr;

						ImGui::PushID(n);
						if (ImGui::Button(ss.str().c_str()))
						{
							jumpaddr = it.addr; lineoffset = 0; is_jump = true;
						}

						ImGui::SameLine();

						ImGui::PushStyleColor(ImGuiCol_Button, it.enabled ? BLUE : RED);
						if (ImGui::Button("Enabled"))
							it.enabled ^= 1;
						ImGui::PopStyleColor();

						ImGui::SameLine();

						if (ImGui::Button("Delete"))
							breakpoints.erase(breakpoints.begin() + n);

						ImGui::SameLine();

						string bps = bptype & bp_read || bptype & bp_vread ? "R" : ".";
						bps += bptype & bp_write || bptype & bp_vwrite ? "W" : ".";
						bps += bptype & bp_exec ? "X" : ".";

						ImGui::Text(bps.c_str());

						ImGui::PopID();

						n++;
					}
					ImGui::ListBoxFooter();
				}
			}
			ImGui::EndChild();
		}
		ImGui::Columns(1);
		ImGui::End();
	}

	void show_memory()
	{
		if (ImGui::Begin("Memory Editor", nullptr, NO_SCROLL))
		{
			ImGui::SetWindowPos(ImVec2(528 + 5, menubarheight + 5 + 680));
			ImGui::SetWindowSize(ImVec2(650, 270));
			if (ImGui::BeginTabBar("##mem_tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("RAM"))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
					mem_edit.DrawContents(MEM::ram.data(), MEM::ram.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("VRAM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
					mem_edit.DrawContents(MEM::vram.data(), MEM::vram.size());

					if (header.mirror == mirrortype::horizontal)
					{
						//memcpy(&MEM::vram[0x2800], &MEM::vram[0x2000], 0x400);
						//memcpy(&MEM::vram[0x2c00], &MEM::vram[0x2400], 0x400);
					}
					else if (header.mirror == mirrortype::vertical)
					{
						//memcpy(&MEM::vram[0x2400], &MEM::vram[0x2000], 0x400);
						//memcpy(&MEM::vram[0x2c00], &MEM::vram[0x2800], 0x400);
					}

					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("OAM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
					mem_edit.DrawContents(MEM::oam.data(), MEM::oam.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("ROM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
					mem_edit.DrawContents(MEM::rom.data(), MEM::rom.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("VROM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
					mem_edit.DrawContents(MEM::vrom.data(), MEM::vrom.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	void show_logger()
	{
		if (ImGui::Begin("Trace Logger", &trace_logger, 0))
		{
			u16 pc = reg.pc;
			vector<disasmentry> entries;

			for (int i = 0; i < 10; i++)
			{
				entries.push_back(get_trace_line(pc, true, false)[0]);
				pc += entries[0].size;
			}

			for (auto& entry : entries)
				ImGui::Text(entry.line.c_str());
		}
		ImGui::End();
	}

	void show_menu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			menubarheight = ImGui::GetWindowHeight();
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Load Rom"))
				{
					OPENFILENAMEA ofn;
					char filename[512]{};
					ZeroMemory(&ofn, sizeof(ofn));
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = nullptr;
					ofn.lpstrTitle = "Open Nes Rom";
					ofn.lpstrFilter = "Nes Roms (*.nes)\0*.nes";
					ofn.lpstrFile = filename;
					ofn.nMaxFile = MAX_PATH;
					ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
					ofn.lpstrDefExt = "nes";
					std::vector<uint8_t> rom;

					if (GetOpenFileNameA(&ofn))
					{
						if (!MEM::load_rom((char*)ofn.lpstrFile))
						{
							cpu.state = cstate::debugging;
							MEM::rom.clear();
						}
						else
						{
							if (MEM::rom_loaded && !debug_enable)
								cpu.state = cstate::running;
						}
					}
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Emulator"))
			{
				if (ImGui::MenuItem("Run", nullptr, false, &emu_run))
					run_emu();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Emulator"))
			{
				if (ImGui::MenuItem("Reset", nullptr, false, &emu_reset))
					reset_emu();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Tools"))
			{
				ImGui::MenuItem("Cheats", nullptr, &cheat_opened);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Debugger", false, &debug_enable))
				{
					//resize_window();
				}
				ImGui::EndMenu();
			}
			if (debug_enable)
			{
				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("PPU Viewer", false, &ppu_enable);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("Memory Viewer", false, &mem_viewer);
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("Trace Logger", nullptr, &trace_logger);
					ImGui::EndMenu();
				}
			}

			if (ImGui::BeginMenu("Misc"))
			{
				if (ImGui::BeginMenu("Debug"))
				{
					ImGui::MenuItem("Memory Viewer", false, &mem_viewer);
					ImGui::EndMenu();
				}
				ImGui::MenuItem("Style Editor", nullptr, &style_editor);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void show_disassembly(u16 pc)
	{
		if (ImGui::BeginChild("Disassembly", ImVec2(385, 500), true, NO_SCROLL))
		{
			float mousewheel = ImGui::GetIO().MouseWheel;
			if (mousewheel != 0 && (pc + lineoffset < 0x10000))
			{
				if (ImGui::IsWindowHovered())
				{
					if (mousewheel > 0)
						lineoffset -= 6;
					else
						lineoffset += 6;
				}
			}

			pc += lineoffset;
			for (int i = 0; i < DISASSEMBLY_LINES; i++)
			{
				disasmentry entry = get_trace_line(pc, false, false)[0];
				stringstream ss;
				ss << setw(4) << hex << uppercase << setfill('0') << pc;

				ImGui::PushID(pc);
				bool bpenabled = false;
				for (auto& it : breakpoints)
				{
					if (it.enabled && it.addr == pc)
						bpenabled = true;
				}

				ImGui::PushStyleColor(ImGuiCol_Button, bpenabled ? BLUE : DEFCOLOR);
				if (ImGui::Button(ss.str().c_str(), ImVec2(BUTTON_W - 50, 0)))
					bp_add(pc, bp_exec, true);

				ImGui::PopStyleColor();
				ImGui::SameLine();

				is_pc = false;
				if (pc == reg.pc || pc == jumpaddr && is_jump)
					is_pc = true;

				if (entry.line != "")
					ImGui::Selectable(entry.line.c_str(), is_pc);

				if (follow_pc)
				{
					ImGui::SetScrollHereY(0);
					lineoffset = 0;
				}

				follow_pc = false;

				ImGui::PopID();
				pc += entry.size;
			}
		}
		ImGui::EndChild();
	}

	void show_rom_info()
	{
		if (ImGui::BeginChild("Mapper Info", ImVec2(0, 100), true, NO_SCROLL))
		{
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 120);

			ImGui::Text("%-15s", "Mapper Number"); ImGui::NextColumn(); ImGui::Text("%d", header.mappernum); ImGui::NextColumn();
			ImGui::Text("%-15s", "PRG Total"); ImGui::NextColumn(); ImGui::Text("%d", header.prgnum); ImGui::NextColumn();
			ImGui::Text("%-15s", "CHR Total"); ImGui::NextColumn(); ImGui::Text("%d", header.chrnum); ImGui::NextColumn();
			ImGui::Text("%-15s", "Mirroring"); ImGui::NextColumn(); ImGui::Text("%s", mirrornames[header.mirror]); ImGui::NextColumn();
			ImGui::Columns(1);
			ImGui::Separator();

			if (!MEM::rom_loaded && header.mappernum > 0)
			{
				ImGui::TextColored(RED, "Mapper %d not supported", header.mappernum);
			}
			ImGui::Columns(1);
			ImGui::EndChild();

			if (ImGui::BeginChild("Mapper Banks", ImVec2(0, 220), true, NO_SCROLL))
			{
				if (MEM::mapper)
				{
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 120);
					vector<u8> prg = MEM::mapper->get_prg();
					for (int i = 0; i < prg.size(); i++)
					{
						ImGui::Text("prg%02X", i); ImGui::NextColumn(); ImGui::Text("%02X", prg[i]); ImGui::NextColumn();
					}

					ImGui::Separator();

					if (header.mappernum > 0)
					{
						vector<u8> chr = MEM::mapper->get_chr();
						for (int i = 0; i < chr.size(); i++)
						{
							ImGui::Text("chr%02X", i); ImGui::NextColumn(); ImGui::Text("%02X", chr[i]); ImGui::NextColumn();
						}
					}
					set_spacing(1);

				}
				ImGui::Columns(1);
			}
		}
		ImGui::EndChild();
	}

	void cheat_dialog()
	{
		if (ImGui::Begin("Cheats", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			if (ImGui::InputTextMultiline("##cheatbox", &cheattext))
			{
				int yu = 0;
			}

			if (ImGui::Button("Add", ImVec2(BUTTON_W, 0)))
			{
				//memset(&cheattext, 0, cheattext));
			}
		}
		ImGui::End();
	}

	void open_dialog()
	{
		if (ImGui::BeginPopupModal("Add breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::PushItemWidth(90);
			if (ImGui::InputText("##bpadd", (char*)bpaddrtext, IM_ARRAYSIZE(bpaddrtext), INPUT_FLAGS))
			{
				std::istringstream ss(bpaddrtext);
				ss >> std::hex >> inputaddr;
				lineoffset = 0;
			}
			ImGui::PopItemWidth();

			set_spacing(5);

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_read ? BLUE : DEFCOLOR);
			if (ImGui::Button("Cpu Read", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_read;
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_write ? BLUE : DEFCOLOR);
			if (ImGui::Button("Cpu Write", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_write;
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_exec ? BLUE : DEFCOLOR);
			if (ImGui::Button("Cpu Exec", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_exec;
			ImGui::PopStyleColor();

			set_spacing(5);

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_vread ? BLUE : DEFCOLOR);
			if (ImGui::Button("Ppu Read", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_vread;
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_vwrite ? BLUE : DEFCOLOR);
			if (ImGui::Button("Ppu Write", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_vwrite;
			ImGui::PopStyleColor();

			set_spacing(15);

			if (ImGui::Button("OK", ImVec2(BUTTON_W, 0)))
			{
				bp_add(inputaddr, bptype, true); ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(BUTTON_W, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	void run_emu()
	{
		if (MEM::rom_loaded)
		{
			if (logging)
				log_to_file(reg.pc);

			CPU::step();
			cpu.state = cstate::running;
			is_jump = false;
		}
	}

	void reset_emu()
	{
		if (MEM::rom_loaded)
		{
			MEM::set_mapper();
			create_close_log(false);
			follow_pc = true;
			if (!debug_enable)
				cpu.state = cstate::running;
			else
				cpu.state = cstate::debugging;
		}
	}

	bool init()
	{
		IMGUI_CHECKVERSION();
		if (!ImGui::CreateContext())
			return false;

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Platform/Renderer backends
		ImGui_ImplSDL2_InitForOpenGL(SDL::window, SDL::context);
		ImGui_ImplOpenGL3_Init(SDL::glsl_version);

		return true;
	}

	void clean()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		SDL::clean();
	}
}