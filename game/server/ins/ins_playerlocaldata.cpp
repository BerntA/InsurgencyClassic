//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_playerlocaldata.h"
#include "ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
BEGIN_SEND_TABLE_NOBASE( CINSPlayerLocalData, DT_INSLocal )

	SendPropBool( SENDINFO( m_bAllowTeamChange ) ),

	SendPropFloat( SENDINFO( m_flWeightFactor ) ),

	SendPropFloat( SENDINFO( m_flLean ) ),

	SendPropEHandle( SENDINFO( m_BandagingPlayer ) ),
	SendPropFloat( SENDINFO( m_flBandageThreshold ) )

END_SEND_TABLE( )

//=========================================================
//=========================================================
BEGIN_SIMPLE_DATADESC( CINSPlayerLocalData )

END_DATADESC( )

//=========================================================
//=========================================================
CINSPlayerLocalData::CINSPlayerLocalData( )
{

}