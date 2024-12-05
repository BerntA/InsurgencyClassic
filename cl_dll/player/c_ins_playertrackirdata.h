//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#ifndef C_INS_PLAYERTRACKIRDATA_H
#define C_INS_PLAYERTRACKIRDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "dt_recv.h"

//=========================================================
//=========================================================
class C_INSPlayerTrackIRData
{
	DECLARE_CLASS_NOBASE( C_INSPlayerTrackIRData );
	DECLARE_EMBEDDED_NETWORKVAR( );

public:
	C_INSPlayerTrackIRData( );

public:
	bool m_bActive;

	QAngle m_angHeadAngles;

	float m_flLean;
	float m_flLookOver;
	float m_flZoom;
};

//=========================================================
//=========================================================
EXTERN_RECV_TABLE( DT_INSPlayerTrackIR );

#endif // C_INS_PLAYERTRACKIRDATA_H
