//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Squad management class. Contains all the details for a specific squad.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_squad_shared.h"
#include "team_lookup.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"

#ifdef GAME_DLL
#include "ins_gamerules.h"
#else
#include "c_playerresource.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
SquadData_t::SquadData_t( )
{
	m_iSquadID = DISABLED_SQUAD;
}

SquadData_t::SquadData_t( int iSquadID, int iSlotID )
{
	m_iSquadID = iSquadID;
	m_iSlotID = iSlotID;
}

void SquadData_t::Reset( void )
{
	m_iSquadID = INVALID_SQUAD;
	m_iSlotID = INVALID_SLOT;
}

bool SquadData_t::IsValid( void ) const
{
	return ( m_iSquadID != INVALID_SQUAD && m_iSlotID != INVALID_SLOT );
}

bool SquadData_t::operator==( const SquadData_t &SquadData ) const
{
	return ( m_iSquadID == SquadData.m_iSquadID && m_iSlotID == SquadData.m_iSlotID );
}

SquadData_t &SquadData_t::operator=( const SquadData_t &SquadData )
{
	m_iSquadID = SquadData.m_iSquadID;
	m_iSlotID = SquadData.m_iSlotID;

	return *this;
}

struct EncodedSquadInfoData_t
{
	unsigned squad : 2;
	unsigned slot : 4;
};

void SquadData_t::ParseEncodedInfo( EncodedSquadData_t EncodedSquadInfo )
{
	int *pValue = &EncodedSquadInfo;
	EncodedSquadInfoData_t *pSquadData = ( EncodedSquadInfoData_t* )pValue;

	m_iSquadID = pSquadData->squad - 1;
	m_iSlotID = pSquadData->slot - 1;
}

EncodedSquadData_t SquadData_t::GetEncodedData( void ) const
{
	int iSquadMemory = 0;
	int *pSquadMemory = &iSquadMemory;
	EncodedSquadInfoData_t *pSquadData = ( EncodedSquadInfoData_t* )pSquadMemory;

	pSquadData->squad = m_iSquadID + 1;
	pSquadData->slot = m_iSlotID + 1;

	return iSquadMemory;
}

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( INSSquad, DT_INSSquad )

BEGIN_NETWORK_TABLE_NOBASE( CINSSquad, DT_INSSquad )

#ifdef GAME_DLL

	SendPropInt( SENDINFO( m_iParentTeam ) ),

	SendPropString( SENDINFO( m_szName ) ),

	SendPropArray( SendPropIntWithMinusOneFlag( SENDINFO_ARRAY( m_iSlots ) ), m_iSlots ),
	SendPropArray( SendPropIntWithMinusOneFlag( SENDINFO_ARRAY( m_iPlayerSlots ) ), m_iPlayerSlots ),

	SendPropBool( SENDINFO( m_bEnabled ) )

#else

	RecvPropInt( RECVINFO( m_iParentTeam ) ),

	RecvPropString( RECVINFO( m_szName ) ),

	RecvPropArray( RecvPropIntWithMinusOneFlag( RECVINFO( m_iSlots[ 0 ] ) ), m_iSlots ),
	RecvPropArray( RecvPropIntWithMinusOneFlag( RECVINFO( m_iPlayerSlots[ 0 ] ) ), m_iPlayerSlots ),

	RecvPropBool( RECVINFO( m_bEnabled ) )

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
CTeamLookup *CINSSquad::GetTeamLookup( void ) const
{
	return GetGlobalPlayTeam( m_iParentTeam )->GetTeamLookup( );
}

//=========================================================
//=========================================================
bool CINSSquad::HasFreeSlot( void ) const
{
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		if( IsSlotEmpty( i ) )
			return true;
	}

	return false;
}

//=========================================================
//=========================================================
bool CINSSquad::IsSlotEmpty( int iSlotID ) const
{
	if( !IsValidSlotID( iSlotID ) )
	{
		Assert( false );
		return true;
	}

	return ( m_iPlayerSlots[ iSlotID ] == INVALID_SLOT );
}

//=========================================================
//=========================================================
bool CINSSquad::HasCommander( void ) const
{
	// This gets more complicated with dynamic commanders
	for(int i = 0; i < MAX_SQUAD_SLOTS; ++i)
	{
		CINSPlayer * pPlayer = GetPlayer(i);
		if( pPlayer != 0 && pPlayer->IsCommander())
		{
			return true;
		}
	}
	return false;
}

//=========================================================
//=========================================================
CINSPlayer *CINSSquad::GetCommander( void )
{
	// This gets more complicated with dynamic commanders
	for(int i = 0; i < MAX_SQUAD_SLOTS; ++i)
	{
		CINSPlayer * pPlayer = GetPlayer(i);
		if( pPlayer != 0 && pPlayer->IsCommander())
		{
			return pPlayer;
		}
	}
	return 0;
}

//=========================================================
//=========================================================
CPlayerClass *CINSSquad::GetClass( int iSlotID ) const
{
	if( !IsValidSlotID( iSlotID ) )
	{
		Assert( false );
		return NULL;
	}

	return GetTeamLookup( )->GetClass( m_iSlots[ iSlotID ] );
}

//=========================================================
//=========================================================
int CINSSquad::GetClassID( int iSlotID ) const
{
	if(!IsValidSlotID( iSlotID ) )
		return INVALID_CLASS;

	return m_iSlots[ iSlotID ];
}

//=========================================================
//=========================================================
int CINSSquad::CountFreeSlots( void ) const
{
	int iCount = 0;

	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		if( IsSlotEmpty( i ) )
			iCount++;
	}

	return iCount;
}

//=========================================================
//=========================================================
int CINSSquad::CountOccupiedSlots( void ) const
{
	return ( MAX_SQUAD_SLOTS - CountFreeSlots( ) );
}

//=========================================================
//=========================================================
bool CINSSquad::HasPlayers( void ) const
{
	return ( CountOccupiedSlots( ) > 0 );
}

//=========================================================
//=========================================================
bool CINSSquad::IsValidSquad( int iSquadID )
{
	return ( iSquadID >= 0 && iSquadID < MAX_SQUADS );
}

//=========================================================
//=========================================================
bool CINSSquad::IsValidSlotID( int iSlotID )
{
	return ( iSlotID >= 0 && iSlotID < MAX_SQUAD_SLOTS );
}

//=========================================================
//=========================================================
bool CINSSquad::IsEnabled( void ) const
{
	return m_bEnabled;
}

//=========================================================
//=========================================================
int CINSSquad::GetScore(void)
{
	int iRet=0;
	for (int j=0;j<MAX_SQUAD_SLOTS;j++)
	{
#ifdef CLIENT_DLL
		int iID=GetPlayerID(j);
		if (iID)
		{
			iRet+= g_PR->GetMorale(iID);
		}
#else
		CINSPlayer* pPlayer=GetPlayer(j);
		if (pPlayer)
		{			
			iRet+=pPlayer->GetMorale();
		}
#endif // CLIENT_DLL
	}

	return iRet;
}