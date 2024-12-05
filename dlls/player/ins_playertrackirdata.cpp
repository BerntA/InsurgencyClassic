//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_playertrackirdata.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
BEGIN_SEND_TABLE_NOBASE( CINSPlayerTrackIRData, DT_INSPlayerTrackIR )

	SendPropQAngles( SENDINFO( m_angHeadAngles ) ),

END_SEND_TABLE( )

//=========================================================
//=========================================================
CINSPlayerTrackIRData::CINSPlayerTrackIRData( )
{
	m_flLean = 0.0f;
}