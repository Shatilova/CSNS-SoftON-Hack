#pragma once

#include <Windows.h>

#include <gl/GL.h>
#include <gl/GLU.h>

#include <array>
#include <tuple>

#include "valve-sdk/common/event_hook.h"
#include "valve-sdk/misc/usermsg.h"
#include "valve-sdk/engine/Cdll_int.h"
#include "valve-sdk/engine/r_studioint.h"
#include "valve-sdk/common/com_model.h"
#include "valve-sdk/engine/usercmd.h"
#include "valve-sdk/engine/pm_defs.h"
#include "valve-sdk/engine/eiface.h"
#include "valve-sdk/common/ref_params.h"
#include "valve-sdk/engine/client.h"
#include "valve-sdk/vgui/Interface.h"
#include "valve-sdk/vgui/IGameConsole.h"
#include "valve-sdk/vgui/IPanel.h"
#include "valve-sdk/vgui/IEngineVGui.h"

#include "imgui/imgui.h"

#include "local_player.h"
#include "player.h"

// PlayersList is a structure that provides access to player with specified
// index like raw array, but it also allows to iterate through players
// using range-based for-loop
// 
// there is another little difference from array yet:
// in GoldSrc first N entities IDs is reserved:
// 0 - world entity
// 1:33 - players entities (max players count is 32)
// when raw array is used for storing players,
// its size must be not 32, but 33,
// because MAX_PLAYERS + WORLD_ENTITY_OFFSET = 32 + 1 = 33
//
// in case of PlayersList, overloaded 'operator[]' subtract 1
// from given index so we get access to desired player
// it's not big deal, but it's some optimization
//
// also PlayersList is specified as a friend for Player class
// to set players ID's during PlayersList during its object construction
class PlayersList {
public:
	PlayersList() {
		size_t id = 1;
		for (auto& player : this->players)
			player.id = id++;
	}

	Player& operator [] (size_t index) { return players.at(index - WORLD_ENTITY_OFFSET); }

	auto begin() { return players.begin(); }
	auto end() { return players.end(); }

private:
	const size_t WORLD_ENTITY_OFFSET = 1;
	std::array<Player, MAX_PLAYERS> players;
};

// simple structure to store colors
struct ColorEntry {
	float r, g, b, a = 1.f;

	ColorEntry() = default;

	ColorEntry(float _r, float _g, float _b, float _a = 1.f) :
		r(_r), g(_g), b(_b), a(_a) {}

	ColorEntry(double _r, double _g, double _b, double _a = 1.f) :
		r(_r), g(_g), b(_b), a(_a) {}

	ColorEntry(int _r, int _g, int _b, int _a = 255) :
		r(static_cast<float>(_r / 255.f)),
		g(static_cast<float>(_g / 255.f)),
		b(static_cast<float>(_b / 255.f)),
		a(static_cast<float>(_a / 255.f))
	{}

	bool operator == (const ColorEntry& other) const {
		return std::tie(r, g, b, a) == std::tie(other.r, other.g, other.b, other.a);
	}

	bool operator != (const ColorEntry& other) const {
		return !(*this == other);
	}

	ImColor toImColor() const { return ImColor(r, g, b, a); }

	float* toFloat() const {
		float col[4] = { r, g, b, a };
		return col;
	}
	int* toInt() const {
		int col[4] = { r * 255, g * 255, b * 255, a * 255 };
		return col;
	}
};

// HackSettings stuff begins
typedef int HotKey;

enum class CFG_GSTypes {
	NONE,
	GS,
	JUMPGS,
	SGS,
	JUMPSGS,
};
enum class CFG_WeatherType {
	NONE,
	RAIN,
};
enum class CFG_KillEffect {
	NONE,
	YELLOW_SPLASH,
	WHITE_SPLASH,
	RED_PARTICLES,
	COLLAPSE,
	IMPLOSION
};
enum class CFG_Chams {
	NONE,
	COLORING,
	SHINE
};

struct HackSettings {
	struct SVisual {
		bool LightMap = false;
		ColorEntry CLightmap = { 1.f, 1.f, 1.f };
		float LightmapBr = 1.f;

		int FOV = 90;
		bool ShowSpeed = true;
		ColorEntry MenuColor = { 111, 255, 0 };
		bool Rain = false;

		CFG_KillEffect KillEffect = CFG_KillEffect::RED_PARTICLES;

		bool NoTextures = false;
		bool NoInvise = true;
		bool NoGlow = true;
		bool NoFog = true;
		bool Lambert = true;
		bool WallHack = true;
		bool NoSmoke = true;
		bool NoFlash = true;
		ColorEntry Flash = { 1.f, 0.f, 0.f, 0.25f };

		CFG_Chams ChamsType = CFG_Chams::SHINE;
		ColorEntry CChamsFV = { 0.f, 1.f, 0.f },
			CChamsEV = { 0.992, 0.914, 0.063 },
			CChamsF = { 0.f, 0.749f, 0.f },
			CChamsE = { 0.988, 0.624, 0.071 };
		bool Chams_OnlyEnemies = true;

		bool RandSpray = true;
		int SprayID = 82;
	} Visual;

	struct SKreedz {
		bool Bhop = true;
		HotKey BhopKey = VK_SPACE;

		CFG_GSTypes GSType = CFG_GSTypes::NONE;
		HotKey GSKey = VK_LSHIFT;
	} Kreedz;

	struct SAimBot {
		bool Enabled = true;
		HotKey Key = VK_MBUTTON;
		bool DM = false;
		float FOV = 3.f;

		bool DrawFOV = true;
		ColorEntry CDrawFOV = { 0.f, 0.f, 1.f };

		bool Prediction = true;
		int PredictionFac = 90;
	} AimBot;

	struct SESP {
		bool PlayerLights = false;
		ColorEntry CPlayerLightsF = { 0.f, 0.749f, 0.f },
			CPlayerLightsE = { 0.988, 0.624, 0.071 };
		float PlayerLightsRad = 1.f;
		bool OnlyEnemies = true;
	} ESP;
};
// HackSettings stuff ends



// since client-table functions hooking isn't allowed
// (it isn't detected, but it crashes the game),
// we have to find a workaround to deal with game buttons,
// e.g. for GS and Bhop features.
//
// buttons state is stored inside game memory of course,
// and can be easily found using IDA Pro for example.
// read more about that here: https://www.unknowncheats.me/forum/2957219-post2.html
// i'm sure my ButtonsBits structure is slightly incomplete:
// i saw something about movement buttons state a little 'higher'
// than ButtonsBits::use, but it represented isn't like 'kbutton_s',
// but i was too lazy to reconstuct it, so I left it
//
// i also should notice that original idea is taken from:
// https://www.unknowncheats.me/forum/1936073-post1.html
struct ButtonsBits {
	kbutton_s use, jump, attack,
		attack2, duck, reload, unk, score;
};

namespace g {
	extern IGameConsole* pConsole;
	extern vgui::IPanel* pIPanel;

	extern cl_enginefunc_t* pEngine;
	extern cl_clientfuncs_s* pClient;
	extern engine_studio_api_t* pStudio;
	extern r_studio_interface_t* pStudioAPI;

	extern cl_enginefunc_t Engine;
	extern cl_clientfuncs_s Client;
	extern engine_studio_api_t Studio;
	extern r_studio_interface_t StudioAPI;

	extern playermove_t* pPlayerMove;
	extern client_state_t* pClientState;
	extern client_static_t* pClientStatic;
	extern ButtonsBits* pButtonsBits;
	extern PUserMsg pUserMsgBase;
	extern event_hook_s* pEventBase;
	extern DWORD Speed;

	extern HackSettings Settings;
	extern LocalPlayer Local;
	extern PlayersList Players;
	extern Vector2D ScreenSize;

	extern bool InFocus, ScreenUpdated;
}