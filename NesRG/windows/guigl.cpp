#include "guigl.h"
#include "breakpoints.h"
#include "cpu.h"
#include "ppu.h"
#include "sdlcc.h"
#include "tracer.h"
#include "main.h"
#include "mappers.h"
#include "mem.h"

namespace GUIGL
{
	ImGui::FileBrowser fileDialog;

	void update()
	{
		glClear(GL_COLOR_BUFFER_BIT);
		show_game_view();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		show_filebrowser();
		show_menu();
		if (debug_enable)
		{
			show_ppu_debug();
			show_disassembly();
			show_memory();
		}

		// Rendering
		ImGui::Render();
		glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
		//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		//glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}

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

		if (ImGui::Begin("PPU Debug"))
		{
			if (SDL::frame_limit)
			{
				if (ImGui::BeginTabBar("##gfx_tabs", ImGuiTabBarFlags_None))
				{
					if (ImGui::BeginTabItem("Name Tables"))
					{
						if ((PPU::frame % 8) == 0)
							PPU::render_nametable();
						ImVec2 tsize = ImGui::GetContentRegionAvail();
						ImGui::Image((void*)(intptr_t)SDL::nametable, ImVec2(512, 480));
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
		}
		ImGui::End();
	}

	void show_disassembly()
	{
		u16 pc = is_jump ? jumpaddr : reg.pc;
		vector<disasmentry> entries;
		int cyc = 0;

		ImGui::Begin("Debugger", NULL, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

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

		ImGui::Text("PC:%04X A:%02X X:%02X Y:%02X P:%02X", reg.pc, reg.a, reg.x, reg.y, reg.ps);
		ImGui::Text("Scanline:%d Cycle:%3d VRAM:%04X SCRO:%04X Cpu Cycles:%d",
			PPU::scanline, PPU::cycle, lp.v, lp.t, PPU::totalcycles);
		ImGui::TextColored(pstatus.vblank ? GREEN : RED, "VBlank"); ImGui::SameLine();
		ImGui::TextColored(pstatus.sprite0hit ? GREEN : RED, "Sprite0");
		//ImGui::Text("%-15s", "MMC4 Counter"); ImGui::NextColumn(); ImGui::Text("%02X", mmc3.counter); ImGui::NextColumn();

		ImGui::Separator();

		ImGui::Text("FPS:%.1f", ImGui::GetIO().Framerate);
		ImGui::EndGroup();

		ImGui::Separator();
		set_spacing(1);

		ImGui::Columns(2);
		ImGui::BeginChild("##disasm", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
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
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("##mapperinfo", ImVec2(0, 100), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
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
		ImGui::EndChild();
		set_spacing(1);

		ImGui::BeginChild("##mapperbanks", ImVec2(0, 250), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		if (MEM::mapper)
		{
			ImGui::Columns(2);
			//ImGui::SetColumnWidth(0, 60);
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
		}
		ImGui::Columns(1);
		ImGui::EndChild();

		//ImGui::Separator();
		set_spacing(1);

		ImGui::BeginChild("##breakpoints", ImVec2(0, 0), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
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
		ImGui::EndChild();
		ImGui::Columns(1);
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
					fs::path game_dir = "D:\\Emulators+Hacking\\NES\\ppu_nmi";
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
				ImGui::MenuItem("Style Editor", nullptr, &style_editor);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void show_filebrowser()
	{
		fileDialog.Display();
		fileDialog.SetWindowSize(400, 600);

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
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

			// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

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