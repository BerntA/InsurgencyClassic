//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef FIREBULLETS_H
#define FIREBULLETS_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class IFireBullets
{
public:
	virtual CBaseEntity *GetAttacker( void ) = 0;
	virtual CBaseEntity *GetInflictor( void ) = 0;
	virtual int GetBulletType( void ) const = 0;
	virtual int GetDamageType( void ) const { return DMG_BULLET; }
	virtual int GetMuzzleVelocity( void ) const = 0;
	virtual int GetShots( void ) const { return 1; }
	virtual Vector GetSpread( void ) const = 0;
	virtual int GetRange( void ) const = 0;
};

//=========================================================
//=========================================================
extern float UTIL_BulletImpact( int iBulletType, float flVelocity,  float flTravelled, CTakeDamageInfo *pDamage );

//=========================================================
//=========================================================
void UTIL_FireBullets( IFireBullets *pWeapon, const Vector &vecOrigin, const Vector &vecDir, int iTracerType );

#ifdef CLIENT_DLL

extern void UTIL_FireBulletsEffect( IFireBullets *pWeapon, const Vector &vecOrigin, const Vector &vecDir, int iSeed );

#endif

#endif // FIREBULLETS_H