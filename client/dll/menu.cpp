#define STB_IMAGE_IMPLEMENTATION

#include "menu.h"

#include "globals.h"
#include "usermsg.h"
#include "file_system.h"
#include "utils.h"

#include <functional>
#include <algorithm>
#include <sstream>

#include "imgui/imgui.h"
#include "imgui/imgui_hotkey.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"

DWORD ColorFlags = ImGuiColorEditFlags_NoOptions | ImGuiColorEditFlags_NoDragDrop |
ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip |
ImGuiColorEditFlags_NoSidePreview;

const char* gs_items[] = { "None", "GS", "JumpGS", "SGS", "JumpSGS" };
const char* killeffect_items[] = { "None", "Yellow Splash", "White Splash", "Red Particles", "Collapse", "Implosion" };
const char* chams_items[] = { "None", "Coloring", "Shine" };

char cmd_buf[64] = "";

// function to interpret commands entered with hack's command line
void CMD_Interpreter() {
	std::string str(cmd_buf);
	size_t space_pos = str.find(" ");

	// if space found, probably ingame cvar is entered
	if (space_pos) {
		str = str.substr(0, space_pos);

		// check is cvar is exists
		cvar_s* c = g::pEngine->GetCvarPointer(str.c_str());

		// some cvars protected by flags, and can't be modified
		// while we unset flags
		if (c)
			c->flags = 0;
	}

	// pass entered command to the game command line
	g::Engine.ClientCmd(cmd_buf);
	memset(cmd_buf, 0, 64);
}

WNDPROC OrigWndProc = NULL;
IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HkWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// InFocus check is needed to prevent work of GS/Bhop/AimBot
	// when game is out of focus, but feature hotkey is pressed
	if (uMsg == WM_SETFOCUS)
		g::InFocus = true;
	if (uMsg == WM_KILLFOCUS)
		g::InFocus = false;

	if (menu::IsVisible() && ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProcA(OrigWndProc, hwnd, uMsg, wParam, lParam);
}

enum class MenuTabs {
	NONE,

	AIMBOT,
	VISUAL,
	KREEDZ,
	MISC,
};

enum class MenuSubTabs {
	NONE,

	VISUAL_REMOVAL,
	VISUAL_ESP,
	VISUAL_OTHER,
};

const float HALF_WIDTH_OF_TAB = 117.5;
const size_t DEFAULT_WINDOW_WIDTH = 1024;
const size_t DEFAULT_WINDOW_HEIGTH = 768;

namespace menu {
	bool is_visible;

	ImFont* little_font, * big_font;
	ImVec2 cursor_pos, menu_pos;

	ImVec2 hovered_tab_pos, active_tab_pos;
	std::string hovered_tab_name, active_tab_name;

	MenuTabs cur_tab = MenuTabs::NONE;
	MenuSubTabs cur_subab = MenuSubTabs::NONE;

	void UpdateImGUIColors() {
		const ColorEntry& c = g::Settings.Visual.MenuColor;

		auto clr_alpha = [&c](float a) -> ImVec4 {
			return ImVec4(c.r, c.g, c.b, a);
		};

		auto clr_brightness = [&c](float d) ->ImVec4 {
			float* c_ = c.toFloat();
			return ImVec4(c_[0] * d, c_[1] * d, c_[2] * d, c_[3]);
		};

		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Header] = clr_alpha(0.5);
		colors[ImGuiCol_ScrollbarGrab] = clr_alpha(1.0);
		colors[ImGuiCol_HeaderActive] = clr_alpha(0.3);

		colors[ImGuiCol_FrameBg] = clr_brightness(0.5);
		colors[ImGuiCol_FrameBgHovered] = clr_brightness(0.65);
		colors[ImGuiCol_FrameBgActive] = clr_brightness(0.75);

		colors[ImGuiCol_Button] = clr_brightness(0.5);
		colors[ImGuiCol_ButtonHovered] = clr_brightness(0.65);
		colors[ImGuiCol_ButtonActive] = clr_brightness(0.75);

		colors[ImGuiCol_CheckMark] = clr_brightness(1);
	}

	void Initialize() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(FindWindowA(NULL, "Counter-Strike Nexon: Studio"));
		ImGui_ImplOpenGL2_Init();

		ImGui::StyleColorsDark();

		ImGui::GetStyle().ScrollbarSize = 10.f;
		ImGui::GetStyle().Colors[ImGuiCol_Border] = ImVec4(0.f, 0.f, 0.f, 0.f);
		ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
		ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
		ImGui::GetStyle().WindowPadding = ImVec2(18, 8);
		ImGui::GetStyle().FrameRounding = 10.f;

		UpdateImGUIColors();

		const auto& font = internal_fs::GetFileByName("RobotoBlack.ttf");
		void* font_raw = reinterpret_cast<void*>(const_cast<unsigned char*>(font.raw.data()));
		little_font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_raw, font.header.size, 16.f);
		big_font = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(font_raw, font.header.size, 60.f);

		OrigWndProc = (WNDPROC)SetWindowLong(FindWindowA(NULL, "Counter-Strike Nexon: Studio"), -4, (LONG)(LONG_PTR)HkWndProc);

		is_visible = true;
	}

	void LoadTextures() {
		internal_fs::GenerateImage("MenuBG.png");
		internal_fs::GenerateImage("FuncBG.png");
		internal_fs::GenerateImage("Aimbot.png");
		internal_fs::GenerateImage("Kreedz.png");
		internal_fs::GenerateImage("Misc.png");
		internal_fs::GenerateImage("ESP.png");
		internal_fs::GenerateImage("Other.png");
		internal_fs::GenerateImage("Removal.png");
		internal_fs::GenerateImage("Visual.png");
		internal_fs::GenerateImage("Back.png");
	}

	void Tab(std::string imgName, MenuTabs tab, std::string tabName) {
		internal_fs::Image img = internal_fs::GetImageByName(imgName);
		ImGui::Image((GLuint*)img.id, ImVec2(img.w, img.h));

		if (ImGui::IsItemClicked()) {
			cur_tab = tab;

			if (tab == MenuTabs::VISUAL)
				cur_subab = MenuSubTabs::VISUAL_ESP;
		}

		if (ImGui::IsItemHovered()) {
			hovered_tab_pos = ImVec2(menu_pos.x + ImGui::GetCursorPosX() + (HALF_WIDTH_OF_TAB - ImGui::CalcTextSize(tabName.c_str()).x * 2.f), menu_pos.y + ImGui::GetCursorPosY() - 75);
			hovered_tab_name = tabName;
		}
	}

	void SubTab(std::string imgName, MenuSubTabs tab, std::string tabName) {
		internal_fs::Image img = internal_fs::GetImageByName(imgName);
		ImGui::Image((GLuint*)img.id, ImVec2(img.w, img.h));

		if (ImGui::IsItemClicked())
			cur_subab = tab;

		if (ImGui::IsItemHovered()) {
			hovered_tab_pos = ImVec2(menu_pos.x + ImGui::GetCursorPosX() + (HALF_WIDTH_OF_TAB - ImGui::CalcTextSize(tabName.c_str()).x * 2.f), menu_pos.y + ImGui::GetCursorPosY() - 75);
			hovered_tab_name = tabName;
		}

		if (cur_subab == tab) {
			active_tab_pos = ImVec2(menu_pos.x + ImGui::GetCursorPosX() + (HALF_WIDTH_OF_TAB - ImGui::CalcTextSize(tabName.c_str()).x * 4.f / 2.f), menu_pos.y + ImGui::GetCursorPosY() - 75);
			active_tab_name = tabName;
		}
	}

	void Draw() {
		const ImVec2 SCREEN_CENTER = ImVec2(138, 140),
			MENU_BEGIN_POS = ImVec2(18, 40),
			FEATURES_WINDOW_BEGIN_POS = ImVec2(265, 44),
			MENU_SIZE = ImVec2(709, 489);

		static bool first = true;
		if (first) {
			ImGui::SetNextWindowPos(SCREEN_CENTER);
			first = false;
		}

		if (GetAsyncKeyState(VK_INSERT) & 1 && g::InFocus) {
			if (is_visible) {
				is_visible = false;

				if (g::pClientStatic->state == ca_active)
					g::Engine.ClientCmd((char*)"-commandmenu");
			}
			else {
				is_visible = true;

				if (g::pClientStatic && g::pClientStatic->state == ca_active) {
					g::Engine.ClientCmd((char*)"+commandmenu");
					ImGui::SetNextWindowPos(
						ImVec2(
						((g::ScreenSize.x > 0 ? g::ScreenSize.x : DEFAULT_WINDOW_WIDTH) / 2) - MENU_SIZE.x / 2,
						((g::ScreenSize.y > 0 ? g::ScreenSize.y : DEFAULT_WINDOW_HEIGTH) / 2) - MENU_SIZE.y / 2
						)
					);
				}
				else
					ImGui::SetNextWindowPos(SCREEN_CENTER);
			}
		}

		if (!is_visible)
			return;

		if (menu::is_visible) {
			ImGui::SetNextWindowSize(MENU_SIZE);
			ImGui::SetNextWindowBgAlpha(0.f);
			ImGui::Begin("###MainMenu", (bool*)1, ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar); {
				hovered_tab_name.clear();
				active_tab_name.clear();
				menu_pos = ImGui::GetWindowPos();

				std::stringstream ss;
				ss << "SoftON Hack [compiled at " << __DATE__ <<
					" " << __TIME__ << "]";

				const std::string tmp_str = ss.str();
				ImVec2 pos = ImGui::CalcTextSize(tmp_str.c_str());
				ImGui::SetCursorPos(ImVec2((MENU_SIZE.x / 2) - (pos.x / 2), 16));
				ImGui::Text(tmp_str.c_str());
				ImGui::SetCursorPos(MENU_BEGIN_POS);

				if (cur_tab == MenuTabs::NONE) {
					Tab("Aimbot", MenuTabs::AIMBOT, "Aimbot");
					Tab("Visual", MenuTabs::VISUAL, "Visual");
					Tab("Kreedz", MenuTabs::KREEDZ, "Kreedz");
					Tab("Misc", MenuTabs::MISC, "Misc");
				}

				const internal_fs::Image funcBG = internal_fs::GetImageByName("FuncBG");
				ImGui::GetWindowDrawList()->AddImage((unsigned int*)funcBG.id,
					ImVec2(ImGui::GetWindowPos().x + 261, ImGui::GetWindowPos().y + 40),
					ImVec2(ImGui::GetWindowPos().x + funcBG.w + 257, ImGui::GetWindowPos().y + funcBG.h + 40));

				switch (cur_tab) {
				case MenuTabs::VISUAL:
					SubTab("ESP", MenuSubTabs::VISUAL_ESP, "ESP");
					SubTab("Removal", MenuSubTabs::VISUAL_REMOVAL, "Removal");
					SubTab("Other", MenuSubTabs::VISUAL_OTHER, "Other");
					Tab("Back", MenuTabs::NONE, "Back");
					break;
				case MenuTabs::AIMBOT:
				case MenuTabs::KREEDZ:
				case MenuTabs::MISC:
					Tab("Back", MenuTabs::NONE, "Back");
					break;
				case MenuTabs::NONE:
					ImGui::SetCursorPos(FEATURES_WINDOW_BEGIN_POS);
					ImGui::Text(
						R"(The hack partially uses Phase Framework (thanks for it, Hardee!)
Also so much thanks to my tester, UrMustDie. Love you, folks <3

Credits:
- Viktoriya Shatilova (developer)
- Deacon
- Joshua Jackson
- Yury Vovk (Stack Overflow user)
- sagaceilo (Github user)
- digEmAll (Stack Overflow user)
- Valve & Nexon
- People involved in the ImGUI development
- GD & UC & SO Community
- Any others guys whose code I use <3

Press "Insert" to open/close hack's menu
Hack's files directory:
%s)", internal_fs::GetRoot());
					break;
				}

				if (active_tab_name.size()) {
					ImGui::PushFont(big_font);
					ImGui::GetWindowDrawList()->AddText(active_tab_pos, g::Settings.Visual.MenuColor.toImColor(), active_tab_name.c_str());
					ImGui::PopFont();
				}

				if (hovered_tab_name.size() && hovered_tab_name != active_tab_name) {
					ImGui::PushFont(big_font);
					ImGui::GetWindowDrawList()->AddText(hovered_tab_pos, ImColor(0.85f, 0.85f, 0.85f), hovered_tab_name.c_str());
					ImGui::PopFont();
				}

				if (cur_tab != MenuTabs::NONE) {
					ImGui::SetCursorPos(FEATURES_WINDOW_BEGIN_POS);
					ImGui::BeginChild("###SoftONMenu2", ImVec2(420, 420), false,
						ImGuiWindowFlags_NoNavInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground); {
						switch (cur_tab) {
						case MenuTabs::AIMBOT:
							ImGui::Checkbox("Enabled", &g::Settings.AimBot.Enabled);
							ImGui::HotKey("HotKey", &g::Settings.AimBot.Key, ImVec2(100, 25));
							ImGui::Checkbox("Deathmatch", &g::Settings.AimBot.DM);
							ImGui::DragFloat("FOV", &g::Settings.AimBot.FOV, 0.1f, 1.f, 60.f, "%.1f");
							ImGui::Checkbox("Draw FOV", &g::Settings.AimBot.DrawFOV);
							if (g::Settings.AimBot.DrawFOV) {
								ImGui::ColorEdit3("Color", (float*)& g::Settings.AimBot.CDrawFOV, ColorFlags);
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
							}

							ImGui::Checkbox("Prediction", &g::Settings.AimBot.Prediction);
							if (g::Settings.AimBot.Prediction) {
								ImGui::DragInt("Factor", (int*)& g::Settings.AimBot.PredictionFac, 1, 0, 100);
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
							}

							break;

						case MenuTabs::VISUAL:
							switch (cur_subab) {
							case MenuSubTabs::VISUAL_ESP:
								ImGui::Checkbox("Player Lights", &g::Settings.ESP.PlayerLights);
								if (g::Settings.ESP.PlayerLights) {
									ImGui::ColorEdit3("Friend", (float*)& g::Settings.ESP.CPlayerLightsF, ColorFlags);
									ImGui::ColorEdit3("Enemy", (float*)& g::Settings.ESP.CPlayerLightsE, ColorFlags);
									ImGui::DragFloat("Radius", &g::Settings.ESP.PlayerLightsRad, 0.01f, 0.f, 1.f, "%.2f");
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
								}

								ImGui::Checkbox("Only Enemies", &g::Settings.ESP.OnlyEnemies);
								break;

							case MenuSubTabs::VISUAL_REMOVAL:
								ImGui::Checkbox("Player Shadows", &g::Settings.Visual.Lambert);

								ImGui::Checkbox("Player Textures", &g::Settings.Visual.NoTextures);
								ImGui::Checkbox("Glow", &g::Settings.Visual.NoGlow);
								ImGui::Checkbox("Invisibility", &g::Settings.Visual.NoInvise);

								if (ImGui::Checkbox("Fog", &g::Settings.Visual.NoFog))
									g::Settings.Visual.NoFog ? g::Engine.ClientCmd((char*)"gl_fog 0") : g::Engine.ClientCmd((char*)"gl_fog 1");

								ImGui::Checkbox("Smoke", &g::Settings.Visual.NoSmoke);

								ImGui::Checkbox("Flash", &g::Settings.Visual.NoFlash);
								if (g::Settings.Visual.NoFlash) {
									ImGui::ColorEdit4("Color", (float*)& g::Settings.Visual.Flash, ColorFlags);
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
								}
								break;

							case MenuSubTabs::VISUAL_OTHER:
								ImGui::Checkbox("WallHack", &g::Settings.Visual.WallHack);

								if (ImGui::DragInt("FOV", &g::Settings.Visual.FOV, 1, 10, 150))
									(*pSetFOV)("SetFOV", 1, &g::Settings.Visual.FOV);

								ImGui::Combo("Kill Effect", (int*)& g::Settings.Visual.KillEffect, killeffect_items, IM_ARRAYSIZE(killeffect_items));

								ImGui::Checkbox("Lightmapping", &g::Settings.Visual.LightMap);
								if (g::Settings.Visual.LightMap) {
									ImGui::ColorEdit3("Color", (float*)& g::Settings.Visual.CLightmap, ColorFlags);
									ImGui::DragFloat("Brightness", &g::Settings.Visual.LightmapBr, 0.01f, 0.f, 1.f);
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
								}

								ImGui::Combo("Chams", (int*)& g::Settings.Visual.ChamsType, chams_items, IM_ARRAYSIZE(chams_items));
								if (g::Settings.Visual.ChamsType != CFG_Chams::NONE) {
									ImGui::ColorEdit3("Visible Friend", (float*)& g::Settings.Visual.CChamsFV, ColorFlags);
									ImGui::ColorEdit3("Non Visible Friend", (float*)& g::Settings.Visual.CChamsF, ColorFlags);
									ImGui::ColorEdit3("Visible Enemy", (float*)& g::Settings.Visual.CChamsEV, ColorFlags);
									ImGui::ColorEdit3("Non Visible Enemy", (float*)& g::Settings.Visual.CChamsE, ColorFlags);
									ImGui::Checkbox("Only Enemies", &g::Settings.Visual.Chams_OnlyEnemies);
									ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
								}

								if (ImGui::Checkbox("Rain", &g::Settings.Visual.Rain))
									(*pReceiveW)("ReceiveW", 1, &g::Settings.Visual.Rain);
								break;

							default:
								break;
							}
							break;

						case MenuTabs::KREEDZ:
							ImGui::Checkbox("Bhop", &g::Settings.Kreedz.Bhop);
							if (g::Settings.Kreedz.Bhop) {
								ImGui::HotKey("Bhop Key", &g::Settings.Kreedz.BhopKey, ImVec2(100, 25));
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
							}

							ImGui::Combo("Ground Strafe Mode", (int*)& g::Settings.Kreedz.GSType, gs_items, IM_ARRAYSIZE(gs_items));
							if (g::Settings.Kreedz.GSType != CFG_GSTypes::NONE) {
								ImGui::HotKey("Ground Strafe Key", &g::Settings.Kreedz.GSKey, ImVec2(100, 25));
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
							}

							ImGui::Checkbox("Show Speed", &g::Settings.Visual.ShowSpeed);
							break;

						case MenuTabs::MISC:
							if (ImGui::Button("Save CFG", ImVec2(100, 20)))
								utils::SaveCFG();
							ImGui::SetCursorPos(ImVec2(125, 0));
							if (ImGui::Button("Load CFG", ImVec2(100, 20))) {
								utils::LoadCFG();
								UpdateImGUIColors();
							}

							ImGui::Text("Enter console command:");
							if (ImGui::InputText("", cmd_buf, 63, ImGuiInputTextFlags_EnterReturnsTrue))
								CMD_Interpreter();
							ImGui::SetCursorPos(ImVec2(283, 44));
							if (ImGui::Button("Enter", ImVec2(100, 20)))
								CMD_Interpreter();

							ImGui::SetCursorPosY(70);
							if (ImGui::ColorEdit3("Menu color", (float*)& g::Settings.Visual.MenuColor, ColorFlags))
								UpdateImGUIColors();

							ImGui::Checkbox("Random Spray", &g::Settings.Visual.RandSpray);
							if (!g::Settings.Visual.RandSpray) {
								ImGui::DragInt("Spray Changer", &g::Settings.Visual.SprayID, 1, 0, 154);
								ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);
							}
							break;
						}
					}
					ImGui::EndChild();
				}
			}
			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(menu_pos.x, menu_pos.y));
			ImGui::SetNextWindowSize(MENU_SIZE);
			ImGui::SetNextWindowBgAlpha(0.f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

			ImGui::Begin("###SoftONMainMenuBG", (bool*)1, ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
			{
				const internal_fs::Image menuBG = internal_fs::GetImageByName("MenuBG");
				ImGui::GetWindowDrawList()->AddImage((unsigned int*)menuBG.id,
					ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y),
					ImVec2(ImGui::GetWindowPos().x + MENU_SIZE.x, ImGui::GetWindowPos().y + MENU_SIZE.y),
					ImVec2(0, 0), ImVec2(1, 1), g::Settings.Visual.MenuColor.toImColor());
			}
			ImGui::End();
			ImGui::PopStyleVar();
		}
	}

	bool IsVisible() { return is_visible; }
}