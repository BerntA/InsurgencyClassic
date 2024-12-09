//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_killzone.h"
#include "hint_helper.h"
#include "world.h"
#include "firebullets.h"
#include "ammodef.h"
#include "ins_player_shared.h"

//=========================================================
//=========================================================
class CSniperZoneWeapon : public IFireBullets
{
public:
	CBaseEntity *GetAttacker( void ) { return GetWorldEntity( ); }
	CBaseEntity *GetInflictor( void ) { return GetWorldEntity( ); }
	int GetBulletType( void ) const { return BULLET_762NATO; }
	int GetMuzzleVelocity( void ) const { return 600; }
	int GetShots( void ) const { return 1; }
	Vector GetSpread( void ) const { return vec3_origin; }
	int GetRange( void ) const { return 4096; }

private:
	CBaseEntity *m_pAttacker;
	int m_iAmmoType;
	int m_iShots;
	Vector m_vecSpread;
	int m_iRange;
};

//=========================================================
//=========================================================
class CSniperZone : public CKillZone
{
public:
	DECLARE_CLASS( CSniperZone, CKillZone );

private:
	int GetHintDisplay( void ) const { return HINT_SNIPER; }

	void KillPlayer( CINSPlayer *pPlayer, const Vector &vecPoint );

};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_sniperzone, CSniperZone );

//=========================================================
//=========================================================
#define HEAD_BONE 10

void CSniperZone::KillPlayer( CINSPlayer *pPlayer, const Vector &vecPoint )
{
	Vector vecTarget;
	QAngle angTemp;

	pPlayer->GetBonePosition( HEAD_BONE, vecTarget, angTemp );

	CSniperZoneWeapon SniperWeapon;
	UTIL_FireBullets( &SniperWeapon, vecPoint, vecTarget - vecPoint, TRACERTYPE_NONE );
}