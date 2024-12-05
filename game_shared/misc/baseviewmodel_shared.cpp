//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "baseviewmodel_shared.h"
#include "datacache/imdlcache.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define VIEWMODEL_ANIMATION_PARITY_BITS 3

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

extern void RecvProxy_EffectFlags( const CRecvProxyData *pData, void *pStruct, void *pOut );

//=========================================================
//=========================================================
static void RecvProxy_Weapon( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CBaseViewModel *pViewModel = ( ( CBaseViewModel* )pStruct );
	CBaseCombatWeapon *pOldWeapon = pViewModel->GetOwningWeapon( );

	// chain through to the default receive proxy ...
	RecvProxy_IntToEHandle( pData, pStruct, pOut );

	// ... and reset our cycle index if the server is switching weapons on us
	CBaseCombatWeapon *pNewWeapon = pViewModel->GetOwningWeapon( );

	if( pNewWeapon != pOldWeapon )
	{
		// restart animation at frame 0
		pViewModel->SetCycle( 0 );
		pViewModel->m_flAnimTime = gpGlobals->curtime;
	}
}

//=========================================================
//=========================================================
void RecvProxy_SequenceNum( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	CBaseViewModel *model = (CBaseViewModel *)pStruct;
	if (pData->m_Value.m_Int != model->GetSequence())
	{
		model->SetSequence(pData->m_Value.m_Int);
		model->m_flAnimTime = gpGlobals->curtime;
		model->SetCycle(0);
	}
}

#endif

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CBaseViewModel )

	DEFINE_PRED_FIELD( m_nModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX ),
	DEFINE_PRED_FIELD( m_nSkin, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_nBody, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_NOERRORCHECK ),
	DEFINE_PRED_FIELD( m_nSequence, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD_TOL( m_flPlaybackRate, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, 0.125f ),
	DEFINE_PRED_FIELD( m_fEffects, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_OVERRIDE ),
	DEFINE_PRED_FIELD( m_nAnimationParity, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_hWeapon, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_flAnimTime, FIELD_FLOAT, 0 ),

	DEFINE_FIELD( m_hOwner, FIELD_EHANDLE ),
	DEFINE_FIELD( m_flTimeWeaponIdle, FIELD_FLOAT ),
	DEFINE_FIELD( m_Activity, FIELD_INTEGER ),
	DEFINE_FIELD( m_flCycle, FIELD_FLOAT ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( BaseViewModel, DT_BaseViewModel )

BEGIN_NETWORK_TABLE_NOBASE( CBaseViewModel, DT_BaseViewModel )

#ifdef CLIENT_DLL

	RecvPropInt( RECVINFO( m_nModelIndex ) ),
	RecvPropInt( RECVINFO( m_nSkin ) ),
	RecvPropInt( RECVINFO( m_nBody ) ),
	RecvPropInt( RECVINFO( m_nSequence ), 0, RecvProxy_SequenceNum ),
	RecvPropFloat( RECVINFO( m_flPlaybackRate ) ),
	RecvPropInt( RECVINFO( m_fEffects ), 0, RecvProxy_EffectFlags ),
	RecvPropInt( RECVINFO( m_nAnimationParity ) ),
	RecvPropEHandle( RECVINFO( m_hWeapon ), RecvProxy_Weapon ),
	RecvPropEHandle( RECVINFO( m_hOwner ) ),

	RecvPropInt( RECVINFO( m_nNewSequenceParity )),
	RecvPropInt( RECVINFO( m_nResetEventsParity )),
	RecvPropInt( RECVINFO( m_nMuzzleFlashParity )),

	RecvPropArray( RecvPropFloat( RECVINFO( m_flPoseParameter[ 0 ] ) ), m_flPoseParameter ),

#else

	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt( SENDINFO( m_nBody ), 8 ),
	SendPropInt( SENDINFO( m_nSkin ), 10 ),
	SendPropInt( SENDINFO( m_nSequence ), 8, SPROP_UNSIGNED ),
	SendPropFloat( SENDINFO( m_flPlaybackRate ), 8,	SPROP_ROUNDUP, -4.0, 12.0f ),
	SendPropInt( SENDINFO( m_fEffects ), 10, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nAnimationParity ), 3, SPROP_UNSIGNED ),
	SendPropEHandle( SENDINFO( m_hWeapon ) ),
	SendPropEHandle( SENDINFO( m_hOwner ) ),

	SendPropInt( SENDINFO( m_nNewSequenceParity ), EF_PARITY_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nResetEventsParity ), EF_PARITY_BITS, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_nMuzzleFlashParity ), EF_MUZZLEFLASH_BITS, SPROP_UNSIGNED ),

	SendPropArray( SendPropFloat( SENDINFO_ARRAY( m_flPoseParameter ), 8, 0, 0.0f, 1.0f ), m_flPoseParameter ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
CBaseViewModel::CBaseViewModel( )
{
#ifdef CLIENT_DLL

	// NOTE: we do this here because the color is never transmitted for the view model.
	m_nOldAnimationParity = 0;
	m_EntClientFlags |= ENTCLIENTFLAG_ALWAYS_INTERPOLATE;

#endif

	SetRenderColor( 255, 255, 255, 255 );

	m_sVMName = NULL_STRING;		
	m_sAnimationPrefix = NULL_STRING;

	m_nAnimationParity = 0;
}

//=========================================================
//=========================================================
CBaseViewModel::~CBaseViewModel( )
{
}

//=========================================================
//=========================================================
void CBaseViewModel::UpdateOnRemove( void )
{
	BaseClass::UpdateOnRemove( );
}

//=========================================================
//=========================================================
void CBaseViewModel::Precache( void )
{
}

//=========================================================
//=========================================================
void CBaseViewModel::Spawn( void )
{
	Precache( );
	SetSize( Vector( -8, -4, -2 ), Vector( 8, 4, 2 ) );
	SetSolid( SOLID_NONE );
}

//=========================================================
//=========================================================
void CBaseViewModel::SetOwner( CBasePlayer *pPlayer )
{
	m_hOwner = pPlayer;
}

//=========================================================
//=========================================================
void CBaseViewModel::SetWeaponModel( const char *modelname, CBaseCombatWeapon *weapon )
{
	m_hWeapon = weapon;

#ifdef CLIENT_DLL

	SetModel( modelname );

#else

	string_t str;

	if( modelname != NULL )
		str = MAKE_STRING( modelname );
	else
		str = NULL_STRING;

	if( str != m_sVMName )
	{
		m_sVMName = str;
		SetModel( STRING( m_sVMName ) );
	}

#endif
}

//=========================================================
//=========================================================
CBaseCombatWeapon *CBaseViewModel::GetOwningWeapon( void )
{
	return m_hWeapon.Get( );
}

//=========================================================
//=========================================================
void CBaseViewModel::SendViewModelMatchingSequence( int sequence )
{
	// since all we do is send a sequence number down to the client, 
	// set this here so other weapons code knows which sequence is playing.
	SetSequence( sequence );

	m_nAnimationParity = ( m_nAnimationParity + 1 ) & ( ( 1 << VIEWMODEL_ANIMATION_PARITY_BITS ) - 1 );

#ifdef CLIENT_DLL

	m_nOldAnimationParity = m_nAnimationParity;

	// force frame interpolation to start at exactly frame zero
	m_flAnimTime = gpGlobals->curtime;

#endif

	// restart animation at frame 0
	SetCycle( 0 );
	ResetSequenceInfo( );
}