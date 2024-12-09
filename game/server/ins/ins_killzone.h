//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_KILLZONE_H
#define INS_KILLZONE_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_planezone_shared.h"

//=========================================================
//=========================================================
class CKillZone : public CPlaneZone
{
public:
	DECLARE_CLASS( CKillZone, CPlaneZone );
	DECLARE_DATADESC( );

protected:

	virtual void KillPlayer( CINSPlayer *pPlayer, const Vector &vecPoint ) = 0;

private:

	bool Init( void );

	void PlayerThink( void );

	void PlayerStartTouch( CINSPlayer *pPlayer );
	void PlayerEndTouch( CINSPlayer *pPlayer );

	bool MustFindProtectedSurface( void ) const { return true; } // NOTE: when not killing always, should be false
	bool StopFirstProtectedSurface( void ) const { return false; }
	void FoundProtectedSurface( Vector &vecNormal, float flPerpLength, Vector &vecCentre );

private:
	CUtlVector< CINSPlayer* > m_Players;

	CUtlVector< ZonedPlaneData_t > m_PlaneData;
};

#endif // INS_KILLZONE_H