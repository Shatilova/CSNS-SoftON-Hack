#include "render.h"

#include <sstream>
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"
#include "vmt-hook/vmt_hook.h"

#include "globals.h"
#include "menu.h"
#include "visual.h"
#include "aimbot.h"

VHookTable panelHook;

void HUD() {
	// 'g::pClientStatic->state' is 'ca_active' if and only if
	// player on the server
	if (g::pClientStatic->state == ca_active) {
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(g::ScreenSize.x, g::ScreenSize.y));
		ImGui::SetNextWindowBgAlpha(0.f);

		ImGui::Begin("###SoftONHUD", (bool*)1, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
		{
			// if menu is drawn we also drawn barely visible background
			if (menu::IsVisible())
				ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(g::ScreenSize.x, g::ScreenSize.y), ImColor(0.f, 0.f, 0.f, 0.1f));

			// display information about hackers on the server
			std::stringstream ss;
			size_t hacker_number = 1;
			for (const auto& player : g::Players) {
				auto is_hacker = player.is_hacker();
				if (is_hacker)
					ss << hacker_number++ << " " << player.get_nickname() << std::endl << "~ " << *is_hacker << std::endl << std::endl;
			}
			const std::string tmp_str = ss.str();
			if (!tmp_str.empty())
				ImGui::GetWindowDrawList()->AddText(ImVec2(10, 225), IM_COL32_WHITE,
					std::string("Hackers list:\n" + tmp_str).c_str());

			ShowSpeed();
			DrawAimFOV();
			LightMap();
			Flashbang();
		}

		ImGui::End();
	}
}

int StudioDrawModel(int flags) {
	cl_entity_s* localEnt = g::Engine.GetLocalPlayer();

	for (auto& player : g::Players) {
		cl_entity_s* cur_ent = g::Engine.GetEntityByIndex(player.get_id());

		if (cur_ent->player && cur_ent->index != localEnt->index) {
			// first wallhack fix
			// required to make wallhack almost perfect
			if (g::Settings.Visual.WallHack) {
				cur_ent->curstate.rendermode = kRenderTransColor;
				cur_ent->curstate.renderfx = kRenderFxNone;
				cur_ent->curstate.renderamt = 255;
			}

			// if user is a hacker then he are rendered in a black color.
			// this feature doesn't work for users that sent unknown data in a variable
			// that use for transfer hack information, 
			// but such users are displayed in hackers list with a prefix "Unknown: "
			auto is_hacker = player.is_hacker();
			if (is_hacker && is_hacker.value().find("Unknown: ") == is_hacker.value().npos) {
				cur_ent->curstate.rendermode = kRenderTransColor;
				cur_ent->curstate.renderfx = kRenderFxShadow;
				cur_ent->curstate.renderamt = 255;
			}
			else {
				// NoInvise implementation
				if (g::Settings.Visual.ChamsType == CFG_Chams::NONE && g::Settings.Visual.NoInvise) {
					cur_ent->curstate.rendermode = kRenderTransColor;
					cur_ent->curstate.renderamt = 255;
				}

				// NoGlow implementation
				if (g::Settings.Visual.ChamsType == CFG_Chams::NONE && g::Settings.Visual.NoGlow) {
					cur_ent->curstate.renderfx = kRenderFxNone;
					cur_ent->curstate.renderamt = 255;
				}

				// Chams implementation
				if (g::Settings.Visual.ChamsType != CFG_Chams::NONE &&
					(!g::Settings.Visual.Chams_OnlyEnemies || (g::Settings.Visual.Chams_OnlyEnemies && player.get_team() != g::Local.get_team()))) {
					ColorEntry col;

					// set a color depends on player team and visibleness state
					if (player.get_team() == g::Local.get_team() && player.is_visible())
						col = g::Settings.Visual.CChamsFV;
					else if (player.get_team() == g::Local.get_team() && !player.is_visible())
						col = g::Settings.Visual.CChamsF;
					else if (player.get_team() != g::Local.get_team() && player.is_visible())
						col = g::Settings.Visual.CChamsEV;
					else if (player.get_team() != g::Local.get_team() && !player.is_visible())
						col = g::Settings.Visual.CChamsE;

					cur_ent->curstate.rendercolor.r = col.toInt()[0];
					cur_ent->curstate.rendercolor.g = col.toInt()[1];
					cur_ent->curstate.rendercolor.b = col.toInt()[2];

					cur_ent->curstate.rendermode =
						(g::Settings.Visual.ChamsType == CFG_Chams::COLORING) ?
						kRenderTransAlpha : kRenderTransAdd;
					cur_ent->curstate.renderfx = kRenderFxShine;
					cur_ent->curstate.renderamt = 255;
				}
			}
		}
	}

	// second wallhack fix
	// required to make wallhack almost perfect
	cl_entity_s* tmp_ent = g::Studio.GetCurrentEntity();
	if (g::Settings.Visual.WallHack && !tmp_ent->player)
		tmp_ent->curstate.rendermode = kRenderNormal;

	return g::StudioAPI.StudioDrawModel(flags);
}

int StudioDrawPlayer(int flags, entity_state_s * pplayer) {
	// one-line xqz wallhack implementation
	// it's 'dirty' enough without respectively fixes in
	// 'StudioDrawModel' function
	if (g::Settings.Visual.WallHack && g::Local.is_alive())
		glClear(GL_DEPTH_BUFFER_BIT);

	PlayerLights();

	if (g::Settings.Visual.NoTextures) {
		glDisable(GL_TEXTURE_2D);
		int res = g::StudioAPI.StudioDrawPlayer(flags, pplayer);
		glEnable(GL_TEXTURE_2D);
		return res;
	}
	else
		return g::StudioAPI.StudioDrawPlayer(flags, pplayer);
}

void StudioSetupLighting(alight_s * plighting) {
	cl_entity_s* ent = g::Studio.GetCurrentEntity();

	if (ent->player && g::Settings.Visual.Lambert)
		plighting->ambientlight = 128;

	return g::Studio.StudioSetupLighting(plighting);
}

void StudioEntityLight(struct alight_s* plight) {
	cl_entity_s* ent = g::Studio.GetCurrentEntity();

	if (!ent || !ent->player || ent->index == g::Local.get_id() || !g::Players[ent->index].is_alive())
		return g::Studio.StudioEntityLight(plight);

	UpdateAimBone(ent);

	return g::Studio.StudioEntityLight(plight);
}

void WINAPI PaintTraversePanel(vgui::IPanel * vguiPanel, bool forceRepaint, bool allowForce) {
	panelHook.UnHook();
	g::pIPanel->PaintTraverse(vguiPanel, forceRepaint, allowForce);

	static bool first_frame = false;
	if (!first_frame) {
		menu::LoadTextures();
		first_frame = true;
	}

	// screen size must be up-to-date,
	// 'g::ScreenUpdated' is set to 'false' anytime 
	// when client is disconnected from server
	if (g::pClientStatic->state == ca_disconnected)
		g::ScreenUpdated = false;

	// this function is used for draw menu when player in a lobby.
	// draw inside this function is unpreferable, because
	// it has bugs and low perfomance
	if (g::pClientStatic->state != ca_active) {
		ImGui_ImplOpenGL2_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		menu::Draw();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
	}

	panelHook.ReHook();
}

void HookRender() {
	const int PAINT_TRAVERSE_PANEL_ID = 50;
	if (panelHook.HookTable((DWORD)g::pIPanel))
		panelHook.HookIndex(PAINT_TRAVERSE_PANEL_ID, PaintTraversePanel);

	g::pStudio->StudioSetupLighting = StudioSetupLighting;
	g::pStudio->StudioEntityLight = StudioEntityLight;

	g::pStudioAPI->StudioDrawPlayer = StudioDrawPlayer;
	g::pStudioAPI->StudioDrawModel = StudioDrawModel;
}