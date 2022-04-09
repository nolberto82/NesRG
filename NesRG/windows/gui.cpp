#include "gui.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "sdlcore.h"
#include "tracer.h"
#include "main.h"
#include "mappers.h"

ImGui::FileBrowser fileDialog;

void gui_update()
{
	ImGui_ImplSDLRenderer_NewFrame();
	ImGui_ImplSDL2_NewFrame(sdl.window);
	ImGui::NewFrame();

	fileDialog.Display();
	fileDialog.SetWindowSize(500, 800);

	if (fileDialog.HasSelected())
	{
		if (!load_rom((char*)fileDialog.GetSelected().u8string().c_str()))
		{
			cpu.state = cstate::debugging;
			rom.clear();
		}
		fileDialog.ClearSelected();
	}

	ImGuiViewport* view = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(view->Pos, ImGuiCond_Once);
	ImGui::SetNextWindowSize(view->Size, ImGuiCond_Once);

	ImGui::Begin("MainWindow", NULL, MAIN_WINDOW);

	ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
	ImGui::DockSpace(dockspace_id);

	gui_show_disassembly();
	gui_show_memory();
	gui_show_display();
	gui_show_registers();
	gui_show_breakpoints();
	gui_show_logger();
	gui_show_menu();

	ImGui::End();

	if (style_editor)
		ImGui::ShowStyleEditor();

	ImGui::Render();
	SDL_SetRenderDrawColor(sdl.renderer, (u8)(clear_color.x * 255), (u8)(clear_color.y * 255), (u8)(clear_color.z * 255), (u8)(clear_color.w * 255));
	SDL_RenderClear(sdl.renderer);
	ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());
	if (sdl.frame_limit)
		SDL_framerateDelay(&sdl.fpsman);
	SDL_RenderPresent(sdl.renderer);
}

void gui_show_display()
{
	if (ImGui::Begin("Display"))
	{
		if (cpu.state == cstate::scanlines || cpu.state == cstate::cycles)
			sdl_frame(ppu.screen_pixels, cpu.state);
		ImGui::Image((void*)sdl.screen, ImVec2(256 * 2, 240 * 1.75f));
	}
	ImGui::End();

	if (ImGui::Begin("PPU Debug"))
	{
		if (ImGui::BeginTabBar("##gfx_tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Name Tables"))
			{
				for (int i = 0; i < 4; i++)
				{
					u16 ntaddr = 0;
					switch (header.mirror)
					{
						case mirrortype::single_nt0: ntaddr = mirrornt0[i]; break;
						case mirrortype::single_nt1: ntaddr = mirrornt1[i]; break;
						case mirrortype::vertical: ntaddr = mirrorver[i] * 0x400; break;
						case mirrortype::horizontal: ntaddr = mirrorhor[i] * 0x400; break;
					}
					process_nametables(ntaddr, 0, ppu.ntable_pixels[i]);
				}

				sdl_nttable();
				//ImDrawList* draw_list = ImGui::GetWindowDrawList();

				ImGui::Image((void*)sdl.ntscreen, ImVec2(256 * 2, 240 * 2));
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Pattern Tables"))
			{
				ImDrawList* list = ImGui::GetWindowDrawList();

				process_pattern();
				ImVec2 pos = ImGui::GetCursorScreenPos();
				float startx = pos.x;
				float starty = pos.y;
				float x = startx;
				float y = starty;

				for (int i = 0; i < 2; i++)
				{
					for (int j = 0; j < PATTERN_WIDTH * PATTERN_HEIGHT; j++)
					{
						u8 r = ppu.pattern_pixels[i][j] & 0xff;
						u8 g = ppu.pattern_pixels[i][j] >> 8;
						u8 b = ppu.pattern_pixels[i][j] >> 16;
						ImU32 color = ImColor(r, g, b);
						list->AddRectFilled(ImVec2(x, y), ImVec2(x + 2, y + 2), color);
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
					u8 r = ppu.palettes[vram[0x3f00 + i]] & 0xff;
					u8 g = ppu.palettes[vram[0x3f00 + i]] >> 8;
					u8 b = ppu.palettes[vram[0x3f00 + i]] >> 16;
					ImU32 color = ImColor(r, g, b);
					list->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 24, pos.y + 24), color);
					r = ppu.palettes[vram[0x3f10 + i]] & 0xff;
					g = ppu.palettes[vram[0x3f10 + i]] >> 8;
					b = ppu.palettes[vram[0x3f10 + i]] >> 16;
					color = ImColor(r, g, b);
					list->AddRectFilled(ImVec2(pos.x, pos.y + 48), ImVec2(pos.x + 24, pos.y + 25), color);
					pos.x += 27;
				}
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void gui_show_disassembly()
{
	u16 pc = is_jump ? jumpaddr : reg.pc;
	vector<disasmentry> entries;
	int cyc = 0;

	if (ImGui::Begin("Buttons"))
	{
		if (ImGui::Button("Load Rom", ImVec2(BUTTON_W, 0)))
		{
			fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Mapper1";
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
				if (logging)
					log_to_file(reg.pc);

				cyc = cpu_step();
				ppu_step(cyc);
				cpu.state = cstate::running; is_jump = false;
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Reset", ImVec2(BUTTON_W, 0)))
		{
			if (rom_loaded)
			{
				set_mapper();
				create_close_log(false);
				follow_pc = true;
				cpu.state = cstate::debugging;
			}
		}

		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(logging ? BLUE : DEFCOLOR));
		if (ImGui::Button("Trace Log", ImVec2(BUTTON_W, 0)))
			create_close_log(!logging);
		ImGui::PopStyleColor(1);
	}
	ImGui::End();

	if (ImGui::Begin("Disassembly", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar))
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
	ImGui::End();
}

void gui_show_memory()
{
	if (ImGui::Begin("Memory Editor", nullptr))
	{
		if (ImGui::BeginTabBar("##mem_tabs", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("RAM"))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
				mem_edit.DrawContents(ram.data(), ram.size());
				ImGui::PopStyleColor(1);
				ImGui::EndTabItem();
			}
			if ((ImGui::BeginTabItem("VRAM")))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
				mem_edit.DrawContents(vram.data(), vram.size());
				ImGui::PopStyleColor(1);
				ImGui::EndTabItem();
			}
			if ((ImGui::BeginTabItem("OAM")))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
				mem_edit.DrawContents(oam.data(), oam.size());
				ImGui::PopStyleColor(1);
				ImGui::EndTabItem();
			}
			if ((ImGui::BeginTabItem("ROM")))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
				mem_edit.DrawContents(rom.data(), rom.size());
				ImGui::PopStyleColor(1);
				ImGui::EndTabItem();
			}
			if ((ImGui::BeginTabItem("VROM")))
			{
				ImGui::PushStyleColor(ImGuiCol_Text, ALICEBLUE);
				mem_edit.DrawContents(vrom.data(), vrom.size());
				ImGui::PopStyleColor(1);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}
	ImGui::End();
}

void gui_show_registers()
{
	if (ImGui::Begin("Registers", NULL, ImGuiWindowFlags_NoScrollbar))
	{
		ImGui::BeginChild("##regs", ImVec2(300, 240));
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 125);

		ImGui::Text("%15s", "FPS"); ImGui::NextColumn(); ImGui::Text("%.1f", ImGui::GetIO().Framerate); ImGui::NextColumn();
		ImGui::Text("%15s", "PC"); ImGui::NextColumn(); ImGui::Text("%04X", reg.pc); ImGui::NextColumn();
		ImGui::Text("%15s", "SP"); ImGui::NextColumn(); ImGui::Text("%04X", reg.sp | 0x100); ImGui::NextColumn();
		ImGui::Text("%15s", "A"); ImGui::NextColumn(); ImGui::Text("%02X", reg.a); ImGui::NextColumn();
		ImGui::Text("%15s", "X"); ImGui::NextColumn(); ImGui::Text("%02X", reg.x); ImGui::NextColumn();
		ImGui::Text("%15s", "Y"); ImGui::NextColumn(); ImGui::Text("%02X", reg.y); ImGui::NextColumn();
		ImGui::Text("%15s", "Status Flag"); ImGui::NextColumn(); ImGui::Text("%02X", reg.ps); ImGui::NextColumn();
		ImGui::Text("%15s", "Scanline"); ImGui::NextColumn(); ImGui::Text("%d", ppu.scanline); ImGui::NextColumn();
		ImGui::Text("%15s", "Cycle"); ImGui::NextColumn(); ImGui::Text("%d", ppu.cycle); ImGui::NextColumn();
		ImGui::Text("%15s", "Cpu Cycles"); ImGui::NextColumn(); ImGui::Text("%d", ppu.totalcycles); ImGui::NextColumn();
		ImGui::Text("%15s", "Frames"); ImGui::NextColumn(); ImGui::Text("%d", ppu.frame); ImGui::NextColumn();
		ImGui::Text("%15s", "V Address"); ImGui::NextColumn(); ImGui::Text("%04X", lp.v); ImGui::NextColumn();
		ImGui::Text("%15s", "T Address"); ImGui::NextColumn(); ImGui::Text("%04X", lp.t); ImGui::NextColumn();
		ImGui::Text("%15s", "VBlank"); ImGui::NextColumn(); ImGui::Text("%d", pstatus.vblank); ImGui::NextColumn();
		ImGui::Text("%15s", "Sprite 0 Hit"); ImGui::NextColumn(); ImGui::Text("%d", pstatus.sprite0hit); ImGui::NextColumn();
		ImGui::Text("%15s", "mmc4 counter"); ImGui::NextColumn(); ImGui::Text("%02X", mmc3.counter); ImGui::NextColumn();

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

		ImGui::Separator();
		set_spacing(3);

		ImGui::Begin("Mapper Info", NULL, ImGuiWindowFlags_NoScrollbar);
		ImGui::Columns(2);
		if (header.mappernum > 0)
		{
			vector<u8> prg = mappers_get_prg(header.mappernum);
			for (int i = 0; i < prg.size(); i++)
			{
				ImGui::Text("prg%02X", i); ImGui::NextColumn(); ImGui::Text("%02X", prg[i]); ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);

		ImGui::Separator();

		ImGui::Columns(2);
		if (header.mappernum > 0)
		{
			vector<u8> chr = mappers_get_chr(header.mappernum);
			for (int i = 0; i < chr.size(); i++)
			{
				ImGui::Text("chr%02X", i); ImGui::NextColumn(); ImGui::Text("%02X", chr[i]); ImGui::NextColumn();
			}
		}
		ImGui::Columns(1);
		ImGui::End();

		ImGui::Begin("Rom Info", NULL, ImGuiWindowFlags_NoScrollbar);
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 130);

		ImGui::Text("%15s", "Rom Name"); ImGui::NextColumn(); ImGui::Text("%s", header.name.c_str()); ImGui::NextColumn();
		ImGui::Text("%15s", "Mapper Number"); ImGui::NextColumn(); ImGui::Text("%d", header.mappernum); ImGui::NextColumn();
		ImGui::Text("%15s", "PRG Banks"); ImGui::NextColumn(); ImGui::Text("%d", header.prgnum); ImGui::NextColumn();
		ImGui::Text("%15s", "CHR Banks"); ImGui::NextColumn(); ImGui::Text("%d", header.chrnum); ImGui::NextColumn();
		ImGui::Text("%15s", "Mirroring"); ImGui::NextColumn(); ImGui::Text("%s", mirrornames[header.mirror]); ImGui::NextColumn();

		ImGui::Columns(1);

		if (!rom_loaded)
		{
			set_spacing(5);
			ImGui::TextColored(RED, "Mapper not supported");
		}
		ImGui::End();
	}
	ImGui::End();
}

void gui_show_breakpoints()
{
	if (ImGui::Begin("Breakpoints", nullptr, ImGuiWindowFlags_NoScrollbar))
	{
		if (ImGui::Button("Add breakpoint", ImVec2(-1, 0)))
		{
			bptype = 0;
			ImGui::OpenPopup("Add breakpoint");
		}

		gui_open_dialog();

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
					it.enabled = !it.enabled;
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

void gui_show_logger()
{
	if (!trace_logger)
		return;

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

void gui_show_buttons()
{
	fileDialog.Display();
	fileDialog.SetWindowSize(500, 800);

	if (fileDialog.HasSelected())
	{
		//ppu_reset(); clear_pixels();
		if (!load_rom((char*)fileDialog.GetSelected().u8string().c_str()))
		{
			cpu.state = cstate::debugging;
			rom.clear();
		}
		fileDialog.ClearSelected();
	}

	if (ImGui::Begin("Buttons", NULL, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		if (ImGui::Button("Load Rom", ImVec2(-1, 0)))
		{
			fs::path game_dir = "D:\\Emulators+Hacking\\NES\\Mapper1";
			fileDialog.SetTitle("Load Nes Rom");
			fileDialog.SetTypeFilters({ ".nes" });
			fileDialog.SetPwd(game_dir);
			fileDialog.Open();
		}

		set_spacing(15);

		if (ImGui::Button("Run", ImVec2(-1, 0)))
		{
			if (rom_loaded)
			{
				if (logging)
					log_to_file(reg.pc);

				int cyc = cpu_step();
				ppu_step(cyc);
				cpu.state = cstate::running; is_jump = false;
			}
		}

		set_spacing(15);

		if (ImGui::Button("Reset", ImVec2(-1, 0)))
		{
			if (rom_loaded)
			{
				set_mapper();
				create_close_log(false);
				follow_pc = true;
				cpu.state = cstate::debugging;
			}
		}

		set_spacing(15);

		if (ImGui::Button("Dump VRAM", ImVec2(-1, 0)))
		{
			if (rom_loaded)
			{
				ofstream file("vram.bin", ios::binary);
				file.write((char*)vram.data(), vram.size());
				file.close();
			}
		}

		set_spacing(15);

		if (ImGui::Button("Jump To", ImVec2(-1, 0)))
			is_jump = true;

		//Jump To
		ImGui::Indent((ImGui::GetCurrentWindow()->ItemWidthDefault + 50) / 2);
		ImGui::PushItemWidth(34);
		if (ImGui::InputText("##jumpto", (char*)jumpto, IM_ARRAYSIZE(jumpto), INPUT_FLAGS))
		{
			std::istringstream ss(jumpto);
			ss >> std::hex >> jumpaddr;
		}
		ImGui::PopItemWidth();
	}
	ImGui::End();
}

void gui_show_menu()
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Trace Logger", NULL, &trace_logger);
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("Debug"))
		{
			ImGui::MenuItem("Style Editor", NULL, &style_editor);
			ImGui::EndMenu();
		}
		ImGui::EndMenuBar();
	}
}

void gui_open_dialog()
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