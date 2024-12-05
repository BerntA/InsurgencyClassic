//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef GRENADEDEF_H
#define GRENADEDEF_H

#ifdef _WIN32
#pragma once
#endif

#include "explosivedef.h"

//=========================================================
//=========================================================
class CGrenadeData : public CExplosiveData
{
	DECLARE_BASECLASS( CExplosiveData );

public:
	CGrenadeData( );

private:
	void Parse( KeyValues *pData );

public:
	float m_flFuse;
};

//=========================================================
//=========================================================
class CGrenadeDef : public IAmmoType
{
public:
	static const CGrenadeData &GetGrenadeData( int iType );

	// IAmmoType
	const char *GetHeader( void ) const;
	int ConvertID( const char *pszID ) const;

	int GetDataCount( void ) const;
	IAmmoData *GetData( int iID );

private:
	CGrenadeData m_Data[ GRENADE_COUNT ];
};

#endif // GRENADEDEF_H