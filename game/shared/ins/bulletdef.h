//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef BULLETDEF_H
#define BULLETDEF_H

#ifdef _WIN32
#pragma once
#endif

#include "ammodef.h"

//=========================================================
//=========================================================
class CBulletData : public IAmmoData
{
public:
	CBulletData( );

private:
	void Parse( KeyValues *pData );

	void LevelInit( void );

public:
	int	m_iMinSplashSize;
	int	m_iMaxSplashSize;

	int m_iWeight;				// in g
	float m_flXArea;			// in mm
	int m_iMuzzleVelocity;		// in m/s
	float m_flFormFactor;

	float m_flPartPhysicsImpulse;

#ifdef CLIENT_DLL

	char m_szShellModel[ MAX_WEAPON_STRING ];
	model_t *m_pShellModel;

#endif
};

//=========================================================
//=========================================================
class CBulletDef : public IAmmoType
{
public:
	static const CBulletData &GetBulletData( int iType );
	
	// IAmmoType
	const char *GetHeader( void ) const;
	int ConvertID( const char *pszID ) const;

	int GetDataCount( void ) const;
	IAmmoData *GetData( int iID );

private:
	CBulletData m_Data[ BULLET_COUNT ];
};

#endif // BULLETDEF_H
