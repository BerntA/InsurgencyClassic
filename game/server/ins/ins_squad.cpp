//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Squad Management
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_squad_shared.h"
#include "ins_gamerules.h"
#include "ins_obj.h"
#include "team_lookup.h"
#include "ins_player.h"
#include "imc_config.h"
#include "play_team_shared.h"
#include "ins_recipientfilter.h"
#include "voicemgr.h"
#include "ins_utils.h"
#include "viewport_panel_names.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_squad, CINSSquad );

BEGIN_DATADESC(CINSSquad)

	DEFINE_THINKFUNC(CmdrPromotionThink),

END_DATADESC()

//=========================================================
//=========================================================
CINSSquad::CINSSquad( )
{
	m_iParentTeam = 0;

	ResetSlots( );
}

//=========================================================
//=========================================================
void CINSSquad::Init( int iID, int iTeamID, IMCSquadData_t *pSquadData )
{
	ResetEnabled( );

	m_iID = iID;
	m_iParentTeam = iTeamID;
	Q_strncpy( m_szName.GetForModify( ), pSquadData->m_szName, MAX_SQUADNAME_LENGTH );

	memcpy( &m_iSlots, &pSquadData->m_SlotData, sizeof( SlotData_t ) );
	m_iSlots.Set( 0, GetTeamLookup( )->GetCommanderClass( ) );

	NetworkStateChanged( );
	SetThink( &CINSSquad::CmdrPromotionThink );
	SetNextThink( gpGlobals->curtime + CMDR_PROMO_TICK);
	bPromoResponse = false;
	fPromoTimeout = 0.f;
	fPromoAccum = 0.f;
}

//=========================================================
//=========================================================
int CINSSquad::UpdateTransmitState( void )
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

//=========================================================
//=========================================================
void CINSSquad::SendStatusUpdate( void )
{
	// NOTE: reimpliment as needed
	/*IGameEvent *pEvent = gameeventmanager->CreateEvent( "game_squadupdate", true );

	if( pEvent )
	{
		pEvent->SetInt( "teamid", m_iParentTeam );
		gameeventmanager->FireEvent( pEvent );
	}*/
}

//=========================================================
//=========================================================
CINSSquad *CINSSquad::Create( void )
{
	return static_cast< CINSSquad* >( CreateEntityByName( "ins_squad" ) );
}

//=========================================================
//=========================================================
bool CINSSquad::IsValidSquadData( const SquadData_t &SquadData )
{
	return ( SquadData.IsValid( ) && IsEnabled( ) && IsValidSlotID( SquadData.GetSlotID( ) ) && IsSlotEmpty( SquadData.GetSlotID( ) ) );
}

//=========================================================
//=========================================================
void CINSSquad::ResetOrders( void )
{
	ResetObjOrders( );
	ResetUnitOrders( );
}

//=========================================================
//=========================================================
void CINSSquad::ResetSlots( void )
{
	for(int i = 0; i < MAX_SQUAD_SLOTS; i++)
		m_iSlots.Set( i, INVALID_SLOT );

	for(int i = 0; i < MAX_SQUAD_SLOTS; i++)
		m_iPlayerSlots.Set( i, INVALID_SLOT );
}

//=========================================================
//=========================================================
void CINSSquad::ResetEnabled( void )
{
	m_bEnabled = false;
}

//=========================================================
//=========================================================
void CINSSquad::SetEnabled( bool bState )
{
	m_bEnabled = bState;
}

//=========================================================
//=========================================================
void CINSSquad::AddPlayer( CINSPlayer *pPlayer, const SquadData_t &SquadData )
{
	bool bFirst = !HasPlayers();
	int iSlotID = SquadData.GetSlotID( );
	AssertMsg( IsValidSlotID( iSlotID ) && IsValidSquadData( SquadData ), "CINSSquad::AddPlayer, Passed Crappy Data" );

	// set the slots
	m_iPlayerSlots.Set( iSlotID, pPlayer->entindex( ) );

	// tell the playteam
	CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

	if( pTeam )
		pTeam->AddSquadPlayer( m_iID );

	if(bFirst)
	{
		SelectNewCommander(); // This will select that player and send the question
	}
}

//=========================================================
//=========================================================
void CINSSquad::RemovePlayer( CINSPlayer *pPlayer )
{
	int iSlotID = pPlayer->GetSlotID( );

	if( !IsValidSlotID( iSlotID ) )
		return;

	// reset squad and reset
	pPlayer->ResetSquad( );
	m_iPlayerSlots.Set( iSlotID, INVALID_SLOT );

	// tell the playteam
	CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

	if( pTeam )
		pTeam->RemoveSquadPlayer( );

	if(HasCommander() && GetCommander()->GetUserID() == pPlayer->GetUserID())
	{
		pPlayer->SetCommander(false);
		SelectNewCommander();
	}
}

//=========================================================
//=========================================================
CINSPlayer *CINSSquad::GetPlayer( int iSlotID ) const
{
	if( !IsValidSlotID( iSlotID ) || IsSlotEmpty( iSlotID ) )
		return NULL;

	return ToINSPlayer( UTIL_PlayerByIndex( m_iPlayerSlots[ iSlotID ] ) );
}

//=========================================================
//=========================================================
void CINSSquad::CmdrPromotionThink()
{

	// clear out any people on the blacklist who have paid their time
	for(int i = 0; i < m_CmdrBlackList.Count(); ++i)
	{
		if(gpGlobals->curtime >= m_CmdrBlackList[i].fReleaseTime)
		{
			m_CmdrBlackList.Remove(i);
		}
	}

	// If we are waiting for a response from an offer, make sure they dont take too long
	if(bPromoResponse && gpGlobals->curtime >= fPromoTimeout)
	{
		if(pMostEligible != 0)
		{
			// Send a nasty-gram about not responding and blacklist
			ClientPrint( pMostEligible, HUD_PRINTTALK, "You have waited to long to respond to the promotion offer." );

			BlackListEntry_t kickedCmdr;
			kickedCmdr.pPlayer = pMostEligible;
			kickedCmdr.fReleaseTime = gpGlobals->curtime + CMDR_BLACKLIST_LENGTH;
			m_CmdrBlackList.AddToTail(kickedCmdr); 

			pMostEligible = 0;
			bPromoResponse = false;
			SelectNewCommander();
		}
	}
	else
	{
		// If no offers are on the table and there is no commander, try a select 
		if( HasCommander() == false )
		{
			fPromoAccum += CMDR_PROMO_TICK;
			if(fPromoAccum > CMDR_PROMO_OVERFLOW)
			{
				SelectNewCommander();
			}
		}
		else
		{
			fPromoAccum = 0.f;
		}
	}
	SetNextThink(gpGlobals->curtime + CMDR_PROMO_TICK);
}

//=========================================================
//=========================================================
void CINSSquad::KickCommander()
{
	if( HasCommander() )
	{
		CINSPlayer* pCommander = GetCommander();
		// Demote the commander, blacklist them, and seek out another
		pCommander->SetCommander(false);

		ClientPrint( pCommander, HUD_PRINTTALK, "You have been demoted." );


		BlackListEntry_t kickedCmdr;
		kickedCmdr.pPlayer = pCommander;
		kickedCmdr.fReleaseTime = gpGlobals->curtime + CMDR_BLACKLIST_LENGTH;
		m_CmdrBlackList.AddToTail(kickedCmdr); 
	}
	SelectNewCommander();
}

//=========================================================
//=========================================================
void CINSSquad::SelectNewCommander()
{
	CINSPlayer* pCandidate = 0;

	for( int i = 0; i < MAX_SQUAD_SLOTS; ++i )
	{
		CINSPlayer* temp = GetPlayer(i);
		if( !temp ) continue;  // Skip empty slots

		// Check if this player has been blacklisted from commander
		bool bBlackListed = false;
		for( int j = 0; j < m_CmdrBlackList.Count(); ++j )
		{
			if(m_CmdrBlackList[j].pPlayer->GetUserID() == temp->GetUserID())
			{
				bBlackListed = true;
				break;
			}
		}

		if( !bBlackListed ) 
		{
			// Find the most eligible.  People that refuse the offer are blacklisted
			if( pCandidate == 0  || pCandidate->GetMorale() < temp->GetMorale())
			{
				pCandidate = temp;
			}
		}
	}
	
	if( pCandidate != 0 )
	{
		if( HasCommander() && GetCommander()->GetUserID() == pCandidate->GetUserID() )
		{
			// Selection process has grabbed the current commander. We do nothing except cancel Thinking logic
			bPromoResponse = false;
		}
		else
		{
			// Send the offer
			ClientPrint( pCandidate, HUD_PRINTCENTER, "You have been selected for promotion to commander. Accept(Default: PageUp) or Decline(Default:PageDn)" );
			pMostEligible = pCandidate;
			bPromoResponse = true;
			fPromoTimeout = gpGlobals->curtime + CMDR_RESPONSE_LENGTH;
		}
	}
}

//=========================================================
//=========================================================
void CINSSquad::PromotionResponse( CINSPlayer* pPlayer, bool bAccept )
{
	// They have responded
	if( pMostEligible != 0 && pPlayer->GetUserID() == pMostEligible->GetUserID() )
	{
		bPromoResponse = false; 

		if( bAccept )
		{
			if( HasCommander() && GetCommander()->GetUserID() != pMostEligible->GetUserID())
			{
				ClientPrint( GetCommander(), HUD_PRINTTALK, "You have been demoted from commander." );
				GetCommander()->SetCommander( false );
			}

			pPlayer->SetCommander( true );
			ClientPrint( pMostEligible, HUD_PRINTTALK, "Congratulations on your promotion." );
			pMostEligible = 0;
		}
		else
		{
			BlackListEntry_t kickedCmdr;
			kickedCmdr.pPlayer = pPlayer;
			kickedCmdr.fReleaseTime = gpGlobals->curtime + ( CMDR_BLACKLIST_LENGTH/2 );
			m_CmdrBlackList.AddToTail( kickedCmdr ); 

			ClientPrint( pMostEligible, HUD_PRINTTALK, "We're sorry to hear that." );
			pMostEligible = 0;

			SelectNewCommander();
		}
	}
}

//=========================================================
//=========================================================
void CINSSquad::SpawnPlayers( int iType, CUtlVector< CINSPlayer* > &PlayersSpawned )
{
	CPlayTeam *pTeam = GetParentTeam( );

	if( !pTeam )
		return;

	int iMaxScore = 0;
	CINSPlayer* pHighScorer = 0;

	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( !pPlayer )
			continue;

		if(pPlayer->GetMorale() > iMaxScore)
		{
			iMaxScore = pPlayer->GetMorale();
			pHighScorer = pPlayer;
		}
	
		if( pTeam->IsValidForRespawn( iType, pPlayer ) )
		{
			pPlayer->SpawnReinforcement( );

			PlayersSpawned.AddToTail( pPlayer );
		}

		pPlayer->PunishTKRemove( );
	}
	if( iType == PRESPAWNTYPE_REINFORCEMENT)
	{
		if( HasCommander() )
		{
			// Make sure the commander score isnt too far behind the top score on the squad
			int iCommanderScore = GetCommander()->GetMorale();
			if(iCommanderScore < iMaxScore && (iMaxScore - 20) > iCommanderScore)
			{
				SelectNewCommander();
			}
		}
	}
}

//=========================================================
//=========================================================
bool CINSSquad::SpawnPlayersValid( void )
{
	CPlayTeam *pTeam = GetParentTeam( );

	if( !pTeam )
		return false;

	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer && pTeam->IsValidForRespawn( PRESPAWNTYPE_REINFORCEMENT, pPlayer ) )
			return true;
	}

	return false;
}

//=========================================================
//=========================================================
void CINSSquad::ResetObjOrders( void )
{
	m_ObjOrders.Reset( );

	CReliableSquadRecipientFilter filter( this );
	m_ObjOrders.Send( filter );
}

//=========================================================
//=========================================================
bool CINSSquad::AssignObjOrders( CINSObjective *pObjective )
{
	Assert( INSRules( )->IsValidObjOrder( m_iParentTeam, pObjective ) );

	// work out the type
	int iOrderType = UTIL_CaculateObjType( m_iParentTeam, pObjective );

	if( iOrderType == ORDERTYPE_OBJ_NONE )
		return false;

	// ensure it's not the same order
	if( m_ObjOrders.HasOrder( pObjective ) )
		return false;

	// update data
	m_ObjOrders.Init( pObjective );

	// tell all the players
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer )
			pPlayer->AssignedObjOrders( iOrderType );
	}

	// send over network
	CReliableSquadRecipientFilter filter( this );
	m_ObjOrders.Send( filter );

	return true;
}

//=========================================================
//=========================================================
bool CINSSquad::HasObjOrders( void ) const
{
	return m_ObjOrders.HasOrders( );
}

//=========================================================
//=========================================================
bool CINSSquad::IsFollowingAttackOrders( CINSObjective *pObjective ) const
{
	return ( m_ObjOrders.m_pObjective == pObjective );
}

//=========================================================
//=========================================================
void CINSSquad::NotifyInvalidObjOrders( void )
{
	CINSPlayer *pCommander = GetCommander( );

	if( pCommander )
		UTIL_SendHint( pCommander, HINT_INITIALORDERS );
}

//=========================================================
//=========================================================
void CINSSquad::ResetUnitOrders( void )
{
	m_UnitOrders.Reset( );

	CReliableSquadRecipientFilter filter( this );
	m_UnitOrders.Send( filter );
}

//=========================================================
//=========================================================
bool CINSSquad::AssignUnitOrders( int iOrderType, const Vector &vecPosition )
{
	// update data
	m_UnitOrders.Init( iOrderType, vecPosition );

	// tell all the players
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer )
			pPlayer->AssignedUnitOrders( iOrderType );
	}

	// send over network
	CReliableSquadRecipientFilter filter( this );
	m_UnitOrders.Send( filter );

	return true;
}

//=========================================================
//=========================================================
bool CINSSquad::HasUnitOrders( void ) const
{
	return m_UnitOrders.HasOrders( );
}

//=========================================================
//=========================================================
void CINSSquad::UpdateOrders( CINSPlayer *pPlayer )
{
	CReliablePlayerRecipientFilter filter( pPlayer );

	m_ObjOrders.Send( filter );
	m_UnitOrders.Send( filter );
}

//=========================================================
//=========================================================
void CINSSquad::AssignPlayerOrders( int iOrderType )
{
	// ensure valid order
	if( !UTIL_ValidPlayerOrder( iOrderType ) )
		return;

	CINSPlayer *pCommander = GetCommander( );
	Assert( pCommander );

	if( !pCommander )
		return;

	// assign new orders to players
	SetPlayerOrders( iOrderType );

	// send out voice command
	KeyValues *pData = new KeyValues( "voicedata" );
	pData->SetInt( "order", iOrderType );

	UTIL_SendVoice( VOICEGRP_ORDER_PLAYER, pCommander, pData );
}

//=========================================================
//=========================================================
void CINSSquad::SetPlayerOrders( int iOrderType )
{
	// ensure valid order
	Assert( UTIL_ValidPlayerOrder( iOrderType ) );

	// assign new orders to players
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer )
			pPlayer->AssignPlayerOrders( iOrderType );
	}
}

//=========================================================
//=========================================================
#ifdef TESTING

void CINSSquad::ForcePlayerOrders( int iOrderType )
{
	if( !UTIL_ValidPlayerOrder( iOrderType ) )
		return;

	SetPlayerOrders( iOrderType );
}

#endif

//=========================================================
//=========================================================
CON_COMMAND( ins_kick_commander, "Kicks the current commander from that position. ins_kick_commander [team#] [squad#]" )
{
	if (args.ArgC() == 3)
	{		
		int teamid = atoi(args[1]);
		int squadid = atoi(args[2]);

#ifdef _DEBUG
		Msg("Teamid: %i, squadid: %i\n", teamid, squadid);
#endif // _DEBUG

		CINSSquad * sqd = GetGlobalPlayTeam( teamid )->GetSquad( squadid );
		if(sqd->HasCommander())
		{
			sqd->KickCommander();
		}
	}
}

#ifdef _DEBUG
//=========================================================
//=========================================================
CON_COMMAND( ins_force_commander, "Forces a commaner select [team#] [squad#]" )
{
	if (args.ArgC() == 3)
	{
		int teamid = atoi(args[1]);
		int squadid = atoi(args[2]);

#ifdef _DEBUG
		Msg("Teamid: %i, squadid: %i\n", teamid, squadid);
#endif // _DEBUG

		CINSSquad * sqd = GetGlobalPlayTeam( teamid )->GetSquad( squadid );
		sqd->SelectNewCommander();
	}
}
#endif // _DEBUG

//=========================================================
//=========================================================
CPlayTeam *CINSSquad::GetParentTeam( void ) const
{
	return GetGlobalPlayTeam( m_iParentTeam );
}

//=========================================================
//=========================================================
#ifdef TESTING

CON_COMMAND( checksquads, "Prints Squad Information" )
{
	Msg( "------------------\n\n" );
	Msg( "Printing Squad Information ...\n" );

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetGlobalPlayTeam( i )->PrintSquads( );

	Msg( "\n------------------\n" );
}

//=========================================================
//=========================================================
void CINSSquad::PrintSquad( void )
{
	for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer )
			Msg( "Slot %i: %s (%s)\n", i, pPlayer->GetPlayerName( ), pPlayer->GetClass( )->GetName( ) );
	}
}

#endif