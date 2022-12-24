#ifndef CDLL_INT_H
#define CDLL_INT_H

extern "C" {

#include "const.h"
#include "wrect.h"
#include "util_vector.h"
#include "cl_dll.h"
#include "cvardef.h"
#include "cl_entity.h"
#include "../common/event_api.h"
#include "../common/r_efx.h"
#include "../common/net_api.h"

#define MAX_PLAYERS 32

typedef int SptiteHandle_t; // handle to a graphic

#define SCRINFO_SCREENFLASH 1
#define SCRINFO_STRETCHED	2

typedef struct SCREENINFO_s
{
	int		iSize;
	int		iWidth;
	int		iHeight;
	int		iFlags;
	int		iCharHeight;
	short	charWidths[256];
} SCREENINFO;


typedef struct client_data_s
{
	// fields that cannot be modified  (ie. have no effect if changed)
	vec3_t origin;

	// fields that can be changed by the cldll
	vec3_t viewangles;
	int		iWeaponBits;
	float	fov;	// field of view
} client_data_t;

typedef struct client_sprite_s
{
	char szName[64];
	char szSprite[64];
	int hspr;
	int iRes;
	wrect_t rc;
} client_sprite_t;

typedef struct client_textmessage_s
{
	int		effect;
	byte	r1, g1, b1, a1;		// 2 colors for effects
	byte	r2, g2, b2, a2;
	float	x;
	float	y;
	float	fadein;
	float	fadeout;
	float	holdtime;
	float	fxtime;
	const char *pName;
	const char *pMessage;
} client_textmessage_t;

typedef struct hud_player_info_s
{
	int unk1;
	char *name;
	short ping;
	byte thisplayer;  // TRUE if this is the calling player

  // stuff that's unused at the moment,  but should be done
	byte spectator;
	byte packetloss;

	char *model;
	short topcolor;
	short bottomcolor;

} hud_player_info_t;

typedef void(*xcommand_t)(void);
typedef struct _cmd_s
{
	struct _cmd_s* next;
	const char* name;
	xcommand_t function;
	int flags;
} _cmd_t, * _pcmd_t;

typedef struct cl_enginefuncs_s
{
	cl_enginefuncs_s() = default;
	cl_enginefuncs_s(const cl_enginefuncs_s& other) {
		*this = other;
	}

	cl_enginefuncs_s& operator = (const cl_enginefuncs_s& other) {
		memcpy(this, &other, sizeof(cl_enginefuncs_s));
		return *this;
	}

	void(*Unknown1) (int arg1);

	// sprite handlers
	HSPRITE						(*SPR_Load)			(const char* szPicName);
	int							(*SPR_Frames)			(HSPRITE hPic);
	int							(*SPR_Height)			(HSPRITE hPic, int frame);
	int							(*SPR_Width)			(HSPRITE hPic, int frame);
	void						(*SPR_Set)				(HSPRITE hPic, int r, int g, int b);
	void						(*SPR_Draw)			(int frame, int x, int y, const wrect_t* prc);
	void						(*SPR_DrawHoles)		(int frame, int x, int y, const wrect_t* prc);
	void						(*SPR_DrawAdditive)	(int frame, int x, int y, const wrect_t* prc);
	void						(*SPR_EnableScissor)	(int x, int y, int width, int height);
	void						(*SPR_DisableScissor)	(void);
	client_sprite_t* (*SPR_GetList)			(char* psz, int* piCount);
	
	// screen handlers
	void						(*FillRGBA)			(int x, int y, int width, int height, int r, int g, int b, int a);
	int							(*GetScreenInfo) 		(SCREENINFO* pscrinfo);
	void						(*SetCrosshair)		(HSPRITE hspr, wrect_t rc, int r, int g, int b);

	// cvar handlers
	cvar_s* (*RegisterVariable)	(char* szName, char* szValue, int flags);
	float						(*GetCvarFloat)		(char* szName);
	char* (*GetCvarString)		(char* szName);

	// command handlers
	int							(*AddCommand)			(char* cmd_name, void (*function)(void));
	int							(*HookUserMsg)			(char* szMsgName, pfnUserMsgHook);
	int							(*ServerCmd)			(char* szCmdString);
	int							(*ClientCmd)			(char* szCmdString);

	void						(*GetPlayerInfo)		(int ent_num, hud_player_info_t* pinfo);

	// sound handlers
	void						(*PlaySoundByName)		(char* szSound, float volume);
	void						(*PlaySoundByIndex)	(int iSound, float volume);

	// vector helpers
	void						(*AngleVectors)		(const float* vecAngles, float* forward, float* right, float* up);

	// text message system
	client_textmessage_t* (*TextMessageGet)		(const char* pName);
	int							(*DrawCharacter)		(int x, int y, int number, int r, int g, int b);
	int							(*DrawConsoleString)	(int x, int y, char* string);
	void						(*DrawSetTextColor)	(float r, float g, float b);
	void						(*DrawConsoleStringLen)(const char* string, int* length, int* height);

	void						(*ConsolePrint)		(const char* string);
	void						(*CenterPrint)			(const char* string);


	// Added for user input processing
	int							(*GetWindowCenterX)		(void);
	int							(*GetWindowCenterY)		(void);
	void						(*GetViewAngles)			(float*);
	void						(*SetViewAngles)			(float*);
	int							(*GetMaxClients)			(void);
	void						(*Cvar_SetValue)			(char* cvar, float value);

	int       					(*Cmd_Argc)					(void);
	char* (*Cmd_Argv)				(int arg);
	void						(*Con_Printf)				(char* fmt, ...);
	void						(*Con_DPrintf)			(char* fmt, ...);
	void						(*Con_NPrintf)			(int pos, char* fmt, ...);
	void						(*Con_NXPrintf)			(struct con_nprint_s* info, char* fmt, ...);

	const char* (*PhysInfo_ValueForKey)	(const char* key);
	const char* (*ServerInfo_ValueForKey)(const char* key);
	float						(*GetClientMaxspeed)		(void);
	int							(*CheckParm)				(char* parm, char** ppnext);
	void						(*Key_Event)				(int key, int down);
	void						(*GetMousePosition)		(int* mx, int* my);
	int							(*IsNoClipping)			(void);

	cl_entity_s* (*GetLocalPlayer)		(void);
	cl_entity_s* (*GetViewModel)			(void);
	cl_entity_s* (*GetEntityByIndex)		(int idx);

	float						(*GetClientTime)			(void);
	
	void						(*V_CalcShake)			(void);
	void						(*V_ApplyShake)			(float* origin, float* angles, float factor);

	int							(*PM_PointContents)		(float* point, int* truecontents);
	int							(*PM_WaterEntity)			(float* p);
	struct pmtrace_s* (*PM_TraceLine)			(float* start, float* end, int flags, int usehull, int ignore_pe);

	struct model_s* (*CL_LoadModel)			(const char* modelname, int* index);
	int							(*CL_CreateVisibleEntity)	(int type, struct cl_entity_s* ent);

	const struct model_s* (*GetSpritePointer)		(HSPRITE hSprite);
	void						(*PlaySoundByNameAtLocation)	(char* szSound, float volume, float* origin);

	unsigned short				(*PrecacheEvent)		(int type, const char* psz);
	
	void						(*PlaybackEvent)		(int flags, const struct edict_s* pInvoker, unsigned short eventindex, float delay, float* origin, float* angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2);
	
	void						(*WeaponAnim)			(int iAnim, int body);
	float						(*RandomFloat)			(float flLow, float flHigh);
	long						(*RandomLong)			(long lLow, long lHigh);
	void						(*HookEvent)			(char* name, void (*Event)(struct event_args_s* args));
	int							(*Con_IsVisible)			();
	const char* (*GetGameDirectory)	(void);
	cvar_s* (*GetCvarPointer)		(const char* szName);
	const char* (*Key_LookupBinding)		(const char* pBinding);
	const char* (*GetLevelName)		(void);
	void						(*GetScreenFade)		(struct screenfade_s* fade);
	void						(*SetScreenFade)		(struct screenfade_s* fade);
	void* (*VGui_GetPanel)         ();
	void                         (*VGui_ViewportPaintBackground) (int extents[4]);

	byte* (*COM_LoadFile)				(char* path, int usehunk, int* pLength);
	char* (*COM_ParseFile)			(char* data, char* token);
	void						(*COM_FreeFile)				(void* buffer);

	struct triangleapi_s* pTriAPI;
	efx_api_s* pEfxAPI;
	event_api_s* pEventAPI;
	struct demo_api_s* pDemoAPI;
	net_api_s* pNetAPI;
	struct IVoiceTweak_s* pVoiceTweak;

	// returns 1 if the client is a spectator only (connected to a proxy), 0 otherwise or 2 if in dev_overview mode
	int							(*IsSpectateOnly) (void);
	struct model_s* (*LoadMapSprite)			(const char* filename);

	// file search functions
	void						(*COM_AddAppDirectoryToSearchPath) (const char* pszBaseDir, const char* appName);
	int							(*COM_ExpandFilename)				 (const char* fileName, char* nameOutBuffer, int nameOutBufferSize);

	// User info
	// playerNum is in the range (1, MaxClients)
	// returns NULL if player doesn't exit
	// returns "" if no value is set
	const char* (*PlayerInfo_ValueForKey)(int playerNum, const char* key);
	void						(*PlayerInfo_SetValueForKey)(const char* key, const char* value);

	// Gets a unique ID for the specified player. This is the same even if you see the player on a different server.
	// iPlayer is an entity index, so client 0 would use iPlayer=1.
	// Returns false if there is no player on the server in the specified slot.
	qboolean(*GetPlayerUniqueID)(int iPlayer, char playerID[16]);

	// TrackerID access
	int							(*GetTrackerIDForPlayer)(int playerSlot);
	int							(*GetPlayerForTrackerID)(int trackerID);

	// Same as ServerCmd, but the message goes in the unreliable stream so it can't clog the net stream
	// (but it might not get there).
	int							(*ServerCmdUnreliable)(char* szCmdString);

	void						(*GetMousePos)(struct tagPOINT* ppt);
	void						(*SetMousePos)(int x, int y);
	void						(*SetMouseEnable)(qboolean fEnable);
	struct cvar_s* (*GetCvarList)(void);
	struct _cmd_s* (*GetCmdList)(void);

	char* (*GetCvarName)(struct cvar_s* cvar);
	char* (*GetCmdName)(struct _cmd_s* cmd);
	
	float (*GetServerTime)(void);
	float (*GetGravity)(void);
	const struct model_s* (*PrecacheSprite)(HSPRITE spr);
	void (*OverrideLightmap)(int override);
	void (*SetLightmapColor)(float r, float g, float b);
	void (*SetLightmapDarkness)(float dark);

	//this will always fail with the current engine
	int (*GetSequenceByName)(int flags, const char* seq);

	void (*SPR_DrawGeneric)(int frame, int x, int y, const wrect_t* prc, int blendsrc, int blenddst, int unknown1, int unknown2);

	// this will always fail with engine, don't call
	// it actually has paramenters but i dunno what they do
	// new args!!!
	// seems it's NOT LoadSentence
	void (*LoadSentence)(void);

	// localizes hud string, uses Legacy font from skin def
	// also supports unicode strings
	// new args!!!
	int (*DrawLocalizedHudString)(int x, int y, const char* str, int r, int g, int b);

	// i can't get this to work for some reason, don't use this
	// new args!!!
	int (*DrawLocalizedConsoleString)(int x, int y, const char* str);

	// gets keyvalue for local player, useful for querying vgui menus or autohelp
	const char* (*LocalPlayerInfo_ValueForKey)(const char* key);

	// another vgui2 text drawing function, i dunno how it works
	// it doesn't localize though
	void (*DrawText1)(int x, int y, const char* text, unsigned long font);

	int (*DrawUnicodeCharacter)(int x, int y, short number, int r, int g, int b, unsigned long hfont);

	// checks sound header of a sound file, determines if its a supported type
	int (*CheckSoundFile)(const char* path);

	//for condition zero, returns interface from GameUI
	void* (*GetCareerGameInterface)(void);

	void (*Cvar_Set)(const char* cvar, const char* value);

	// this actually checks for if the CareerGameInterface is found
	// and if a server is being run
	int (*IsSinglePlayer)(void);

	void (*PlaySound1)(const char* sound, float vol, float pitch);

	void (*PlayMp3)(const char* mp3, int flags);

	// get the systems current time as a float
	float (*Sys_FloatTime)(void);

	void (*SetArray)(int* array, int size);
	void (*SetClearArray)(int* array, int size);
	void (*ClearArray)(void);

	void (*PlaySound2)(const char* sound, float vol, float pitch);

	void	(*TintRGBA)			(int x, int y, int width, int height, int r, int g, int b, int a);

	void (*Unknown2)(void);
	void (*Unknown3)(void);
	void (*Unknown4)(void);

	void (*SPR_DrawHolesScaled)(void);
	void (*SPR_Replace)(void);
	void (*SPR_DrawAdditiveScaled)(void);

	void (*Unknown5)(void);
	void (*Unknown6)(void);
	void (*Unknown7)(void);
	PDWORD(*GetCServerChannelManager)(void); // +init
	bool (*Unknown9)(void); // get smth
	PDWORD (*GetInventory)(void);
	PDWORD (*GetIGClassMenuMgr)(void);
	PDWORD (*GetIGBuyMenuMgr)(void);
	PDWORD (*GetChattingManager)(void);
	PDWORD (*GetGamePlayerManager)(void);
	PDWORD (*GetGameOptionManager)(void);
	PDWORD (*GetBuyMenuManager)(void);
	PDWORD (*GetItemTable)(); // +init
	PDWORD (*GetCFavoriteWeaponsManager)(void);
	PDWORD (*GetMenuQueueMgr)(void);
	PDWORD (*GetCCrossHairMgr)(void);
	PDWORD (*GetSharedDataMgr)(void);
	PDWORD (*GetCGameRoomManager)(void);
	PDWORD (*GetClanMgr)(void);
	PDWORD (*GetItemShop)(void);
	void (*Unknown25)(void);
	PDWORD (*GetLocationTable)(); // +init
	PDWORD (*GetRankingManager)(void);
	PDWORD (*GetStrGen)(void);
	PDWORD (*GetCVideoMode_OpenGL)(void);
	PDWORD (*GetCGameUserManager)(void);
	PDWORD (*GetCSOFacade)(void);
	void (*Unknown32)(void);
	PDWORD (*GetClanStorage)(void);
	void (*GetCvarListToConsole)(); // need to check
	void (*Unknown35)(const char* name, float val, int unk); // it's about reg cvars
	void (*Unknown36)(void);
	PDWORD (*GetMainGameOptionManager)(void);
	PDWORD (*GetGiftBox)(void);
	PDWORD (*Unknown39)(void); // call HUD_NoticePrint from client table
	void (*Unknown40)(int unk1, int unk2, int unk3, int unk4, int unk5);
	int (*Unknown41)(); // GetUNK from other server struct
	PDWORD (*GetCBanMgr)(void);
	PDWORD (*GetCZBEnhanceMgr)(void);
	PDWORD (*GetCUserSurveyMgr)(void);
	PDWORD (*GetCQuestMgr)(void);
	PDWORD (*GetCUMsgMgr)(void);
	PDWORD (*GetCMiniGameMgr)(void);
	PDWORD (*GetCEventMgr)(void);
	PDWORD (*GetWeaponMgr)(void);
	void (*Unknown50)(void);
	PDWORD (*GetCFriendManager)(void);
	void (*Unknown52)(void);
	PDWORD (*GetC2ndPassword)(void);
	void (*Unknown54)(void);
	PDWORD (*GetClanMatchMgr)(void); // +init
	PDWORD (*GetCVoxelAdapter)(void);
	PDWORD (*GetCMileageBingoMgr)(void);
	PDWORD (*GetCSaleCoupon)(void);
	void (*Unknown59)(void);
	void (*Unknown60)(void);
	PDWORD (*GetCCSOCoreSDM)(void);
	void (*Unknown62)(void);
	PDWORD (*GetCMonthlyWeaponMgr)(void);
	PDWORD (*GetCVoxelOutUIMgr)(void);
	void (*Unknown65)(void);
	bool (*Unknown66)(int unk);
	PDWORD (*GetCWeaponAuctionEventMgr)(void);
	PDWORD (*GetVipSystemManager)(void);
	PDWORD (*GetGameMatchMgr)(void);// +init
	PDWORD(*GetCGameMatchRoomListManager)(); // +init, returns class +0x04 why
	PDWORD (*GetCMessenger)(void); // +init
	PDWORD (*GetLiveStreamManager)(void);
	PDWORD (*GetCCoDisassembleManager)(void);
	PDWORD (*GetCMileageShopMgr)(void);
	PDWORD(*GetOverlayTutorManager)(void); // +init
	void (*LogToFile)(const char* str, ...); 
	void (*Unknown77)(void); // nullsub
	PDWORD(*GetPopularInfoMgr)(); // +init when calling first time
	PDWORD(*GetCSeasonSystemMgr)(void); // +init
	void (*Unknown80)(void); // nullsub
	void (*Unknown81)(void); // nullsub
	void (*Unknown82)(void);
	PDWORD(*GetCEncyclopediaManager)(); // +init
	void (*Unknown84)(void); // another xor eax, eax
	PDWORD(*GetCExpeditionManager)();
	void (*Unknown86)(void); // another xor eax, eax
	void (*GetLeagueManager)();

	void(*Unknown88); // 2020 12 23
	void(*Unknown89); // 2020 12 23

	int(*CL)                ();
	int(*Init)              ();
	int(*HUD_Init)              ();
	int(*HUD_VidInit)               ();
	int(*HUD_Redraw)                ();
	int(*HUD_UpdateClientData)      ();
	int(*HUD_Reset)     ();
	int(*HUD_PlayerMove)            ();
	int(*HUD_PlayerMoveInit)        ();
	int(*HUD_PlayerMoveTexture)     ();
	int(*IN_ActivateMouse)      ();
	int(*IN_DeactivateMouse)        ();
	int(*IN_MouseEvent)     ();
	int(*IN_ClearStates)        ();
	int(*IN_Accumulate)     ();
	int(*CL_CreateMove)     ();
	int(*CL_IsThirdPerson) ();
	int(*CL_CameraOffset)       ();
	int(*KB_Find)       ();
	int(*CAM_Think)     ();
	int(*V_CalcRefdef)      ();
	int(*HUD_AddEntity)     ();
	int(*HUD_CreateEntities)        ();
	int(*HUD_DrawNormalTriangles)       ();
	int(*HUD_DrawTransparentTriangles)      ();
	int(*HUD_StudioEvent)       ();
	int(*HUD_PostRunCmd)        ();
	int(*HUD_Shutdown)      ();
	int(*HUD_TxferLocalOverrides)(); 
	int(*HUD_ProcessPlayerState)        ();
	int(*HUD_TxferPredictionData)       ();
	int(*Demo_ReadBuffer)       ();
	int(*HUD_ConnectionlessPacket)      ();
	int(*HUD_GetHullBounds)     ();
	int(*HUD_Frame)     ();
	int(*HUD_KeyEvent)     ();
	int(*HUD_TempEntUpdate)     ();
	int(*HUD_GetUserEntity)     ();
	int(*HUD_VoiceStatus)       ();
	int(*HUD_DirectorMessage)       ();
	int(*HUD_GetStudioModelInterface)       ();
	int(*HUD_ChatInputPosition)        ();
	int(*HUD_GetPlayerTeam)         (); 
	int(*ClientFactory) ();
	int(*HUD_VidSetMode)        ();
	int(*HUD_NoticePrint)       ();
	int(*CL_ClearKeyState)      ();
	int(*ProcessCLByEngine)     ();
	int(*HUD_CreateBeams)       ();
	int(*HUD_ClearCaches) (); 
} cl_enginefunc_t;

typedef struct kbutton_t
{
	// key nums holding it down
	int down[2];
	// low bit is down state
	int state;
}kbutton_s;

typedef struct cl_clientfuncs_s
{
	cl_clientfuncs_s() = default;
	cl_clientfuncs_s(const cl_clientfuncs_s& other) {
		*this = other;
	}

	cl_clientfuncs_s& operator = (const cl_clientfuncs_s& other) {
		memcpy(this, &other, sizeof(cl_clientfuncs_s));
		return *this;
	}

	int(*Initialize) (cl_enginefunc_t* pEnginefuncs, int iVersion);
	int(*HUD_Init) (void);
	int(*HUD_VidInit) (void);
	void(*HUD_Redraw) (float time, int intermission);
	int(*HUD_UpdateClientData) (client_data_t* pcldata, float flTime);
	int(*HUD_Reset) (void);
	void(*HUD_PlayerMove) (struct playermove_s* ppmove, int server);
	void(*HUD_PlayerMoveInit) (struct playermove_s* ppmove);
	char(*HUD_PlayerMoveTexture) (char* name);
	void(*IN_ActivateMouse) (void);
	void(*IN_DeactivateMouse) (void);
	void(*IN_MouseEvent) (int mstate);
	void(*IN_ClearStates) (void);
	void(*IN_Accumulate) (void);
	void(*CL_CreateMove) (float frametime, struct usercmd_s* cmd, int active);
	int(*CL_IsThirdPerson) (void);
	void(*CL_CameraOffset) (float* ofs);
	kbutton_s* (*KB_Find) (const char* name);
	void(*CAM_Think) (void);
	void(*V_CalcRefdef) (struct ref_params_s* pparams);
	int(*HUD_AddEntity) (int type, struct cl_entity_s* ent, const char* modelname);
	void(*HUD_CreateEntities) (void);
	void(*HUD_DrawNormalTriangles) (void);
	void(*HUD_DrawTransparentTriangles) (void);
	void(*HUD_StudioEvent) (const struct mstudioevent_s* event, const struct cl_entity_s* entity);
	void(*HUD_PostRunCmd) (struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
	void(*HUD_Shutdown) (void);
	void(*HUD_TxferLocalOverrides) (struct entity_state_s* state, const struct clientdata_s* client);
	void(*HUD_ProcessPlayerState) (struct entity_state_s* dst, const struct entity_state_s* src);
	void(*HUD_TxferPredictionData) (struct entity_state_s* ps, const struct entity_state_s* pps, struct clientdata_s* pcd, const struct clientdata_s* ppcd, struct weapon_data_s* wd, const struct weapon_data_s* pwd);
	void(*Demo_ReadBuffer) (int size, unsigned char* buffer);
	int(*HUD_ConnectionlessPacket) (struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size);
	int(*HUD_GetHullBounds) (int hullnumber, float* mins, float* maxs);
	void(*HUD_Frame) (double time);
	int(*HUD_KeyEvent) (int down, int keynum, const char* pszCurrentBinding);
	void(*HUD_TempEntUpdate) (double frametime, double client_time, double cl_gravity, struct tempent_s** ppTempEntFree, struct tempent_s** ppTempEntActive, int(*Callback_AddVisibleEntity)(struct cl_entity_s* pEntity), void(*Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp));
	struct cl_entity_s* (*HUD_GetUserEntity) (int index);
	int(*HUD_VoiceStatus) (int entindex, qboolean bTalking);
	int(*HUD_DirectorMessage) (unsigned char command, unsigned int firstObject, unsigned int secondObject, unsigned int flags);
	int(*HUD_GetStudioModelInterface) (int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio);
	void(*HUD_ChatInputPosition) (int* x, int* y);
	int(*HUD_GetPlayerTeam)         (int id);
	void(*ClientFactory) (void);
	int(*HUD_VidSetMode)        ();
	int(*HUD_NoticePrint)       (const char* message, int color);
	int(*CL_ClearKeyState)      ();
	int(*ProcessCLByEngine)     ();
	int(*HUD_CreateBeams)       ();
	int(*HUD_ClearCaches) (); 
} cl_clientfunc_t;

#ifndef IN_BUTTONS_H
#include "in_buttons.h"
#endif

}

#endif // CDLL_INT_H
