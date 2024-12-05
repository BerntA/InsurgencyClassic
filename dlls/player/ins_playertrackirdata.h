//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_PLAYERTRACKIRDATA_H
#define INS_PLAYERTRACKIRDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

//=========================================================
//=========================================================
class CINSPlayerTrackIRData
{
	DECLARE_CLASS_NOBASE( CINSPlayerTrackIRData );
	DECLARE_EMBEDDED_NETWORKVAR( );

public:
	CINSPlayerTrackIRData( );

public:
	CNetworkVar( QAngle, m_angHeadAngles );

	float m_flLean;
};

//=========================================================
//=========================================================
EXTERN_SEND_TABLE( DT_INSPlayerTrackIR );

#endif // INS_PLAYERTRACKIRDATA_H



