//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player_resource.h"
#include "ins_gamerules.h"
#include "ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_SERVERCLASS_ST_NOBASE( CPlayerResource, DT_PlayerResource )

	SendPropArray3( SENDINFO_ARRAY3( m_bConnected ), SendPropBool( SENDINFO_ARRAY( m_bConnected ) ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iPing ), SendPropInt( SENDINFO_ARRAY( m_iPing ), 10, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iDeveloper ), SendPropInt( SENDINFO_ARRAY( m_iDeveloper ), 2, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iTeamID ), SendPropInt( SENDINFO_ARRAY( m_iTeamID ), 2, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_bAlive ), SendPropBool( SENDINFO_ARRAY( m_bAlive ) ) ),

	SendPropArray3( SENDINFO_ARRAY3( m_iMorale ), SendPropInt( SENDINFO_ARRAY( m_iMorale ), 12 ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iKills ), SendPropInt( SENDINFO_ARRAY( m_iKills ), 12 ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iDeaths ), SendPropInt( SENDINFO_ARRAY( m_iDeaths ), 12 ) ),

END_SEND_TABLE( )

BEGIN_DATADESC( CPlayerResource )

	DEFINE_FUNCTION( ResourceThink ),

END_DATADESC()

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( player_manager, CPlayerResource );

//=========================================================
//=========================================================
CPlayerResource *g_pPlayerResource;

//=========================================================
//=========================================================
CPlayerResource::~CPlayerResource( )
{
	g_pPlayerResource = NULL;
}

//=========================================================
//=========================================================
 void CPlayerResource::Create( void )
 {
	 Assert( !g_pPlayerResource );
	 g_pPlayerResource = ( CPlayerResource* )CBaseEntity::Create( "player_manager", vec3_origin, vec3_angle );
 }

//=========================================================
//=========================================================
void CPlayerResource::Spawn( void )
{
	for ( int i = 0; i < MAX_PLAYERS + 1; i++ )
	{
		m_bConnected.Set( i, false );
		m_iPing.Set( i, 0 );
		m_iDeveloper.Set( i, 0 );
		m_iTeamID.Set( i, 0 );
		m_bAlive.Set( i, false );

		m_iMorale.Set( i, 0 );
		m_iKills.Set( i, 0 );
		m_iDeaths.Set( i, 0 );
	}

	SetThink( &CPlayerResource::ResourceThink );
	SetNextThink( gpGlobals->curtime );
	m_nUpdateCounter = 0;
}

//=========================================================
//=========================================================
int CPlayerResource::ObjectCaps( void )
{
	return ( BaseClass::ObjectCaps( ) | FCAP_DONT_SAVE );
}

//=========================================================
//=========================================================
int CPlayerResource::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//=========================================================
//=========================================================
void CPlayerResource::ResourceThink( void )
{
	m_nUpdateCounter++;

	UpdatePlayerData( );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//=========================================================
//=========================================================
void CPlayerResource::UpdatePlayerData( void )
{
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CINSPlayer *pPlayer = ToINSPlayer( UTIL_PlayerByIndex( i ) );
		
		if ( pPlayer && pPlayer->IsConnected( ) )
		{
			m_bConnected.Set( i, true );

			// don't update ping every update
			if( !( m_nUpdateCounter % 20 ) )
			{
				// update ping all 20 think ticks = ( 20 * 0.1 = 2seconds )
				int iPing, iPacketLoss;
				UTIL_GetPlayerConnectionInfo( i, iPing, iPacketLoss );

				// calc avg for scoreboard so it's not so jittery
				iPing = 0.8f * m_iPing.Get( i ) + 0.2f * iPing;

				m_iPing.Set( i, iPing );
			}

			m_iTeamID.Set( i, pPlayer->GetTeamID( ) );
			m_bAlive.Set( i, pPlayer->IsAlive( ) );

			m_iMorale.Set( i, pPlayer->GetMorale( ) );

			const CGamePlayerStats &PlayerStats = pPlayer->GetUpdateStats( );
			m_iKills.Set( i, PlayerStats.m_iKills );
			m_iDeaths.Set( i, PlayerStats.m_iDeaths );
		}
		else
		{
			m_bConnected.Set( i, false );
		}
	}
}

//=========================================================
//=========================================================
void CPlayerResource::UpdatePlayerTeamID( int iID, int iTeamID )
{
	m_iTeamID.Set(iID, iTeamID);
}

//=========================================================
//=========================================================
void CPlayerResource::UpdatePlayerDeveloper(int iID, int iDeveloper)
{
	m_iDeveloper.Set(iID, iDeveloper);
}

//=========================================================
//=========================================================
CPlayerResource *PlayerResource( void )
{
	Assert( g_pPlayerResource );
	return g_pPlayerResource;
}
