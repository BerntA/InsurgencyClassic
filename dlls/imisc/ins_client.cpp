//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "ins_player.h"
#include "ins_gamerules.h"
#include "imc_config.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// adamjmac [
// Refer to the one function needed this way so this file is not bot-implementation specific
// also optimizes the size of this file
// #include "../bots/ins_bot.h"
extern void Bot_Update(); // implemented by ins_bot.cpp
// ] adamjmac

//=========================================================
//=========================================================
void FinishClientPutInServer( CINSPlayer *pPlayer )
{
	char sName[128];
	Q_strncpy( sName, pPlayer->GetPlayerName(), sizeof( sName ) );
	
	// First parse the name and remove any %'s or #'s
	for ( char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++ )
	{
		// Replace it with a space
		if ( *pApersand == '%' || *pApersand == '#')
				*pApersand = ' ';
	}

	pPlayer->InitialSpawn( );
	pPlayer->Spawn( );

	// TODO: move to inital spawn?
	pPlayer->FadeToBlack( 0.0f );

	if( !pPlayer->IsBot( ) )
		pPlayer->SetThink( NULL );
}

//=========================================================
//=========================================================
void ClientPutInServer( edict_t *pEdict, const char *pszPlayerName )
{
	CINSPlayer *pPlayer = CINSPlayer::CreatePlayer( "player", pEdict );
	pPlayer->SetPlayerName( pszPlayerName );
}

//=========================================================
//=========================================================
void ClientActive( edict_t *pEdict, bool bLoadGame )
{
	// Can't load games in CS!
	Assert( !bLoadGame );

	CINSPlayer *pPlayer = ToINSPlayer( CBaseEntity::Instance( pEdict ) );
	FinishClientPutInServer( pPlayer );
}

//=========================================================
//=========================================================
void ClientGamePrecache( void )
{
	// materials used by the client effects
	CBaseEntity::PrecacheModel( "sprites/white.vmt" );
	CBaseEntity::PrecacheModel( "sprites/physbeam.vmt" );
}

//=========================================================
//=========================================================
void GameStartFrame( void )
{
	VPROF( "GameStartFrame" );

	gpGlobals->teamplay = true;

	// adamjmac [
	// bot_enabled cvar is handled by Bot_Update() now
	//	if( bot_enabled.GetBool( ) )
	// ] adamjmac

	Bot_Update();
}

//=========================================================
//=========================================================
void InstallGameRules( void )
{
	// load IMC config
	CIMCConfig *pIMCConfig = CIMCConfig::IMCConfig( );
	Assert( pIMCConfig );

	pIMCConfig->Setup( );

	// update convar
	pIMCConfig->UpdateCurrentProfile( );

	// create the gamerules
	int iGameType = pIMCConfig->GetGameType( );

	const char *pszGameTypeClassname = g_pszGameTypeClassnames[ iGameType ];

	if( !pszGameTypeClassname )
	{
		AssertMsg( false, "InstallGameRules, Unknown Gametype" );
		return;
	}

	CreateGameRulesObject( pszGameTypeClassname );

	INSRules( )->InitRules( );

#ifdef TESTING

	Msg( "Using %s Game-Mode\n", INSRules( )->GetGameTypeName( ) );

#endif
}