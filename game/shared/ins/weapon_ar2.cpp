//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "weapon_defines.h"
#include "effect_dispatch_data.h"
#include "ins_player_shared.h"
#include "firebullets.h"
#include "weapon_developer_base.h"
#include "firebullets.h"

#ifdef GAME_DLL

#include "te_effect_dispatch.h"

#else

#include "c_te_effect_dispatch.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CWeaponAR2 C_WeaponAR2

#endif

//=========================================================
//=========================================================
#define AR2_CLASSNAME		"weapon_ar2"

#define AR2_VMODEL			"models/weapons/v_irifle.mdl"
#define AR2_WMODEL			"models/weapons/w_irifle.mdl"

#define AR2_ANIMSUFFIX		"AK47"

#define AR2_SOUND_EMPTY		"Weapon_AR2.Empty"
#define AR2_SOUND_SINGLE	"Weapon_AR2.Single"

#define AR2_MUZZLEVELOCITY	2000
#define AR2_EFFECTIVERANGE	2000

//=========================================================
//=========================================================
class CWeaponAR2 : public CWeaponDeveloperBase, public IFireBullets
{
	DECLARE_CLASS( CWeaponAR2, CWeaponDeveloperBase );

	DECLARE_NETWORKCLASS( );
	DECLARE_PREDICTABLE( );

public:
	CWeaponAR2( );

	int GetWeaponID( void ) const { return WEAPON_AR2; }
	int GetWeaponClass( void ) const { return WEAPONCLASS_PLASMA; }

	bool ShouldPrimaryAttack( void ) { return true; }
	void PrimaryAttack( void );

	CBaseEntity *GetAttacker( void );
	CBaseEntity *GetInflictor( void ) { return this; }
	int GetBulletType( void ) const;
	int GetDamageType( void ) const;
	int GetMuzzleVelocity( void ) const;
	Vector GetSpread( void ) const;
	int GetRange( void ) const;

	bool CanUseIronsights( void ) { return false; }

	void DoImpactEffect( trace_t &tr, int nDamageType );
	
private:
	CWeaponAR2(const CWeaponAR2 &);
};

//=========================================================
//=========================================================
class AR2WeaponInfo_t : public DeveloperWeaponInfo_t
{
	DECLARE_BASECLASS( DeveloperWeaponInfo_t );

public:
	AR2WeaponInfo_t( )
	{
		Q_strncpy( szClassName, AR2_CLASSNAME, MAX_WEAPON_STRING );
		Q_strncpy( szViewModel, AR2_VMODEL, MAX_WEAPON_STRING );
		Q_strncpy( szWorldModel, AR2_WMODEL, MAX_WEAPON_STRING );
		Q_strncpy( szAnimationSuffix, AR2_ANIMSUFFIX, MAX_WEAPON_PREFIX );

		memset( aShootSounds, 0, sizeof( aShootSounds ) );
		Q_strncpy( aShootSounds[ EMPTY ], AR2_SOUND_EMPTY, MAX_WEAPON_STRING );
		Q_strncpy( aShootSounds[ SHOT_SINGLE ], AR2_SOUND_SINGLE, MAX_WEAPON_STRING );
	}
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAR2, DT_WeaponAR2 )

BEGIN_NETWORK_TABLE( CWeaponAR2, DT_WeaponAR2 )
END_NETWORK_TABLE( )

BEGIN_PREDICTION_DATA( CWeaponAR2 )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_ar2, CWeaponAR2 );
PRECACHE_WEAPON_REGISTER( weapon_ar2 );
REGISTER_WEAPON_DATA( WEAPON_AR2, WEAPONTYPE_PRIMARY, weapon_ar2, "AR2" );
REGISTER_WEAPON_DATAHELPER( WEAPON_AR2, AR2WeaponInfo_t );

//=========================================================
//=========================================================
CWeaponAR2::CWeaponAR2( )
{
}

//=========================================================
//=========================================================
#define AR2_CYCLETIME 0.075f

void CWeaponAR2::PrimaryAttack( void )
{
	static int iShotsFired = 0;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	// make a sound
	WeaponSound( SHOT_SINGLE, m_flNextPrimaryAttack );

	// find positions
	Vector vBulletStart;
	QAngle angMuzzle;
	pPlayer->GetMuzzle( vBulletStart, angMuzzle );

	Vector vForward;
	AngleVectors( angMuzzle + pPlayer->GetPunchAngle( ), &vForward );

	VectorMA( vBulletStart, -25, vForward, vBulletStart );

	// muzzle flash
    pPlayer->DoMuzzleFlash( );

	// fire the bullet
	Vector vecOrigin, vecForward;

	if( FindMuzzle( vecOrigin, vecForward, false ) )
		UTIL_FireBullets( this, vecOrigin, vecForward, TRACERTYPE_PLASMA );

#ifdef GAME_DLL

	RegisterShot( );

#endif

	m_flNextPrimaryAttack = gpGlobals->curtime + AR2_CYCLETIME;

	SendWeaponAnim( GetPrimaryAttackActivity( ) );
    DoAnimationEvent( PLAYERANIMEVENT_WEAP_FIRE1 );
}

//=========================================================
//=========================================================
CBaseEntity *CWeaponAR2::GetAttacker( void )
{
	return GetOwner( );
}

//=========================================================
//=========================================================
int CWeaponAR2::GetBulletType( void ) const
{
	return BULLET_762RUSSIAN;
}

//=========================================================
//=========================================================
int CWeaponAR2::GetDamageType( void ) const
{
	return DMG_PLASMA;
}

//=========================================================
//=========================================================
int CWeaponAR2::GetMuzzleVelocity( void ) const
{
	return AR2_MUZZLEVELOCITY;
}

//=========================================================
//=========================================================
Vector CWeaponAR2::GetSpread( void ) const
{
	return vec3_origin;
}

//=========================================================
//=========================================================
int CWeaponAR2::GetRange( void ) const
{
	return AR2_EFFECTIVERANGE;
}

//=========================================================
//=========================================================
void CWeaponAR2::DoImpactEffect( trace_t &tr, int nDamageType )
{
	CEffectData data;

	data.m_vOrigin = tr.endpos + ( tr.plane.normal * 1.0f );
	data.m_vNormal = tr.plane.normal;

	DispatchEffect( "AR2Impact", data );

	BaseClass::DoImpactEffect( tr, nDamageType );
}
