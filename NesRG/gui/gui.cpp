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

	void update()
	{
		ImGuiStyle& style = ImGui::GetStyle();
		style.FrameBorderSize = 1.0f;

		if (light_mode)
			ImGui::StyleColorsLight();
		else
			ImGui::StyleColorsDark();

		if (!debug_enable)
			show_game_view();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		show_menu();

		if (cheat_opened)
			cheat_dialog();

		if (style_editor)
			ImGui::ShowStyleEditor();

		if (debug_enable)
		{
			show_ppu_debug();
			show_debugger();
			show_memory();

			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
			if (ImGui::Begin("Display", nullptr, NO_SCROLL))
			{
				ImGui::SetWindowPos(ImVec2(5, menubarheight + 5));
				ImGui::SetWindowSize(ImVec2(512, 400));
				ImVec2 tsize = ImGui::GetContentRegionAvail();
				SDL::render_screen_debug(SDL::screen, PPU::screen_pix.data(), 256, 240, menubarheight);
				ImGui::Image((void*)SDL::screen, tsize);
			}
			ImGui::End();
			ImGui::PopStyleColor();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);
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
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 1));
		if (ImGui::Begin("PPU Debug", nullptr, NO_SCROLL))
		{
			ImGui::SetWindowPos(ImVec2(5, menubarheight + 5 + 410));
			if (ImGui::BeginTabBar("##gfx_tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Name Tables"))
				{
					if ((PPU::frame % 8) == 0 || PPU::frame == 1)
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
		}
		ImGui::End();
		ImGui::PopStyleColor();
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

			ImVec4 colorregs = BLUE;

			if (!light_mode)
				colorregs = GREEN;

			u8 shift = 0x80;
			for (int i = 0; i < 8; i++)
			{
				char* c = (char*)flag_names[i];
				bool f = (reg.ps & (shift >> i));
				ImGui::TextColored(f ? colorregs : RED, "%c", c);
				if (i < 7)
					ImGui::SameLine();
			}

			ImGui::TextColored(pstatus.vblank ? colorregs : RED, "VBlank"); ImGui::SameLine();
			ImGui::TextColored(pstatus.sprite0hit ? colorregs : RED, "Sprite0");; ImGui::SameLine();
			ImGui::TextColored(pmask.background ? colorregs : RED, "BgStatus");; ImGui::SameLine();
			ImGui::TextColored(pmask.sprite ? colorregs : RED, "SprStatus");

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

						ImVec4 color = GREEN;
						if (!light_mode)
							color = BLUE;

						ImGui::PushStyleColor(ImGuiCol_Button, it.enabled ? color : RED);
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
			ImVec4 color = light_mode ? BLUE : ALICEBLUE;
			ImGui::SetWindowPos(ImVec2(528 + 5, menubarheight + 5 + 680));
			ImGui::SetWindowSize(ImVec2(650, 270));
			if (ImGui::BeginTabBar("##mem_tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("RAM"))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, color);
					mem_edit.DrawContents(MEM::ram.data(), MEM::ram.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("VRAM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, color);
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
					ImGui::PushStyleColor(ImGuiCol_Text, color);
					mem_edit.DrawContents(MEM::oam.data(), MEM::oam.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("ROM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, color);
					mem_edit.DrawContents(MEM::rom.data(), MEM::rom.size());
					ImGui::PopStyleColor(1);
					ImGui::EndTabItem();
				}
				if ((ImGui::BeginTabItem("VROM")))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, color);
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
				ImGui::MenuItem("Debugger", false, &debug_enable);
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
				ImGui::MenuItem("Theme Mode", false, &light_mode);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Misc"))
			{
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

				ImVec4 color = GREEN;
				if (!light_mode)
					color = BLUE;

				ImGui::PushStyleColor(ImGuiCol_Button, bpenabled ? color : DEFCOLOR);
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
		if (ImGui::Begin("Cheats", &cheat_opened, ImGuiWindowFlags_AlwaysAutoResize))
		{
			int sel = -1;
			int seladdr = -1;
			static int selected = 0;
			if (ImGui::BeginChild("Cheats", ImVec2(430, 300), true, NO_SCROLL))
			{
				if (ImGui::ListBoxHeader("##cht", ImVec2(-1, -1)))
				{
					int n = 0;
					ImGui::Columns(2);
					ImGui::SetColumnWidth(0, 330);
					ImGui::SetColumnWidth(1, 85);
					for (auto& it : MEM::cheats)
					{
						ImGui::PushID(n);

						ImGui::AlignTextToFramePadding();
						ImGui::Checkbox("##cheatenabled", &it.enabled);
						ImGui::SameLine();
						if (ImGui::Selectable(it.name.c_str(), selected == n))
						{
							selected = n;
							cheatstr = it.name + '\n';
							for (int i = 0; i < it.gglines.size(); i++)
							{
								cheatstr += it.gglines[i] + '\n';
							}
						}
						ImGui::SameLine();
						ImGui::NextColumn();
						ImGui::AlignTextToFramePadding();
						if (ImGui::Button("Delete"))
						{
							if (MEM::cheats.size() > 0)
							{
								MEM::cheats.erase(MEM::cheats.begin() + n);
								MEM::mapper->update();
							}
						}
						ImGui::NextColumn();
						ImGui::PopID();

						n++;
					}
					ImGui::Columns(1);
					ImGui::ListBoxFooter();
				}
			}
			ImGui::EndChild();

			ImGui::AlignTextToFramePadding();
			ImGui::PushItemWidth(-1);
			ImGui::Text("Cheat Code Input");
			ImGui::PopItemWidth();
			ImGui::InputTextMultiline("##chttext", &cheatstr, ImVec2(-1, 200), INPUT_FLAGS);

			if (ImGui::BeginPopupContextItem("##chttext", ImGuiMouseButton_Right))
			{
				if (ImGui::MenuItem("Paste", nullptr, false))
				{
					cheatstr = ImGui::GetClipboardText();
				}
				ImGui::EndPopup();
			}

			ImGui::BeginDisabled(!MEM::rom_loaded);
			if (ImGui::Button("Add", ImVec2(BUTTON_W, 0)))
			{
				vector<string> data;
				istringstream iss(cheatstr);
				string s;
				bool name = false;
				int id = 0;

				while (getline(iss, s))
				{
					while (s.find(" ") == 0)
						s.erase(0, 1);

					if (!s.empty() && s[s.size() - 1] == '\r')
						s.erase(s.size() - 1);

					if (!name)
					{
						snprintf(cheatname, sizeof(cheatname), "%s", s.c_str());
						name = true;
						continue;
					}

					data.push_back(s);
				}

				Cheats c;
				CheatLine l;
				for (int i = 0; i < data.size(); i++)
				{
					if (data[i].empty())
						continue;

					if (decrypt_genie(data[i].c_str()) == 0)
					{
						c.id = MEM::cheats.size();
						c.name = cheatname;
						l.addr = strlen(cheataddr) ? (u16)stoul(cheataddr, nullptr, 16) : 0;
						l.value = strlen(cheatval) ? (u8)stoul(cheatval, nullptr, 16) : 0;
						l.compare = strlen(cheatcmp) && strlen(s.c_str()) == 8 ? (u8)stoul(cheatcmp, nullptr, 16) : -1;
						l.size = strlen(cheatgenie);
						c.gglines.push_back(data[i]);
						c.lines.push_back(l);
						c.enabled = 1;
					}
				}
				MEM::cheats.push_back(c);
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			ImGui::BeginDisabled(MEM::cheats.size() == 0);
			if (ImGui::Button("Save", ImVec2(BUTTON_W, 0)))
			{
				string temp = get_exec_path();

				if (!fs::is_directory(temp + "/cheats"))
					fs::create_directory(temp + "/cheats");

				int num = 0;
				ofstream chtfile(temp + "/cheats/" + header.name + ".cht");
				for (const auto& cht : MEM::cheats)
				{
					chtfile << cht.name << "," << cht.enabled << "\n";
					for (const auto& line : cht.lines)
					{
						if (strlen(cheatname) == 0)
						{
							cheatname[0] = 0x20;
							cheatname[1] = 0x00;
						}

						chtfile << hex << setw(4) << setfill('0') << line.addr << ",";
						chtfile << hex << setw(2) << setfill('0') << (int)line.value << ",";
						chtfile << hex << setw(2) << setfill('0') << (line.compare & 0xff) << "\n";

						num++;
					}
					chtfile << "\n";
				}
				chtfile.close();
			}
			ImGui::EndDisabled();

			ImGui::SameLine();

			ImGui::BeginDisabled(!MEM::rom_loaded);
			if (ImGui::Button("Load", ImVec2(BUTTON_W, 0)))
			{
				load_cheats();
			}
		}
		ImGui::EndDisabled();
		ImGui::End();
	}

	void open_dialog()
	{
		if (ImGui::BeginPopupModal("Add breakpoint", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImVec4 color = GREEN;
			if (!light_mode)
				color = BLUE;

			ImGui::PushItemWidth(90);
			if (ImGui::InputText("##bpadd", (char*)bpaddrtext, IM_ARRAYSIZE(bpaddrtext), INPUT_FLAGS))
			{
				std::istringstream ss(bpaddrtext);
				ss >> std::hex >> inputaddr;
				lineoffset = 0;
			}
			ImGui::PopItemWidth();

			set_spacing(5);

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_read ? color : DEFCOLOR);
			if (ImGui::Button("Cpu Read", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_read;
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_write ? color : DEFCOLOR);
			if (ImGui::Button("Cpu Write", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_write;
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_exec ? color : DEFCOLOR);
			if (ImGui::Button("Cpu Exec", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_exec;
			ImGui::PopStyleColor();

			set_spacing(5);

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_vread ? color : DEFCOLOR);
			if (ImGui::Button("Ppu Read", ImVec2(BUTTON_W, 0)))
				bptype ^= bp_vread;
			ImGui::PopStyleColor();
			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, bptype & bp_vwrite ? color : DEFCOLOR);
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

	int decrypt_genie(const char* code)
	{
		vector<u8> res;
		memset(&cheatcmp, 0, sizeof(cheatcmp));

		for (int i = 0; i < strlen(code); i++)
		{
			char c = genieletters.find(code[i]);
			if (c == -1)
				return -1;
			res.push_back(c);
		}

		if (strlen(code) == 6 || strlen(code) == 8)
		{
			u16 addr = (res[3] & 7) << 12 | (res[5] & 7) << 8
				| (res[2] & 7) << 4 | (res[4] & 7) | (res[4] & 8) << 8
				| (res[1] & 8) << 4 | (res[3] & 8);
			snprintf(cheataddr, sizeof(cheataddr), "%04X", addr | 0x8000);

			if (strlen(code) == 6)
			{
				u8 val = (res[1] & 7) << 4 | (res[0] & 8) << 4
					| res[0] & 7 | res[5] & 8;
				snprintf(cheatval, sizeof(cheatval), "%02X", val);
			}
			else
			{
				u8 val = (res[1] & 7) << 4 | (res[0] & 8) << 4
					| res[0] & 7 | res[7] & 8;
				snprintf(cheatval, sizeof(cheatval), "%02X", val);

				u8 cmp = (res[7] & 7) << 4 | (res[6] & 8) << 4
					| res[6] & 7 | res[5] & 8;
				snprintf(cheatcmp, sizeof(cheatcmp), "%02X", cmp);
			}
		}
		return 0;
	}

	int encrypt_genie(const char* code)
	{
		vector<u8> res;
		memset(&cheatcmp, 0, sizeof(cheatcmp));

		for (int i = 0; i < strlen(code); i++)
		{
			char c = genieletters.find(code[i]);
			if (c == -1)
				return -1;
			res.push_back(c);
		}

		if (strlen(code) == 6 || strlen(code) == 8)
		{
			u16 addr = (res[3] & 7) << 12 | (res[5] & 7) << 8
				| (res[2] & 7) << 4 | (res[4] & 7) | (res[4] & 8) << 8
				| (res[1] & 8) << 4 | (res[3] & 8);
			snprintf(cheataddr, sizeof(cheataddr), "%04X", addr | 0x8000);

			if (strlen(code) == 6)
			{
				u8 val = (res[1] & 7) << 4 | (res[0] & 8) << 4
					| res[0] & 7 | res[5] & 8;
				snprintf(cheatval, sizeof(cheatval), "%02X", val);
			}
			else
			{
				u8 val = (res[1] & 7) << 4 | (res[0] & 8) << 4
					| res[0] & 7 | res[7] & 8;
				snprintf(cheatval, sizeof(cheatval), "%02X", val);

				u8 cmp = (res[7] & 7) << 4 | (res[6] & 8) << 4
					| res[6] & 7 | res[5] & 8;
				snprintf(cheatcmp, sizeof(cheatcmp), "%02X", cmp);
			}
		}
		return 0;
	}

	void load_cheats()
	{
		string temp = get_exec_path();

		if (fs::exists(temp + "/cheats/" + header.name + ".cht"))
		{
			ifstream chtfile(temp + "/cheats/" + header.name + ".cht");
			string s;
			stringstream ss;

			MEM::cheats.swap(vector<Cheats>());

			while (getline(chtfile, s, '\n'))
			{
				Cheats c;
				stringstream ss1(s);
				getline(ss1, s, ',');
				c.name = s;
				getline(ss1, s, ',');
				c.enabled = stoul(s.c_str(), nullptr);
				while (getline(chtfile, s, '\n'))
				{
					if (s.empty())
						break;

					CheatLine l;
					stringstream ss(s);
					char* ptr;
					getline(ss, s, ',');
					l.addr = (u16)strtoul(s.c_str(), &ptr, 16);
					getline(ss, s, ',');
					l.value = (u8)strtoul(s.c_str(), &ptr, 16);
					getline(ss, s, ',');
					l.compare = (u8)strtoul(s.c_str(), &ptr, 16);
					c.lines.push_back(l);
				}

				if (c.lines.size())
					MEM::cheats.push_back(c);
			}

			chtfile.close();
		}
	}

	bool init()
	{
		IMGUI_CHECKVERSION();
		if (!ImGui::CreateContext())
			return false;

		string path = "/assets/Consolas.ttf";
		string fpath = fs::current_path().generic_string().c_str();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		//io.Fonts->Fonts.Size = 15;
		//ImFont* font1 = io.Fonts->AddFontFromFileTTF(path.c_str(), 14);
		//ImFont* font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(font_compressed_data_base85, 13);
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.GetClipboardTextFn
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