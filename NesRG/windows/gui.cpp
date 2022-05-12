#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "sdlgfx.h"
#include "tracer.h"
#include "main.h"
#include "mappers.h"
#include "mem.h"

namespace GUI
{
	ImGui::FileBrowser fileDialog;

	void update(ImGuiIO io)
	{
		ImGuiViewport* v = ImGui::GetMainViewport();
		// Start the Dear ImGui frame
		ImGui_ImplSDLRenderer_NewFrame();
		ImGui_ImplSDL2_NewFrame(SDL::window);
		ImGui::NewFrame();

		fileDialog.Display();
		//if (!debug_enable)
		//{
		//	int w, h;
		//	SDL_GetWindowSize(SDL::window, &w, &h);
		//	fileDialog.SetWindowSize(w, h - 50);
		//}
		//else
		fileDialog.SetWindowSize(400, 400);

		if (fileDialog.HasSelected())
		{
			if (!MEM::load_rom((char*)fileDialog.GetSelected().u8string().c_str()))
			{
				cpu.state = cstate::debugging;
				MEM::rom.clear();
			}
			else
			{
				if (MEM::rom_loaded && !debug_enable)
					cpu.state = cstate::running;
			}
			fileDialog.ClearSelected();
			emu_rom = false;
		}

		if (debug_enable)
		{
			show_buttons();
			show_disassembly();
			show_registers();
			show_breakpoints();
			show_ppu_debug();
			show_memory();
			if (trace_logger)
				show_logger();
		}

		show_menu();

		if (debug_enable)
		{
			SDL::draw_frame(PPU::screen_pixels, 0);
			ImGui::Begin("Display", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::SetWindowSize(ImVec2(256 * 2, 208 * 2));
			ImVec2 pos = ImGui::GetWindowPos();
			pos.x = pos.x + 512 + 2;
			ImGui::SetNextWindowPos(pos);
			ImGui::Image((void*)SDL::screen, ImVec2(256 * 2, 200 * 2));
			ImGui::End();
		}

		if (style_editor)
			ImGui::ShowStyleEditor();

		// Rendering
		ImGui::Render();
		SDL_SetRenderDrawColor(SDL::renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255), (u8)(clear_color.z * 255), (u8)(clear_color.w * 255));
		SDL_RenderClear(SDL::renderer);

		if (!debug_enable)
		{
			SDL::draw_frame(PPU::screen_pixels, 0);
		}

		ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
		//if (SDL::frame_limit)
		//	SDL_framerateDelay(&SDL::fpsman);
		SDL_RenderPresent(SDL::renderer);
	}

	void show_ppu_debug()
	{
		if (ImGui::Begin("PPU Debug"))
		{
			if (ImGui::BeginTabBar("##gfx_tabs", ImGuiTabBarFlags_None))
			{
				if (ImGui::BeginTabItem("Name Tables"))
				{
					ImDrawList* list = ImGui::GetWindowDrawList();
					ImVec2 p = ImGui::GetCursorScreenPos();

					for (int i = 0; i < 4; i++)
					{
						u16 ntaddr = 0;
						memset(PPU::ntable_pixels[i], 0, sizeof(PPU::ntable_pixels[i]));
						switch (header.mirror)
						{
						case mirrortype::single_nt0: ntaddr = MEM::mirrornt0[i]; break;
						case mirrortype::single_nt1: ntaddr = MEM::mirrornt1[i]; break;
						case mirrortype::vertical: ntaddr = MEM::mirrorver[i] * 0x400; break;
						case mirrortype::horizontal: ntaddr = MEM::mirrorhor[i] * 0x400; break;
						}
						PPU::render_nametables(ntaddr, 0, PPU::ntable_pixels[i]);
					}

					SDL::draw_nttable();
					//ImVec2 tsize = ImGui::GetContentRegionAvail();
					ImGui::Image((void*)SDL::ntscreen, ImVec2(256 * 2, 240 * 2));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Pattern Tables"))
				{
					ImDrawList* list = ImGui::GetWindowDrawList();

					PPU::render_pattern();
					ImVec2 pos = ImGui::GetCursorScreenPos();
					float startx = pos.x;
					float starty = pos.y;
					float x = startx;
					float y = starty;

					for (int i = 0; i < 2; i++)
					{
						for (int j = 0; j < PATTERN_WIDTH * PATTERN_HEIGHT; j++)
						{
							u8 r = PPU::pattern_pixels[i][j] & 0xff;
							u8 g = PPU::pattern_pixels[i][j] >> 8;
							u8 b = PPU::pattern_pixels[i][j] >> 16;
							list->AddRectFilled(ImVec2(x, y), ImVec2(x + 2, y + 2), ImColor(r, g, b));
							x += 2;
							if ((j + 1) % PATTERN_WIDTH == 0)
							{
								if (i == 0)
									x = startx;
								else
									x = startx + PATTERN_WIDTH * 2 + 5;
								y += 2;
							}
						}
						x = startx + PATTERN_WIDTH * 2 + 5; y = starty;
					}

					pos = ImGui::GetCursorScreenPos();
					startx = pos.x;
					starty = pos.y += PATTERN_HEIGHT * 2 + 15;
					for (int i = 0; i < 16; i++)
					{
						u8 r = PPU::palettes[MEM::vram[0x3f00 + i]] & 0xff;
						u8 g = PPU::palettes[MEM::vram[0x3f00 + i]] >> 8;
						u8 b = PPU::palettes[MEM::vram[0x3f00 + i]] >> 16;
						list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 24, pos.y + 24), ImColor(r, g, b));
						r = PPU::palettes[MEM::vram[0x3f10 + i]] & 0xff;
						g = PPU::palettes[MEM::vram[0x3f10 + i]] >> 8;
						b = PPU::palettes[MEM::vram[0x3f10 + i]] >> 16;
						list->AddRectFilled(ImVec2(pos.x, pos.y + 48), ImVec2(pos.x + 24, pos.y + 25), ImColor(r, g, b));
						pos.x += 27;
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
		}
		ImGui::End();
	}

	void show_disassembly()
	{
		u16 pc = is_jump ? jumpaddr : reg.pc;
		vector<disasmentry> entries;
		int cyc = 0;

		if (ImGui::Begin("Disassembly", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
		{
			//ImGui::SetNextWindowSize(ImVec2(1000, -1));
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
		ImGui::End();
	}

	void show_memory()
	{
		if (ImGui::Begin("Memory Editor", nullptr))
		{
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

	void show_registers()
	{
		if (ImGui::Begin("Registers", NULL, ImGuiWindowFlags_NoScrollbar))
		{
			ImGui::BeginChild("##regs", ImVec2(300, 240));
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100);

			ImGui::Text("%-15s", "PC"); ImGui::NextColumn(); ImGui::Text("%04X", reg.pc); ImGui::NextColumn();
			ImGui::Text("%-15s", "SP"); ImGui::NextColumn(); ImGui::Text("%04X", reg.sp | 0x100); ImGui::NextColumn();
			ImGui::Text("%-15s", "A"); ImGui::NextColumn(); ImGui::Text("%02X", reg.a); ImGui::NextColumn();
			ImGui::Text("%-15s", "X"); ImGui::NextColumn(); ImGui::Text("%02X", reg.x); ImGui::NextColumn();
			ImGui::Text("%-15s", "Y"); ImGui::NextColumn(); ImGui::Text("%02X", reg.y); ImGui::NextColumn();
			ImGui::Text("%-15s", "Status Flag"); ImGui::NextColumn(); ImGui::Text("%02X", reg.ps); ImGui::NextColumn();
			ImGui::Text("%-15s", "Scanline"); ImGui::NextColumn(); ImGui::Text("%d", PPU::scanline); ImGui::NextColumn();
			ImGui::Text("%-15s", "Cycle"); ImGui::NextColumn(); ImGui::Text("%d", PPU::cycle); ImGui::NextColumn();
			ImGui::Text("%-15s", "Cpu Cycles"); ImGui::NextColumn(); ImGui::Text("%d", PPU::totalcycles); ImGui::NextColumn();
			ImGui::Text("%-15s", "Frames"); ImGui::NextColumn(); ImGui::Text("%d", PPU::frame); ImGui::NextColumn();
			ImGui::Text("%-15s", "V Address"); ImGui::NextColumn(); ImGui::Text("%04X", lp.v); ImGui::NextColumn();
			ImGui::Text("%-15s", "T Address"); ImGui::NextColumn(); ImGui::Text("%04X", lp.t); ImGui::NextColumn();
			ImGui::Text("%-15s", "VBlank"); ImGui::NextColumn(); ImGui::Text("%d", pstatus.vblank); ImGui::NextColumn();
			ImGui::Text("%-15s", "Sprite 0 Hit"); ImGui::NextColumn(); ImGui::Text("%d", pstatus.sprite0hit); ImGui::NextColumn();
			//ImGui::Text("%-15s", "MMC4 Counter"); ImGui::NextColumn(); ImGui::Text("%02X", mmc3.counter); ImGui::NextColumn();
			ImGui::Columns(1);
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::Begin("Flags", NULL, ImGuiWindowFlags_NoScrollbar);
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
			ImGui::End();

			ImGui::Begin("Rom/Mapper Info", NULL, ImGuiWindowFlags_NoScrollbar);
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 60);

			ImGui::Text("%-15s", "Mapper"); ImGui::NextColumn(); ImGui::Text("%d", header.mappernum); ImGui::NextColumn();
			ImGui::Text("%-15s", "PRG"); ImGui::NextColumn(); ImGui::Text("%d", header.prgnum); ImGui::NextColumn();
			ImGui::Text("%-15s", "CHR"); ImGui::NextColumn(); ImGui::Text("%d", header.chrnum); ImGui::NextColumn();
			ImGui::Text("%-15s", "Mirror"); ImGui::NextColumn(); ImGui::Text("%s", mirrornames[header.mirror]); ImGui::NextColumn();
			ImGui::Columns(1);
			set_spacing(5);

			if (!MEM::rom_loaded && header.mappernum > 0)
			{
				ImGui::TextColored(RED, "Mapper %d not supported", header.mappernum);
			}

			ImGui::Separator();
			set_spacing(10);

			ImGui::Separator();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 60);

			if (MEM::mapper)
			{
				vector<u8> prg = MEM::mapper->get_prg();
				for (int i = 0; i < prg.size(); i++)
				{
					ImGui::Text("prg%02X", i); ImGui::NextColumn(); ImGui::Text("%02X", prg[i]); ImGui::NextColumn();
				}
				ImGui::Columns(1);

				ImGui::Separator();
				set_spacing(10);
				ImGui::Separator();
				ImGui::Columns(2);

				if (header.mappernum > 0)
				{
					vector<u8> chr = MEM::mapper->get_chr();
					for (int i = 0; i < chr.size(); i++)
					{
						ImGui::Text("chr%02X", i); ImGui::NextColumn(); ImGui::Text("%02X", chr[i]); ImGui::NextColumn();
					}
				}
			}
			ImGui::Columns(1);
			ImGui::End();
		}
		ImGui::End();
	}

	void show_breakpoints()
	{
		if (ImGui::Begin("Breakpoints", nullptr, ImGuiWindowFlags_NoScrollbar))
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
						it.enabled = ~it.enabled;
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

	void show_buttons()
	{
		if (ImGui::Begin("Buttons"))
		{
			ImGui::Text("%s", header.name.c_str());
			set_spacing(5);

			if (ImGui::Button("Run", ImVec2(BUTTON_W, 0)))
			{
				run_emu();
			}

			ImGui::SameLine();

			if (ImGui::Button("Reset", ImVec2(BUTTON_W, 0)))
			{
				reset_emu();
			}

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(logging ? BLUE : DEFCOLOR));
			if (ImGui::Button("Trace Log", ImVec2(BUTTON_W, 0)))
				create_close_log(!logging);
			ImGui::PopStyleColor(1);

			ImGui::SameLine();

			ImGui::Text("%15s%.1f", "FPS - ", ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}

	void show_menu()
	{
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Load Rom"))
				{
					fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Mapper1";
					fileDialog.SetTitle("Load Nes Rom");
					fileDialog.SetTypeFilters({ ".nes" });
					fileDialog.SetPwd(game_dir);
					fileDialog.Open();
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
			if (ImGui::BeginMenu("Debug"))
			{
				if (ImGui::MenuItem("Debugger", false, &debug_enable))
				{
					int w, h;
					int x, y;
					SDL_GetWindowSize(SDL::window, &w, &h);
					SDL_GetWindowPosition(SDL::window, &x, &y);
					if (debug_enable)
					{
						SDL_SetWindowSize(SDL::window, 1300, 980);
						SDL_SetWindowPosition(SDL::window, x, y);
					}
					else if (!debug_enable)
					{
						SDL_SetWindowSize(SDL::window, NES_SCREEN_WIDTH * 2, NES_SCREEN_HEIGHT * 2);
					}
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
				ImGui::MenuItem("Style Editor", nullptr, &style_editor);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
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
		if (ImGui::CreateContext())
		{
			ImGui_ImplSDL2_InitForSDLRenderer(SDL::window, SDL::renderer);
			ImGui_ImplSDLRenderer_Init(SDL::renderer);

			ImGui::StyleColorsLight();
			ImGui::StyleColorsDark();
			ImGuiStyle* style = &ImGui::GetStyle();
			ImVec4 windowbgcol = ImVec4(0, 0, 0, 180 / 255.0f);
			style->ItemSpacing = ImVec2(8, 1);
			style->FrameBorderSize = 1.0f;

			return true;
		}
		return false;
	}
}