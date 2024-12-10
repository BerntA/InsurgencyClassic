//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_PLANEZONE_SHARED_H
#define INS_PLANEZONE_SHARED_H

#ifdef _WIN32
#pragma once
#endif

#include "ins_touch.h"

//=========================================================
//=========================================================
class CINSPlayer;

//=========================================================
//=========================================================
struct ZonedPlaneData_t
{
	Vector m_vecNormal, m_vecCentre;
	float m_flPerpLength;
};

//=========================================================
//=========================================================
class CPlaneZone : public CINSTouch
{
public:
	DECLARE_CLASS( CPlaneZone, CINSTouch );

protected:

#ifdef GAME_DLL

	bool NotSolidSpawn( void ) const { return false; }
	bool TouchSetupSpawn( void ) const { return false; }

	void Spawn( void );

	virtual bool Init( void ) = 0;

	virtual void PlayerStartTouch( CINSPlayer *pPlayer );

	virtual int GetHintDisplay( void ) const;

	virtual bool MustFindProtectedSurface( void ) const = 0;
	virtual bool StopFirstProtectedSurface( void ) const = 0;
	int DetermineProtectedSurfaces( void );
	virtual void FoundProtectedSurface( Vector &vecNormal, float flPerpLength, Vector &vecCentre ) = 0;

#endif

	float GetInternalPlaneDistance( const ZonedPlaneData_t &PlaneData, const Vector &vecPoint );
	float GetInternalPlaneDistance( const Vector &vecNormal, const Vector &vecCentre, const Vector &vecPoint );
};

#endif // INS_PLANEZONE_SHARED_H