//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_exhaustible_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponExhaustibleBase, DT_LocalExhaustibleWeaponData )

#ifdef GAME_DLL

	SendPropBool( SENDINFO( m_bRedraw ) ),

#else

	RecvPropBool( RECVINFO( m_bRedraw ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponExhaustibleBase, DT_WeaponExhaustibleBase )

BEGIN_NETWORK_TABLE( CWeaponExhaustibleBase, DT_WeaponExhaustibleBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalExhaustibleWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalExhaustibleWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalExhaustibleWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalExhaustibleWeaponData ) ),

#endif
	
END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponExhaustibleBase )

	DEFINE_PRED_FIELD( m_bRedraw, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponExhaustibleBase::CWeaponExhaustibleBase( )
{
	m_bRedraw = false;
}

//=========================================================
//=========================================================
void CWeaponExhaustibleBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

	m_bRedraw = false;
}

//=========================================================
//=========================================================
bool CWeaponExhaustibleBase::CanHolster( void )
{
	return ( BaseClass::CanHolster( ) && !m_bRedraw );
}

//=========================================================
//=========================================================
bool CWeaponExhaustibleBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	m_bRedraw = false;

	return BaseClass::Holster( pSwitchingTo );
}

//=========================================================
//=========================================================
void CWeaponExhaustibleBase::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame( );

	if( m_bRedraw && IsViewModelSequenceFinished( ) )
		Reload( );
}

//=========================================================
//=========================================================
bool CWeaponExhaustibleBase::CanReload( void )
{
	return false;
}

//=========================================================
//=========================================================
void CWeaponExhaustibleBase::Reload( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	if( ( m_bRedraw ) && ( m_flNextPrimaryAttack <= gpGlobals->curtime ) && ( m_flNextSecondaryAttack <= gpGlobals->curtime ) )
	{
		// mark this as done
		m_bRedraw = false;

		// set next redraw time
		m_flNextPrimaryAttack = m_flNextSecondaryAttack =
			m_flNextTertiaryAttack = m_flTimeWeaponIdle = gpGlobals->curtime;

		// switch to next best weapon
		if( !HasAmmo( ) )
		{
			pPlayer->SwitchToNextBestWeapon( this );
		}
		else
		{
			// reset the clip
			FinishReload( );

			// take action according to cmdregister
			if( pPlayer->GetCmdValue( CMDREGISTER_SWITCHDRAW ) )
			{
				pPlayer->SwitchToNextBestWeapon( this );
			}
			else
			{
				// redraw the weapon
				SendWeaponAnim( ACT_VM_DRAW );

				// update our times
				m_flNextPrimaryAttack = m_flNextSecondaryAttack =
					m_flNextTertiaryAttack = m_flTimeWeaponIdle = gpGlobals->curtime + SequenceDuration( );
			}
		}
	}
}

//=========================================================
//=========================================================
void CWeaponExhaustibleBase::UpdateIdleState( void )
{
	if( m_bRedraw )
		return;

	BaseClass::UpdateIdleState( );
}