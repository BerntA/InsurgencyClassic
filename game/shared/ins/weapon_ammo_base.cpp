//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "weapon_ammo_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define AMMOCOUNT_BITS 3

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponAmmoBase, DT_LocalAmmoWeaponData )

#ifdef GAME_DLL

	SendPropInt( SENDINFO( m_iAmmoCount ), AMMOCOUNT_BITS ),

#else

	RecvPropInt( RECVINFO( m_iAmmoCount ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponAmmoBase, DT_WeaponAmmoBase )

BEGIN_NETWORK_TABLE( CWeaponAmmoBase, DT_WeaponAmmoBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalAmmoWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalAmmoWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalAmmoWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalAmmoWeaponData ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponAmmoBase )

	DEFINE_PRED_FIELD( m_iAmmoCount, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponAmmoBase::CWeaponAmmoBase( )
{
	m_iAmmoCount = 0;
}

//=========================================================
//=========================================================
bool CWeaponAmmoBase::HasAmmo( void ) const
{
	return ( m_iAmmoCount > 0 );
}

//=========================================================
//=========================================================
int CWeaponAmmoBase::GetAmmoCount( void ) const
{
	return m_iAmmoCount;
}

//=========================================================
//=========================================================
void CWeaponAmmoBase::GiveAmmo( int iCount )
{
	m_iAmmoCount += iCount;
}

//=========================================================
//=========================================================
void CWeaponAmmoBase::StripAmmo( void )
{
	m_iAmmoCount = max( m_iAmmoCount - 1, 0 );
}