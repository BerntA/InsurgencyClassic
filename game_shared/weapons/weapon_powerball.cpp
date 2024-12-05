//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "weapon_defines.h"
#include "weapon_ins_base.h"
#include "ins_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL
#define CWeaponPowerball C_WeaponPowerball
#endif

//=========================================================
//=========================================================
class CWeaponPowerball : public CWeaponINSBase
{
	DECLARE_CLASS( CWeaponPowerball, CWeaponINSBase );

	DECLARE_NETWORKCLASS( ); 
	DECLARE_PREDICTABLE( );

public:
	CWeaponPowerball( );

private:
	CWeaponPowerball( const CWeaponPowerball & );

	int GetWeaponID( void ) const { return WEAPON_POWERBALL; }
	int GetWeaponClass( void ) const { return WEAPONCLASS_MISC; }
	int GetWeaponSize( void ) const { return WEAPONSIZE_SMALLARM; }

   	void ItemPostFrame( void );

	bool CanHolster( void );

	bool ShouldPrimaryAttack( void );
	void PrimaryAttack( void );
	void DelayedAttack( void );

	bool CanUseIronSights( void ) { return false; }

private:
	float m_flDelayedFire;
	bool m_bShotDelayed;

private:
#ifdef GAME_DLL
	DECLARE_ACTTABLE( );
#endif
};

//=========================================================
//=========================================================
class PowerballWeaponInfo_t : public FileWeaponInfo_t
{
	typedef FileWeaponInfo_t BaseClass;

public:
	PowerballWeaponInfo_t( )
	{
		Q_strncpy( szClassName, "weapon_powerball", MAX_WEAPON_STRING );
		Q_strncpy( szViewModel, "models/weapons/v_irifle.mdl", MAX_WEAPON_STRING );
		Q_strncpy( szWorldModel, "models/weapons/w_irifle.mdl", MAX_WEAPON_STRING );
		Q_strncpy( szAnimationSuffix, "AK47", MAX_WEAPON_PREFIX );

#ifdef CLIENT_DLL
		iconSelect = 0;
#endif

		memset( aShootSounds, 0, sizeof( aShootSounds ) );
		Q_strncpy( aShootSounds[ EMPTY ], "Weapon_AR2.Empty", MAX_WEAPON_STRING );
		Q_strncpy( aShootSounds[ SHOT_DOUBLE ], "Weapon_AR2.Double", MAX_WEAPON_STRING );
	}

	bool ShouldParse( void ) const { return false; }
};

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponPowerball, DT_WeaponPowerball )

BEGIN_NETWORK_TABLE( CWeaponPowerball, DT_WeaponPowerball )
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CWeaponPowerball )
END_PREDICTION_DATA( )

LINK_ENTITY_TO_CLASS( weapon_powerball, CWeaponPowerball );
PRECACHE_WEAPON_REGISTER( weapon_powerball );
REGISTER_WEAPON_DATA( WEAPON_POWERBALL, WEAPONTYPE_EQUIPMENT, weapon_powerball, "PowerBall" );
REGISTER_WEAPON_DATAHELPER( WEAPON_POWERBALL, PowerballWeaponInfo_t );

//=========================================================
//=========================================================
#ifndef CLIENT_DLL

acttable_t	CWeaponPowerball::m_acttable[ ] = 
{
    { ACT_INS_STAND,                ACT_INS_STAND_AK47,            false },
    { ACT_INS_STAND_AIM,            ACT_INS_STAND_AIM_AK47,        false },
    { ACT_INS_WALK,                 ACT_INS_WALK_AK47,             false },
    { ACT_INS_WALK_AIM,             ACT_INS_WALK_AIM_AK47,         false },
    { ACT_INS_RUN,                  ACT_INS_RUN_AK47,              false },
    { ACT_INS_CROUCH,               ACT_INS_CROUCH_AK47,           false },
    { ACT_INS_CROUCH_AIM,           ACT_INS_CROUCH_AIM_AK47,       false },
    { ACT_INS_CROUCH_WALK,          ACT_INS_CROUCH_WALK_AK47,      false },
    { ACT_INS_CROUCH_WALK_AIM,      ACT_INS_CROUCH_WALK_AIM_AK47,  false },
    { ACT_INS_PRONE,                ACT_INS_PRONE_AK47,            false },
    { ACT_INS_CRAWL,                ACT_INS_CRAWL_AK47,            false },
    { ACT_INS_JUMP,                 ACT_INS_JUMP_AK47,             false },
};

IMPLEMENT_ACTTABLE( CWeaponPowerball );

#endif

//=========================================================
//=========================================================
CWeaponPowerball::CWeaponPowerball( )
{
	m_bShotDelayed = false;
	m_flDelayedFire = 0.0f;
}

//=========================================================
//=========================================================
void CWeaponPowerball::ItemPostFrame( void )
{
	if ( m_bShotDelayed && gpGlobals->curtime > m_flDelayedFire )
		DelayedAttack( );

	BaseClass::ItemPostFrame( );
}

//=========================================================
//=========================================================
bool CWeaponPowerball::CanHolster( void )
{
	return ( !m_bShotDelayed && BaseClass::CanHolster( ) );
}

//=========================================================
//=========================================================
bool CWeaponPowerball::ShouldPrimaryAttack( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && pPlayer->HasPowerball( ) );
}

//=========================================================
//=========================================================
void CWeaponPowerball::PrimaryAttack( void )
{
	if( m_bShotDelayed )
		return;

	m_bShotDelayed = true;
	m_flNextPrimaryAttack = m_flDelayedFire = gpGlobals->curtime + 0.5f;

	SendWeaponAnim( ACT_VM_FIDGET );
	WeaponSound( SPECIAL1 );
}

//=========================================================
//=========================================================
void CWeaponPowerball::DelayedAttack( void )
{
	m_bShotDelayed = false;
	
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	
	if( pPlayer == NULL )
		return;

	SendWeaponAnim( ACT_VM_SECONDARYATTACK );

	pPlayer->DoMuzzleFlash( );
	
	WeaponSound( SHOT_DOUBLE );

	CINSCombineBall::CreateCombineBall(	pPlayer->Weapon_ShootPosition( ), 
						pPlayer->Weapon_ShootDirection( ), 
						pPlayer );

	color32 White = { 255, 255, 255, 64 };
	UTIL_ScreenFade( pPlayer, White, 0.1f, 0, FFADE_IN );
	
	QAngle angAngles = pPlayer->GetLocalAngles( );

	angAngles.x += random->RandomInt( -4, 4 );
	angAngles.y += random->RandomInt( -4, 4 );
	angAngles.z = 0;

	pPlayer->SnapEyeAngles( angAngles );
	
	pPlayer->ViewPunch( QAngle( random->RandomInt( -8, -12 ), random->RandomInt( 1, 2 ), 0 ) );

	pPlayer->StripPowerball( false );
}
