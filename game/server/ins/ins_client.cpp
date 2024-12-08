//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "gamerules.h"
#include "entitylist.h"
#include "physics.h"
#include "ins_player.h"
#include "ins_gamerules.h"
#include "imc_config.h"
#include "engine/IEngineSound.h"
#include "tier0/vprof.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern CBaseEntity* FindPickerEntityClass(CBasePlayer* pPlayer, char* classname);
extern bool			g_fGameOver;

void FinishClientPutInServer(CINSPlayer* pPlayer)
{
	char sName[128];
	Q_strncpy(sName, pPlayer->GetPlayerName(), sizeof(sName));

	// First parse the name and remove any %'s
	for (char* pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++)
	{
		// Replace it with a space
		if (*pApersand == '%' || *pApersand == '#')
			*pApersand = ' ';
	}

	// notify other clients of player joining the game
	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "#Game_connected", sName[0] != 0 ? sName : "<unconnected>");

	pPlayer->InitialSpawn();
	pPlayer->Spawn();
	pPlayer->FadeToBlack(0.0f); // TODO: move to inital spawn?
	pPlayer->SetThink(NULL);
}

void ClientPutInServer(edict_t* pEdict, const char* playername)
{
	CINSPlayer* pPlayer = CINSPlayer::CreatePlayer("player", pEdict);
	pPlayer->SetPlayerName(playername);
}

void ClientActive(edict_t* pEdict, bool bLoadGame)
{
	// Can't load games in CS!
	Assert(!bLoadGame);

	CINSPlayer* pPlayer = ToINSPlayer(CBaseEntity::Instance(pEdict));
	FinishClientPutInServer(pPlayer);
}

CBaseEntity* FindEntity(edict_t* pEdict, char* classname)
{
	// If no name was given set bits based on the picked
	if (FStrEq(classname, ""))
	{
		return (FindPickerEntityClass(static_cast<CBasePlayer*>(GetContainingEntity(pEdict)), classname));
	}
	return NULL;
}

void ClientGamePrecache(void)
{
	CBaseEntity::PrecacheModel("sprites/white.vmt");
	CBaseEntity::PrecacheModel("sprites/physbeam.vmt");

	CBaseEntity::PrecacheModel("models/player.mdl");
	CBaseEntity::PrecacheModel("models/gibs/agibs.mdl");

	CBaseEntity::PrecacheScriptSound("FX_AntlionImpact.ShellImpact");
	CBaseEntity::PrecacheScriptSound("Missile.ShotDown");
	CBaseEntity::PrecacheScriptSound("Bullets.DefaultNearmiss");
	CBaseEntity::PrecacheScriptSound("Bullets.GunshipNearmiss");
	CBaseEntity::PrecacheScriptSound("Bullets.StriderNearmiss");
}

void GameStartFrame(void)
{
	VPROF("GameStartFrame()");
}

void InstallGameRules()
{
	// load IMC config
	CIMCConfig* pIMCConfig = CIMCConfig::IMCConfig();
	Assert(pIMCConfig);

	pIMCConfig->Setup();

	// update convar
	pIMCConfig->UpdateCurrentProfile();

	// create the gamerules
	int iGameType = pIMCConfig->GetGameType();

	const char* pszGameTypeClassname = g_pszGameTypeClassnames[iGameType];

	if (!pszGameTypeClassname)
	{
		AssertMsg(false, "InstallGameRules, Unknown Gametype");
		return;
	}

	CreateGameRulesObject(pszGameTypeClassname);

	INSRules()->InitRules();
}