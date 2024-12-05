//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Implementation of bot interface to the game engine
//
//=============================================================================//
#include "ins_bot.h"

// global to this file: a navigation mesh is ready for AI
bool b_NavMeshExists = false;

void BotEnabledCallback( ConVar *pVar, char const *pszOldString )
{
	if( !INSRules( ) )
		return;

	if( !pVar->GetBool( ) && atoi( pszOldString ) != 0 )
		INSRules( )->PlayerKickBots( false );
}

// console variable to enable bots
ConVar bot_enabled( "bot_enabled", BOTS_ENABLED, 0, "Enable bots on this server.", BotEnabledCallback );

// bot debug commands
ConVar bot_freeze( "bot_freeze", "0", 0, "Freeze bots" );
ConVar bot_freeze_allow( "bot_freeze_allow", "0", 0, "Freeze bots" );
ConVar bot_mimic( "bot_mimic", "0", 0, "Bot uses usercmd of player by index." );
ConVar bot_mimic_yaw_offset( "bot_mimic_yaw_offset", "0", 0, "Offsets the bot yaw." );
ConVar bot_disallowshoot( "bot_disallowshoot", "0", 0, "Disallowing Bots from Attacking", true, 0, true, 1 );

// Contains all players currently alive in the server
// Bots are placed at the head of the list, real players at the tail
CUtlVector<CINSPlayer *> PlayerList;

// Execute the Run() method on all bots in the server
// Builds the PlayerList array used by AI
// Called only by ins_client.cpp -> GameStartFrame()
void Bot_Update() {
	int i, iNumBots = 0;

	if (bot_enabled.GetBool()) {

		PlayerList.RemoveAll();

		// build the list from all players connected
		for (i=1; i <= gpGlobals->maxClients; i++) {
			CINSPlayer *pPlayer = ToINSPlayer( UTIL_PlayerByIndex( i ) );

			// player is alive: 
			if( pPlayer && pPlayer->IsRunningAround() ) {

				// bots go at the beginning of the list
				if( pPlayer->IsBot() ) {
					PlayerList.AddToHead(pPlayer);
					iNumBots++;
				} else {
					PlayerList.AddToTail(pPlayer);
				}

			} else if (pPlayer && pPlayer->IsBot() ) {

				CINSBot *pBot = dynamic_cast< CINSBot* >( pPlayer );

				// bot was previously alive; it just died
				if (pBot->bIsAlive) {
					pBot->bIsAlive = false;
					pBot->Death();
				}
			}
		}

		// now call Run() on each bot
		for (i=0; i < iNumBots; i++) {
			CINSBot *pBot = dynamic_cast< CINSBot* >( PlayerList.Element(i) );

			pBot->PostClientMessagesSent( );

			if (pBot->Debug()) continue;

			// only process AI if there is a NavMesh for this map
			if (b_NavMeshExists) {
				pBot->bIsAlive = true;
				pBot->Run();
			} else {
				CUserCmd blank;
				pBot->CmdExecute(blank);
			}

		}
	}

	// draw navmesh editing helpers, update if generating
	TheNavMesh->Update();
}

// Called by CServerGameDLL.ServerActivate() each time the map loads
void Bot_Initialize() {

	// load the navigation mesh for this map
	NavErrorType result = TheNavMesh->Load();
	if (result == NAV_OK) {
		Msg("NavMesh loaded successfully.\n");
		b_NavMeshExists = true;
	} else {
		Warning("NavMesh could not be loaded.\n");
		b_NavMeshExists = false;
		return;
	}

}

LINK_ENTITY_TO_CLASS( ins_bot, CINSBot );
/*
void CINSBot::PhysicsSimulate( void ) {
	BaseClass::PhysicsSimulate( );

	// http://developer.valvesoftware.com/wiki/SDK_Known_Issues_List#Bot_physics.2Fhitbox_not_accurate
	// since this isn't called for bots ... call it here
	UpdateVPhysicsPosition( m_vNewVPhysicsPosition, m_vNewVPhysicsVelocity, gpGlobals->frametime );
}
*/

int CINSBot::GetDeathMenuType( void ) {
	return DEATHINFOTYPE_NONE;
}

// Execute the user command built by Run()
void CINSBot::CmdExecute( CUserCmd &cmd ) {

	SetTimeBase( gpGlobals->curtime );

	MoveHelperServer( )->SetHost( this );
	PlayerRunCommand( &cmd, MoveHelperServer( ) );

	SetLastUserCommand( cmd );

	pl.fixangle = FIXANGLE_NONE;

	MoveHelperServer( )->SetHost( NULL );
}

bool CINSBot::Debug() {
	if (Debug_Mimic() || Debug_Frozen())
		return true;
	return false;
}

bool CINSBot::Debug_Mimic() {

	if( bot_mimic.GetInt( ) <= 0 )
		return false;

	if( bot_mimic.GetInt( ) > gpGlobals->maxClients )
		return false;
	
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( bot_mimic.GetInt( ) );

	if( !pPlayer )
		return false;

	if ( !pPlayer->GetLastUserCommand( ) )
		return false;

	CUserCmd cmd = *pPlayer->GetLastUserCommand( );
	cmd.viewangles[ YAW ] += bot_mimic_yaw_offset.GetFloat( );

	CmdExecute(cmd);
	return true;
}

bool CINSBot::Debug_Frozen( ) {

//	if( !IsEFlagSet( EFL_BOT_FROZEN ) )
//		return false;

	if( bot_freeze_allow.GetInt( ) != 0 && ( bot_freeze_allow.GetInt( ) - 1 ) == entindex( ) )
		return false;

	if( !bot_freeze.GetBool( ) )
		return false;

	CUserCmd cmd;
	CmdExecute(cmd);
	return true;
}


class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *pszPlayerName )
	{
		// this tells it which edict to use rather than creating a new one
		CBasePlayer::s_PlayerEdict = pEdict;

		CINSBot *pPlayer = static_cast< CINSBot* >( CreateEntityByName( "ins_bot" ) );

		if ( pPlayer )
		{
			char szTrimmedName[ MAX_PLAYER_NAME_LENGTH ];
			Q_strncpy( szTrimmedName, pszPlayerName, sizeof( szTrimmedName ) );
			pPlayer->PlayerData( )->netname = AllocPooledString( szTrimmedName );
		}

		return pPlayer;
	}
};

CBasePlayer *BotPutInServer( bool bFrozen, int iTeamID )
{
	static int g_CurBotNumber = 1;
	char szBotName[ 64 ];
	Q_snprintf( szBotName, sizeof( szBotName ), "[BOT] Macky%02i", g_CurBotNumber );

	// this trick lets us create a CINSBot for this client instead of the CINSPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	edict_t *pEdict = engine->CreateFakeClient( szBotName );
	ClientPutInServerOverride( NULL );

	if( !pEdict )
	{
		Msg( "Failed to create bot.\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CINSBot *pPlayer = ( ( CINSBot* )CBaseEntity::Instance( pEdict ) );

    pPlayer->ClearFlags( );
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	strcpy(pPlayer->m_Name, szBotName);
	INSRules( )->PlayerConnected( );
	INSRules( )->SetupPlayerTeam( pPlayer, iTeamID );
	INSRules( )->SetupPlayerSquad( pPlayer, NULL );

	pPlayer->RemoveAllItems( );

	g_CurBotNumber++;

	return pPlayer;
}

//=========================================================
//=========================================================
extern int FindEngineArgInt( const char *pName, int defaultVal );
extern const char* FindEngineArg( const char *pName );

void BotAdd( int iTeamID )
{
	// ensure bots are enabled
	if( !bot_enabled.GetBool( ) )
		return;

	// ensure that the gamerules allow it
	if( !INSRules( )->AllowBotAdd( ) )
		return;

	// look at -count
	int count = FindEngineArgInt( "-count", 1 );
	count = clamp( count, 1, 16 );

	// look at -frozen
	bool bFrozen = !!FindEngineArg( "-frozen" );
	
	// ok, spawn all the bots
	while( --count >= 0 )
		BotPutInServer( bFrozen, iTeamID );
}

//=========================================================
//=========================================================
void BotAdd_f( void )
{
	BotAdd( TEAMSELECT_AUTOASSIGN );
}

ConCommand botadd( "bot_add", BotAdd_f, "Add a bot that auto-assigns to a team." );

void BotAdd_T1_f( void )
{
	BotAdd( TEAMSELECT_ONE );
}

ConCommand botaddt1( "bot_add_t1", BotAdd_T1_f, "Add a bot that joins team one." );

void BotAdd_T2_f( void )
{
	BotAdd( TEAMSELECT_TWO );
}

ConCommand botaddt2( "bot_add_t2", BotAdd_T2_f, "Add a bot that joins team two." );

#ifdef _DEBUG

void BotLogin_f( void )
{
	if( engine->Cmd_Argc( ) != 2 )
		return;

	const char *pszBotName = engine->Cmd_Argv( 1 );

	if( !pszBotName )
		return;

	CINSPlayer *pPlayer = ToINSPlayer( UTIL_PlayerByName( pszBotName ) );

	if( !pPlayer || !pPlayer->IsBot( ) )
		return;

	GetINSStats( )->LoginPlayer( pPlayer, NULL, NULL );
}

ConCommand botlogin( "bot_login", BotLogin_f, "Login a bot.", 0 );

#endif

