//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_rifle_grenade_base.h"
#include "ins_gamerules.h"
#include "ammodef.h"
#include "ins_utils.h"

#ifdef GAME_DLL

#include "missile_launched_base.h"

#else

#include "keyvalues.h"
#include "inshud.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
RifleGrenadeInfo_t::RifleGrenadeInfo_t( )
{
#ifdef CLIENT_DLL

	szGrenadeAmmoTex[ 0 ] = '\0';
	iGrenadeAmmoTexID = 0;

#endif
}

//=========================================================
//=========================================================
void RifleGrenadeInfo_t::Parse( KeyValues *pKeyValuesData, const char *pszWeaponName )
{
	BaseClass::Parse( pKeyValuesData, pszWeaponName );

#ifdef CLIENT_DLL

	Q_strncpy( szGrenadeAmmoTex, pKeyValuesData->GetString( "grenadetex", "" ), MAX_WEAPON_STRING );

#endif
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponRifleGrenadeBase, DT_LocalRifleGrenadeWeaponData )

#ifdef GAME_DLL

	SendPropBool( SENDINFO( m_bSightsUp ) ),
	SendPropTime( SENDINFO( m_flSightChangeTime ) ),

	SendPropBool( SENDINFO( m_bGrenadeLoaded ) ),
	SendPropInt( SENDINFO( m_iGrenades ) ),

#else

	RecvPropBool( RECVINFO( m_bSightsUp ) ),
	RecvPropTime( RECVINFO( m_flSightChangeTime ) ),

	RecvPropBool( RECVINFO( m_bGrenadeLoaded ) ),
	RecvPropInt( RECVINFO( m_iGrenades ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponRifleGrenadeBase, DT_WeaponRifleGrenadeBase )

BEGIN_NETWORK_TABLE( CWeaponRifleGrenadeBase, DT_WeaponRifleGrenadeBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalRifleGrenadeWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalRifleGrenadeWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalRifleGrenadeWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalRifleGrenadeWeaponData ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponRifleGrenadeBase )

	DEFINE_PRED_FIELD( m_bSightsUp, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),			
	DEFINE_PRED_FIELD_TOL( m_flSightChangeTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),	

	DEFINE_PRED_FIELD( m_bGrenadeLoaded, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponRifleGrenadeBase::CWeaponRifleGrenadeBase( )
{
	m_bSightsUp = false;
	m_flSightChangeTime = 0.0f;

	m_bGrenadeLoaded = false;
	m_iGrenades = 0;
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

	m_flSightChangeTime = 0.0f;
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::IsEmptyAttack( void )
{
	if( m_bSightsUp )
		return false;

	return BaseClass::IsEmptyAttack( );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::CanAttack( void )
{
	return ( BaseClass::CanAttack( ) && !InSightChange( ) );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::CanPrimaryAttack( void )
{
	if( m_bSightsUp )
		return m_bGrenadeLoaded;

	return BaseClass::CanPrimaryAttack( );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::AllowSprintStart( void )
{
	return ( BaseClass::AllowSprintStart( ) && !InSightChange( ) );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::AllowJump( void )
{
	return ( BaseClass::AllowJump( ) && !InSightChange( ) );
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::PrimaryAttack( void )
{
	if( m_bSightsUp )
	{
		// take the ammo away from the player
		CINSPlayer *pPlayer = GetINSPlayerOwner( );

		if( !pPlayer )
			return;

		BaseClass::WeaponSound( SHOT_DOUBLE );

	#ifdef GAME_DLL

		// create the grenade
		Vector vecOrigin, vecDirection;
		QAngle angMuzzle;

		pPlayer->GetMuzzle( vecOrigin, angMuzzle );

		AngleVectors( angMuzzle, &vecDirection );
		VectorMA( vecOrigin, MISSILE_LAUNCHED_LENGTH, vecDirection, vecOrigin );

		CBaseLaunchedMissile::CreateLaunchedMissile( pPlayer, GetLaunchedMissileID( ), vecOrigin, angMuzzle, vecDirection );

	#endif

		if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
			SendWeaponAnim( ACT_VM_ISHOOT_M203 );
		else
			SendWeaponAnim( ACT_VM_SECONDARYATTACK );

		// player "shoot" animation
		pPlayer->SetAnimation( PLAYER_ATTACK1 );

		// remove grenade
		m_bGrenadeLoaded = false;

		// adjust timers
		m_flNextPrimaryAttack = gpGlobals->curtime + 1.0f;
		m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;
	}
	else
	{
		BaseClass::PrimaryAttack( );
	}
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::SecondaryAttack( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	if( pPlayer->IsSprinting( ) )
		return;

	if( !m_bSightsUp && !HasGrenades( ) )
		return;

	SetSights( !m_bSightsUp );

#ifdef CLIENT_DLL

	if( m_bSightsUp )
		GetINSHUDHelper( )->WeaponInfo( )->ShowAmmoInfo( );

#endif
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::SetSights( bool bState )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		SetIronsights( false, true );

	if( m_bSightsUp )
	{
		SendWeaponAnim( GetSightHolsterActivity( ) );
	}
	else
	{
		SendWeaponAnim( GetSightDrawActivity( ) );

		if( !m_bGrenadeLoaded && HasGrenades( ) )
			LoadGrenade( );
	}

	m_bSightsUp = bState;
	m_flSightChangeTime = gpGlobals->curtime + SequenceDuration( );
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::LoadGrenade( void )
{
	m_bGrenadeLoaded = true;
	m_iGrenades = max( m_iGrenades - 1, 0 );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::InSightChange( void )
{
	return ( m_flSightChangeTime != 0.0f && m_flSightChangeTime >= gpGlobals->curtime );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::CanReload( void )
{
	if( !BaseClass::CanReload( ) )
		return false;

	if( m_bSightsUp )
		return !m_bGrenadeLoaded;

	return BaseClass::CanReload( );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::IsEmptyReload( void )
{
	if( m_bSightsUp )
		return !HasGrenades( );

	return BaseClass::IsEmptyReload( );
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::FinishReload( void )
{
	if( m_bSightsUp )
	{
		m_bInReload = false;

		LoadGrenade( );
	}

	BaseClass::FinishReload( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetDrawActivity( void )
{
	if( m_bSightsUp )
		return GetSightDrawFullActivity( );

	return BaseClass::GetDrawActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetHolsterActivity( void )
{
	if( m_bSightsUp )
		return GetSightHolsterFullActivity( );

	return BaseClass::GetHolsterActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetNormalActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && m_bSightsUp )
	{
		if( pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
			return GetSightIronsightIdleActivity( );
		else
			return GetSightIdleActivity( );
	}

	return BaseClass::GetNormalActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetCrawlActivity( void ) const
{
	if( m_bSightsUp )
		return GetSightCrawlActivity( );

	return BaseClass::GetCrawlActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetDownActivity( void ) const
{
	if( m_bSightsUp )
		return GetSightDownActivity( );

	return BaseClass::GetDownActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetReloadActivity( void ) const
{
	if( m_bSightsUp )
		return GetSightReloadActivity( );

	return BaseClass::GetReloadActivity();
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetIronsightInActivity( void ) const
{
	if( m_bSightsUp )
		return GetSightIronsightInActivity( );

	return BaseClass::GetIronsightInActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetIronsightOutActivity( void ) const
{
	if( m_bSightsUp )
		return GetSightIronsightOutActivity( );

	return BaseClass::GetIronsightOutActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightShootActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && pOwner->GetFlags( ) & FL_PLAYER_IRONSIGHTS )
		return GetSightShootIronsightsActivity( );

	return GetSightShootHippedActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightDrawActivity( void )
{
	if( !m_bGrenadeLoaded )
		return GetSightDrawReadyActivity( );

	return GetSightDrawDrawActivity( );
}

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::GiveAmmo( int iCount )
{
	m_iGrenades += iCount;
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::HasAmmo( void ) const
{
	return ( BaseClass::HasAmmo( ) || HasGrenades( ) );
}

//=========================================================
//=========================================================
bool CWeaponRifleGrenadeBase::HasGrenades( void ) const
{
	return ( m_bGrenadeLoaded || m_iGrenades > 0 );
}

//=========================================================
//=========================================================
int CWeaponRifleGrenadeBase::GetAmmoCount( void ) const
{
	if( m_bSightsUp )
	{
		int iAmmoCount = m_iGrenades;

		if( m_bGrenadeLoaded )
			iAmmoCount++;

		return iAmmoCount;
	}

	return BaseClass::GetAmmoCount( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightDrawFullActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightHolsterFullActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightIdleActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightCrawlActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightDownActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightShootIronsightsActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightShootHippedActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightHolsterActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightDrawDrawActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightDrawReadyActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightReloadActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightIronsightInActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightIronsightIdleActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
Activity CWeaponRifleGrenadeBase::GetSightIronsightOutActivity( void ) const
{
	return ACT_INVALID;
}

//=========================================================
//=========================================================
int CWeaponRifleGrenadeBase::GetLaunchedMissileID( void ) const
{
	return INVALID_AMMODATA;
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
void CWeaponRifleGrenadeBase::PrecacheResources( void )
{
	BaseClass::PrecacheResources( );

	LoadAmmoTex( GET_WEAPON_DATA_CUSTOM( RifleGrenadeInfo_t, szGrenadeAmmoTex ), GET_WEAPON_DATA_CUSTOM( RifleGrenadeInfo_t, iGrenadeAmmoTexID ) );
}

//=========================================================
//=========================================================
int CWeaponRifleGrenadeBase::GetAmmoTexID( void ) const
{
	if( m_bSightsUp )
		return GET_WEAPON_DATA_CUSTOM( RifleGrenadeInfo_t, iGrenadeAmmoTexID );

	return BaseClass::GetAmmoTexID( );
}

#endif
