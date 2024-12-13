//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef EXPLOSIVEDEF_H
#define EXPLOSIVEDEF_H

#ifdef _WIN32
#pragma once
#endif

#include "ammodef.h"

//=========================================================
//=========================================================
class CExplosiveData : public IAmmoData
{
public:
	CExplosiveData( );

protected:
	void Parse( KeyValues *pData );

	void Precache( void );

public:
	char m_szModel[ MAX_WEAPON_STRING ];

#ifdef GAME_DLL

	char m_szDetonationSound[ 64 ];
	bool m_bPrecacheDetonateSound;

	int	m_iDamage;
	int m_iDamageRadius;

#endif
};

#endif // EXPLOSIVEDEF_H