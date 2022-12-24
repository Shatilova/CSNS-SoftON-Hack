#include "usermsg.h"

#include "valve-sdk/misc/parsemsg.h"

#include "render.h"
#include "hook.h"
#include "hack_detector.h"

pfnUserMsgHook pReceiveW = nullptr;
pfnUserMsgHook pSetFOV = nullptr;
pfnUserMsgHook pTeamInfo = nullptr;
pfnUserMsgHook pDeathMsg = nullptr;
pfnUserMsgHook pResetHUD = nullptr;

// ResetHUD is invoked in beginning of every game round.
// good enough place to update some game states that
// could be changed by server
int ResetHUD(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	hack_detector::SendHackInfo();
	(*pReceiveW)(pszName, iSize, &g::Settings.Visual.Rain);
	g::Settings.Visual.NoFog ? g::Engine.ClientCmd((char*)"gl_fog 0") : g::Engine.ClientCmd(const_cast<char*>("gl_fog 1"));

	return (*pResetHUD)(pszName, iSize, pbuf);
}

// this function is a friend to Player and LocalPlayer classes
// to update player's team directly, 
// without the need for the setter method for 'team' field
int TeamInfo(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int id = READ_BYTE();
	char* team = READ_STRING();

	if (!strcmp(team, "C")) {
		if (id == g::Local.get_id())
			g::Local.team = Team::CT;
		else
			g::Players[id].team = Team::CT;
	}
	else if (!strcmp(team, "T")) {
		if (id == g::Local.get_id())
			g::Local.team = Team::TR;
		else
			g::Players[id].team = Team::TR;
	}
	else
		g::Players[id].team = Team::UNK;

	return pTeamInfo(pszName, iSize, pbuf);
}

int SetFOV(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int fov = READ_BYTE();

	// default player FOV is 90.
	// here we check if user is not in a sniper rifle scope
	// or something like that (then FOV less than 90)
	// to prevent incorrect custom FOV work
	// when player can't actually use sniper scope
	const size_t DEFAULT_FOV = 90;
	if (fov != DEFAULT_FOV)
		return (*pSetFOV)(pszName, iSize, pbuf);

	return (*pSetFOV)(pszName, iSize, &g::Settings.Visual.FOV);
}

int DeathMsg(const char* pszName, int iSize, void* pbuf) {
	BEGIN_READ(pbuf, iSize);

	int killed_id = READ_BYTE();
	int victim_id = READ_BYTE();

	if (killed_id == g::Local.get_id() && g::Settings.Visual.KillEffect != CFG_KillEffect::NONE) {
		if (g::Settings.Visual.KillEffect == CFG_KillEffect::YELLOW_SPLASH)
			g::Engine.pEfxAPI->R_BlobExplosion(g::Players[victim_id].get_origin());
		else if (g::Settings.Visual.KillEffect == CFG_KillEffect::WHITE_SPLASH)
			g::Engine.pEfxAPI->R_TeleportSplash(g::Players[victim_id].get_origin());
		else if (g::Settings.Visual.KillEffect == CFG_KillEffect::RED_PARTICLES)
			g::Engine.pEfxAPI->R_LavaSplash(g::Players[victim_id].get_origin());
		else if (g::Settings.Visual.KillEffect == CFG_KillEffect::COLLAPSE)
			g::Engine.pEfxAPI->R_LargeFunnel(g::Players[victim_id].get_origin(), 0);
		else if (g::Settings.Visual.KillEffect == CFG_KillEffect::IMPLOSION)
			g::Engine.pEfxAPI->R_Implosion(g::Players[victim_id].get_origin(), 150, 100, 2.0);
	}

	return pDeathMsg(pszName, iSize, pbuf);
}

int ReceiveW(const char* pszName, int iSize, void* pbuf) {
	return (*pReceiveW)(pszName, iSize, &g::Settings.Visual.Rain);
}

void HookUserMsg() {
	pSetFOV = hook::UserMsg("SetFOV", SetFOV);
	pReceiveW = hook::UserMsg("ReceiveW", ReceiveW);
	pTeamInfo = hook::UserMsg("TeamInfo", TeamInfo);
	pDeathMsg = hook::UserMsg("DeathMsg", DeathMsg);
	pResetHUD = hook::UserMsg("ResetHUD", ResetHUD);
}