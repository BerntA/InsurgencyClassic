//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "ins_gameinterface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CUtlLinkedList< CMapEntityRef, unsigned short > g_MapEntityRefs;

//=========================================================
//=========================================================
void CServerGameClients::GetPlayerLimits( int &minplayers, int &maxplayers, int &defaultMaxPlayers ) const
{
	minplayers = defaultMaxPlayers = 2; 
	maxplayers = MAX_PLAYERS;
}

//=========================================================
//=========================================================
class CINSMapLoadEntityFilter : public IMapEntityFilter
{
	bool ShouldCreateEntity( const char *pClassname )
	{
		// during map load, create all the entities.
		return true;
	}

	CBaseEntity *CreateNextEntity( const char *pClassname )
	{
		CBaseEntity *pRet = CreateEntityByName( pClassname );

		CMapEntityRef ref;
		ref.m_iEdict = -1;
		ref.m_iSerialNumber = -1;

		if( pRet )
		{
			ref.m_iEdict = pRet->entindex( );

			if ( pRet->edict( ) )
				ref.m_iSerialNumber = pRet->edict()->m_NetworkSerialNumber;
		}

		g_MapEntityRefs.AddToTail( ref );
		return pRet;
	}
};

//=========================================================
//=========================================================
void CServerGameDLL::LevelInit_ParseAllEntities( const char *pMapEntities )
{
	g_MapEntityRefs.Purge( );
	CINSMapLoadEntityFilter filter;
	MapEntity_ParseAllEntities( pMapEntities, &filter );
}