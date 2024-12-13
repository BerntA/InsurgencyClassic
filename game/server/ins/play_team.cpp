//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "play_team_shared.h"
#include "ins_player.h"
#include "team_lookup.h"
#include "imc_config.h"
#include "ins_squad_shared.h"
#include "ins_gamerules.h"
#include "player_resource.h"
#include "voicemgr.h"
#include "ins_utils.h"
#include "ins_recipientfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CPositionList::CPositionList( )
{
}

CPositionList::CPositionList( const CPositionList &PositionList )
{
	m_Positions.AddVectorToTail( PositionList.m_Positions );
}

void CPositionList::Add( int iSquadID, int iSlotID )
{
	SquadData_t SquadData( iSquadID, iSlotID );
	m_Positions.AddToTail( SquadData );
}

//=========================================================
//=========================================================
bool ClassPositionLess( const int &iLeft, const int &iRight )
{
	return ( iLeft > iRight );
}

//=========================================================
//=========================================================
CClassPositionList::CClassPositionList( )
{
	m_Classes.SetLessFunc( ClassPositionLess );
}

//=========================================================
//=========================================================
void CClassPositionList::Add( int iPlayerClass, int iSquadID, int iSlotID )
{
	int iClassIndex = m_Classes.Find( iPlayerClass );

	if( iClassIndex == m_Classes.InvalidIndex( ) )
		iClassIndex = m_Classes.Insert( iPlayerClass );

	CPositionList &ClassPositions = m_Classes[ iClassIndex ];
	ClassPositions.Add( iSquadID, iSlotID );
}

//=========================================================
//=========================================================
void CClassPositionList::Init( const CClassPositionList &SourceList )
{
	const ClassList_t &ClassList = SourceList.m_Classes;

	for( int i = 0; i < ( int )ClassList.Count( ); i++ )
		m_Classes.Insert( ClassList.Key( i ), ClassList.Element( i ) );
}

//=========================================================
//=========================================================
void AddTeamRecipients( CTeam *pTeam, CSendProxyRecipients *pRecipients )
{
	if( !pTeam )
		return;

	for( int i = 0; i < pTeam->GetNumPlayers( ); i++ )
	{
		CBasePlayer *pPlayer = pTeam->GetPlayer( i );

		if( !pPlayer )
			continue;

		pRecipients->SetRecipient( pPlayer->GetClientIndex( ) );
	}
}

void *SendProxy_SendTeamResourceDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	CPlayTeam *pTeam = (CPlayTeam*)pStruct;

	pRecipients->ClearAllRecipients( );

	if( pTeam )
	{
		AddTeamRecipients( pTeam, pRecipients );
		AddTeamRecipients( GetGlobalTeam( TEAM_SPECTATOR ), pRecipients );
	}

	return (void*)pVarData;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendTeamResourceDataTable );

/*void* SendProxy_SendMedicResourceDataTable(const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID)
{
	CPlayTeam *pTeam = (CPlayTeam*)pStruct;

	pRecipients->ClearAllRecipients();

	if(pTeam)
	{
		for(int i = 0; i < pTeam->GetNumPlayers(); i++)
		{
			CINSPlayer *pPlayer = ToINSPlayer(pTeam->GetPlayer(i));

			if(!pPlayer || (!pPlayer->IsBeingBandaged() && !pPlayer->IsMedic()))
				continue;

			pRecipients->SetRecipient(pPlayer->GetClientIndex());
		}
	}

	return (void*)pVarData;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendMedicResourceDataTable );*/

void SendProxy_TeamLookup( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CPlayTeam *pTeam = ( CPlayTeam* )pStruct;
	pOut->m_Int = pTeam->GetTeamLookupID( );
}

void SendProxy_TeamColor( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CPlayTeam *pTeam = ( CPlayTeam* )pStruct;
	pOut->m_Int = pTeam->GetColor( ).GetRawColor( );
}

int SendProxyArrayLength_SquadArray( const void *pStruct, int objectID )
{
	CPlayTeam *pTeam = ( CPlayTeam* )pStruct;
	return pTeam->GetSquadCount( );
}

void SendProxy_SquadList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CPlayTeam *pTeam = ( CPlayTeam* )pStruct;
	const CUtlVector< CINSSquad* > &Squads = pTeam->GetSquads( );

	Assert( pTeam->IsValidSquad( iElement ) );

	CINSSquad *pSquad = Squads[ iElement ];
	pOut->m_Int = pSquad->entindex( );
}

void SendProxy_NumWaves( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CPlayTeam *pTeam = ( CPlayTeam* )pStruct;
	pOut->m_Int = pTeam->GetIMCTeamConfig( )->GetNumWaves( ) + 1;
}

void SendProxy_TimeWave( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CPlayTeam *pTeam = ( CPlayTeam* )pStruct;
	pOut->m_Int = pTeam->GetReinforcementTime( );
}

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( pteam_manager, CPlayTeam )

/*BEGIN_SEND_TABLE_NOBASE( CPlayTeam, DT_MedicResource )


END_SEND_TABLE()*/

BEGIN_SEND_TABLE_NOBASE( CPlayTeam, DT_TeamResource )

	SendPropArray2( 
		SendProxyArrayLength_SquadArray,
		SendPropInt( "squad_element", 0, 4, 10, SPROP_UNSIGNED, SendProxy_SquadList ),
		MAX_SQUADS,
		0, 
		"squad_array"
		),

	SendPropArray3( SENDINFO_ARRAY3( m_iSquadData ), SendPropInt( SENDINFO_ARRAY( m_iSquadData ), 8, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iHealthType ), SendPropInt( SENDINFO_ARRAY( m_iHealthType ), 2, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iRank ), SendPropIntWithMinusOneFlag( SENDINFO_ARRAY( m_iRank ), 3 ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iOrder ), SendPropIntWithMinusOneFlag( SENDINFO_ARRAY( m_iOrder ), MAX_PORDER_BITS ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iStatusType ), SendPropIntWithMinusOneFlag( SENDINFO_ARRAY( m_iStatusType ), MAX_PSTATUSTYPE_BITS ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_iStatusID ), SendPropIntWithMinusOneFlag( SENDINFO_ARRAY( m_iStatusID ), MAX_PSTATUSID_BITS ) ),
	SendPropArray3( SENDINFO_ARRAY3( m_bNeedsHelp ), SendPropBool( SENDINFO_ARRAY( m_bNeedsHelp ) ) ),

	SendPropInt( "numwaves", 0, 1, 6, SPROP_UNSIGNED, SendProxy_NumWaves ),
	SendPropInt( "timewave", 0, 1, 9, SPROP_UNSIGNED, SendProxy_TimeWave ),

	SendPropArray3( SENDINFO_ARRAY3( m_iReinforcementsLeft ), SendPropInt( SENDINFO_ARRAY( m_iReinforcementsLeft ), 7, SPROP_UNSIGNED ) ),
	SendPropTime( SENDINFO( m_flStartReinforcementTime ) ),
	SendPropBool( SENDINFO( m_bEmergencyReinforcement ) ),

	SendPropArray3( SENDINFO_ARRAY3( m_iEmergencyReinforcementsLeft ), SendPropInt( SENDINFO_ARRAY( m_iEmergencyReinforcementsLeft ), 7, SPROP_UNSIGNED ) ),

END_SEND_TABLE()

IMPLEMENT_SERVERCLASS_ST( CPlayTeam, DT_PlayTeam )

	SendPropInt( "team_lookupid", 0, 1, 4, SPROP_UNSIGNED, SendProxy_TeamLookup ),

	SendPropInt( "color", 0, SIZEOF_IGNORE, 32, SPROP_UNSIGNED, SendProxy_TeamColor ),

	SendPropInt( SENDINFO( m_iScore ) ),

	// data that only gets sent to players on their own team.
	SendPropDataTable( "TeamResource", 0, &REFERENCE_SEND_TABLE( DT_TeamResource ), SendProxy_SendTeamResourceDataTable ),

	// only send to players who are medics or bandaging
	//SendPropDataTable( "MedicResource", 0, &REFERENCE_SEND_TABLE(DT_MedicResource), SendProxy_SendMedicResourceDataTable ),

END_SEND_TABLE()

//=========================================================
//=========================================================
BEGIN_DATADESC( CPlayTeam )

	DEFINE_FUNCTION( ResourceThink ),

END_DATADESC()

//=========================================================
//=========================================================
int CPlayTeam::m_iObjectiveCaptures = 0;
int CPlayTeam::m_iTotalPlayerCount = 0;

CPlayTeam::CPlayTeam( )
{
	m_pIMCTeamConfig = NULL;

	m_iScore = 0;

	m_iEnabledArea = 0;
	m_iFreeSlots = 0;

	m_flStartReinforcementTime = m_flLastReinforcementTime = 0.0f;
	m_bEmergencyReinforcement = false;

	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		m_iReinforcementsLeft.Set( i, 0 );
		m_iEmergencyReinforcementsLeft.Set( i, 0 );
	}

	m_bHadDeployment = m_bHadReinforcement = false;

	for( int i = 0; i < MAX_PLAYERS + 1; i++ )
	{
		m_iSquadData.Set( i, 0 );
		m_iHealthType.Set( i, 0 );
		m_iRank.Set( i, INVALID_RANK );
		m_iOrder.Set( i, INVALID_PORDER );
		m_iStatusType.Set( i, INVALID_PSTATUSTYPE );
		m_iStatusID.Set( i, INVALID_PSTATUSID );
		m_bNeedsHelp.Set( i, false );
	}

#ifdef _DEBUG

	m_bDisabledSpawn = false;

#endif

	m_bClanReady = false;

	m_bDeploying = false;

	m_iObjectiveCaptures = 0;
}

//=========================================================
//=========================================================
void CPlayTeam::Create( CIMCTeamConfig *pIMCTeamConfig )
{
	CPlayTeam *pTeam = ( CPlayTeam* )CreateEntityByName( "pteam_manager" );

	pTeam->Init( pIMCTeamConfig );
	pTeam->Spawn( );
}

//=========================================================
//=========================================================
void CPlayTeam::Spawn( void )
{
	SetThink( &CPlayTeam::ResourceThink );
	SetNextThink( gpGlobals->curtime );
}

//=========================================================
//=========================================================
void CPlayTeam::LevelInit( void )
{
	m_iTotalPlayerCount = 0;
}

//=========================================================
//=========================================================
void CPlayTeam::ResourceThink( void )
{
	// PNOTE: need to think about if it's wise to have such "important"
	// data such as squaddata being sent every 0.1f (should be ASAP)

	for( int i = 0; i < m_Players.Count( ); i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );
		int iEntIndex = pPlayer->entindex( );

		if( !pPlayer )
			continue;

		m_iSquadData.Set( iEntIndex, pPlayer->GetEncodedSquadData( ) );
		m_iHealthType.Set( iEntIndex, pPlayer->GetHealthType( ) );
		m_iRank.Set( iEntIndex, pPlayer->GetRank( ) );
		m_iOrder.Set( iEntIndex, pPlayer->GetPlayerOrder( ) );
		m_iStatusType.Set( iEntIndex, pPlayer->GetStatusType( ) );
		m_iStatusID.Set( iEntIndex, pPlayer->GetStatusID( ) );
		m_bNeedsHelp.Set( iEntIndex, pPlayer->NeedsHelp( ) );
	}
	
	SetNextThink( gpGlobals->curtime + 0.1f );
}

//=========================================================
//=========================================================
void CPlayTeam::Precache( void )
{
	GetTeamLookup( )->Precache( );
}

//=========================================================
//=========================================================
void CPlayTeam::Init( CIMCTeamConfig *pTeamConfig )
{
	int iTeamID = pTeamConfig->GetID( );

	// init baseclass stuff
	BaseClass::Init( iTeamID );

	// put pointer together
	m_pIMCTeamConfig = pTeamConfig;

	// create squads
	for( int i = 0; i < pTeamConfig->GetSquadCount( ); i++ )
	{
		IMCSquadData_t *pSquadData = pTeamConfig->GetSquad( i );

		if( !pSquadData )
			continue;

		CINSSquad *pSquad = CINSSquad::Create( );

		if( !pSquad )
		{
			AssertMsg( false, "CPlayTeam::Init, Failed to Create Squad" );
			continue;
		}

		// init
		pSquad->Init( i, iTeamID, pSquadData );

		// add to the class positions
		for( int j = 0; j < MAX_SQUAD_SLOTS; j++ )
			m_ClassPositionList.Add( pSquadData->m_SlotData[ j ], i, j );

		// add to the squad and update
		m_Squads.AddToTail( pSquad );
	}

	// init free slots
	m_iFreeSlots = GetSquadCount( ) * MAX_SQUAD_SLOTS;

	// reset reinforcements
	ResetReinforcements( );
}

//=========================================================
//=========================================================
const char *CPlayTeam::GetName( void ) const
{
	return GetTeamLookup( )->GetName( );
}

//=========================================================
//=========================================================
const char *CPlayTeam::GetModel( void ) const
{
	return GetTeamLookup( )->GetModelData( )->GetPath( );
}

//=========================================================
//=========================================================
void CPlayTeam::AddPlayer( CINSPlayer *pPlayer )
{
	BaseClass::AddPlayer( pPlayer );

	// tell gamerules
	INSRules( )->PlayerJoinedTeam( pPlayer, this );

	// update squad area
	UpdateSquadArea( );

	// update numbers
	m_iTotalPlayerCount++;

	// draw debug lines
#ifdef _DEBUG

	UTIL_DrawLineList( );

#endif
}

//=========================================================
//=========================================================
void CPlayTeam::RemovePlayer( CINSPlayer *pPlayer )
{
	// remove the player
	BaseClass::RemovePlayer( pPlayer );

	// tell gamerules
	INSRules( )->PlayerLeftTeam( pPlayer, this );

	// remove from squad
	CINSSquad *pSquad = pPlayer->GetSquad( );
		
	if( pSquad )
		pSquad->RemovePlayer( pPlayer );

	// update numbers
	Assert( m_iTotalPlayerCount > 0 );
	m_iTotalPlayerCount = max( m_iTotalPlayerCount - 1, 0 );

	// update fireteams
	UpdateSquadArea( );
}

//=========================================================
//=========================================================
int CPlayTeam::PlayerSpawnType( void ) const
{
	return PSPAWN_NONE;
}

//=========================================================
//=========================================================
int CPlayTeam::GetSquadCount( void ) const
{
	return m_Squads.Count( );
}

//=========================================================
//=========================================================
CINSSquad *CPlayTeam::GetSquad( int iID ) const
{
	if( !IsValidSquad( iID ) )
		return NULL;

	return m_Squads[ iID ];
}

//=========================================================
//=========================================================
CINSSquad *CPlayTeam::GetSquad( const SquadData_t &SquadData ) const
{
	return GetSquad( SquadData.GetSquadID( ) );
}

//=========================================================
//=========================================================
void CPlayTeam::AddSquadPlayer( int iSquadID )
{
	m_iFreeSlots++;

	StartReinforcement( iSquadID );
}

//=========================================================
//=========================================================
void CPlayTeam::RemoveSquadPlayer( void )
{
	m_iFreeSlots--;
}

//=========================================================
//=========================================================
void CPlayTeam::MassSquadRemove( void )
{
	for( int i = 0; i < GetNumPlayers( ); i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( !pPlayer )
			continue;

		CINSSquad *pSquad = pPlayer->GetSquad( );

		if( pSquad )
			pSquad->RemovePlayer( pPlayer );
	}

	UpdateSquadArea( true );
}

//=========================================================
//=========================================================
bool CPlayTeam::GetRandomSquad( SquadData_t &SquadData ) const
{
	if( IsFull( ) )
		return false;

	int iSquadCount = GetSquadCount();

	SquadData.m_iSquadID = random->RandomInt( 0, iSquadCount - 1 );
	SquadData.m_iSlotID = random->RandomInt( 0, MAX_SQUAD_SLOTS - 1 );

	// did we strike it luckly?
	if( IsValidSquadData( SquadData ) )
		return true;

	// ... otherwise start at the top and search down
	for( int i = 0; i < iSquadCount; i++ )
	{
		CINSSquad *pSquad = GetSquad( i );

		if( !pSquad || !pSquad->IsEnabled( ) )
			continue;

		SquadData.m_iSquadID = i;

		for( int j = 0; j < MAX_SQUAD_SLOTS; j++ )
		{
			SquadData.m_iSlotID = j;

			if( !pSquad->IsValidSquadData( SquadData ) )
				continue;

			return true;
		}
	}

	// there should always be space!
	AssertMsg( false, "CPlayTeam::GetRandomSquad, Failed" );

	return false;
}

//=========================================================
//=========================================================
bool CPlayTeam::IsValidSquad( int iID ) const
{
	return m_Squads.IsValidIndex( iID );
}

//=========================================================
//=========================================================
bool CPlayTeam::IsValidSquadData( const SquadData_t &SquadData ) const
{
	CINSSquad *pSquad = GetSquad( SquadData );
	return ( pSquad && pSquad->IsValidSquadData( SquadData ) );
}

//=========================================================
//=========================================================
void TeamSizeCallback(IConVar* var, const char* pOldValue, float flOldValue)
{
	ConVar* pVar = (ConVar*)var;

	if( !INSRules( ) )
		return;

	if( pVar->GetInt( ) == 0 )
		return;

	CTeam *pInvalidTeam = NULL;

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( i );

		if( !pTeam )
			continue;

		bool bSetInvalid = false;

		if( pInvalidTeam )
		{
			if( pTeam->GetNumPlayers( ) > pInvalidTeam->GetNumPlayers( ) )
				bSetInvalid = true;
		}
		else
		{
			if( pTeam->GetNumPlayers( ) > pVar->GetInt( ) )
				bSetInvalid = true;
		}

		if( bSetInvalid )
			pInvalidTeam = pTeam;
	}

	if( pInvalidTeam )
	{
		Msg( "Invalid Team Size (Team %s has %i Players)\n", pInvalidTeam->GetName( ), pInvalidTeam->GetNumPlayers( ) );
		pVar->SetValue( 0 );
	}
}

ConVar teamsize( "ins_teamsize", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Maximum Team Size", true, 0, true, 32, TeamSizeCallback );

bool CPlayTeam::IsFull( void ) const
{
	if( m_iFreeSlots == 0 )
		return true;

	if( teamsize.GetInt( ) == 0 )
		return false;

	return ( GetNumPlayers( ) < teamsize.GetInt( ) );
}

//=========================================================
//=========================================================
void CPlayTeam::UpdateSquadArea( bool bFullReset )
{
	// reset when required
	if( bFullReset )
	{
		for( int i = 0; i < GetSquadCount( ); i++ )
		{
			CINSSquad *pSquad = GetSquad( i );

			if( pSquad )
				pSquad->ResetEnabled( );
		}

		m_iEnabledArea = 0;
	}

	// find the quota
	int iAreaQuota = 0;

	// ... when squads aren't locked, enable them all
	if( INSRules( )->SquadsForcedOpen( ) || INSRules()->ShouldBeWaitingForPlayers( ) )
		iAreaQuota = GetAreaCount( );
	else
		iAreaQuota = GetAreaQuota( );

	// don't update if they're the same
	if( m_iEnabledArea == iAreaQuota )
		return;

	// now close/open squads
	bool bOpeningUp = ( iAreaQuota > m_iEnabledArea );
	int iAreaDiff = abs( iAreaQuota - m_iEnabledArea );

	ModifySquadArea( iAreaDiff, bOpeningUp );
}

//=========================================================
//=========================================================
int CPlayTeam::GetAreaCount(void) const
{
	return m_Squads.Count( );
}

//=========================================================
//=========================================================
int CPlayTeam::GetAreaQuota( void ) const
{
	int iNumPlayers = GetNumPlayers( );

	if( iNumPlayers == 0 )
		return 0;

	// if we have more players than one area
	// size, then work out how many we'll needed
	if( iNumPlayers > MAX_SQUAD_SLOTS )
	{
		int iAreaQuota = ( int )( ( float )iNumPlayers / MAX_SQUAD_SLOTS );

		// if it doesn't easily divide, increase by one to make the room
		if( ( iAreaQuota % MAX_SQUAD_SLOTS ) != 0 )
			iAreaQuota++;

		return iAreaQuota;
	}

	return 1;
}

//=========================================================
//=========================================================
void CPlayTeam::ModifySquadArea( int iAreaDiff, bool bOpeningUp )
{
	int iAreaChange = 0;

	for( int i = 0 ; i < GetSquadCount() && iAreaChange != iAreaDiff; i++ )
	{
		CINSSquad *pSquad = GetSquad( i );

		if( !pSquad || pSquad->IsEnabled( ) == bOpeningUp || pSquad->HasPlayers( ) )
			continue;

		pSquad->SetEnabled( bOpeningUp );
		iAreaChange++;

		pSquad->SendStatusUpdate( );
	}

	// adjust the area avaible
	m_iEnabledArea = m_iEnabledArea + ( bOpeningUp ? iAreaChange : -iAreaChange );
}

//=========================================================
//=========================================================
CTeamLookup *CPlayTeam::GetTeamLookup( void ) const
{
	return GetIMCTeamConfig( )->GetTeamLookup( );
}

//=========================================================
//=========================================================
int CPlayTeam::GetTeamLookupID( void )
{
	return GetIMCTeamConfig( )->GetClass( );
}

//=========================================================
//=========================================================
void CPlayTeam::IncrementScore( void )
{	
	m_iScore++;
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CPlayTeam::ResetScores( void )
{
	m_iScore = 0;

	for( int i = 0; i < m_Players.Count( ); i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer )
			pPlayer->ResetStats( );
	}

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CPlayTeam::UpdateRanks( void )
{
	for( int i = 0; i < m_Players.Count(); i++ )
	{
		CINSPlayer *pPlayer = GetPlayer( i );

		if( pPlayer )
			pPlayer->UpdateRank( );
	}
}

void CPlayTeam::UpdatePlayerRanks( void )
{
	CUtlVector<CINSPlayer*> apPlayers;
	int iMax=CINSPlayer::m_RankBoundaries[RANK_SERGEANT];
	for ( int i = 0; i < m_Players.Count(); i++ )
	{
		CINSPlayer* pPlayer=GetPlayer( i );
		if (pPlayer->GetMorale()>iMax)
		{
			iMax=pPlayer->GetMorale();
			apPlayers.RemoveAll();
			apPlayers.AddToTail(pPlayer);
		}
		else if (pPlayer->GetMorale()==iMax)
		{
			apPlayers.AddToTail(pPlayer);
		}
	}
	for ( int i = 0; i < apPlayers.Count(); i++ )
	{
		apPlayers[i]->SetRank(RANK_LIEUTENANT);
	}
}

//=========================================================
//=========================================================
void CPlayTeam::PlayerKilled( CINSPlayer *pPlayer )
{
	// start reinforcement when the player is valid and in a squad
	if( pPlayer && pPlayer->InSquad( ) )
		StartReinforcement( pPlayer->GetSquadID( ) );
}

//=========================================================
//=========================================================
bool CPlayTeam::AreAllPlayersDead( void ) const
{
	for( int i = 0; i < m_Players.Count( ); i++ )
	{
		CINSPlayer *pPlayer = m_Players[ i ];

		if( !pPlayer )
		{
			Assert( false );
			return false;
		}
		
		if( pPlayer->IsAlive( ) )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CPlayTeam::StartReinforcement( int iSquadID )
{
	// don't start when we have already or we can't
	if( HasReinforcementStarted( ) || !HasRedeployments( iSquadID ) )
		return;

	StartReinforcement( iSquadID, false );
}

//=========================================================
//=========================================================
void CPlayTeam::SendReinforcementMsg( CUtlVector< CINSPlayer* > &SpawnedPlayers )
{
	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		CINSSquad *pSquad = GetSquad( i );

		if( !pSquad )
			continue;

		CReliableSquadRecipientFilter filter( pSquad );

		for( int j = 0; j < SpawnedPlayers.Count( ); j++ )
			filter.RemoveRecipient( SpawnedPlayers[ j ] );

		UserMessageBegin( filter, "ReinforceMsg" );
	
			WRITE_BYTE( m_iReinforcementsLeft[ i ] );

		MessageEnd( );
	}
}

//=========================================================
//=========================================================
void CPlayTeam::StartReinforcement( int iSquadID, bool bEmergency )
{
	// mark the new reinforcement time
	m_flStartReinforcementTime = gpGlobals->curtime;

	// set the type
	m_bEmergencyReinforcement = bEmergency;

	// cannot send out an emergency one until past half the reinforcement time
	if( bEmergency )
		m_flNextEmergencyReinforcement = m_flStartReinforcementTime + ( GetReinforcementTime( ) * 0.5f );

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
bool CPlayTeam::IsRedeploymentAllowed( void ) const
{
	return ( INSRules( )->IsModeRunning( )&& !INSRules( )->RunningMode( )->IsRestarting( ) );
}

//========================================================
//=========================================================
bool CPlayTeam::HasRedeployments( int iSquadID ) const
{
	// we can't deploy when the game is restarting
	if( !IsRedeploymentAllowed( ) )
		return false;

	// always have when unlimited
	if( IsUnlimitedWaves( ) )
		return true;

	// check both squads when invalid squad
	if( iSquadID == INVALID_SQUAD )
	{
		for( int i = 0; i < MAX_SQUADS; i++ )
		{
			if( !IsValidSquad( i ) )
				continue;

			if( m_iReinforcementsLeft[ i ] > 0 )
				return true;
		}

		return false;
	}

	// ensure valid squad
	if( !IsValidSquad( iSquadID ) )
	{
		Assert( false );
		return false;
	}

	return ( m_iReinforcementsLeft[ iSquadID ] > 0 );
}

//=========================================================
//=========================================================
void CPlayTeam::CheckReinforcements( void )
{
	if( HasReinforcementStarted( ) )
	{
		if( IsRedeploymentAllowed( ) )
		{
			if( ( gpGlobals->curtime >= EndReinforcementTime( ) ) )
				DoReinforcement( PRESPAWNTYPE_REINFORCEMENT );
		}
		else
		{
			ResetReinforcementTime( );
		}
	}
}

//=========================================================
//=========================================================
float CPlayTeam::EndReinforcementTime( void ) const
{
	return ( m_flStartReinforcementTime + GetReinforcementTime( ) );
}

//=========================================================
//=========================================================
int CPlayTeam::AverageReinforcementsLeft( void )
{
	if( IsUnlimitedWaves( ) )
		return 0;

	int iSquads, iLeft;
	iSquads = iLeft = 0;

	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		if( IsValidSquad( i ) )
		{
			iSquads++;
			iLeft += m_iReinforcementsLeft[ i ];
		}
	}

	return iLeft;
}

//=========================================================
//=========================================================
bool CPlayTeam::HasReinforcementLeft( void ) const
{
	if( IsUnlimitedWaves( ) )
		return true;

	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		if( IsValidSquad( i ) && m_iReinforcementsLeft[ i ] > 0 )
			return true;
	}

	return false;
}

//=========================================================
//=========================================================
void CPlayTeam::ResetReinforcements( void )
{
	m_bHadDeployment = m_bHadReinforcement = false;

	m_bEmergencyReinforcement = false;

	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		bool bIsValidSquad = IsValidSquad( i );

		if( !IsUnlimitedWaves( ) )
			m_iReinforcementsLeft.Set( i, bIsValidSquad ? GetNumWaves( ) : 0 );

		m_iEmergencyReinforcementsLeft.Set( i, bIsValidSquad ? GetEmergencyWaves( ) : 0 );
	}

	m_flNextEmergencyReinforcement = 0.0f;

	ResetReinforcementTime( );

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CPlayTeam::ResetReinforcementTime( void )
{
	m_flStartReinforcementTime = 0.0f;

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
#define RTIME_WAITINGPLAYERS 2.0f

int CPlayTeam::GetReinforcementTime( void ) const
{
	return GetReinforcementTime( m_bEmergencyReinforcement );
}

//=========================================================
//=========================================================
#define EMERGENCYDEPLOY_TIME 1.0f

int CPlayTeam::GetReinforcementTime( bool bEmergency ) const
{
	float flBaseTime = 0.0f;

	if( bEmergency )
	{
		flBaseTime = EMERGENCYDEPLOY_TIME;
	}
	else
	{
		if( INSRules( )->IsModeRunning( ) && INSRules( )->RunningMode( )->IsWaitingForPlayers( ) )
			flBaseTime = RTIME_WAITINGPLAYERS;
		else
			flBaseTime = GetIMCTeamConfig( )->GetTimeWave( );
	}

	return ( flBaseTime + DEATH_ANIMATION_TIME );
}

//=========================================================
//=========================================================
void CPlayTeam::DoReinforcement( int iType, int iSquadID, bool bProtectReinforcements )
{
	if( !INSRules( ) )
		return;

	if( iType < 0 || iType >= PRESPAWNTYPE_COUNT )
	{
		Assert( false );
		return; 
	}

	// stop reinforcements when they're disabled
	if( iType == PRESPAWNTYPE_REINFORCEMENT )
	{
	#ifdef _DEBUG

		if( m_bDisabledSpawn )
			return;

	#endif

		ResetReinforcementTime( );
	}

	// ensure valid squad
	if( iSquadID != INVALID_SQUAD && !IsValidSquad( iSquadID ) )
	{
		Assert( false );
		return;
	}

	// no longer an emergency
	m_bEmergencyReinforcement = false;

	// respawn all the squads

	// ... set us as deploying
	m_bDeploying = true;

	// ... respawn
	CUtlVector< CINSPlayer* > PlayersSpawned;

	for( int i = 0; i < GetSquadCount( ); i++ )
	{
		// ensure we're spawning this squad and we
		// have the reinforcements
		if( ( iSquadID != INVALID_SQUAD && iSquadID != i ) && ( IsUnlimitedWaves( ) || m_iReinforcementsLeft[ i ] > 0 ) )
			continue;

		CINSSquad *pSquad = GetSquad( i );

		if( pSquad )
			pSquad->SpawnPlayers( iType, PlayersSpawned );
	}

	// ... not deploying anymore
	m_bDeploying = false;

	// if we haven't respawned anymore, so don't start
	if( PlayersSpawned.Count( ) == 0 )
		return;

	// had a deployment
	m_bHadDeployment = true;

	// if this is a reinforcement, then reset the time
	if( iType == PRESPAWNTYPE_REINFORCEMENT )
	{
		// had a reinforcement
		m_bHadReinforcement = true; 

		// reduce reinforcement when we don't have unlimited and we're not protecting
		if( !IsUnlimitedWaves( ) && !bProtectReinforcements )
		{
			if( iSquadID == INVALID_SQUAD )
			{
				int iOldValue = 0;

				for( int i = 0; i < MAX_SQUADS; i++ )
				{
					if( !IsValidSquad( i ) )
						continue;

					iOldValue = m_iReinforcementsLeft[ i ];

					if( iOldValue == 0 )
						continue;

					m_iReinforcementsLeft.Set( i, iOldValue - 1 );
				}
			}
			else
			{
				m_iReinforcementsLeft.Set( iSquadID, m_iReinforcementsLeft[ iSquadID ] - 1 );
			}
		}

		// send out the message
		SendReinforcementMsg( PlayersSpawned );
	}

	// restock the weapon crates 
	INSRules( )->RestockWeaponCaches( GetTeamID( ) );

	// update network state etc
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
int CPlayTeam::DoEmergencyReinforcement( int iSquadID )
{
	// ensure valid
	if( iSquadID == INVALID_SQUAD || !IsValidSquad( iSquadID ) )
	{
		Assert( false );
		return EDRET_INVALID;
	}

	// find data
	CINSSquad *pSquad = GetSquad( iSquadID );

	if( !pSquad )
		return EDRET_INVALID;

	// ensure allowed
	if( !IsRedeploymentAllowed( ) )
		return EDRET_INVALID;

	int iEmergencyReinforcementsLeft = m_iEmergencyReinforcementsLeft[ iSquadID ];

	// ensure one hasn't already been called
	if( m_bEmergencyReinforcement )
		return EDRET_DOING;

	// ensure we have some remaining
	if( iEmergencyReinforcementsLeft == 0 )
		return EDRET_NONE;
		
	// ensure we are outside restricted time
	if( m_flNextEmergencyReinforcement != 0.0f && m_flNextEmergencyReinforcement > gpGlobals->curtime )
		return EDRET_NOTATM;

	// ensure that the reinforcement time hasn't slipped past the emergency time
	// because there would be no point if it has
	if( m_flStartReinforcementTime != 0.0f && ( ( EndReinforcementTime( ) - gpGlobals->curtime ) < GetReinforcementTime( true ) ) )
		return EDRET_NONEED_TIME;

	// check that there are players to reinforce
	if( !pSquad->SpawnPlayersValid( ) )
		return EDRET_NONEED_PLAYERS;

	// reduce count
	m_iEmergencyReinforcementsLeft.Set( iSquadID, iEmergencyReinforcementsLeft - 1 );

	// do the reinforcement
	StartReinforcement( iSquadID, true );

	return EDRET_DONE;
}

//=========================================================
//=========================================================
bool CPlayTeam::IsValidForRespawn( int iType, CINSPlayer *pPlayer )
{
	// never allow when invalid squad
	if( !pPlayer->IsValidSquad( ) )
		return false;

	// always allow when restart type
	if( iType == PRESPAWNTYPE_RESTART )
		return true;

	// allow when not running around and reinforcements are enabled
	if( !pPlayer->IsRunningAround( ) && pPlayer->AllowReinforcement( ) )
		return !pPlayer->IsTKPunished( );

	return false;
}

//=========================================================
//=========================================================
#ifdef _DEBUG

CON_COMMAND( forcerespawn, "Force the Teams to Redeploy" )
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetGlobalPlayTeam( i )->ForceRespawnPlayers( );
}

CON_COMMAND( disablerespawn, "Ignore Respawns" )
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetGlobalPlayTeam( i )->DisableRespawn( );
}

void CPlayTeam::ForceRespawnPlayers( void )
{
	if( !IsRedeploymentAllowed( ) )
	{
		Msg( "Redelopment is not Allowed\n" );
		return;
	}

	DoReinforcement( PRESPAWNTYPE_REINFORCEMENT );
}

void CPlayTeam::DisableRespawn( void )
{
	m_bDisabledSpawn = !m_bDisabledSpawn;
	Msg( "Team %i, Respawning for %s has been %s\n", GetTeamID( ), GetName( ), m_bDisabledSpawn ? "Disabled" : "Enabled" );
}

#endif

//=========================================================
//=========================================================
int CPlayTeam::GetNumWaves( void ) const
{
	return GetIMCTeamConfig( )->GetNumWaves( );
}

//=========================================================
//=========================================================
int CPlayTeam::GetType( void ) const
{
	return GetTeamLookup( )->GetType( );
}

//=========================================================
//=========================================================
int CPlayTeam::GetEmergencyWaves( void ) const
{
	return GetIMCTeamConfig( )->GetEmergencyWaves( );
}

//=========================================================
//=========================================================
bool CPlayTeam::HasReinforcementStarted( void ) const
{
	return ( m_flStartReinforcementTime != 0.0f );
}

//=========================================================
//=========================================================
void CPlayTeam::CapturedObjective( void )
{
	if( IsUnlimitedWaves( ) )
		return;

	int iAddWaves = GetIMCTeamConfig( )->GetObjWaves( );

	if( iAddWaves == 0 )
		return;

	// add it to the reinforcements left
	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		if( !IsValidSquad( i ) )
			continue;

		m_iReinforcementsLeft.Set( i, m_iReinforcementsLeft[ i ] + iAddWaves );
	}
}

//=========================================================
//=========================================================
void CPlayTeam::RoundReset( void )
{
	// update reinforcements
	ResetReinforcements( );

	// reset players
	DoReinforcement( PRESPAWNTYPE_RESTART );

	// update ranks
	UpdateRanks( );

	// reset captures
	m_iObjectiveCaptures = 0;

	// reset orders
	for( int i = 0; i < m_Squads.Count( ); i++ )
	{
		CINSSquad *pSquad = m_Squads[ i ];

		if( pSquad )
			pSquad->ResetOrders( );
	}

	// update network state etc
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CPlayTeam::RoundUnfrozen( void )
{
	for( int i = 0; i < m_Squads.Count( ); i++ )
	{
		CINSSquad *pSquad = m_Squads[ i ];

		if( !pSquad )
			continue;

		CINSPlayer *pCommander = pSquad->GetCommander( );

		if( pCommander )
		{
			UTIL_SendVoice( VOICEGRP_MOVEOUT, pCommander, NULL );
		}
		else
		{
			for( int i = 0; i < MAX_SQUAD_SLOTS; i++ )
			{
				CINSPlayer *pPlayer = pSquad->GetPlayer( i );

				if( pPlayer )
					UTIL_SendHint( pPlayer, HINT_NOCOMMANDER );
			}
		}
	}
}

//=========================================================
//=========================================================
void CPlayTeam::RegisterClanLeader( CINSPlayer *pPlayer )
{
	m_ClanLeader = pPlayer;
}

//=========================================================
//=========================================================
bool CPlayTeam::HasClanLeader( void ) const
{
	return ( GetClanLeader( ) != NULL );
}

//=========================================================
//=========================================================
CINSPlayer *CPlayTeam::GetClanLeader( void ) const
{
	return ToINSPlayer( m_ClanLeader );
}

//=========================================================
//=========================================================
void CPlayTeam::SetClanReady( bool bState )
{
	m_bClanReady = bState;
}

//=========================================================
//=========================================================
void CPlayTeam::ObjectiveCaptured( void )
{
	m_iObjectiveCaptures++;
}

//=========================================================
//=========================================================
const Color &CPlayTeam::GetColor( void ) const
{
	return GetIMCTeamConfig( )->GetColor( );
}

//=========================================================
//=========================================================
bool OnSameSquad( CINSPlayer *pP1, CINSPlayer *pP2 )
{
    if( !OnSameTeam( pP1, pP2 ) )
		return false;

	return ( pP1->GetSquadID( ) == pP2->GetSquadID( ) );
}

//=========================================================
//=========================================================
#ifdef TESTING

void CPlayTeam::PrintSquads( void )
{
	const CUtlVector< CINSSquad* > &Squads = GetSquads( );

	Msg( "------------------\n\n" );
	Msg( "%s Squad Layout\n", GetName( ) );
	Msg( "------------------\n\n" );

	for( int i = 0; i < Squads.Count( ); i++ )
		Squads[ i ]->PrintSquad( );
}

#endif

//=========================================================
//=========================================================
CPlayTeam *GetGlobalPlayTeam( int iTeamID )
{
	Assert( IsPlayTeam( iTeamID ) );
	return ( CPlayTeam* )GetGlobalTeam( iTeamID );
}
