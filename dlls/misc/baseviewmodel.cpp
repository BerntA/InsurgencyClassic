//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "animation.h"
#include "baseviewmodel.h"
#include "player.h"
#include <keyvalues.h>
#include "studio.h"
#include "vguiscreen.h"
#include "saverestore_utlvector.h"
#include "hltvdirector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void SendProxy_AnimTime( const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID );
void SendProxy_SequenceChanged( const void *pStruct, const void *pVarData, DVariant *pOut, int iElement, int objectID );

//=========================================================
//=========================================================
BEGIN_DATADESC( CBaseViewModel )

	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),

	DEFINE_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT ),
	DEFINE_FIELD( m_nAnimationParity, FIELD_INTEGER ),

	DEFINE_FIELD( m_hWeapon, FIELD_EHANDLE ),

END_DATADESC( )

//=========================================================
//=========================================================
int CBaseViewModel::UpdateTransmitState( void )
{
	CBaseCombatWeapon *pWeapon = GetOwningWeapon( );

	if( !pWeapon || IsEffectActive( EF_NODRAW ) )
		return SetTransmitState( FL_EDICT_DONTSEND );

	return SetTransmitState( FL_EDICT_FULLCHECK );
}

//=========================================================
//=========================================================
int CBaseViewModel::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	// check if receipient owns this weapon viewmodel
	CBasePlayer *pOwner = ToBasePlayer( m_hOwner );

	if ( pOwner && pOwner->edict( ) == pInfo->m_pClientEnt )
		return FL_EDICT_ALWAYS;

	// check if recipient spectates the own of this viewmodel
	CBaseEntity *pRecipientEntity = CBaseEntity::Instance( pInfo->m_pClientEnt );

	if( pRecipientEntity->IsPlayer( ) )
	{
		CBasePlayer *pPlayer = static_cast< CBasePlayer* >( pRecipientEntity );

		// if this is the HLTV client, transmit all viewmodels in our PVS
		if( pPlayer->IsHLTV( ) )
			return FL_EDICT_PVSCHECK;
	}

	// don't send to anyone else except the local player or his spectators
	return FL_EDICT_DONTSEND;
}

//=========================================================
//=========================================================
void CBaseViewModel::SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways )
{
	// are we already marked for transmission?
	if( pInfo->m_pTransmitEdict->Get( entindex( ) ) )
		return;

	BaseClass::SetTransmit( pInfo, bAlways );
}