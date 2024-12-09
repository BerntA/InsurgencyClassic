//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CServerGameClients::GetPlayerLimits(int& minplayers, int& maxplayers, int& defaultMaxPlayers) const
{
	minplayers = 2;
	maxplayers = 64;
	defaultMaxPlayers = 32;
}

void CServerGameDLL::LevelInit_ParseAllEntities(const char* pMapEntities)
{
}