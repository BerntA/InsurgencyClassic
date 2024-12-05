//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_PLAYERLOCALDATA_H
#define INS_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

//=========================================================
//=========================================================
class CINSPlayerLocalData
{
	DECLARE_SIMPLE_DATADESC( );
	DECLARE_CLASS_NOBASE( CINSPlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR( );

public:
	CINSPlayerLocalData( );

public:
	CNetworkVar( bool, m_bAllowTeamChange );

	CNetworkVar( float, m_flWeightFactor );

	CNetworkVar( float, m_flLean );

	CNetworkHandle( CBaseEntity, m_BandagingPlayer );
	CNetworkVar( float, m_flBandageThreshold );
};

//=========================================================
//=========================================================
EXTERN_SEND_TABLE( DT_INSLocal );

#endif // INS_PLAYERLOCALDATA_H
