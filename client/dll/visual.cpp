#include "visual.h"

#include <sstream>
#include "imgui/imgui.h"

#include "globals.h"

#define RAD2DEG( x )  ( (float)(x) * (float)(180.f / M_PI) )
#define DEG2RAD( x ) ( (float)(x) * (float)(M_PI / 180.f) )

void DrawAimFOV() {
	if (g::Settings.AimBot.DrawFOV && g::Local.is_alive()) {
		float radius = tanf(DEG2RAD(g::Settings.AimBot.FOV) * 0.5f) / tanf(DEG2RAD(g::Settings.Visual.FOV) * 0.5f) * g::ScreenSize.x;

		ImGui::GetWindowDrawList()->AddCircle(ImVec2(g::ScreenSize.x * 0.5f, g::ScreenSize.y * 0.5f), radius,
			g::Settings.AimBot.CDrawFOV.toImColor());
	}
}

void LightMap() {
	static bool replaced = false;
	static ColorEntry prev_col = g::Settings.Visual.CLightmap;

	if (g::Settings.Visual.LightMap) {
		if (!replaced) {
			g::Engine.OverrideLightmap(1);
			g::Engine.SetLightmapColor(g::Settings.Visual.CLightmap.r, g::Settings.Visual.CLightmap.g, g::Settings.Visual.CLightmap.b);
			g::Engine.SetLightmapDarkness(g::Settings.Visual.LightmapBr);
			replaced = true;
		}
		else {
			if (prev_col != g::Settings.Visual.CLightmap)
				replaced = false;
		}
	}
	else {
		g::Engine.OverrideLightmap(0);
		replaced = false;
	}
}

void Flashbang() {
	if (g::Settings.Visual.NoFlash) {
		screenfade_s sf;
		g::Engine.GetScreenFade(&sf);

		float fade_end = 0.0f;
		if (sf.fadeEnd > g::pEngine->GetClientTime()) {
			fade_end = sf.fadeEnd;

			const ColorEntry& col = g::Settings.Visual.Flash;

			if (sf.fadealpha > col.a)
				sf.fadealpha = col.a;

			sf.fader = col.r;
			sf.fadeg = col.g;
			sf.fadeb = col.b;
		}

		g::Engine.SetScreenFade(&sf);
	}
}

// the low-perfomance ESP, but also very legit 
void PlayerLights() {
	cl_entity_s* ent = g::Studio.GetCurrentEntity();

	if (g::Players[ent->index].is_alive() && ent->index != g::Local.get_id() &&
		(!g::Settings.ESP.OnlyEnemies || (g::Settings.ESP.OnlyEnemies && g::Players[ent->index].get_team() != g::Local.get_team()))) {
		if (g::Settings.ESP.PlayerLights) {
			dlight_t* light = g::Engine.pEfxAPI->CL_AllocDlight(0);
			light->color.r = g::Players[ent->index].get_team() == g::Local.get_team() ? g::Settings.ESP.CPlayerLightsF.toInt()[0] : g::Settings.ESP.CPlayerLightsE.toInt()[0];
			light->color.g = g::Players[ent->index].get_team() == g::Local.get_team() ? g::Settings.ESP.CPlayerLightsF.toInt()[1] : g::Settings.ESP.CPlayerLightsE.toInt()[1];
			light->color.b = g::Players[ent->index].get_team() == g::Local.get_team() ? g::Settings.ESP.CPlayerLightsF.toInt()[2] : g::Settings.ESP.CPlayerLightsE.toInt()[2];
			light->origin = ent->origin;
			light->die = g::Engine.GetClientTime() + 0.1;
			light->radius = g::Settings.ESP.PlayerLightsRad * 255;
		}
	}
}

void ShowSpeed() {
	if (g::Settings.Visual.ShowSpeed && g::Local.is_alive() && g::pPlayerMove->velocity.Length2D() > 0.f) {
		std::stringstream ss;
		ss << static_cast<int>(g::pPlayerMove->velocity.Length2D()) <<
			" units/sec";

		const std::string tmp_str = ss.str();
		ImGui::GetWindowDrawList()->AddText(
			ImVec2(g::ScreenSize.x - 10 - ImGui::CalcTextSize(ss.str().c_str()).x, g::ScreenSize.y / 2),
			ImColor(1.0f, 1.0f, 1.0f, 1.0f),
			tmp_str.c_str());
	}
}