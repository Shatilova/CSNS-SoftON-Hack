#include "globals.h"

namespace g {
	IGameConsole* pConsole = nullptr;
	vgui::IPanel* pIPanel = nullptr;

	cl_enginefunc_t* pEngine = nullptr;
	cl_clientfuncs_s* pClient = nullptr;
	engine_studio_api_t* pStudio = nullptr;
	r_studio_interface_t* pStudioAPI = nullptr;

	cl_enginefunc_t Engine;
	cl_clientfuncs_s Client;
	engine_studio_api_t Studio;
	r_studio_interface_t StudioAPI;

	playermove_t* pPlayerMove = nullptr;
	client_state_t* pClientState = nullptr;
	client_static_t* pClientStatic = nullptr;
	ButtonsBits* pButtonsBits = nullptr;
	PUserMsg pUserMsgBase = nullptr;
	event_hook_s* pEventBase = nullptr;
	DWORD Speed;

	HackSettings Settings;
	LocalPlayer Local;
	PlayersList Players;
	Vector2D ScreenSize;

	bool InFocus = true, ScreenUpdated = false;
}