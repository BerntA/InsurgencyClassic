//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_SPAWNPROTECTION_SHARED_H
#define INS_SPAWNPROTECTION_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_planezone_shared.h"

#ifdef CLIENT_DLL

#define CSpawnProtection C_SpawnProtection

#endif

//=========================================================
//=========================================================
class CSpawnProtection : public CPlaneZone
{
public:
	DECLARE_CLASS( CSpawnProtection, CPlaneZone );
	DECLARE_NETWORKCLASS( );

#ifdef GAME_DLL

	DECLARE_DATADESC( );

#endif

public:
	void ClampMoveVelocity( CINSPlayer *pPlayer, Vector &vecVelocity );

private:

#ifdef GAME_DLL

	bool Init( void );

	void PlayerStartTouch( CINSPlayer *pPlayer );
	void PlayerEndTouch( CINSPlayer *pPlayer );

	bool MustFindProtectedSurface( void ) const { return true; }
	bool StopFirstProtectedSurface( void ) const { return true; }
	void FoundProtectedSurface( Vector &vecNormal, float flPerpLength, Vector &vecCentre );

#endif

private:

#ifdef _DEBUG

	CUtlVector< CINSPlayer* > m_Players;

#endif

	CNetworkVar( int, m_iObjID );

	CNetworkVector( m_vecStop );
	CNetworkVar( int, m_iStopLength );

	CNetworkVector( m_vecPlaneNormal );
};

#endif // INS_SPAWNPROTECTION_SHARED_H