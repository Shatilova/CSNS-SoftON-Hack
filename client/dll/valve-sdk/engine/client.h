// taken from 
// https://github.com/s1lentq/hitboxtracker/blob/master/dep/hlsdk/engine/client.h

#pragma once

#include "../common/crc.h"
#include "../common/screenfade.h"
#include "usercmd.h"
#include "cl_entity.h"
#include "../common/dlight.h"
#include "pm_info.h"
#include "../common/netadr.h"
#include "../common/com_model.h"
#include "custom.h"
#include "net.h"

const int MAX_DEMOS = 32;

#define MAX_EVENTS			256
#define MAX_MODEL_INDEX_BITS		9	// sent as a short
#define MAX_MODELS			(1<<MAX_MODEL_INDEX_BITS)
#define MAX_SOUND_INDEX_BITS		9
#define MAX_SOUNDS			(1<<MAX_SOUND_INDEX_BITS)

//EVENT
typedef struct event_s
{
	unsigned short index;
	const char* filename;
	int filesize;
	const char* pszScript;
} event_t;
//

//DELTA PACKET
typedef struct packet_entities_s
{
	int num_entities;
	unsigned char flags[32];
	entity_state_t* entities;
} packet_entities_t;
//

//SFX
const int CVOXFILESENTENCEMAX = 1536;

typedef struct sfx_s
{
	char name[64];
	cache_user_t cache;
	int servercount;
} sfx_t;
//

//CONSUSTENCY
const int MAX_CONSISTENCY_LIST = 512;

typedef struct consistency_s
{
	char* filename;
	int issound;
	int orig_index;
	int value;
	int check_type;
	float mins[3];
	float maxs[3];
} consistency_t;
//

typedef enum cactive_e
{
	ca_dedicated,
	ca_disconnected,
	ca_connecting,
	ca_connected,
	ca_uninitialized,
	ca_active,
} cactive_t;

typedef struct cmd_s
{
	usercmd_t cmd;
	float senttime;
	float receivedtime;
	float frame_lerp;
	qboolean processedfuncs;
	qboolean heldback;
	int sendsize;
} cmd_t;

typedef struct frame_s
{
	double receivedtime;
	double latency;
	qboolean invalid;
	qboolean choked;
	entity_state_t playerstate[32];
	double time;
	clientdata_t clientdata;
	weapon_data_t weapondata[64];
	packet_entities_t packet_entities;
	uint16 clientbytes;
	uint16 playerinfobytes;
	uint16 packetentitybytes;
	uint16 tentitybytes;
	uint16 soundbytes;
	uint16 eventbytes;
	uint16 usrbytes;
	uint16 voicebytes;
	uint16 msgbytes;
} frame_t;

typedef struct soundfade_s
{
	int nStartPercent;
	int nClientSoundFadePercent;
	double soundFadeStartTime;
	int soundFadeOutTime;
	int soundFadeHoldTime;
	int soundFadeInTime;
} soundfade_t;

typedef struct client_static_s
{
	cactive_t state;

	int unk1;

	netchan_t netchan;
	sizebuf_t datagram;
	byte datagram_buf[MAX_DATAGRAM];
	double connect_time;
	int connect_retry;
	int challenge;
	byte authprotocol;
	int userid;
	char trueaddress[32];
	float slist_time;
	int signon;
	char servername[MAX_PATH];
	char mapstring[64];
	char spawnparms[2048];
	char userinfo[256];

	char unk2[12718];

	float nextcmdtime;

	int lastoutgoingcommand;
	int demonum;
	char demos[MAX_DEMOS][16];
	qboolean demorecording;
	qboolean demoplayback;
	qboolean timedemo;
	float demostarttime;
	int demostartframe;
	int forcetrack;
	FileHandle_t demofile;
	FileHandle_t demoheader;
	qboolean demowaiting;
	qboolean demoappending;
	char demofilename[MAX_PATH];
	int demoframecount;
	int td_lastframe;
	int td_startframe;
	float td_starttime;
	incomingtransfer_t dl;
	float packet_loss;
	double packet_loss_recalc_time;
	int playerbits;
	soundfade_t soundfade;
	char physinfo[MAX_PHYSINFO_STRING];
	unsigned char md5_clientdll[16];
	netadr_t game_stream;
	netadr_t connect_stream;
	qboolean passive;
	qboolean spectator;
	qboolean director;
	qboolean fSecureClient;
	qboolean isVAC2Secure;
	uint64_t GameServerSteamID;
	int build_num;
} client_static_t;

typedef struct client_state_s
{
	//char unk0[1229724];
	//UNKNOWN AREA BEGIN
	/*int max_edicts;
	resource_t resourcesonhand;
	resource_t resourcesneeded;
	resource_t resourcelist[9038]; //size should be MAX_RESOURCE_LIST
	int num_resources;
	qboolean need_force_consistency_response;
	char serverinfo[512];
	int servercount;*/
	//UNKNOWN AREA END

	int validsequence;
	int parsecount;
	int parsecountmod;
	int stats[32]; //first s incremeanting num from 0 to 70
	//second is local HP, -2 when local died
	//third is 0
	//fourth is curWeaponID

	int unk1;
	int unk2;
	int unk3;
	int unk4;
	int unk5;
	//int weapons;

	usercmd_t cmd;

	int unk6;

	vec3_t viewangles;
	vec3_t punchangle;
	vec3_t crosshairangle;
	vec3_t simorg;
	vec3_t simvel;
	vec3_t simangles;
	vec3_t predicted_origins[64];
	vec3_t prediction_error;
	float idealpitch;
	vec3_t viewheight;
	screenfade_t sf;
	qboolean paused;//NULL IN NZ
	int onground;
	int moving;//NULL IN NZ
	int waterlevel;//NULL IN NZ
	int usehull;//NULL IN NZ
	float maxspeed;
	int pushmsec;
	int light_level;
	int intermission;
	double mtime[2];
	double time;
	double oldtime;
	frame_t frames[64];
	char unk7[4887652];
	/*
	cmd_t commands[64];
	local_state_t predicted_frames[64];
	int delta_sequence;
	int playernum;
	event_t event_precache[MAX_EVENTS];
	model_t* model_precache[MAX_MODELS];
	int model_precache_count;
	sfx_s* sound_precache[MAX_SOUNDS];
	consistency_t consistency_list[MAX_CONSISTENCY_LIST];
	int num_consistency;
	int highentity;
	char levelname[40];
	int maxclients;
	int gametype;
	int viewentity;
	model_t* worldmodel;
	efrag_t* free_efrags;
	int num_entities;
	int num_statics;
	cl_entity_t viewent;
	int cdtrack;
	int looptrack;
	CRC32_t serverCRC;
	unsigned char clientdllmd5[16];
	float weaponstarttime;
	int weaponsequence;
	int fPrecaching;
	dlight_t* pLight;*/
	
	player_info_t players[32];
	entity_state_t instanced_baseline[64];
	int instanced_baseline_number;
	CRC32_t mapCRC;
	event_state_t events;
	char downloadUrl[128];
} client_state_t;