//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_sniper_base.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponSniperBase, DT_WeaponSniperBase )

BEGIN_NETWORK_TABLE( CWeaponSniperBase, DT_WeaponSniperBase )

#ifdef GAME_DLL

	SendPropFloat( SENDINFO( m_flFinishIronsight ) ),
	SendPropBool( SENDINFO( m_bHideViewModel ) )

#else

	RecvPropTime( RECVINFO( m_flFinishIronsight ) ),
	RecvPropBool( RECVINFO( m_bHideViewModel ) )

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponSniperBase )

	DEFINE_PRED_FIELD_TOL( m_flFinishIronsight, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE ),	
	DEFINE_PRED_FIELD( m_bHideViewModel, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),	

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponSniperBase::CWeaponSniperBase( )
{
	m_flFinishIronsight = 0.0f;
	m_bHideViewModel = false;

#ifdef GAME_DLL

	m_bResetFOV = false;

#endif
}

//=========================================================
//=========================================================
void CWeaponSniperBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

#ifdef GAME_DLL

	UTIL_SendHint( GetINSPlayerOwner( ), HINT_SNIPER );

#endif
}

//=========================================================
//=========================================================
bool CWeaponSniperBase::ShouldDrawViewModel( void ) const
{
	return !m_bHideViewModel;
}

//=========================================================
//=========================================================
void CWeaponSniperBase::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame( );

#ifdef GAME_DLL

	if( m_bResetFOV && gpGlobals->curtime >= m_flNextPrimaryAttack )
	{
		SetPlayerIronsightFOV( true, 0.1f );

		m_bResetFOV = false;

		m_bHideViewModel = true;
	}

#endif

	if( CanUseIronsights( ) && m_flFinishIronsight != 0.0f && m_flFinishIronsight <= gpGlobals->curtime )
	{
		FinishedIronsights( );

		m_flFinishIronsight = 0.0f;
	}
}

//=========================================================
//=========================================================
#define SNIPER_IIN_FRACTION 0.25f

void CWeaponSniperBase::SetIronsightsState( bool bState )
{
	if( !Using3DScopes( ) )
	{
	#ifdef GAME_DLL

		m_bResetFOV = false;

	#endif

		if( bState )
		{
			m_flFinishIronsight = gpGlobals->curtime + ( SequenceDuration( ) * SNIPER_IIN_FRACTION );
		}
		else
		{
			m_bHideViewModel = false;
			m_flFinishIronsight = 0.0f;
		}
	}

	return BaseClass::SetIronsightsState( bState );
}

//=========================================================
//=========================================================
void CWeaponSniperBase::FinishedIronsights( void )
{
	m_bHideViewModel = true;
}

//=========================================================
//=========================================================
void CWeaponSniperBase::PrimaryAttack( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	BaseClass::PrimaryAttack( );

	if( IsBoltAction( ) && !Using3DScopes( ) )
	{
		m_bHideViewModel = false;

	#ifdef GAME_DLL

		if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		{
			SetPlayerIronsightFOV( false, 0.1f );

			m_bResetFOV = true;
		}

	#endif
	}
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CWeaponSniperBase::UseFreeaim( void ) const
{
	// RELEASEHACK: not needed for hack
   	/*CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) == 0 );*/

	return true;
}

#endif

//=========================================================
//=========================================================
bool CWeaponSniperBase::Using3DScopes( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && pPlayer->GetCmdValue( CMDREGISTER_3DSCOPE ) != 0 );
}

//=========================================================
//=========================================================
bool CWeaponSniperBase::CanUseIronsights( void )
{
	if( !BaseClass::CanUseIronsights( ) )
		return true;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );
	return ( pPlayer && ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD ) == 0 );
}