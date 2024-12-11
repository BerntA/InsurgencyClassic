//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "ins_recipientfilter.h"
#include "team_shared.h"
#include "ins_player.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CReliableRecipientFilter::CReliableRecipientFilter( )
{
	MakeReliable( );
}

//=========================================================
//=========================================================
CReliablePlayTeamRecipientFilter::CReliablePlayTeamRecipientFilter( )
{
	AddRecipientsByTeam( GetGlobalTeam( TEAM_ONE ) );
	AddRecipientsByTeam( GetGlobalTeam( TEAM_TWO ) );
}

//=========================================================
//=========================================================
CReliablePlayerRecipientFilter::CReliablePlayerRecipientFilter( CINSPlayer *pPlayer )
{
	AddRecipient( pPlayer );
}

//=========================================================
//=========================================================
CReliableSquadRecipientFilter::CReliableSquadRecipientFilter( CINSSquad *pSquad )
{
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = pSquad->GetPlayer( i );

		if( pPlayer )
			AddRecipient( pPlayer );
	}
}

//=========================================================
//=========================================================
typedef void ( CRecipientFilter::*FilterMod_t )( const CBasePlayer *pPlayer );

void UnitFilter( CINSPlayer *pFilterPlayer, CINSSquad *pSquad, CRecipientFilter *pFilter, FilterMod_t FilterMod )
{
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = pSquad->GetPlayer( i );

		if( pPlayer )
			( pFilter->*FilterMod )( pPlayer );
	}
}

//=========================================================
//=========================================================
CUnitRecipientFilter::CUnitRecipientFilter( CINSPlayer *pPlayer )
{
	Setup( pPlayer );
}

void CUnitRecipientFilter::Setup( CINSPlayer *pPlayer )
{
	CINSSquad *pSquad = pPlayer->GetSquad( );

	if( pSquad )
		UnitFilter( pPlayer, pSquad, this, &CUnitRecipientFilter::AddRecipient );
}