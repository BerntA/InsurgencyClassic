//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#ifndef WEAPON_DEFINES_H
#define WEAPON_DEFINES_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_defines_shared.h"
#include "stringlookup.h"

//=========================================================
//=========================================================
DECLARE_STRING_LOOKUP_CONSTANTS( int, weaponids )
#define LookupWeaponIDType( k, j ) STRING_LOOKUP( weaponids, k, j )

//=========================================================
//=========================================================
enum AmmoType_t
{
	AMMOTYPE_BULLET = 0,
	AMMOTYPE_GRENADE,
	AMMOTYPE_MISSILE,

	AMMOTYPE_COUNT
};

#define INVALID_AMMODATA -1

//=========================================================
//=========================================================
enum BulletTypes_t
{
	BULLET_556NATO = 0,
	BULLET_762NATO,
	BULLET_9MMNATO,
	BULLET_762RUSSIAN,
	BULLET_545RUSSIAN,
	BULLET_45,
	BULLET_303,
	BULLET_BUCKSHOT,

	BULLET_COUNT,
};

DECLARE_STRING_LOOKUP_CONSTANTS( int, bullettypes )
#define LookupBulletType( k, j ) STRING_LOOKUP( bullettypes, k, j )

//=========================================================
//=========================================================
enum GrenadeTypes_t
{
	GRENADE_M67 = 0,
	GRENADE_M18,
	GRENADE_RGD5,
	GRENADE_C4,

	GRENADE_COUNT
};

DECLARE_STRING_LOOKUP_CONSTANTS( int, grenadetypes )
#define LookupGrenadeType( k, j ) STRING_LOOKUP( grenadetypes, k, j )

//=========================================================
//=========================================================
enum MissileTypes_t
{
	MISSILE_OG7V = 0,
	MISSILE_M406,

	MISSILE_COUNT,
};

DECLARE_STRING_LOOKUP_CONSTANTS( int, missileids )
#define LookupMissileType( k, j ) STRING_LOOKUP( missileids, k, j )

//=========================================================
//=========================================================
enum WeaponType_t
{
	WEAPONTYPE_INVALID = -1,
	WEAPONTYPE_PRIMARY = 0,
	WEAPONTYPE_SECONDARY,
	WEAPONTYPE_EQUIPMENT,
	WEAPONTYPE_MELEE,
	WEAPONTYPE_COUNT
};

#define WEAPONTYPE_MAJOR_COUNT 2

DECLARE_STRING_LOOKUP_CONSTANTS( int, weapontypes )
#define LookupWeaponTypeID( k, j ) STRING_LOOKUP( weapontypes, k, j )

//=========================================================
//=========================================================
struct WeaponData_t
{
	int m_iWeaponType;
	const char *m_pszWeaponName;
	const char *m_pszWeaponAlias;
};

extern WeaponData_t g_WeaponData[ MAX_WEAPONS ];

class CWeaponData
{
public:
	CWeaponData( int iWeaponID, int iWeaponType, const char *pszWeaponName, const char *pszWeaponAlias )
	{
		g_WeaponData[ iWeaponID ].m_iWeaponType = iWeaponType;
		g_WeaponData[ iWeaponID ].m_pszWeaponName = pszWeaponName;
		g_WeaponData[ iWeaponID ].m_pszWeaponAlias = pszWeaponAlias;
	}
};

#define REGISTER_WEAPON_DATA( weaponID, weapontype, weaponname, weaponalias ) \
	static CWeaponData g_Weapon__##weaponname__Data( weaponID, weapontype, #weaponname, weaponalias );

//=========================================================
//=========================================================
class FileWeaponInfo_t;
typedef FileWeaponInfo_t* ( *WeaponDataCreate_t )( void );

struct WeaponDataCreate_Helper_t
{
	WeaponDataCreate_t m_WeaponDataCreate;
};

extern WeaponDataCreate_Helper_t g_WeaponDataHelpers[MAX_WEAPONS];

class CWeaponInfo
{
public:
	CWeaponInfo(int iWeaponID, WeaponDataCreate_t WeaponDataCreate)
	{
		g_WeaponDataHelpers[iWeaponID].m_WeaponDataCreate = WeaponDataCreate;
	}
};

#define REGISTER_WEAPON_DATAHELPER(weaponID, parsetype) \
	static FileWeaponInfo_t *g_WeaponDataHelper__##weaponID__DataCreate(void) { \
		return new parsetype; \
	} \
	static CWeaponInfo g_Weapon__##weaponID__DataHelper(weaponID, g_WeaponDataHelper__##weaponID__DataCreate);

//=========================================================
//=========================================================
enum TracerType_t
{
	TRACERTYPE_NONE = 0,
	TRACERTYPE_DEFAULT,
	TRACERTYPE_WATER,
	TRACERTYPE_RED,
	TRACERTYPE_GREEN,
	TRACERTYPE_PLASMA,
	MAX_TRACERTYPES
};

DECLARE_STRING_LOOKUP_CONSTANTS( int, tracertypes )
#define LookupTracerType( k, j ) STRING_LOOKUP( tracertypes, k, j )

//=========================================================
//=========================================================
enum BulletFormType_t
{
	BULLETFT_FLATNOSELEAD = 0,
	BULLETFT_ROUNDNOSELEAD,
	BULLETFT_ROUNDNOSEJACKETED,
	BULLETFT_SEMIPOINTEDSOFTPOINT,
	BULLETFT_POINTEDSOFTPOINT,
	BULLETFT_POINTEDFULLJACKET,
	BULLETFT_POINTEDFULLJACKETBOATTAILED,
	BULLETFT_COUNT,
};

DECLARE_STRING_LOOKUP_CONSTANTS(int, bulletformtypes)
#define GetBulletFormType(k, j) STRING_LOOKUP(bulletformtypes, k, j)

//=========================================================
//=========================================================
extern const char *WeaponIDToAlias(int iWeaponID);
extern const char *WeaponIDToName(int iWeaponID);
extern int WeaponNameToID(const char *pszWeapon);
extern int WeaponIDToType(int iWeaponID);

//=========================================================
//=========================================================
#define GET_WEAPON_DATA_CUSTOM(type, var) \
	((type*)GetWpnData())->var

#define GET_WEAPON_DATA(var) \
	GetWpnData()->var

enum WeaponClass_t
{
	WEAPONCLASS_MISC = 0,
	WEAPONCLASS_MELEE,
	WEAPONCLASS_GRENADE,
	WEAPONCLASS_PISTOL,
	WEAPONCLASS_RIFLE,
	WEAPONCLASS_LMG,
	WEAPONCLASS_SHOTGUN,
	WEAPONCLASS_SNIPER,
	WEAPONCLASS_RPG,
	WEAPONCLASS_PLASMA,
};

//=========================================================
//=========================================================
extern const char *g_pszGrenadeList[ GRENADE_COUNT ];
extern const char *g_pszMissileList[ MISSILE_COUNT ];

class CDetonatorRegister
{
public:
	CDetonatorRegister( int iAmmoType, int iAmmoID, const char *pszClass )
	{
		if( iAmmoType == AMMOTYPE_GRENADE )
			g_pszGrenadeList[ iAmmoID ] = pszClass;
		else if( iAmmoType == AMMOTYPE_MISSILE )
			g_pszMissileList[ iAmmoID ] = pszClass;
	}
};

#define REGISTER_DETONATOR( ammotype, ammoid, classname ) \
	CDetonatorRegister g_Detonator__##ammoid( ammotype, ammoid, #classname );

#endif // WEAPON_DEFINES_H