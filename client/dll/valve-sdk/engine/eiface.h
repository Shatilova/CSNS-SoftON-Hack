#ifndef EIFACE_H
#define EIFACE_H

#ifdef HLDEMO_BUILD
#define INTERFACE_VERSION       001
#else  // !HLDEMO_BUILD, i.e., regular version of HL
#define INTERFACE_VERSION		140
#endif // !HLDEMO_BUILD

#include <stdio.h>
#include "custom.h"
#include "cvardef.h"
//
// Defines entity interface between engine and DLLs.
// This header file included by engine files and DLL files.
//
// Before including this header, DLLs must:
//		include progdefs.h
// This is conveniently done for them in extdll.h
//

#ifdef _WIN32
#define DLLEXPORT __stdcall
#else
#define DLLEXPORT /* */
#endif

typedef enum
	{
	at_notice,
	at_console,		// same as at_notice, but forces a ConPrintf, not a message box
	at_aiconsole,	// same as at_console, but only shown if developer level is 2!
	at_warning,
	at_error,
	at_logged		// Server print to console ( only in multiplayer games ).
	} ALERT_TYPE;

// 4-22-98  JOHN: added for use in pfnClientPrintf
typedef enum
	{
	print_console,
	print_center,
	print_chat,
	} PRINT_TYPE;

// For integrity checking of content on clients
typedef enum
{
	force_exactfile,			// File on client must exactly match server's file
	force_model_samebounds,		// For model files only, the geometry must fit in the same bbox
	force_model_specifybounds,	// For model files only, the geometry must fit in the specified bbox
} FORCE_TYPE;

// Returned by TraceLine
typedef struct
	{
	int		fAllSolid;			// if true, plane is not valid
	int		fStartSolid;		// if true, the initial point was in a solid area
	int		fInOpen;
	int		fInWater;
	float	flFraction;			// time completed, 1.0 = didn't hit anything
	vec3_t	vecEndPos;			// final position
	float	flPlaneDist;
	vec3_t	vecPlaneNormal;		// surface normal at impact
	edict_t	*pHit;				// entity the surface is on
	int		iHitgroup;			// 0 == generic, non zero is specific body part
	} TraceResult;

// CD audio status
typedef struct 
{
	int	fPlaying;// is sound playing right now?
	int	fWasPlaying;// if not, CD is paused if WasPlaying is true.
	int	fInitialized;
	int	fEnabled;
	int	fPlayLooping;
	float	cdvolume;
	//BYTE 	remap[100];
	int	fCDRom;
	int	fPlayTrack;
} CDStatus;

#include "../common/crc.h"

// Engine hands this to DLLs for functionality callbacks
typedef struct enginefuncs_s
{
	int			(*PrecacheModel)			(char* s);
	int			(*PrecacheSound)			(char* s);
	void		(*SetModel)				(edict_t *e, const char *m);
	int			(*ModelIndex)			(const char *m);
	int			(*ModelFrames)			(int modelIndex);
	void		(*SetSize)				(edict_t *e, const float *rgflMin, const float *rgflMax);
	void		(*ChangeLevel)			(char* s1, char* s2);
	void		(*GetSpawnParms)			(edict_t *ent);
	void		(*SaveSpawnParms)		(edict_t *ent);
	float		(*VecToYaw)				(const float *rgflVector);
	void		(*VecToAngles)			(const float *rgflVectorIn, float *rgflVectorOut);
	void		(*MoveToOrigin)			(edict_t *ent, const float *pflGoal, float dist, int iMoveType);
	void		(*ChangeYaw)				(edict_t* ent);
	void		(*ChangePitch)			(edict_t* ent);
	edict_t*	(*FindEntityByString)	(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue);
	int			(*GetEntityIllum)		(edict_t* pEnt);
	edict_t*	(*FindEntityInSphere)	(edict_t *pEdictStartSearchAfter, const float *org, float rad);
	edict_t*	(*FindClientInPVS)		(edict_t *pEdict);
	edict_t* (*EntitiesInPVS)			(edict_t *pplayer);
	void		(*MakeVectors)			(const float *rgflVector);
	void		(*AngleVectors)			(const float *rgflVector, float *forward, float *right, float *up);
	edict_t*	(*CreateEntity)			(void);
	void		(*RemoveEntity)			(edict_t* e);
	edict_t*	(*CreateNamedEntity)		(int className); //new arg!
	void		(*MakeStatic)			(edict_t *ent);
	int			(*EntIsOnFloor)			(edict_t *e);
	int			(*DropToFloor)			(edict_t* e);
	int			(*WalkMove)				(edict_t *ent, float yaw, float dist, int iMode);
	void		(*SetOrigin)				(edict_t *e, const float *rgflOrigin);
	void		(*EmitSound)				(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch);
	void		(*EmitAmbientSound)		(edict_t *entity, float *pos, const char *samp, float vol, float attenuation, int fFlags, int pitch);
	void		(*TraceLine)				(const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
	void		(*TraceToss)				(edict_t* pent, edict_t* pentToIgnore, TraceResult *ptr);
	int			(*TraceMonsterHull)		(edict_t *pEdict, const float *v1, const float *v2, int fNoMonsters, edict_t *pentToSkip, TraceResult *ptr);
	void		(*TraceHull)				(const float *v1, const float *v2, int fNoMonsters, int hullNumber, edict_t *pentToSkip, TraceResult *ptr);
	void		(*TraceModel)			(const float *v1, const float *v2, int hullNumber, edict_t *pent, TraceResult *ptr);
	const char *(*TraceTexture)			(edict_t *pTextureEntity, const float *v1, const float *v2 );
	void		(*TraceSphere)			(const float *v1, const float *v2, int fNoMonsters, float radius, edict_t *pentToSkip, TraceResult *ptr);
	void		(*GetAimVector)			(edict_t* ent, float speed, float *rgflReturn);
	void		(*ServerCommand)			(char* str);
	void		(*ServerExecute)			(void);
	void		(*ClientCommand)			(edict_t* pEdict, char* szFmt, ...);
	void		(*ParticleEffect)		(const float *org, const float *dir, float color, float count);
	void		(*LightStyle)			(int style, char* val);
	int			(*DecalIndex)			(const char *name);
	int			(*PointContents)			(const float *rgflVector);
	void		(*MessageBegin)			(int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
	void		(*MessageEnd)			(void);
	void		(*WriteByte)				(int iValue);
	void		(*WriteChar)				(int iValue);
	void		(*WriteShort)			(int iValue);
	void		(*WriteLong)				(int iValue);

	//new
	void		(*WriteLongNEW)				(int unk1, int unk2);
	void		(*WriteFloat)			(float flValue);

	void		(*WriteAngle)			(float flValue);
	void		(*WriteCoord)			(float flValue);
	void		(*WriteString)			(const char *sz);
	void		(*WriteEntity)			(int iValue);
	void		(*CVarRegister)			(cvar_t *pCvar);
	float		(*CVarGetFloat)			(const char *szVarName);
	const char*	(*CVarGetString)			(const char *szVarName);
	void		(*CVarSetFloat)			(const char *szVarName, float flValue);
	void		(*CVarSetString)			(const char *szVarName, const char *szValue);
	void		(*AlertMessage)			(ALERT_TYPE atype, char *szFmt, ...);
	void		(*EngineFprintf)			(FILE *pfile, char *szFmt, ...);
	void*		(*PvAllocEntPrivateData)	(edict_t *pEdict, long cb);
	void*		(*PvEntPrivateData)		(edict_t *pEdict);
	void		(*FreeEntPrivateData)	(edict_t *pEdict);
	const char*	(*SzFromIndex)			(int iString);
	int			(*AllocString)			(const char *szValue);
	struct entvars_s*	(*GetVarsOfEnt)			(edict_t *pEdict);
	edict_t*	(*PEntityOfEntOffset)	(int iEntOffset);
	int			(*EntOffsetOfPEntity)	(const edict_t *pEdict);
	int			(*IndexOfEdict)			(const edict_t *pEdict);
	edict_t*	(*PEntityOfEntIndex)		(int iEntIndex);
	edict_t*	(*FindEntityByVars)		(struct entvars_s* pvars);
	void*		(*GetModelPtr)			(edict_t* pEdict);
	int			(*RegUserMsg)			(const char *pszName, int iSize);

	//MAYBE NOT! CUZ THERE IS NULLSUB ON ITS PLACE
	void		(*AnimationAutomove)		(const edict_t* pEdict, float flTime);
	
	//MAYBE NOT! THERE IS NEW ARG
	void		(*GetBonePosition)		(const edict_t* pEdict, int iBone, float *rgflOrigin, float *rgflAngles );
	
	unsigned long (*FunctionFromName)	( const char *pName );
	const char *(*NameForFunction)		( unsigned long function );
	void		(*ClientPrintf)			( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg ); // JOHN: engine callbacks so game DLL can print messages to individual clients
	void		(*ServerPrint)			( const char *szMsg );
	const char *(*Cmd_Args)				( void );		// these 3 added 
	const char *(*Cmd_Argv)				( int argc );	// so game DLL can easily 
	int			(*Cmd_Argc)				( void );		// access client 'cmd' strings
	
	//MAYBE NOT! THERE ISN'T ONE OF ARGS
	void		(*GetAttachment)			(const edict_t *pEdict, int iAttachment, float *rgflOrigin, float *rgflAngles );
	
	void		(*CRC32_Init)			(CRC32_t *pulCRC);
	void        (*CRC32_ProcessBuffer)   (CRC32_t *pulCRC, void *p, int len);
	void		(*CRC32_ProcessByte)     (CRC32_t *pulCRC, unsigned char ch);
	CRC32_t		(*CRC32_Final)			(CRC32_t pulCRC);
	long		(*RandomLong)			(long  lLow,  long  lHigh);
	float		(*RandomFloat)			(float flLow, float flHigh);
	void		(*SetView)				(const edict_t *pClient, const edict_t *pViewent );
	float		(*Time)					( void );
	void		(*CrosshairAngle)		(const edict_t *pClient, float pitch, float yaw);
	byte *      (*COM_LoadFile)         (char *filename, int *pLength);
	void        (*COM_FreeFile)              (void *buffer);
	void        (*EndSection)            (const char *pszSectionName); // trigger_endsection
	int 		(*CompareFileTime)       (char *filename1, char *filename2, int *iCompare);
	void        (*GetGameDir)            (char *szGetGameDir);
	void		(*Cvar_RegisterVariable) (cvar_t *variable);
	void        (*FadeClientVolume)      (const edict_t *pEdict, int fadePercent, int fadeOutSeconds, int holdTime, int fadeInSeconds);
	void        (*SetClientMaxspeed)     (const edict_t *pEdict, float fNewMaxspeed);
	
	//I'M NOT SURE
	edict_t* (*CreateFakeClientNEW)		(int unk1);

	edict_t *	(*CreateFakeClient)		(const char *netname);	// returns NULL if fake client can't be created
	void		(*RunPlayerMove)			(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, byte impulse, byte msec );
	int			(*NumberOfEntities)		(void);
	char*		(*GetInfoKeyBuffer)		(edict_t *e);	// passing in NULL gets the serverinfo
	char*		(*InfoKeyValue)			(char *infobuffer, char *key);

	//THERE IS NOT ONE OF ARGS
	void		(*SetKeyValue)			(char *infobuffer, char *key, char *value);

	void		(*SetClientKeyValue)		(int clientIndex, char *infobuffer, char *key, char *value);
	int			(*IsMapValid)			(char *filename);
	void		(*StaticDecal)			( const float *origin, int decalIndex, int entityIndex, int modelIndex );
	int			(*PrecacheGeneric)		(char* s);
	int			(*GetPlayerUserId)		(edict_t *e ); // returns the server assigned userid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients
	void		(*BuildSoundMsg)			(edict_t *entity, int channel, const char *sample, /*int*/float volume, float attenuation, int fFlags, int pitch, int msg_dest, int msg_type, const float *pOrigin, edict_t *ed);
	int			(*IsDedicatedServer)		(void);// is this a dedicated server?
	cvar_t		*(*CVarGetPointer)		(const char *szVarName);
	
	//RETURNS -1 ALWAYS FOR NOW
	unsigned int (*GetPlayerWONId)		(edict_t *e); // returns the server assigned WONid for this player.  useful for logging frags, etc.  returns -1 if the edict couldn't be found in the list of clients

	void		(*Info_RemoveKey)		( char *s, const char *key );
	const char *(*GetPhysicsKeyValue)	( const edict_t *pClient, const char *key );
	void		(*SetPhysicsKeyValue)	( const edict_t *pClient, const char *key, const char *value );
	const char *(*GetPhysicsInfoString)	( const edict_t *pClient );
	unsigned short (*PrecacheEvent)		( int type, const char*psz );
	void		(*PlaybackEvent)			( int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );

	unsigned char *(*SetFatPVS)			( float *org );
	unsigned char *(*SetFatPAS)			( float *org );

	int			(*CheckVisibility )		( const edict_t *entity, unsigned char *pset );

	void		(*DeltaSetField)			( struct delta_s *pFields, const char *fieldname );
	void		(*DeltaUnsetField)		( struct delta_s *pFields, const char *fieldname );
	
	//THERE IS ONLY 2 ARGS
	void		(*DeltaAddEncoder)		( char *name, void (*conditionalencode)( struct delta_s *pFields, const unsigned char *from, const unsigned char *to ) );
	
	int			(*GetCurrentPlayer)		( void );
	int			(*CanSkipPlayer)			( const edict_t *player );
	int			(*DeltaFindField)		( struct delta_s *pFields, const char *fieldname );
	void		(*DeltaSetFieldByIndex)	( struct delta_s *pFields, int fieldNumber );
	void		(*DeltaUnsetFieldByIndex)( struct delta_s *pFields, int fieldNumber );

	void		(*SetGroupMask)			( int mask, int op );

	int			(*CreateInstancedBaseline) ( int classname, struct entity_state_s *baseline );
	void		(*Cvar_DirectSet)		( struct cvar_s *var, char *value );

	// Forces the client and server to be running with the same version of the specified file
	//  ( e.g., a player model ).
	// Calling this has no effect in single player
	void		(*ForceUnmodified)		( FORCE_TYPE type, float *mins, float *maxs, const char *filename );

	void		(*GetPlayerStats)		( const edict_t *pClient, int *ping, int *packet_loss );

	void		(*AddServerCommand)		( char *cmd_name, void (*function) (void) );

	// For voice communications, set which clients hear eachother.
	// NOTE: these functions take player entity indices (starting at 1).
	qboolean	(*Voice_GetClientListening)(int iReceiver, int iSender);
	qboolean	(*Voice_SetClientListening)(int iReceiver, int iSender, qboolean bListen);

	void (*Unknown1)(); //0x0
	int (*GetSequenceByName)(int flags, const char* seq);
	
	//this will always fail with engine, don't call
	//it actually has paramenters but i dunno what they do
	//new args!!!
	//seems it's NOT LoadSentence
	void (*LoadSentence)(void);
	void (*Unknown4)(); //0xC
	
	//checks sound header of a sound file, determines if its a supported type
	int (*CheckSoundFile)(const char* path);
	
	//this actually checks for if the CareerGameInterface is found
	//and if a server is being run
	int (*IsSinglePlayer)(void);
	void (*Unknown7)(); //0x18
	void (*Unknown8)(); //0x1C
	void (*Unknown9)(); //0x20
	void (*SetArray)(int* array, int size);
	void (*SetClearArray)(int* array, int size);
	void (*ClearArray)(void);
	void (*Unknown13)(); //0x30
	void (*Unknown14)(); //0x34
	PDWORD (*GetIGClassMenuMgr)(); //0x38
	PDWORD(*GetIGBuyMenuMgr)(); //0x3C
	PDWORD(*GetGameOptionManager)(); //0x40
	PDWORD(*GetGamePlayerManager)(); //0x44
	PDWORD(*GetCSOFacade)(); //0x48
	PDWORD(*GetCGameRoomManager)(); //0x4C
	PDWORD(*GetSharedDataMgr)(); //0x50
	PDWORD(*GetInventory)(); //0x54
	void (*Unknown23)(); //0x58
	PDWORD(*GetGiftBox)(); //0x5C
	PDWORD(*GetStrGen)(); //0x60
	void (*Unknown26)(); //0x64
	void (*Unknown27)(); //0x68
	PDWORD(*GetWeaponMgr)(); //0x6C
	void (*Unknown29)(); //0x70
	void (*Unknown30)(); //0x74
	void (*Unknown31)(); //0x78
	void (*Unknown32)(); //0x7C
	void (*Unknown33)(); //0x80
	void (*Unknown34)(); //0x84
	void (*Unknown35)(); //0x88
	void (*Unknown36)(); //0x8C
	void (*Unknown37)(); //0x90
	void (*Unknown38)(); //0x94
	void (*Unknown39)(); //0x98
	void (*Unknown40)(); //0x9C
	void (*Unknown41)(); //0xA0
	void (*Unknown42)(); //0xA4
	void (*Unknown43)(); //0xA8 
	void (*Unknown44)(); //0xAC
	void (*Unknown45)(); //0xB0
	void (*Unknown46)(); //0xB4
	void (*Unknown47)(); //0xB8
	void (*Unknown48)(); //0xBC
	PDWORD(*GetCVoxelAdapter)(); //0xC0
	void (*Unknown50)(); //0xC4
	PDWORD(*GetCMonthlyWeaponMgr)(); //0xC8
	void (*Unknown52)(); //0xCC
	void (*Unknown53)(); //0xD0 nullsub
	void (*Unknown54)(); //0xD4
	PDWORD (*GetCVoxelOutUIMgr)(); //0xD8
	void (*Unknown56)(); //0xDC
} enginefuncs_t;
// ONLY ADD NEW FUNCTIONS TO THE END OF THIS STRUCT.  INTERFACE VERSION IS FROZEN AT 138

// Passed to pfnKeyValue
typedef struct KeyValueData_s
{
	char	*szClassName;	// in: entity classname
	char	*szKeyName;		// in: name of key
	char	*szValue;		// in: value of key
	long	fHandled;		// out: DLL sets to true if key-value pair was understood
} KeyValueData;


typedef struct
{
	char		mapName[ 32 ];
	char		landmarkName[ 32 ];
	edict_t	*pentLandmark;
	vec3_t		vecLandmarkOrigin;
} LEVELLIST;
#define MAX_LEVEL_CONNECTIONS	16		// These are encoded in the lower 16bits of ENTITYTABLE->flags

typedef struct 
{
	int			id;				// Ordinal ID of this entity (used for entity <--> pointer conversions)
	edict_t	*pent;			// Pointer to the in-game entity

	int			location;		// Offset from the base data of this entity
	int			size;			// Byte size of this entity's data
	int			flags;			// This could be a short -- bit mask of transitions that this entity is in the PVS of
	string_t	classname;		// entity class name

} ENTITYTABLE;

#define FENTTABLE_PLAYER		0x80000000
#define FENTTABLE_REMOVED		0x40000000
#define FENTTABLE_MOVEABLE		0x20000000
#define FENTTABLE_GLOBAL		0x10000000

typedef struct saverestore_s SAVERESTOREDATA;

#ifdef _WIN32
typedef 
#endif
struct saverestore_s
{
	char		*pBaseData;		// Start of all entity save data
	char		*pCurrentData;	// Current buffer pointer for sequential access
	int			size;			// Current data size
	int			bufferSize;		// Total space for data
	int			tokenSize;		// Size of the linear list of tokens
	int			tokenCount;		// Number of elements in the pTokens table
	char		**pTokens;		// Hash table of entity strings (sparse)
	int			currentIndex;	// Holds a global entity table ID
	int			tableCount;		// Number of elements in the entity table
	int			connectionCount;// Number of elements in the levelList[]
	ENTITYTABLE	*pTable;		// Array of ENTITYTABLE elements (1 for each entity)
	LEVELLIST	levelList[ MAX_LEVEL_CONNECTIONS ];		// List of connections from this level

	// smooth transition
	int			fUseLandmark;
	char		szLandmarkName[20];// landmark we'll spawn near in next level
	vec3_t		vecLandmarkOffset;// for landmark transitions
	float		time;
	char		szCurrentMapName[32];	// To check global entities

} 
#ifdef _WIN32
SAVERESTOREDATA 
#endif
;

typedef enum _fieldtypes
{
	FIELD_FLOAT = 0,		// Any floating point value
	FIELD_STRING,			// A string ID (return from ALLOC_STRING)
	FIELD_ENTITY,			// An entity offset (EOFFSET)
	FIELD_CLASSPTR,			// CBaseEntity *
	FIELD_EHANDLE,			// Entity handle
	FIELD_EVARS,			// EVARS *
	FIELD_EDICT,			// edict_t *, or edict_t *  (same thing)
	FIELD_VECTOR,			// Any vector
	FIELD_POSITION_VECTOR,	// A world coordinate (these are fixed up across level transitions automagically)
	FIELD_POINTER,			// Arbitrary data pointer... to be removed, use an array of FIELD_CHARACTER
	FIELD_INTEGER,			// Any integer or enum
	FIELD_FUNCTION,			// A class function pointer (Think, Use, etc)
	FIELD_BOOLEAN,			// boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT,			// 2 byte integer
	FIELD_CHARACTER,		// a byte
	FIELD_TIME,				// a floating point time (these are fixed up automatically too!)
	FIELD_MODELNAME,		// Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME,		// Engine string that is a sound name (needs precache)

	FIELD_TYPECOUNT,		// MUST BE LAST
} FIELDTYPE;

#ifndef offsetof
#define offsetof(s,m)	(size_t)&(((s *)0)->m)
#endif

#define _FIELD(type,name,fieldtype,count,flags)		{ fieldtype, #name, offsetof(type, name), count, flags }
#define DEFINE_FIELD(type,name,fieldtype)			_FIELD(type, name, fieldtype, 1, 0)
#define DEFINE_ARRAY(type,name,fieldtype,count)		_FIELD(type, name, fieldtype, count, 0)
#define DEFINE_ENTITY_FIELD(name,fieldtype)			_FIELD(entvars_t, name, fieldtype, 1, 0 )
#define DEFINE_ENTITY_GLOBAL_FIELD(name,fieldtype)	_FIELD(entvars_t, name, fieldtype, 1, FTYPEDESC_GLOBAL )
#define DEFINE_GLOBAL_FIELD(type,name,fieldtype)	_FIELD(type, name, fieldtype, 1, FTYPEDESC_GLOBAL )


#define FTYPEDESC_GLOBAL			0x0001		// This field is masked for global entity save/restore

typedef struct 
{
	FIELDTYPE		fieldType;
	char			*fieldName;
	int				fieldOffset;
	short			fieldSize;
	short			flags;
} TYPEDESCRIPTION;

#define ARRAYSIZE(p)		(sizeof(p)/sizeof(p[0]))

typedef struct 
{
	// Initialize/shutdown the game (one-time call after loading of game .dll )
	void			(*pfnGameInit)			( void );				
	int				(*pfnSpawn)				( edict_t *pent );
	void			(*pfnThink)				( edict_t *pent );
	void			(*pfnUse)				( edict_t *pentUsed, edict_t *pentOther );
	void			(*pfnTouch)				( edict_t *pentTouched, edict_t *pentOther );
	void			(*pfnBlocked)			( edict_t *pentBlocked, edict_t *pentOther );
	void			(*pfnKeyValue)			( edict_t *pentKeyvalue, KeyValueData *pkvd );
	void			(*pfnSave)				( edict_t *pent, SAVERESTOREDATA *pSaveData );
	int 			(*pfnRestore)			( edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity );
	void			(*pfnSetAbsBox)			( edict_t *pent );

	void			(*pfnSaveWriteFields)	( SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int );
	void			(*pfnSaveReadFields)	( SAVERESTOREDATA *, const char *, void *, TYPEDESCRIPTION *, int );

	void			(*pfnSaveGlobalState)		( SAVERESTOREDATA * );
	void			(*pfnRestoreGlobalState)	( SAVERESTOREDATA * );
	void			(*pfnResetGlobalState)		( void );

	qboolean		(*pfnClientConnect)		( edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[ 128 ] );
	
	void			(*pfnClientDisconnect)	( edict_t *pEntity );
	void			(*pfnClientKill)		( edict_t *pEntity );
	void			(*pfnClientPutInServer)	( edict_t *pEntity );
	void			(*pfnClientCommand)		( edict_t *pEntity );
	void			(*pfnClientUserInfoChanged)( edict_t *pEntity, char *infobuffer );

	void			(*pfnServerActivate)	( edict_t *pEdictList, int edictCount, int clientMax );
	void			(*pfnServerDeactivate)	( void );

	void			(*pfnPlayerPreThink)	( edict_t *pEntity );
	void			(*pfnPlayerPostThink)	( edict_t *pEntity );

	void			(*pfnStartFrame)		( void );
	void			(*pfnParmsNewLevel)		( void );
	void			(*pfnParmsChangeLevel)	( void );

	 // Returns string describing current .dll.  E.g., TeamFotrress 2, Half-Life
	const char     *(*pfnGetGameDescription)( void );     

	// Notify dll about a player customization.
	void            (*pfnPlayerCustomization) ( edict_t *pEntity, customization_t *pCustom );  

	// Spectator funcs
	void			(*pfnSpectatorConnect)		( edict_t *pEntity );
	void			(*pfnSpectatorDisconnect)	( edict_t *pEntity );
	void			(*pfnSpectatorThink)		( edict_t *pEntity );

	// Notify game .dll that engine is going to shut down.  Allows mod authors to set a breakpoint.
	void			(*pfnSys_Error)			( const char *error_string );

	void			(*pfnPM_Move) ( struct playermove_s *ppmove, qboolean server );
	void			(*pfnPM_Init) ( struct playermove_s *ppmove );
	char			(*pfnPM_FindTextureType)( char *name );
	void			(*pfnSetupVisibility)( struct edict_s *pViewEntity, struct edict_s *pClient, unsigned char **pvs, unsigned char **pas );
	void			(*pfnUpdateClientData) ( const struct edict_s *ent, int sendweapons, struct clientdata_s *cd );
	int				(*pfnAddToFullPack)( struct entity_state_s *state, int e, edict_t *ent, edict_t *host, int hostflags, int player, unsigned char *pSet );
	void			(*pfnCreateBaseline) ( int player, int eindex, struct entity_state_s *baseline, struct edict_s *entity, int playermodelindex, vec3_t player_mins, vec3_t player_maxs );
	void			(*pfnRegisterEncoders)	( void );
	int				(*pfnGetWeaponData)		( struct edict_s *player, struct weapon_data_s *info );

	void			(*pfnCmdStart)			( const edict_t *player, const struct usercmd_s *cmd, unsigned int random_seed );
	void			(*pfnCmdEnd)			( const edict_t *player );

	// Return 1 if the packet is valid.  Set response_buffer_size if you want to send a response packet.  Incoming, it holds the max
	//  size of the response_buffer, so you must zero it out if you choose not to respond.
	int				(*pfnConnectionlessPacket )	( const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size );

	// Enumerates player hulls.  Returns 0 if the hull number doesn't exist, 1 otherwise
	int				(*pfnGetHullBounds)	( int hullnumber, float *mins, float *maxs );

	// Create baselines for certain "unplaced" items.
	void			(*pfnCreateInstancedBaselines) ( void );

	// One of the pfnForceUnmodified files failed the consistency check for the specified player
	// Return 0 to allow the client to continue, 1 to force immediate disconnection ( with an optional disconnect message of up to 256 characters )
	int				(*pfnInconsistentFile)( const struct edict_s *player, const char *filename, char *disconnect_message );

	// The game .dll should return 1 if lag compensation should be allowed ( could also just set
	//  the sv_unlag cvar.
	// Most games right now should return 0, until client-side weapon prediction code is written
	//  and tested for them.
	int				(*pfnAllowLagCompensation)( void );
} DLL_FUNCTIONS;

extern DLL_FUNCTIONS		gEntityInterface;

// Current version.
#define NEW_DLL_FUNCTIONS_VERSION	1

typedef struct
{
	// Called right before the object's memory is freed. 
	// Calls its destructor.
	void			(*pfnOnFreeEntPrivateData)(edict_t *pEnt);
	void			(*pfnGameShutdown)(void);
	int				(*pfnShouldCollide)( edict_t *pentTouched, edict_t *pentOther );
} NEW_DLL_FUNCTIONS;
typedef int	(*NEW_DLL_FUNCTIONS_FN)( NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

// Pointers will be null if the game DLL doesn't support this API.
extern NEW_DLL_FUNCTIONS	gNewDLLFunctions;

typedef int	(*APIFUNCTION)( DLL_FUNCTIONS *pFunctionTable, int interfaceVersion );
typedef int	(*APIFUNCTION2)( DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion );

#endif EIFACE_H
