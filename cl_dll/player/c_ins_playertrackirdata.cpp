//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_ins_playertrackirdata.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
BEGIN_RECV_TABLE_NOBASE( C_INSPlayerTrackIRData, DT_INSPlayerTrackIR )

	RecvPropQAngles( RECVINFO( m_angHeadAngles ) ),

END_RECV_TABLE( )

//=========================================================
//=========================================================
C_INSPlayerTrackIRData::C_INSPlayerTrackIRData( )
{
	m_bActive = false;

	m_flLean = 0.0f;
	m_flLookOver = 0.0f;
	m_flZoom = 0.0f;
}