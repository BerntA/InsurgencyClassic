//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_killzone.h"
#include "hint_helper.h"
#include "ins_player_shared.h"
#include "ins_utils.h"
#include "world.h"

//=========================================================
//=========================================================
class CMineField : public CKillZone
{
public:
	DECLARE_CLASS( CMineField, CKillZone );

private:
	int GetHintDisplay( void ) const { return HINT_MINEFIELD; }

	void KillPlayer( CINSPlayer *pPlayer, const Vector &vecPoint );

};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_minefield, CMineField );

//=========================================================
//=========================================================
void CMineField::KillPlayer( CINSPlayer *pPlayer, const Vector &vecPoint )
{
	trace_t tr;
	Vector vecPlayerOrigin = pPlayer->GetAbsOrigin();
	Vector vecPlayerSize = pPlayer->WorldAlignSize();

	UTIL_TraceLine( vecPlayerOrigin, vecPlayerOrigin + Vector( 0, 0, - vecPlayerSize.z * 2.0f ), MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &tr );

	if( !tr.DidHit() )
	{
		// NOTE: what? how could this happen? I guess the mapper is silly.
		Assert( false );
		return;
	}

	UTIL_CreateExplosion( tr.endpos, GetWorldEntity( ), NULL, 100, vecPlayerSize.x + vecPlayerSize.y, DMG_INSTANT | DMG_MINEFIELD );
}