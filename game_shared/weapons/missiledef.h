//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef MISSILEDEF_H
#define MISSILEDEF_H

#ifdef _WIN32
#pragma once
#endif

#include "explosivedef.h"

//=========================================================
//=========================================================
class CMissileData : public CExplosiveData
{
	DECLARE_BASECLASS( CExplosiveData );

public:
	CMissileData( );

private:
	void Parse( KeyValues *pData );

public:
	int m_iSafetyRange;
};

//=========================================================
//=========================================================
class CMissileDef : public IAmmoType
{
public:
	static const CMissileData &GetMissileData( int iType );

	// IAmmoType
	const char *GetHeader( void ) const;
	int ConvertID( const char *pszID ) const;

	int GetDataCount( void ) const;
	IAmmoData *GetData( int iID );

private:
	CMissileData m_Data[ MISSILE_COUNT ];
};

#endif // MISSILEDEF_H