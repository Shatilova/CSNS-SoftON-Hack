#if !defined ( WEAPONINFOH )
#define WEAPONINFOH
#ifdef _WIN32
#pragma once
#endif

// Info about weapons player might have in his/her possession
typedef struct weapon_data_s
{
	int unk5[3];

	int			m_iId;
	int			m_iClip;

	float		m_flNextPrimaryAttack;
	float		m_flNextSecondaryAttack;
	float		m_flTimeWeaponIdle;

	int			m_fInReload;
	int			m_fInSpecialReload;
	float		m_flNextReload;
	float		m_flPumpTime;
	float		m_fReloadTime;

	float		m_fAimedDamage;
	float		m_fNextAimBonus;
	int			m_fInZoom;//null

	int			m_iWeaponState;
	int			m_iWeaponState2;

	int			iuser1; 
	int			iuser2;
	int			iuser3;
	int			iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
	int unk2[38];
} weapon_data_t;

#endif