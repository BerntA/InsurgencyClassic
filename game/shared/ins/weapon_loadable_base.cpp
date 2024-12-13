//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_loadable_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponLoadableBase, DT_LocalLoadableWeaponData )

#ifdef GAME_DLL

	SendPropBool( SENDINFO( m_bLoaded ) ),

#else

	RecvPropBool( RECVINFO( m_bLoaded ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponLoadableBase, DT_WeaponLoadableBase )

BEGIN_NETWORK_TABLE( CWeaponLoadableBase, DT_WeaponLoadableBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalLoadableWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalLoadableWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalLoadableWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalLoadableWeaponData ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponLoadableBase )

	DEFINE_PRED_FIELD( m_bLoaded, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponLoadableBase::CWeaponLoadableBase( )
{
	m_bLoaded = false;
}

//=========================================================
//=========================================================
void CWeaponLoadableBase::Spawn( void )
{
	BaseClass::Spawn( );

	m_bLoaded = false;
}

//=========================================================
//=========================================================
bool CWeaponLoadableBase::HasAmmo( void ) const
{
	if( m_bLoaded )
		return true;

	return BaseClass::HasAmmo( );
}

//=========================================================
//=========================================================
int CWeaponLoadableBase::GetAmmoCount( void ) const
{
	int iAmmoID = BaseClass::GetAmmoCount( );

	if( m_bLoaded )
		iAmmoID++;

	return iAmmoID;
}

//=========================================================
//=========================================================
bool CWeaponLoadableBase::IsEmptyAttack( void )
{
	return IsEmpty( );
}

//=========================================================
//=========================================================
bool CWeaponLoadableBase::CanReload( void )
{
	if( !BaseClass::CanReload( ) )
		return false;

	return IsEmpty( );
}

//=========================================================
//=========================================================
bool CWeaponLoadableBase::IsEmptyReload( void )
{
	return !HasAmmo( );
}

//=========================================================
//=========================================================
void CWeaponLoadableBase::FinishReload( void )
{
	BaseClass::FinishReload( );

	Load( );
}

//=========================================================
//=========================================================
bool CWeaponLoadableBase::IsEmpty( void ) const
{
	return !m_bLoaded;
}

//=========================================================
//=========================================================
void CWeaponLoadableBase::ForceReady( void )
{
	m_bLoaded = true;

	BaseClass::ForceReady( );
}

//=========================================================
//=========================================================
void CWeaponLoadableBase::Load( void )
{
	StripAmmo( );

	m_bLoaded = true;
}