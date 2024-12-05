//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "imc_config.h"
#include "ins_imcsync.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS( imcsync, CIMCSync );

//=========================================================
//=========================================================
IMPLEMENT_SERVERCLASS_ST_NOBASE( CIMCSync, DT_IMCSync )

	SendPropInt( SENDINFO( m_iVersion ) ),
	SendPropBool( SENDINFO( m_bOffical ) ),
	SendPropInt( SENDINFO( m_iRoundLength ) ),
	SendPropBool( SENDINFO( m_bHasCustomGravity ) ),

END_SEND_TABLE()

//=========================================================
//=========================================================
CIMCSync::CIMCSync( void )
{
	m_iVersion = IMCConfig( )->GetVersion( );
	m_bOffical = IMCConfig( )->IsCertifiedIMC( );
	m_iRoundLength = IMCConfig( )->GetRoundLength( );
	m_bHasCustomGravity = IMCConfig( )->HasCustomGravity( );

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CIMCSync::Create( void )
{
	CreateEntityByName( "imcsync" );
}

//=========================================================
//=========================================================
int CIMCSync::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}