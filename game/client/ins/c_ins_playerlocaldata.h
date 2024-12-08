//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#ifndef C_INS_PLAYERLOCALDATA_H
#define C_INS_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "dt_recv.h"

//=========================================================
//=========================================================
class CINSPlayerLocalData
{
	DECLARE_PREDICTABLE( );
	DECLARE_CLASS_NOBASE( CINSPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR( );

public:
	CINSPlayerLocalData( );

public:
	bool m_bAllowTeamChange;

	float m_flWeightFactor;

	float m_flLean;

	EHANDLE m_BandagingPlayer;
	float m_flBandageThreshold;
};

//=========================================================
//=========================================================
EXTERN_RECV_TABLE( DT_INSLocal );

#endif // C_INS_PLAYERLOCALDATA_H
