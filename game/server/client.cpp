//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== client.cpp ========================================================

  client/server game specific stuff

*/

#include "cbase.h"
#include "player.h"
#include "client.h"
#include "soundent.h"
#include "gamerules.h"
#include "game.h"
#include "physics.h"
#include "entitylist.h"
#include "shake.h"
#include "ndebugoverlay.h"
#include "engine/IEngineSound.h"
#include <ctype.h>
#include "tier1/strtools.h"
#include "te_effect_dispatch.h"
#include "globals.h"
#include "team.h"
#include "datacache/imdlcache.h"
#include "voice_gamemgr.h"
#include "GameDefinitions_Shared.h"

#ifdef HL2_DLL
#include "hl2_shared_misc.h"
#endif

#include "weapon_ballistic_base.h"
#include "weapon_defines.h"
#include "ins_player.h"
#include "ins_player_shared.h"
#include "ins_utils.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// For not just using one big ai net
extern CBaseEntity*	FindPickerEntity( CBasePlayer* pPlayer );

ConVar  *sv_cheats = NULL;

void ClientKill( edict_t *pEdict, const Vector &vecForce, bool bExplode = false )
{
	CBasePlayer *pPlayer = static_cast<CBasePlayer*>( GetContainingEntity( pEdict ) );
	if (pPlayer)
		pPlayer->CommitSuicide(vecForce, bExplode);
}

char* CheckChatText(char* text)
{
	char *p = text;

	// invalid if NULL or empty
	if ( !text || !text[0] )
		return NULL;

	int length = Q_strlen( text );

	// remove quotes (leading & trailing) if present
	if (*p == '"')
	{
		p++;
		length -=2;
		p[length] = 0;
	}

	// cut off after 127 chars
	if ( length > 127 )
		text[127] = 0;

	return p;
}

void Host_Say(edict_t* pEdict, const CCommand& args, int chatType)
{	
	int			j;
	char*		p;
	char		text[256];
	char		szTemp[256];
	const char* pcmd = args[0];

	// We can get a raw string now, without the "say " prepended
	if ( args.ArgC() == 0 )
		return;

	if ( !stricmp( pcmd, "say") || !stricmp(pcmd, "say_team") || !stricmp(pcmd, "say_squad"))
	{
		if ( args.ArgC() >= 2 )
		{
			p = (char *)args.ArgS();
		}
		else
		{
			// say with a blank message, nothing to do
			return;
		}
	}
	else  // Raw text, need to prepend argv[0]
	{
		if ( args.ArgC() >= 2 )
		{
			Q_snprintf( szTemp,sizeof(szTemp), "%s %s", ( char * )pcmd, (char *)args.ArgS() );
		}
		else
		{
			// Just a one word command, use the first word...sigh
			Q_snprintf( szTemp,sizeof(szTemp), "%s", ( char * )pcmd );
		}
		p = szTemp;
	}

	CINSPlayer* pPlayer = NULL;
	if ( pEdict )
	{
		pPlayer = ToINSPlayer(CBaseEntity::Instance(pEdict));
		Assert(pPlayer);
	}

	p = CheckChatText(p);
	if ( !p )
		return;

	const char *pszPlayerName = pPlayer ? pPlayer->GetPlayerName() : "Console";

	if ( pEdict )
	{
		// See if the player wants to modify of check the text
		pPlayer->CheckChatText( p, 127 );	// though the buffer szTemp that p points to is 256, 
											// chat text is capped to 127 in CheckChatText above
		Assert((pszPlayerName && pszPlayerName[0]));
	}

	const char *pszFormat = NULL;

	switch (chatType)
	{

	case SAYTYPE_GLOBAL:
	{
		pszFormat = "HL2MP_Chat_All";
		break;
	}

	case SAYTYPE_TEAM:
	{
		pszFormat = "HL2MP_Chat_Team";
		break;
	}

	case SAYTYPE_SQUAD:
	{
		pszFormat = "HL2MP_Chat_Squad";
		break;
	}

	}

	Q_snprintf(text, sizeof(text), "%s: ", pszPlayerName);

	j = sizeof(text) - 2 - strlen(text);  // -2 for /n and null terminator
	if ( (int)strlen(p) > j )
		p[j] = 0;

	Q_strncat( text, p, sizeof( text ), COPY_ALL_CHARACTERS );
	Q_strncat( text, "\n", sizeof( text ), COPY_ALL_CHARACTERS );
 
	CINSPlayer* client = NULL;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		client = ToINSPlayer(UTIL_PlayerByIndex(i));

		if (!client || !client->edict() || !client->IsNetClient())
			continue;
		
		if (client->edict() == pEdict)
			continue;

		if (pPlayer)
		{
			// check dead states
			if (!INSRules()->PlayerCanCommunicate(client, pPlayer))
				continue;

			switch (chatType)
			{

			case SAYTYPE_TEAM:
			{
				if (!OnSameTeam(pPlayer, client))
					continue;
				break;
			}

			case SAYTYPE_SQUAD:
			{
				if (!OnSameTeam(pPlayer, client) || !OnSameSquad(pPlayer, client))
					continue;
				break;
			}

			}
		}

		// skip chat from muted plrs or not ???
		//if ( pPlayer && GetVoiceGameMgr() && GetVoiceGameMgr()->IsPlayerIgnoringPlayer( pPlayer->entindex(), i ) )
		//	continue;

		CSingleUserRecipientFilter user( client );
		user.MakeReliable();

		if ( pszFormat )
		{
			UTIL_SayText2Filter(user, pPlayer, true, pszFormat, pszPlayerName, p);
		}
		else
		{
			UTIL_SayTextFilter( user, text, pPlayer, true );
		}
	}

	if (pPlayer)
	{
		// print to the sending client
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		if (pszFormat)
			UTIL_SayText2Filter(user, pPlayer, true, pszFormat, pszPlayerName, p);
		else
			UTIL_SayTextFilter(user, text, pPlayer, true);
	}

	// echo to server console
	// Adrian: Only do this if we're running a dedicated server since we already print to console on the client.
	if (engine->IsDedicatedServer())
		Msg("%s", text);

	Assert(p);

	int userid = 0;
	const char *networkID = "Console";
	const char *playerName = "Console";
	const char *playerTeam = "Console";

	if ( pPlayer )
	{
		userid = pPlayer->GetUserID();
		networkID = pPlayer->GetNetworkIDString();
		playerName = pPlayer->GetPlayerName();
		CTeam *team = pPlayer->GetTeam();
		if (team)
			playerTeam = team->GetName();

		pPlayer->BumpStat(PLAYERSTATS_TALKED);
	}
	
	switch (chatType)
	{

	case SAYTYPE_TEAM:
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" say_team \"%s\"\n", playerName, userid, networkID, playerTeam, p);
		break;
	}

	case SAYTYPE_SQUAD:
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" say_squad \"%s\"\n", playerName, userid, networkID, playerTeam, p);
		break;
	}

	default:
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" say \"%s\"\n", playerName, userid, networkID, playerTeam, p);
		break;

	}

	IGameEvent* event = gameeventmanager->CreateEvent("player_say", true);
	if (event)
	{
		event->SetInt("userid", userid);
		event->SetString("text", p);
		event->SetInt("priority", 1);	// HLTV event priority, not transmitted
		gameeventmanager->FireEvent(event, true);
	}
}

void ClientPrecache(void)
{
	// Precache cable textures.
	CBaseEntity::PrecacheModel("cable/cable.vmt");
	CBaseEntity::PrecacheModel("cable/cable_lit.vmt");
	CBaseEntity::PrecacheModel("cable/chain.vmt");
	CBaseEntity::PrecacheModel("cable/rope.vmt");
	CBaseEntity::PrecacheModel("sprites/blueglow1.vmt");
	CBaseEntity::PrecacheModel("sprites/purpleglow1.vmt");
	CBaseEntity::PrecacheModel("sprites/purplelaser1.vmt");
	CBaseEntity::PrecacheModel("sprites/glow01.vmt");

	CBaseEntity::PrecacheScriptSound("Player.FallDamage");
	CBaseEntity::PrecacheScriptSound("Player.Swim");

	// General HUD sounds
	CBaseEntity::PrecacheScriptSound("Player.PickupWeapon");
	CBaseEntity::PrecacheScriptSound("Player.DenyWeaponSelection");
	CBaseEntity::PrecacheScriptSound("Player.WeaponSelected");
	CBaseEntity::PrecacheScriptSound("Player.WeaponSelectionClose");
	CBaseEntity::PrecacheScriptSound("Player.WeaponSelectionMoveSlot");

	// General legacy temp ents sounds
	CBaseEntity::PrecacheScriptSound("Bounce.Glass");
	CBaseEntity::PrecacheScriptSound("Bounce.Metal");
	CBaseEntity::PrecacheScriptSound("Bounce.Flesh");
	CBaseEntity::PrecacheScriptSound("Bounce.Wood");
	CBaseEntity::PrecacheScriptSound("Bounce.Shrapnel");
	CBaseEntity::PrecacheScriptSound("Bounce.ShotgunShell");
	CBaseEntity::PrecacheScriptSound("Bounce.Shell");
	CBaseEntity::PrecacheScriptSound("Bounce.Concrete");

	ClientGamePrecache();
}

CON_COMMAND_F( cast_ray, "Tests collision detection", FCVAR_CHEAT )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	
	Vector forward;
	trace_t tr;

	pPlayer->EyeVectors( &forward );
	Vector start = pPlayer->EyePosition();
	UTIL_TraceLine(start, start + forward * MAX_COORD_RANGE, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );

	if ( tr.DidHit() )
	{
		DevMsg(1, "Hit %s\nposition %.2f, %.2f, %.2f\nangles %.2f, %.2f, %.2f\n", tr.m_pEnt->GetClassname(),
			tr.m_pEnt->GetAbsOrigin().x, tr.m_pEnt->GetAbsOrigin().y, tr.m_pEnt->GetAbsOrigin().z,
			tr.m_pEnt->GetAbsAngles().x, tr.m_pEnt->GetAbsAngles().y, tr.m_pEnt->GetAbsAngles().z );
		DevMsg(1, "Hit: hitbox %d, hitgroup %d, physics bone %d, solid %d, surface %s, surfaceprop %s, contents %08x\n", tr.hitbox, tr.hitgroup, tr.physicsbone, tr.m_pEnt->GetSolid(), tr.surface.name, physprops->GetPropName( tr.surface.surfaceProps ), tr.contents );
		NDebugOverlay::Line( start, tr.endpos, 0, 255, 0, false, 10 );
		NDebugOverlay::Line( tr.endpos, tr.endpos + tr.plane.normal * 12, 255, 255, 0, false, 10 );
	}
}

CON_COMMAND_F( cast_hull, "Tests hull collision detection", FCVAR_CHEAT )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	
	Vector forward;
	trace_t tr;

	Vector extents;
	extents.Init(16,16,16);
	pPlayer->EyeVectors( &forward );
	Vector start = pPlayer->EyePosition();
	UTIL_TraceHull(start, start + forward * MAX_COORD_RANGE, -extents, extents, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );
	if ( tr.DidHit() )
	{
		DevMsg(1, "Hit %s\nposition %.2f, %.2f, %.2f\nangles %.2f, %.2f, %.2f\n", tr.m_pEnt->GetClassname(),
			tr.m_pEnt->GetAbsOrigin().x, tr.m_pEnt->GetAbsOrigin().y, tr.m_pEnt->GetAbsOrigin().z,
			tr.m_pEnt->GetAbsAngles().x, tr.m_pEnt->GetAbsAngles().y, tr.m_pEnt->GetAbsAngles().z );
		DevMsg(1, "Hit: hitbox %d, hitgroup %d, physics bone %d, solid %d, surface %s, surfaceprop %s\n", tr.hitbox, tr.hitgroup, tr.physicsbone, tr.m_pEnt->GetSolid(), tr.surface.name, physprops->GetPropName( tr.surface.surfaceProps ) );
		NDebugOverlay::SweptBox( start, tr.endpos, -extents, extents, vec3_angle, 0, 0, 255, 0, 10 );
		Vector end = tr.endpos;// - tr.plane.normal * DotProductAbs( tr.plane.normal, extents );
		NDebugOverlay::Line( end, end + tr.plane.normal * 24, 255, 255, 64, false, 10 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Used to find targets for ent_* commands
//			Without a name, returns the entity under the player's crosshair.
//			With a name it finds entities via name/classname/index
//-----------------------------------------------------------------------------
CBaseEntity *GetNextCommandEntity( CBasePlayer *pPlayer, const char *name, CBaseEntity *ent )
{
	if ( !pPlayer )
		return NULL;

	// If no name was given set bits based on the picked
	if (FStrEq(name,"")) 
	{
		// If we've already found an entity, return NULL. 
		// Makes it easier to write code using this func.
		if ( ent )
			return NULL;

		return FindPickerEntity( pPlayer );
	}

	int index = atoi( name );
	if ( index )
	{
		// If we've already found an entity, return NULL. 
		// Makes it easier to write code using this func.
		if ( ent )
			return NULL;

		return CBaseEntity::Instance( index );
	}
		
	// Loop through all entities matching, starting from the specified previous
	while ( (ent = gEntList.NextEnt(ent)) != NULL )
	{
		if (  (ent->GetEntityName() != NULL_STRING	&& ent->NameMatches(name))	|| 
			  (ent->m_iClassname != NULL_STRING && ent->ClassMatches(name)) )
		{
			return ent;
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: called each time a player uses a "cmd" command
// Input  : pPlayer - the player who issued the command
//-----------------------------------------------------------------------------
void SetDebugBits( CBasePlayer* pPlayer, const char *name, int bit )
{
	if ( !pPlayer )
		return;

	CBaseEntity *pEntity = NULL;
	while ( (pEntity = GetNextCommandEntity( pPlayer, name, pEntity )) != NULL )
	{
		if (pEntity->m_debugOverlays & bit)
		{
			pEntity->m_debugOverlays &= ~bit;
		}
		else
		{
			pEntity->m_debugOverlays |= bit;

#ifdef AI_MONITOR_FOR_OSCILLATION
			if( pEntity->IsNPC() )
			{
				pEntity->MyNPCPointer()->m_ScheduleHistory.RemoveAll();
			}
#endif//AI_MONITOR_FOR_OSCILLATION
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pKillTargetName - 
//-----------------------------------------------------------------------------
void KillTargets( const char *pKillTargetName )
{
	CBaseEntity *pentKillTarget = NULL;

	DevMsg( 2, "KillTarget: %s\n", pKillTargetName );
	pentKillTarget = gEntList.FindEntityByName( NULL, pKillTargetName );
	while ( pentKillTarget )
	{
		UTIL_Remove( pentKillTarget );

		DevMsg( 2, "killing %s\n", STRING( pentKillTarget->m_iClassname ) );
		pentKillTarget = gEntList.FindEntityByName( pentKillTarget, pKillTargetName );
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void ConsoleKillTarget( CBasePlayer *pPlayer, const char *name )
{
	// If no name was given use the picker
	if (FStrEq(name,"")) 
	{
		CBaseEntity *pEntity = FindPickerEntity( pPlayer );
		if ( pEntity )
		{
			UTIL_Remove( pEntity );
			Msg( "killing %s\n", pEntity->GetDebugName() );
			return;
		}
	}
	// Otherwise use name or classname
	KillTargets( name );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointClientCommand : public CPointEntity
{
public:
	DECLARE_CLASS( CPointClientCommand, CPointEntity );
	DECLARE_DATADESC();

	void InputCommand( inputdata_t& inputdata );
};

void CPointClientCommand::InputCommand( inputdata_t& inputdata )
{
	if ( !inputdata.value.String()[0] )
		return;

	edict_t *pClient = NULL;
	if ( gpGlobals->maxClients == 1 )
	{
		pClient = engine->PEntityOfEntIndex( 1 );
	}
	else
	{
		// In multiplayer, send it back to the activator
		CBasePlayer *player = dynamic_cast< CBasePlayer * >( inputdata.pActivator );
		if (player)
			pClient = player->edict();
	}

	if ( !pClient || !pClient->GetUnknown() )
		return;

	engine->ClientCommand( pClient, "%s\n", inputdata.value.String() );
}

BEGIN_DATADESC( CPointClientCommand )
	DEFINE_INPUTFUNC( FIELD_STRING, "Command", InputCommand ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( point_clientcommand, CPointClientCommand );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointServerCommand : public CPointEntity
{
public:
	DECLARE_CLASS( CPointServerCommand, CPointEntity );
	DECLARE_DATADESC();
	void InputCommand( inputdata_t& inputdata );
};

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : inputdata - 
//-----------------------------------------------------------------------------
void CPointServerCommand::InputCommand( inputdata_t& inputdata )
{
	if ( !inputdata.value.String()[0] )
		return;

	engine->ServerCommand( UTIL_VarArgs( "%s\n", inputdata.value.String() ) );
}

BEGIN_DATADESC( CPointServerCommand )
	DEFINE_INPUTFUNC( FIELD_STRING, "Command", InputCommand ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( point_servercommand, CPointServerCommand );

//------------------------------------------------------------------------------
// Purpose : Draw a line betwen two points.  White if no world collisions, red if collisions
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CC_DrawLine( const CCommand &args )
{
	Vector startPos;
	Vector endPos;

	startPos.x = atof(args[1]);
	startPos.y = atof(args[2]);
	startPos.z = atof(args[3]);
	endPos.x = atof(args[4]);
	endPos.y = atof(args[5]);
	endPos.z = atof(args[6]);

	UTIL_AddDebugLine(startPos,endPos,true,true);
}
static ConCommand drawline("drawline", CC_DrawLine, "Draws line between two 3D Points.\n\tGreen if no collision\n\tRed is collides with something\n\tArguments: x1 y1 z1 x2 y2 z2", FCVAR_CHEAT);

//------------------------------------------------------------------------------
// Purpose : Draw a cross at a points.  
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CC_DrawCross( const CCommand &args )
{
	Vector vPosition;

	vPosition.x = atof(args[1]);
	vPosition.y = atof(args[2]);
	vPosition.z = atof(args[3]);

	// Offset since min and max z in not about center
	Vector mins = Vector(-5,-5,-5);
	Vector maxs = Vector(5,5,5);

	Vector start = mins + vPosition;
	Vector end   = maxs + vPosition;
	UTIL_AddDebugLine(start,end,true,true);

	start.x += (maxs.x - mins.x);
	end.x	-= (maxs.x - mins.x);
	UTIL_AddDebugLine(start,end,true,true);

	start.y += (maxs.y - mins.y);
	end.y	-= (maxs.y - mins.y);
	UTIL_AddDebugLine(start,end,true,true);

	start.x -= (maxs.x - mins.x);
	end.x	+= (maxs.x - mins.x);
	UTIL_AddDebugLine(start,end,true,true);
}
static ConCommand drawcross("drawcross", CC_DrawCross, "Draws a cross at the given location\n\tArguments: x y z", FCVAR_CHEAT);

//------------------------------------------------------------------------------
// helper function for kill and explode
//------------------------------------------------------------------------------
void kill_helper( const CCommand &args, bool bExplode )
{
	if ( args.ArgC() > 1 && sv_cheats->GetBool() )
	{
		// Find the matching netname
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CBasePlayer *pPlayer = ToBasePlayer( UTIL_PlayerByIndex(i) );
			if ( pPlayer )
			{
				if ( Q_strstr( pPlayer->GetPlayerName(), args[1] ) )
				{
					pPlayer->CommitSuicide( bExplode );
				}
			}
		}
	}
	else
	{
		CBasePlayer *pPlayer = UTIL_GetCommandClient();
		if ( pPlayer )
		{
			pPlayer->CommitSuicide( bExplode );
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND( kill, "Kills the player with generic damage" )
{
	kill_helper( args, false );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND( explode, "Kills the player with explosive damage" )
{
	kill_helper( args, true );
}

//------------------------------------------------------------------------------
// helper function for killvector and explodevector
//------------------------------------------------------------------------------
void killvector_helper( const CCommand &args, bool bExplode )
{
	CBasePlayer *pPlayer = UTIL_GetCommandClient();
	if ( pPlayer && args.ArgC() == 5 )
	{
		// Find the matching netname.
		for ( int iClient = 1; iClient <= gpGlobals->maxClients; iClient++ )
		{
			CBasePlayer *pPlayer = ToBasePlayer( UTIL_PlayerByIndex( iClient ) );
			if ( pPlayer )
			{
				if ( Q_strstr( pPlayer->GetPlayerName(), args[1] ) )
				{
					// Build world-space force vector.
					Vector vecForce;
					vecForce.x = atof( args[2] );
					vecForce.y = atof( args[3] );
					vecForce.z = atof( args[4] );

					ClientKill( pPlayer->edict(), vecForce, bExplode );
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND_F( killvector, "Kills a player applying force. Usage: killvector <player> <x value> <y value> <z value>", FCVAR_CHEAT )
{
	killvector_helper( args, false );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND_F( explodevector, "Kills a player applying an explosive force. Usage: explodevector <player> <x value> <y value> <z value>", FCVAR_CHEAT )
{
	killvector_helper( args, false );
}

#define TALK_INTERVAL 0.5 // min time between say commands from a client

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND(say, "Display global player message")
{
	CBasePlayer* pPlayer = ToBasePlayer(UTIL_GetCommandClient());
	if (pPlayer)
	{
		if ((pPlayer->LastTimePlayerTalked() + TALK_INTERVAL) < gpGlobals->curtime)
		{
			Host_Say(pPlayer->edict(), args, SAYTYPE_GLOBAL);
			pPlayer->NotePlayerTalked();
		}
	}
	else if (UTIL_GetCommandClientIndex() == 0)
		Host_Say(NULL, args, SAYTYPE_SERVER);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND(say_team, "Display player message to team")
{
	CBasePlayer* pPlayer = ToBasePlayer(UTIL_GetCommandClient());
	if (pPlayer)
	{
		if ((pPlayer->LastTimePlayerTalked() + TALK_INTERVAL) < gpGlobals->curtime)
		{
			Host_Say(pPlayer->edict(), args, SAYTYPE_TEAM);
			pPlayer->NotePlayerTalked();
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND(say_squad, "Display player message to team squad")
{
	CBasePlayer* pPlayer = ToBasePlayer(UTIL_GetCommandClient());
	if (pPlayer)
	{
		if ((pPlayer->LastTimePlayerTalked() + TALK_INTERVAL) < gpGlobals->curtime)
		{
			Host_Say(pPlayer->edict(), args, SAYTYPE_SQUAD);
			pPlayer->NotePlayerTalked();
		}
	}
}

CON_COMMAND_F(give_weapon, "Give a weapon to the player.\n", FCVAR_CHEAT)
{
	if (args.ArgC() < 2)
		return;

	CINSPlayer* pPlayer = ToINSPlayer(UTIL_GetCommandClient());
	if (!pPlayer)
		return;

	const char* pszWeaponName = args[1];
	if (!pszWeaponName || *pszWeaponName == '\0')
		return;

	char szWeaponName[64];
	Q_snprintf(szWeaponName, sizeof(szWeaponName), "weapon_%s", pszWeaponName);

	int iWeaponID = WeaponNameToID(szWeaponName);
	if (iWeaponID == WEAPON_INVALID)
	{
		ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Unknown weapon entity!");
		return;
	}

	pPlayer->AddWeapon(iWeaponID, 2, 0);
}

CON_COMMAND_F(give_clip, "Give ammo clips to the player.\n", FCVAR_CHEAT)
{
	if (args.ArgC() < 2)
		return;

	CINSPlayer* pPlayer = ToINSPlayer(UTIL_GetCommandClient());
	if (!pPlayer)
		return;

	int iCount = atoi(args[1]);
	if (iCount <= 0)
		return;

	CWeaponBallisticBase* pWeapon = dynamic_cast<CWeaponBallisticBase*>(pPlayer->GetActiveINSWeapon());
	if (pWeapon)
		pWeapon->GiveClip(iCount);
}

CON_COMMAND_F(give_ammo, "Give ammo to the player.\n", FCVAR_CHEAT)
{
	if (args.ArgC() < 2)
		return;

	CINSPlayer* pPlayer = ToINSPlayer(UTIL_GetCommandClient());
	if (!pPlayer)
		return;

	int iCount = atoi(args[1]);
	if (iCount <= 0)
		return;

	CWeaponBallisticBase* pWeapon = dynamic_cast<CWeaponBallisticBase*>(pPlayer->GetActiveINSWeapon());
	if (pWeapon)
		pWeapon->GiveAmmo(iCount);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND( give, "Give item to player.\n\tArguments: <item_name>" )
{
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( pPlayer 
		&& (gpGlobals->maxClients == 1 || sv_cheats->GetBool()) 
		&& args.ArgC() >= 2 )
	{
		char item_to_give[ 256 ];
		Q_strncpy( item_to_give, args[1], sizeof( item_to_give ) );
		Q_strlower( item_to_give );

		// Don't allow regular users to create point_servercommand entities for the same reason as blocking ent_fire
		if ( !Q_stricmp( item_to_give, "point_servercommand" ) )
		{
			if ( engine->IsDedicatedServer() )
			{
				// We allow people with disabled autokick to do it, because they already have rcon.
				if ( pPlayer->IsAutoKickDisabled() == false )
					return;
			}
			else if ( gpGlobals->maxClients > 1 )
			{
				// On listen servers with more than 1 player, only allow the host to create point_servercommand.
				CBasePlayer *pHostPlayer = UTIL_GetListenServerHost();
				if ( pPlayer != pHostPlayer )
					return;
			}
		}

		string_t iszItem = AllocPooledString( item_to_give );	// Make a copy of the classname
		pPlayer->GiveNamedItem( STRING(iszItem) );
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
CON_COMMAND( fov, "Change players FOV" )
{
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() );
	if ( pPlayer && sv_cheats->GetBool() )
	{
		if ( args.ArgC() > 1 )
		{
			int nFOV = atoi( args[1] );
			pPlayer->SetDefaultFOV( nFOV );
		}
		else
		{
			ClientPrint( pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs( "\"fov\" is \"%d\"\n", pPlayer->GetFOV() ) );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CC_Player_TestDispatchEffect( const CCommand &args )
{
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() );
	if ( !pPlayer)
		return;
	
	if ( args.ArgC() < 2 )
	{
		Msg(" Usage: test_dispatcheffect <effect name> <distance away> <flags> <magnitude> <scale>\n " );
		Msg("		 defaults are: <distance 1024> <flags 0> <magnitude 0> <scale 0>\n" );
		return;
	}

	// Optional distance
	float flDistance = 1024;
	if ( args.ArgC() >= 3 )
	{
		flDistance = atoi( args[ 2 ] );
	}

	// Optional flags
	float flags = 0;
	if ( args.ArgC() >= 4 )
	{
		flags = atoi( args[ 3 ] );
	}

	// Optional magnitude
	float magnitude = 0;
	if ( args.ArgC() >= 5 )
	{
		magnitude = atof( args[ 4 ] );
	}

	// Optional scale
	float scale = 0;
	if ( args.ArgC() >= 6 )
	{
		scale = atof( args[ 5 ] );
	}

	Vector vecForward;
	QAngle vecAngles = pPlayer->EyeAngles();
	AngleVectors( vecAngles, &vecForward );

	// Trace forward
	trace_t tr;
	Vector vecSrc = pPlayer->EyePosition();
	Vector vecEnd = vecSrc + (vecForward * flDistance);
	UTIL_TraceLine( vecSrc, vecEnd, MASK_ALL, pPlayer, COLLISION_GROUP_NONE, &tr );

	// Fill out the generic data
	CEffectData data;
	// If we hit something, use that data
	if ( tr.fraction < 1.0 )
	{
		data.m_vOrigin = tr.endpos;
		VectorAngles( tr.plane.normal, data.m_vAngles );
		data.m_vNormal = tr.plane.normal;
	}
	else
	{
		data.m_vOrigin = vecEnd;
		data.m_vAngles = vecAngles;
		AngleVectors( vecAngles, &data.m_vNormal );
	}
	data.m_nEntIndex = pPlayer->entindex();
	data.m_fFlags = flags;
	data.m_flMagnitude = magnitude;
	data.m_flScale = scale;
	DispatchEffect( (char *)args[1], data );
}

static ConCommand test_dispatcheffect("test_dispatcheffect", CC_Player_TestDispatchEffect, "Test a clientside dispatch effect.\n\tUsage: test_dispatcheffect <effect name> <distance away> <flags> <magnitude> <scale>\n\tDefaults are: <distance 1024> <flags 0> <magnitude 0> <scale 0>\n", FCVAR_CHEAT);

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CC_Player_Use( const CCommand &args )
{
	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( pPlayer)
	{
		pPlayer->SelectItem(atoi(args[1]));
	}
}
static ConCommand use("use", CC_Player_Use, "Use a particular weapon\t\nArguments: <weapon_name>");

//------------------------------------------------------------------------------
// A small wrapper around SV_Move that never clips against the supplied entity.
//------------------------------------------------------------------------------
static bool TestEntityPosition ( CBasePlayer *pPlayer )
{	
	trace_t	trace;
	UTIL_TraceEntity( pPlayer, pPlayer->GetAbsOrigin(), pPlayer->GetAbsOrigin(), MASK_PLAYERSOLID, &trace );
	return (trace.startsolid == 0);
}

//------------------------------------------------------------------------------
// Searches along the direction ray in steps of "step" to see if 
// the entity position is passible.
// Used for putting the player in valid space when toggling off noclip mode.
//------------------------------------------------------------------------------
static int FindPassableSpace( CBasePlayer *pPlayer, const Vector& direction, float step, Vector& oldorigin )
{
	int i;
	for ( i = 0; i < 100; i++ )
	{
		Vector origin = pPlayer->GetAbsOrigin();
		VectorMA( origin, step, direction, origin );
		pPlayer->SetAbsOrigin( origin );
		if ( TestEntityPosition( pPlayer ) )
		{
			VectorCopy( pPlayer->GetAbsOrigin(), oldorigin );
			return 1;
		}
	}
	return 0;
}

//------------------------------------------------------------------------------
// Noclip
//------------------------------------------------------------------------------
void EnableNoClip( CBasePlayer *pPlayer )
{
	// Disengage from hierarchy
	pPlayer->SetParent( NULL );
	pPlayer->SetMoveType( MOVETYPE_NOCLIP );
	ClientPrint( pPlayer, HUD_PRINTCONSOLE, "noclip ON\n");
	pPlayer->AddEFlags( EFL_NOCLIP_ACTIVE );
}

void CC_Player_NoClip( void )
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	CPlayerState *pl = pPlayer->PlayerData();
	Assert( pl );

	if (pPlayer->GetMoveType() != MOVETYPE_NOCLIP)
	{
		EnableNoClip( pPlayer );
		return;
	}

	pPlayer->RemoveEFlags( EFL_NOCLIP_ACTIVE );
	pPlayer->SetMoveType( MOVETYPE_WALK );

	Vector oldorigin = pPlayer->GetAbsOrigin();
	ClientPrint( pPlayer, HUD_PRINTCONSOLE, "noclip OFF\n");
	if ( !TestEntityPosition( pPlayer ) )
	{
		Vector forward, right, up;

		AngleVectors ( pl->v_angle, &forward, &right, &up);
		
		// Try to move into the world
		if ( !FindPassableSpace( pPlayer, forward, 1, oldorigin ) )
		{
			if ( !FindPassableSpace( pPlayer, right, 1, oldorigin ) )
			{
				if ( !FindPassableSpace( pPlayer, right, -1, oldorigin ) )		// left
				{
					if ( !FindPassableSpace( pPlayer, up, 1, oldorigin ) )	// up
					{
						if ( !FindPassableSpace( pPlayer, up, -1, oldorigin ) )	// down
						{
							if ( !FindPassableSpace( pPlayer, forward, -1, oldorigin ) )	// back
							{
								Msg( "Can't find the world\n" );
							}
						}
					}
				}
			}
		}

		pPlayer->SetAbsOrigin( oldorigin );
	}
}

static ConCommand noclip("noclip", CC_Player_NoClip, "Toggle. Player becomes non-solid and flies.", FCVAR_CHEAT);

CON_COMMAND_F( setpos, "Move player to specified origin (must have sv_cheats).", FCVAR_CHEAT )
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	if ( args.ArgC() < 3 )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage:  setpos x y <z optional>\n");
		return;
	}

	Vector oldorigin = pPlayer->GetAbsOrigin();

	Vector newpos;
	newpos.x = atof( args[1] );
	newpos.y = atof( args[2] );
	newpos.z = args.ArgC() == 4 ? atof( args[3] ) : oldorigin.z;

	pPlayer->SetAbsOrigin( newpos );

	if ( !TestEntityPosition( pPlayer ) )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "setpos into world, use noclip to unstick yourself!\n");
	}
}

void CC_setang_f (const CCommand &args)
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	if ( args.ArgC() < 3 )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage:  setang pitch yaw <roll optional>\n");
		return;
	}

	QAngle oldang = pPlayer->GetAbsAngles();

	QAngle newang;
	newang.x = atof( args[1] );
	newang.y = atof( args[2] );
	newang.z = args.ArgC() == 4 ? atof( args[3] ) : oldang.z;

	pPlayer->SnapEyeAngles( newang );
}

static ConCommand setang("setang", CC_setang_f, "Snap player eyes to specified pitch yaw <roll:optional> (must have sv_cheats).", FCVAR_CHEAT );

static float GetHexFloat( const char *pStr )
{
	if ( ( pStr[0] == '0' ) && ( pStr[1] == 'x' ) )
	{
		uint32 f = (uint32)V_atoi64( pStr );
		return *reinterpret_cast< const float * >( &f );
	}
	
	return atof( pStr );
}

//------------------------------------------------------------------------------
// Move position
//------------------------------------------------------------------------------
CON_COMMAND_F( setpos_exact, "Move player to an exact specified origin (must have sv_cheats).", FCVAR_CHEAT )
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	if ( args.ArgC() < 3 )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage:  setpos_exact x y <z optional>\n");
		return;
	}

	Vector oldorigin = pPlayer->GetAbsOrigin();

	Vector newpos;
	newpos.x = GetHexFloat( args[1] );
	newpos.y = GetHexFloat( args[2] );
	newpos.z = args.ArgC() == 4 ? GetHexFloat( args[3] ) : oldorigin.z;

	pPlayer->Teleport( &newpos, NULL, NULL );

	if ( !TestEntityPosition( pPlayer ) )
	{
		if ( pPlayer->GetMoveType() != MOVETYPE_NOCLIP )
		{
			EnableNoClip( pPlayer );
			return;
		}
	}
}

CON_COMMAND_F( setang_exact, "Snap player eyes and orientation to specified pitch yaw <roll:optional> (must have sv_cheats).", FCVAR_CHEAT )
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	if ( args.ArgC() < 3 )
	{
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "Usage:  setang_exact pitch yaw <roll optional>\n");
		return;
	}

	QAngle oldang = pPlayer->GetAbsAngles();

	QAngle newang;
	newang.x = GetHexFloat( args[1] );
	newang.y = GetHexFloat( args[2] );
	newang.z = args.ArgC() == 4 ? GetHexFloat( args[3] ) : oldang.z;

	pPlayer->Teleport( NULL, &newang, NULL );
	pPlayer->SnapEyeAngles( newang );
}

//------------------------------------------------------------------------------
// Sets client to notarget mode.
//------------------------------------------------------------------------------
void CC_Notarget_f (void)
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	pPlayer->ToggleFlag( FL_NOTARGET );
	if ( !(pPlayer->GetFlags() & FL_NOTARGET ) )
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "notarget OFF\n");
	else
		ClientPrint( pPlayer, HUD_PRINTCONSOLE, "notarget ON\n");
}

ConCommand notarget("notarget", CC_Notarget_f, "Toggle. Player becomes hidden to NPCs.", FCVAR_CHEAT);

//------------------------------------------------------------------------------
// Damage the client the specified amount
//------------------------------------------------------------------------------
void CC_HurtMe_f(const CCommand &args)
{
	if ( !sv_cheats->GetBool() )
		return;

	CBasePlayer *pPlayer = ToBasePlayer( UTIL_GetCommandClient() ); 
	if ( !pPlayer )
		return;

	int iDamage = 10;
	if ( args.ArgC() >= 2 )
	{
		iDamage = atoi( args[ 1 ] );
	}

	pPlayer->TakeDamage( CTakeDamageInfo( pPlayer, pPlayer, iDamage, DMG_PREVENT_PHYSICS_FORCE ) );
}

static ConCommand hurtme("hurtme", CC_HurtMe_f, "Hurts the player.\n\tArguments: <health to lose>", FCVAR_CHEAT);

static bool IsInGroundList( CBaseEntity *ent, CBaseEntity *ground )
{
	if ( !ground || !ent )
		return false;

	groundlink_t *root = ( groundlink_t * )ground->GetDataObject( GROUNDLINK );
	if ( root )
	{
		groundlink_t *link = root->nextLink;
		while ( link != root )
		{
			CBaseEntity *other = link->entity;
			if ( other == ent )
				return true;
			link = link->nextLink;
		}
	}

	return false;

}

static int DescribeGroundList( CBaseEntity *ent )
{
	if ( !ent )
		return 0;

	int c = 1;

	Msg( "%i : %s (ground %i %s)\n", ent->entindex(), ent->GetClassname(), 
		ent->GetGroundEntity() ? ent->GetGroundEntity()->entindex() : -1,
		ent->GetGroundEntity() ? ent->GetGroundEntity()->GetClassname() : "NULL" );
	groundlink_t *root = ( groundlink_t * )ent->GetDataObject( GROUNDLINK );
	if ( root )
	{
		groundlink_t *link = root->nextLink;
		while ( link != root )
		{
			CBaseEntity *other = link->entity;
			if ( other )
			{
				Msg( "  %02i:  %i %s\n", c++, other->entindex(), other->GetClassname() );

				if ( other->GetGroundEntity() != ent )
				{
					Assert( 0 );
					Msg( "   mismatched!!!\n" );
				}
			}
			else
			{
				Assert( 0 );
				Msg( "  %02i:  NULL link\n", c++ );
			}
			link = link->nextLink;
		}
	}

	if ( ent->GetGroundEntity() != NULL )
	{
		Assert( IsInGroundList( ent, ent->GetGroundEntity() ) );
	}

	return c - 1;
}

void CC_GroundList_f(const CCommand &args)
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if ( args.ArgC() == 2 )
	{
		int idx = atoi( args[1] );

		CBaseEntity *ground = CBaseEntity::Instance( idx );
		if ( ground )
		{
			DescribeGroundList( ground );
		}
	}
	else
	{
		CBaseEntity *ent = NULL;
		int linkCount = 0;
		while ( (ent = gEntList.NextEnt(ent)) != NULL )
		{
			linkCount += DescribeGroundList( ent );
		}

		extern int groundlinksallocated;
		Assert( linkCount == groundlinksallocated );

		Msg( "--- %i links\n", groundlinksallocated );
	}
}

static ConCommand groundlist("groundlist", CC_GroundList_f, "Display ground entity list <index>" );

//-----------------------------------------------------------------------------
// Purpose: called each time a player uses a "cmd" command
// Input  : *pEdict - the player who issued the command
//-----------------------------------------------------------------------------
void ClientCommand(CBasePlayer *pPlayer, const CCommand &args)
{
	const char *pCmd = args[0];

	// Is the client spawned yet?
	if (!pPlayer)
		return;

	MDLCACHE_CRITICAL_SECTION();

	bool bIssuedCommand = false;

	if (sv_cheats && sv_cheats->GetBool())
	{
		if (FStrEq(pCmd, "killtarget"))
		{
			if (g_pDeveloper->GetBool() && UTIL_IsCommandIssuedByServerAdmin())
			{
				ConsoleKillTarget(pPlayer, args[1]);
			}
			bIssuedCommand = true;
		}
		else if (FStrEq(pCmd, "demorestart"))
		{
			pPlayer->ForceClientDllUpdate();
			bIssuedCommand = true;
		}
	}

	if ((bIssuedCommand == false) && g_pGameRules && !g_pGameRules->ClientCommand(pPlayer, args))
	{
		if (Q_strlen(pCmd) > 128)
		{
			ClientPrint(pPlayer, HUD_PRINTCONSOLE, "Console command too long.\n");
		}
		else
		{
			// tell the user they entered an unknown command
			ClientPrint(pPlayer, HUD_PRINTCONSOLE, UTIL_VarArgs("Unknown command: %s\n", pCmd));
		}
	}
}