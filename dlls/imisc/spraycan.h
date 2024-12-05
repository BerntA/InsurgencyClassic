//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SPRAYCAN_H
#define SPRAYCAN_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CSprayCan : public CPointEntity
{
	DECLARE_CLASS( CSprayCan, CPointEntity );

public:
	void Spawn( CBasePlayer *pOwner );

private:
	void Think( void );

	void Precache( void );

	int ObjectCaps( void );
};

#endif // SPRAYCAN_H