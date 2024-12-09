//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "ammodef.h"
#include "tier0/vprof.h"
#include "keyvalues.h"

#ifdef GAME_DLL

#include <ctype.h>

#include "game.h"
#include "player_resource.h"
#include "team.h"
#include "view_team.h"
#include "play_team.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CGameRulesProxy *CGameRulesProxy::s_pGameRulesProxy = NULL;

IMPLEMENT_NETWORKCLASS_ALIASED( GameRulesProxy, DT_GameRulesProxy )
	BEGIN_NETWORK_TABLE_NOBASE( CGameRulesProxy, DT_GameRulesProxy )
END_NETWORK_TABLE()


//=========================================================
//=========================================================
CGameRulesProxy::CGameRulesProxy()
{
	Assert( !s_pGameRulesProxy );
	s_pGameRulesProxy = this;
}

//=========================================================
//=========================================================
CGameRulesProxy::~CGameRulesProxy()
{
	Assert( s_pGameRulesProxy );
	s_pGameRulesProxy = NULL;
}

//=========================================================
//=========================================================
int CGameRulesProxy::UpdateTransmitState( void )
{
#ifdef GAME_DLL
	return SetTransmitState( FL_EDICT_ALWAYS );
#else
	return 0;
#endif

}

//=========================================================
//=========================================================
void CGameRulesProxy::NotifyNetworkStateChanged()
{
	if ( s_pGameRulesProxy )
		s_pGameRulesProxy->NetworkStateChanged();
}

//=========================================================
//=========================================================

#ifdef GAME_DLL

//=========================================================
//=========================================================
CGameRules::CGameRules( )
	: CAutoGameSystemPerFrame( "CGameRules" )
{
	Assert( !g_pGameRules );
	g_pGameRules = this;

	ClearMultiDamage( );
}

#else

//=========================================================
//=========================================================
CGameRules::CGameRules( )
	: CAutoGameSystemPerFrame( "CGameRules" )
{
	Assert( !g_pGameRules );
	g_pGameRules = this;
}
#endif

//=========================================================
//=========================================================
CGameRules::~CGameRules( )
{
	Assert( g_pGameRules == this );
	g_pGameRules = NULL;
}

//=========================================================
//=========================================================
bool CGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		::V_swap(collisionGroup0,collisionGroup1);
	}

	if ( collisionGroup0 == COLLISION_GROUP_DEBRIS && collisionGroup1 == COLLISION_GROUP_PUSHAWAY )
	{
		// let debris and multiplayer objects collide
		return true;
	}

	// --------------------------------------------------------------------------
	// NOTE: All of this code assumes the collision groups have been sorted!!!!
	// NOTE: Don't change their order without rewriting this code !!!
	// --------------------------------------------------------------------------

	if ( ( collisionGroup1 == COLLISION_GROUP_DOOR_BLOCKER ) && ( collisionGroup0 != COLLISION_GROUP_NPC ) )
		return false;

	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER ) && ( collisionGroup1 == COLLISION_GROUP_PASSABLE_DOOR ) )
		return false;

	if ( collisionGroup0 == COLLISION_GROUP_DEBRIS || collisionGroup0 == COLLISION_GROUP_DEBRIS_TRIGGER )
	{
		// put exceptions here, right now this will only collide with COLLISION_GROUP_NONE
		return false;
	}

	// Dissolving guys only collide with COLLISION_GROUP_NONE
	if ( (collisionGroup0 == COLLISION_GROUP_DISSOLVING) || (collisionGroup1 == COLLISION_GROUP_DISSOLVING) )
	{
		if ( collisionGroup0 != COLLISION_GROUP_NONE )
			return false;
	}

	// doesn't collide with other members of this group
	// or debris, but that's handled above
	if ( collisionGroup0 == COLLISION_GROUP_INTERACTIVE_DEBRIS && collisionGroup1 == COLLISION_GROUP_INTERACTIVE_DEBRIS )
		return false;

	if ( collisionGroup0 == COLLISION_GROUP_BREAKABLE_GLASS && collisionGroup1 == COLLISION_GROUP_BREAKABLE_GLASS )
		return false;

	// interactive objects collide with everything except debris & interactive debris
	if ( collisionGroup1 == COLLISION_GROUP_INTERACTIVE && collisionGroup0 != COLLISION_GROUP_NONE )
		return false;

	// Projectiles hit everything but debris, weapons, + other projectiles
	if ( collisionGroup1 == COLLISION_GROUP_PROJECTILE )
	{
		if ( collisionGroup0 == COLLISION_GROUP_DEBRIS || 
			collisionGroup0 == COLLISION_GROUP_WEAPON ||
			collisionGroup0 == COLLISION_GROUP_PROJECTILE )
		{
			return false;
		}
	}

	// Don't let vehicles collide with weapons
	// Don't let players collide with weapons...
	// Don't let NPCs collide with weapons
	// Weapons are triggers, too, so they should still touch because of that
	if ( collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		if (collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_NPC)
		{
			return false;
		}
	}

	return true;
}

//=========================================================
//=========================================================
static CViewVectors g_DefaultViewVectors(

	// Standing
	Vector( -16, -16, 0 ),
	Vector(  16,  16, 72 ),
	Vector( 0, 0, 64 ),

	// Crouching
	Vector( -16, -16, 0 ),
	Vector(  16,  16, 45 ),
	Vector( 0, 0, 45 ),

	// Prone
	Vector( -16, -16, 0 ),
	Vector(  16,  16, 16 ),
	Vector( 0,  0, 24 ),
	
	// OBS
	Vector( -10, -10, -10 ),
	Vector(  10,  10,  10 ),
	
	// Dead
	Vector( 0, 0, 14 )
);

//=========================================================
//=========================================================
const CViewVectors* CGameRules::GetViewVectors( void ) const
{
	return &g_DefaultViewVectors;
}

//=========================================================
//=========================================================

#ifdef GAME_DLL

CGameRules*	g_pGameRules = NULL;

//=========================================================
//=========================================================
void CGameRules::LevelInitPreEntity( void )
{
	CPlayerResource::Create( );
}

//=========================================================
//=========================================================
void CGameRules::FrameUpdatePostEntityThink( void )
{
	VPROF( "CGameRules::FrameUpdatePostEntityThink" );

	Think( );
}

//=========================================================
//=========================================================
void CGameRules::EndGameFrame( void )
{
	Assert( g_MultiDamage.IsClear( ) );

	if ( !g_MultiDamage.IsClear( ) )
	{
		Warning( "Unapplied multidamage left in the system:\nTarget: %s\nInflictor: %s\nAttacker: %s\nDamage: %.2f\n", 
			g_MultiDamage.GetTarget( )->GetDebugName(),
			g_MultiDamage.GetInflictor( )->GetDebugName(),
			g_MultiDamage.GetAttacker( )->GetDebugName(),
			g_MultiDamage.GetDamage( ) );
		ApplyMultiDamage( );
	}
}

//=========================================================
//=========================================================
void CGameRules::GetNextLevelName( char *pszNextMap, int bufsize )
{
	char szFirstMapInList[32];
	Q_strncpy( szFirstMapInList, "ins_baghdad" ,sizeof(szFirstMapInList));

	// find the map to change to

	const char *mapcfile = mapcyclefile.GetString();
	Assert( mapcfile != NULL );
	Q_strncpy( pszNextMap, STRING(gpGlobals->mapname) ,bufsize);
	Q_strncpy( szFirstMapInList, STRING(gpGlobals->mapname) ,sizeof(szFirstMapInList));

	int length;
	char *pFileList;
	char *aFileList = pFileList = (char*)UTIL_LoadFileForMe( mapcfile, &length );
	if ( pFileList && length )
	{
		// the first map name in the file becomes the default
		sscanf( pFileList, " %31s", pszNextMap );
		if ( engine->IsMapValid( pszNextMap ) )
			Q_strncpy( szFirstMapInList, pszNextMap ,sizeof(szFirstMapInList));

		// keep pulling mapnames out of the list until the current mapname
		// if the current mapname isn't found,  load the first map in the list
		bool next_map_is_it = false;
		while ( 1 )
		{
			while ( *pFileList && isspace( *pFileList ) ) pFileList++; // skip over any whitespace
			if ( !(*pFileList) )
				break;

			char cBuf[32];
			int ret = sscanf( pFileList, " %31s", cBuf );
			// Check the map name is valid
			if ( ret != 1 || *cBuf < 13 )
				break;

			if ( next_map_is_it )
			{
				// check that it is a valid map file
				if ( engine->IsMapValid( cBuf ) )
				{
					Q_strncpy( pszNextMap, cBuf, bufsize);
					break;
				}
			}

			if ( FStrEq( cBuf, STRING(gpGlobals->mapname) ) )
			{  // we've found our map;  next map is the one to change to
				next_map_is_it = true;
			}

			pFileList += strlen( cBuf );
		}

		UTIL_FreeFile( (byte *)aFileList );
	}

	if ( !engine->IsMapValid(pszNextMap) )
		Q_strncpy( pszNextMap, szFirstMapInList, bufsize);
}

//=========================================================
//=========================================================
void CGameRules::ChangeLevel( void )
{
	char szNextMap[ 32 ];
	GetNextLevelName( szNextMap, sizeof( szNextMap ) );

	Msg( "CHANGE LEVEL: %s\n", szNextMap );
	engine->ChangeLevel( szNextMap, NULL );
}

#endif // GAME_DLL