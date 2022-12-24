#include "engine.h"

#include <sstream>
#include <random>
#include <chrono>
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_opengl2.h"

#include "globals.h"
#include "render.h"
#include "aimbot.h"
#include "kreedz.h"
#include "hook.h"
#include "menu.h"

pfnEventHook pCreateSmoke = nullptr;
xcommand_t pImpulse = nullptr;

// HUD_Frame is invoked every frame when client on the server,
// but its first invoke happens earlier 
// than first invoke of HUD_Redraw function
// so it's a good place to update screen size
int HUD_Frame() {
	// update screen size once
	if (g::pClientStatic->state == ca_active && !g::ScreenUpdated) {
		SCREENINFO screen_info;
		screen_info.iSize = sizeof(SCREENINFO);
		g::Engine.GetScreenInfo(&screen_info);
		g::ScreenSize = {
			static_cast<float>(screen_info.iWidth),
			static_cast<float>(screen_info.iHeight)
		};

		g::ScreenUpdated = true;
	}
	return g::Engine.HUD_Frame();
}

// HUD_Redraw is a function game HUD is drawn.
// in fact i'm not sure this function is used in CSN:S
// for redraw HUD, but it should
//
// HUD_Redraw is not the best place to draw something,
// because hack's drawn elements is overlapped by ingame drawn elements,
// but OpenGL-hooks is detected by BlackCipher2 (game ring0 anticheat)
int HUD_Redraw() {
	int res = g::Engine.HUD_Redraw();

	ImGui_ImplOpenGL2_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	HUD();
	menu::Draw();

	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

	return res;
}

// the player movement is processed in CL_CreateMove
// this function is invoked only if player moves
int CL_CreateMove() {
	int res = g::Engine.CL_CreateMove();

	// reset player speed every frame
	// because it may be set to extremely low value by 'GroundStrafe'
	g::Local.set_speed(1.f);

	if (g::Local.is_alive()) {
		Aimbot();
		Bhop();
		GroundStrafe();
	}

	return res;
}

int HUD_PostRunCmd() {
	int res = g::Engine.HUD_PostRunCmd();

	// update players information
	g::Local.update();
	for (auto& player : g::Players)
		player.update();

	return res;
}

// unfortunately since OpenGL-hooks are detected, 
// we can't make NoSmoke using glBegin (who knows, he knows),
// but we still can hook game event of creating smoke and control its execution
//
// there is the only one weighty problem of this two-line NoSmoke:
// when NoSmoke is enabled during the game, already existent smoke won't be hiden
void CreateSmoke(event_args_s* args) {
	if (!g::Settings.Visual.NoSmoke)
		(*pCreateSmoke)(args);
}

// to make 'Spray Changer' feature we should hook 'impulse' ingame command
// that is invoked not only when player draws graffitti,
// but it's easy enough to weed out unwanted invokes
//
// read more about 'impulse' command (it's for Source engine, but anyway):
// https://developer.valvesoftware.com/wiki/Impulse
void Impulse() {
	// random generator is initialized once 
	static bool rng_ready = false;
	static std::mt19937 rng;
	static std::uniform_int_distribution<int> dis;
	if (!rng_ready) {
		rng = std::mt19937(std::chrono::steady_clock::now().time_since_epoch().count());
		dis = std::uniform_int_distribution<int>(1, 154);

		rng_ready = true;
	}

	// 'impulse' has argument '201'
	// if it's command to draw graffitti
	if (!strcmp(g::Engine.Cmd_Argv(1), "201")) {
		std::stringstream ss;
		ss << "sprayid " <<
			(g::Settings.Visual.RandSpray ? dis(rng) : g::Settings.Visual.SprayID);

		const std::string tmp_str = ss.str();
		g::Engine.ClientCmd((char*)tmp_str.c_str());
	}

	(*pImpulse)();
}

void HookEngine() {
	g::pEngine->HUD_Frame = HUD_Frame;
	g::pEngine->CL_CreateMove = CL_CreateMove;
	g::pEngine->HUD_Redraw = HUD_Redraw;
	g::pEngine->HUD_PostRunCmd = HUD_PostRunCmd;

	pImpulse = hook::Command("impulse", Impulse);
	pCreateSmoke = hook::Event("events/createsmoke.sc", CreateSmoke);
}