//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

#include "ins_gamerules.h"
#include "ins_obj_shared.h"
#include "team.h"
#include "team_spawnpoint.h"
#include "ins_objmarker.h"
#include "imc_format.h"
#include "imc_config.h"
#include "play_team_shared.h"
#include "ins_player.h"
#include "ins_weaponcache.h"
#include "ins_squad_shared.h"
#include "team_lookup.h"
#include "ins_utils.h"
#include "ins_recipientfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define OBJ_PROGRESS_BITS 6

#define OBJ_PROGRESS_TIMEMOD_MIN 1.0f
#define OBJ_PROGRESS_TIMEMOD_MAX 1.5f

#define OBJ_TEXTURE "TOOLS/OBJECTIVE"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_objective, CINSObjective );

BEGIN_DATADESC( CINSObjective )

	DEFINE_KEYFIELD( m_iID, FIELD_INTEGER, "mapid" ),
	DEFINE_KEYFIELD( m_iChaseRadius, FIELD_INTEGER, "radius" ),
	DEFINE_KEYFIELD( m_bShowBrush, FIELD_BOOLEAN, "forceshow" ),

	DEFINE_INPUTFUNC( FIELD_INTEGER, "Capture", InputCapture ),
	DEFINE_OUTPUT( m_OnCapture, "OnCapture" ),

	DEFINE_FUNCTION( CaptureStepThink ),

END_DATADESC()

//=========================================================
//=========================================================
void* SendProxy_Objective( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->ClearAllRecipients( );

	CINSObjective *pObjective = ( CINSObjective* )pStruct;

	if( !pObjective || !pObjective->IsValid( ) )
		return NULL;

	pRecipients->SetAllRecipients( );
		
	return ( void* )pVarData;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_Objective );

void AddPlayerList( CSendProxyRecipients *pRecipients, const CUtlVector< CINSPlayer* > &Players )
{
	for( int i = 0; i < Players.Count( ); i++ )
	{
		CINSPlayer *pPlayer = Players[ i ];

		if( pPlayer )
			pRecipients->SetRecipient( pPlayer->GetClientIndex( ) );
	}
}

void *SendProxy_CaptureObjective( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pStruct;

	if( !pObjective )
		return NULL;

	pRecipients->ClearAllRecipients( );

	AddPlayerList( pRecipients, pObjective->GetCapturePlayers( ) );
	AddPlayerList( pRecipients, pObjective->GetOutsideCapturePlayers( ) );
		
	return ( void* )pVarData;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_CaptureObjective );

//=========================================================
//=========================================================
void SendProxy_ObjName( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;

	char *data = new char[MAX_OBJNAME_LENGTH];
	Q_strncpy(data, pObjective->IsCapturable() ? pObjective->GetName() : "", MAX_OBJNAME_LENGTH);

	pOut->m_pString = data;
}

void SendProxy_ObjPhonetic( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;
	pOut->m_Int = pObjective->IsCapturable( ) ? pObjective->GetPhonetic( ) : 0;
}

void SendProxy_ObjColor( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;
	pOut->m_Int = pObjective->IsCapturable( ) ? pObjective->GetColor( ).GetRawColor( ) : 0;
}

void SendProxy_PausedCapture( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;
	pOut->m_Int = pObjective->IsCapturable( ) ? pObjective->IsCapturePaused( ) : 0;
}

void SendProxy_CaptureProgress( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;

	int iValue = OBJ_CAPTUREINVALID;

	if( pObjective->IsCapturable( ) )
		iValue = ( pObjective->IsCapturePaused( ) ? pObjective->GetPausedCaptureProgress( ) : pObjective->GetCaptureProgress( ) ) + 1;

	pOut->m_Int = iValue;
}

void SendProxy_RequiredPlayers( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;
	pOut->m_Int = pObjective->GetRequiredPlayers( );
}

void SendProxy_PlayersCapturing( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	CINSObjective *pObjective = ( CINSObjective* )pData;
	pOut->m_Int = pObjective->CountCapturePlayers( );
}

BEGIN_SEND_TABLE_NOBASE( CINSObjective, DT_CapturingObj )

	SendPropInt( "capture_progress", 0, SIZEOF_IGNORE, OBJ_PROGRESS_BITS, SPROP_UNSIGNED, SendProxy_CaptureProgress ),

	SendPropInt( "players_required", 0, 1, 12, SPROP_UNSIGNED, SendProxy_RequiredPlayers ),
	SendPropInt( "players_capturing", 0, 1, 12, SPROP_UNSIGNED, SendProxy_PlayersCapturing ),

END_SEND_TABLE( )

IMPLEMENT_SERVERCLASS_ST( CINSObjective, DT_Objective )

	SendPropVector( SENDINFO( m_vecOrigin ), 0, SPROP_COORD ),
	SendPropInt( SENDINFO( m_iOrderID ), 5, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iChaseRadius ), 8, SPROP_UNSIGNED ),
	SendPropString( "name", 0, MAX_OBJNAME_LENGTH, 0, SendProxy_ObjName ),
	SendPropInt( "phonetic", 0, 1, 5, SPROP_UNSIGNED, SendProxy_ObjPhonetic ),
	SendPropInt( "color", 0, 1, 32, SPROP_UNSIGNED, SendProxy_ObjColor ),

	SendPropInt( SENDINFO( m_iCapturedTeam ), 2, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iCaptureTeam ), 2, SPROP_UNSIGNED ),

	SendPropBool( SENDINFO( m_bOrdersAllowed ) ),

	SendPropDataTable( "CaptureObj", 0, &REFERENCE_SEND_TABLE( DT_CapturingObj ), SendProxy_CaptureObjective ),

END_SEND_TABLE( )

//=========================================================
//=========================================================
void Cmd_ObjectivesToggle( void )
{
	for( int i = 0; i < INSRules( )->GetObjectiveCount( ); i++ )
	{
		CINSObjective *pObjective = INSRules( )->GetObjective( i );

		if( pObjective->IsEffectActive( EF_NODRAW ) )
			pObjective->RemoveEffects( EF_NODRAW );
		else
			pObjective->AddEffects( EF_NODRAW );
	}
}

static ConCommand objectivestoggle( "ins_objshow", Cmd_ObjectivesToggle, "Toggle the Visability of the Objectives", FCVAR_CHEAT );

//=========================================================
//=========================================================
void CSpawnGroupHandler::PrintGroup( CSpawnGroup *pGroup, int iTeamID, int iSquadID )
{
	char szBuffer[ 64 ];
	szBuffer[ 0 ] = '\0';
	
	if( iTeamID != INVALID_TEAM )
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( PlayTeamToTeam( iTeamID ) );
		Q_strncat( szBuffer, pTeam->GetName( ), sizeof( szBuffer ) );
	}

	if( iSquadID != INVALID_SQUAD )
	{
		char szSquadBuffer[ 64 ];
		Q_snprintf( szSquadBuffer, sizeof( szSquadBuffer ), " Squad %i", iSquadID );

		Q_strncat( szBuffer, szSquadBuffer, sizeof( szBuffer ) );
	}

	char szEndBuffer[ 64 ];
	Q_snprintf( szEndBuffer, sizeof( szEndBuffer ), " have %i Spawns\n", pGroup->Count( ) );

	Q_strncat( szBuffer, szEndBuffer, sizeof( szBuffer ) );

	Msg( szBuffer );
}

void CSpawnGroupHandler::DrawSpawnGroup( CSpawnGroup &SpawnGroup, const Color &ObjColor )
{
	INSRules( )->DrawSpawnDebug_SpawnGroup( ObjColor, &SpawnGroup );
}

bool CSpawnHandlerTeamSquads::IsValid( CINSObjective *pParent )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
	{
		for( int j = 0; j < MAX_SQUADS; j++ )
		{
			if( !m_SpawnPoints[ i ][ j ].IsValid( pParent, i, j ) )
				return false;
		}
	}

	return true;
}
CSpawnGroup *CSpawnHandlerTeamSquads::GetSpawnGroup( int iTeamID, int iSquadID )
{
	if( !IsPlayTeam( iTeamID ) || !CINSSquad::IsValidSquad( iSquadID ) )
		return NULL;

	return &m_SpawnPoints[ TeamToPlayTeam( iTeamID ) ][ iSquadID ];
}

void CSpawnHandlerTeamSquads::Print( void )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
	{
		for( int j = 0; j < MAX_SQUADS; j++ )
			PrintGroup( &m_SpawnPoints[ i ][ j ], i, j );
	}
}

void CSpawnHandlerTeamSquads::DrawSpawnDebug( const Color &ObjColor )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
	{
		for( int j = 0; j < MAX_SQUADS; j++ )
			DrawSpawnGroup( m_SpawnPoints[ i ][ j ], ObjColor );
	}
}

bool CSpawnHandlerTeam::IsValid( CINSObjective *pParent )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
	{
		if( !m_SpawnPoints[ i ].IsValid( pParent, i, INVALID_SQUAD ) )
			return false;
	}

	return true;
}

CSpawnGroup *CSpawnHandlerTeam::GetSpawnGroup( int iTeamID, int iSquadID )
{
	if( !IsPlayTeam( iTeamID ) )
		return NULL;

	return &m_SpawnPoints[ TeamToPlayTeam( iTeamID ) ];
}

void CSpawnHandlerTeam::Print( void )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
		PrintGroup( &m_SpawnPoints[ i ], i, INVALID_SQUAD );
}

void CSpawnHandlerTeam::DrawSpawnDebug( const Color &ObjColor )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
		DrawSpawnGroup( m_SpawnPoints[ i ], ObjColor );
}

CSpawnGroup *CSpawnHandlerMixedSquads::GetSpawnGroup( int iTeamID, int iSquadID )
{
	if( !CINSSquad::IsValidSquad( iSquadID ) )
		return NULL;

	return &m_SpawnPoints[ iSquadID ];
}

bool CSpawnHandlerMixedSquads::IsValid( CINSObjective *pParent )
{
	for( int i = 0; i < MAX_SQUADS; i++ )
	{
		if( !m_SpawnPoints[ i ].IsValid( pParent, INVALID_TEAM, i ) )
			return false;
	}

	return true;
}

void CSpawnHandlerMixedSquads::Print( void )
{
	for( int i = 0; i < MAX_SQUADS; i++ )
		PrintGroup( &m_SpawnPoints[ i ], INVALID_TEAM, i );
}

void CSpawnHandlerMixedSquads::DrawSpawnDebug( const Color &ObjColor )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
		DrawSpawnGroup( m_SpawnPoints[ i ], ObjColor );
}

bool CSpawnHandlerMixed::IsValid( CINSObjective *pParent )
{
	return m_SpawnPoints.IsValid( pParent, INVALID_TEAM, INVALID_SQUAD );
}

CSpawnGroup *CSpawnHandlerMixed::GetSpawnGroup( int iTeamID, int iSquadID )
{
	return &m_SpawnPoints;
}

void CSpawnHandlerMixed::Print( void )
{
	PrintGroup( &m_SpawnPoints, INVALID_TEAM, INVALID_SQUAD );
}

void CSpawnHandlerMixed::DrawSpawnDebug( const Color &ObjColor )
{
	DrawSpawnGroup( m_SpawnPoints, ObjColor );
}

//=========================================================
//=========================================================
CObjSpawns::CObjSpawns( )
{
	m_pHandler = NULL;
	m_pParent = NULL;
}

CObjSpawns::~CObjSpawns( )
{
	delete m_pHandler;
}

bool CObjSpawns::IsSetup( void ) const
{
	return ( m_pHandler != NULL );
}

void CObjSpawns::Init( CINSObjective *pParent )
{
	m_pParent = pParent;
}

void CObjSpawns::Setup( CSpawnPoint *pSpawnPoint )
{
	bool bSquadOrganise = ( pSpawnPoint->GetSquadID( ) != INVALID_SQUAD );

	if( IsPlayTeam( pSpawnPoint->GetTeamID( ) ) && !m_pParent->IsMixedSpawns( ) )
	{
		if( bSquadOrganise )
			m_pHandler = new CSpawnHandlerTeamSquads;
		else
			m_pHandler = new CSpawnHandlerTeam;
		
	}
	else
	{
		if( bSquadOrganise )
			m_pHandler = new CSpawnHandlerMixedSquads;
		else
			m_pHandler = new CSpawnHandlerMixed;
	}
}

bool CObjSpawns::IsValid( void ) const
{
	if( !m_pHandler )
	{
		if( m_pParent->CheckSpawnGroup( INVALID_TEAM, INVALID_SQUAD ) )
			return false;

		return true;
	}

	if( !m_pHandler->IsValid( m_pParent ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
#ifdef TESTING

CON_COMMAND( ins_objdumplog, "Dump Objective Log" )
{
	CINSObjective::DumpLog( );
}

#endif

#ifdef _DEBUG

ConVar ins_objdebug( "ins_objdebug", "1" );
#define OBJDEBUG ins_objdebug.GetBool( )

#endif

//=========================================================
//=========================================================
#define RADIUS_MAX 256
#define RADIUS_DEFAULT 60

//=========================================================
//=========================================================
CINSObjective::CINSObjective( )
{
	m_pParent = NULL;

	m_iOrderID = INVALID_OBJECTIVE;
	m_iChaseRadius = RADIUS_DEFAULT;

	m_iCapturedTeam = TEAM_NEUTRAL;

	m_iCaptureTeam = TEAM_NEUTRAL;

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		m_iRequiredPlayers[ i ] = 0;

	StopTimedCapture( );

	ResetPausedCapture( );

	m_bCutUp = false;

	m_bOrdersAllowed = true;

	m_pWeaponCacheRandomData = NULL;
	m_iCacheCount = 0;

	m_bShowBrush = false;
}

//=========================================================
//=========================================================
int CINSObjective::UpdateTransmitState( void )
{
	return SetTransmitState( IsCapturable( ) ? FL_EDICT_ALWAYS : FL_EDICT_DONTSEND ); 
}

//=========================================================
//=========================================================
void CINSObjective::Init( void )
{
	// init spawn stuff
	m_MainSpawns.Init( this );
	m_ReinforcementSpawns.Init( this );

	// ensure right material
	const model_t *pModel = GetModel( );
	int iMaterialCount = modelinfo->GetModelMaterialCount( pModel );

	if( iMaterialCount != 1 )
	{
		Warning( "objective %i is covered with different textures!\n", m_iID );
	}
	else
	{
		IMaterial *pMaterial = NULL;

		modelinfo->GetModelMaterials( pModel, 1, &pMaterial );

		if( Q_strcmp( pMaterial->GetName( ), OBJ_TEXTURE ) != 0 )
			Warning( "objective %i is not covered with the right texture!\n", m_iID );
	}

	// setup parent
	if( UTIL_ValidObjectiveID( m_iID ) )
	{
		m_pParent = IMCConfig( )->SetObjParent( m_iID );
	}
	else
	{
	#ifdef _DEBUG

		Msg( "objective %i doesn't have a parent\n", m_iID );

	#endif
	}

	// handle validity
	if( m_pParent )
	{
		Reset( );

		m_pWeaponCacheRandomData = &m_pParent->GetWeaponCacheRandomData( );
	}
	else
	{
		DisableTouch( );
	}

	// update network
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CINSObjective::Reset( void )
{
	Assert( HasParent( ) );

#ifdef _DEBUG

	if( OBJDEBUG )
		Msg( "%s: Reset\n", GetName( ) );

#endif

	// reset capture
	m_iCapturedTeam = GetInitialTeam( );

	// reset players
	m_CapturePlayers.Purge( );
	m_OutsideCapturePlayers.Purge( );
	m_EnemyPlayers.Purge( );

	m_iCaptureTeam = TEAM_NEUTRAL;

	// reset capturing
	StopTimedCapture( );

	ResetPausedCapture( );

	// reset everything else
	m_bCutUp = false;

	m_bOrdersAllowed = true;

	m_iCacheCount = 0;

	// update network
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CINSObjective::FinalSetup( void )
{
	if( !CheckSpawns( ) )
		return;

	if( !IsCapturable( ) )
		return;

	EnableTouch( );

	DispatchUpdateTransmitState( );
}

//=========================================================
// * CAPTURE MANAGEMENT
//=========================================================

//=========================================================
//=========================================================
bool CINSObjective::IsCapturable( void ) const
{
	return ( IsValid( ) && !IsHidden( ) && INSRules( )->IsUsingObjectives( ) );
}

//=========================================================
//=========================================================
bool CINSObjective::CaptureAllowed( int iTeamID ) const
{
	if( iTeamID == TEAM_NEUTRAL )
		return false;

	return ( IsCapturable( ) && m_iCapturedTeam != iTeamID && INSRules( )->ObjectiveCaptureAllowed( this, iTeamID ) );
}

//=========================================================
//=========================================================
bool CINSObjective::IsCaptured( void ) const
{
	return ( m_iCapturedTeam != TEAM_NEUTRAL );
}

//=========================================================
//=========================================================
void CINSObjective::Captured( int iTeamID, bool bAwardPoints )
{
	if( !CaptureAllowed( iTeamID ) )
	{
		Assert( false );
		return;
	}

#ifdef _DEBUG

	if( OBJDEBUG )
		Msg( "%s: Captured\n", GetName( ) );

#endif

	// update status
	m_iCapturedTeam = iTeamID;

	// stop capturing
	StopTimedCapture( );

	// remove all outside capture players
	for( int i = 0; i < m_OutsideCapturePlayers.Count( ); i++ )
	{
		CINSPlayer *pPlayer = m_OutsideCapturePlayers[ i ];

		if( pPlayer )
			pPlayer->ResetCurrentObj( );
	}

	m_OutsideCapturePlayers.Purge( );

	// inform stakeholders
	m_OnCapture.Set( iTeamID, NULL, this );

	INSRules( )->ObjectiveCaptured( this );

	// handle the capture players
	if( bAwardPoints )
	{
		CINSSquad *m_pSquadsPresent[ MAX_SQUADS ];
		memset( m_pSquadsPresent, NULL, sizeof( m_pSquadsPresent ) );

		// give them all objpts
		for( int i = 0; i < m_CapturePlayers.Count( ); i++ )
		{
			CINSPlayer *pPlayer = m_CapturePlayers[ i ];

			if( !pPlayer )
				continue;

			// award pts
			PlayerAwardPoints( pPlayer, true );

			// see what squads are there
			int iSquadID = pPlayer->GetSquadID( );

			if( m_pSquadsPresent[ iSquadID ] )
				continue;

			CINSSquad *pSquad = pPlayer->GetSquad( );

			if( pSquad && pSquad->IsFollowingAttackOrders( this ) )
				m_pSquadsPresent[ iSquadID ] = pSquad;

			// tell them that have got points
			UTIL_SendHint( pPlayer, HINT_CAPTURE_CAPTURED );
		}

		// give leaders points
		/*for( int i = 0; i < MAX_SQUADS; i++ )
		{
			CINSSquad *pSquad = m_pSquadsPresent[ i ];

			if( !pSquad )
				continue;

			CINSPlayer *pCommander = pSquad->GetCommander( );

			if( pCommander )
				pCommander->IncrementStat( PLAYERSTATS_LEADERPTS, INSRules( )->ObjectiveLeaderPoints( ) );
		}*/
	}

	// send to everybody else
	CReliablePlayTeamRecipientFilter filter;
	SendObjectiveUpdate( filter, OBJ_CAPTURED );

	// update network
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CINSObjective::InputCapture( inputdata_t &inputdata )
{
	CBaseEntity *pActivator = inputdata.pActivator;
	CINSPlayer *pPlayer = ToINSPlayer( pActivator );

	int iTeamID = inputdata.value.Int( );

	// work out correct team
	if( !IsPlayTeam( iTeamID ) )
	{
		if( !pPlayer || !pPlayer->OnPlayTeam( ) )
			return;

		iTeamID = pPlayer->GetTeamID( );
	}
	
	// capture it
	Captured( iTeamID, false );

	// award points
	if( pPlayer && iTeamID == pPlayer->GetTeamID( ) )
	{
		int iPts=INSRules( )->ObjectiveCapturePoints( true );
		pPlayer->IncrementStat( PLAYERSTATS_GAMEPTS, iPts );
		pPlayer->SendStatNotice(iPts,"Capture");
	}
}

//=========================================================
// * TIMED CAPTURE MANAGEMENT
//=========================================================

//=========================================================
// + PLAYER MANAGEMENT
//=========================================================

//=========================================================
//=========================================================
void CINSObjective::TouchHook( CBaseEntity *pOther, INSTouchCall_t TouchCall )
{
	// should always be capturable here
	if( pOther->IsPlayer( ) && !IsCapturable( ) )
	{
		Assert( false );
		return;
	}

	BaseClass::TouchHook( pOther, TouchCall );
}

//=========================================================
//=========================================================
void CINSObjective::PlayerStartTouch( CINSPlayer *pPlayer )
{
#ifdef _DEBUG

	if( OBJDEBUG )
		Msg(" %s: %s StartTouch\n", GetName( ), pPlayer->GetPlayerName( ) );

#endif

	// enter the current area
	pPlayer->EnterCurrentArea( this );

	// ensure that they can capture
	if( !CanPlayerCapture( pPlayer ) )
		return;

	// update network
	NetworkStateChanged( );

	// handle players capturing outside
	if( pPlayer->IsOutsideCapturing( ) )
	{
		// get the current obj
		CINSObjective *pOldObjective = pPlayer->GetCurrentObjective( );
		Assert( pOldObjective );

		// force leaving
		if( pOldObjective )
		{
			// if the old objective is us, remove from outside list, otherwise
			// force a leave from the other objective
			if( pOldObjective == this )
				PlayerRemove( pPlayer, m_OutsideCapturePlayers );
			else
				pOldObjective->PlayerLeft( pPlayer, true );
		}

		// reset time
		pPlayer->ResetObjExitTime( );
	}

	// make sure its the current obj here
	pPlayer->SetCurrentObjective( this );

	// when the objective has capture players and the player
	// isn't on the same team as them, add them to enemy players
	if( m_iCaptureTeam != pPlayer->GetTeamID( ) && HasCapturePlayers( ) )
	{
		UTIL_SendHint( pPlayer, HINT_CAPTURE_BLOCK );

		PlayerAdd( pPlayer, m_EnemyPlayers );
		return;
	}
	
	// add to capturing players
	PlayerAdd( pPlayer, m_CapturePlayers );

	// if this is the first capture player set
	// the new capture team
	if( m_CapturePlayers.Count( ) != 0 )
		m_iCaptureTeam = pPlayer->GetTeamID( );

	// try and capture
	AttemptTimedCapture( );

	// send out hint when team the same
	if( pPlayer->SendHints( ) )
	{
		int iHintID = HINT_CAPTURE_CANNOT;

		if( IsCapturing( ) )
			iHintID = HINT_CAPTURE_CAPTURING;
		else if( m_iCapturedTeam == pPlayer->GetTeamID( ) )
			iHintID = HINT_CAPTURE_DEFENDING;
		else if( !EnoughPlayersForCapture( ) )
			iHintID = HINT_CAPTURE_NEP;

		UTIL_SendHint( pPlayer, iHintID );
	}
}

//=========================================================
//=========================================================
void CINSObjective::PlayerEndTouch( CINSPlayer *pPlayer )
{
#ifdef _DEBUG

	if( OBJDEBUG )
		Msg( "%s: %s EndTouch\n", GetName( ), pPlayer->GetPlayerName( ) );

#endif

	PlayerLeft( pPlayer, false );
}

//=========================================================
//=========================================================
void CINSObjective::PlayerLeft( CINSPlayer *pPlayer, bool bForceExit )
{
	// left the current area
	pPlayer->LeaveCurrentArea( );

	// handle the exit
	bool bExit = true;

	if( pPlayer->GetTeamID( ) != m_iCaptureTeam )
	{
		PlayerRemove( pPlayer, m_EnemyPlayers );
	}
	else
	{
		if( pPlayer->IsOutsideCapturing( ) )
		{
			PlayerRemove( pPlayer, m_OutsideCapturePlayers );
		}
		else
		{
			PlayerRemove( pPlayer, m_CapturePlayers );

			if( !bForceExit && IsCapturing( ) && CanPlayerCapture( pPlayer ) && CaptureAllowed( m_iCaptureTeam ) )
			{
				// add to outside capture players
				PlayerAdd( pPlayer, m_OutsideCapturePlayers );

				// tell the player we are 
				pPlayer->LeftCurrentObj( );

				// pause the capture when there aren't enough to capture
				if( !EnoughPlayersForCapture( ) )
				{
					StartPausedCapture( m_iCaptureProgress );
					m_iCaptureProgress = OBJ_CAPTUREINVALID;
				}

				// don't exit
				bExit = false;
			}
		}
	}

	// handle if a full exit
	if( bExit )
	{
		// reset player's current obj
		pPlayer->ResetCurrentObj( );

		// handle any new captures
		PlayerRemoved( );

		// do a capture check
		CheckTimedCapture( );

		// check capture speed
		UpdateCaptureSpeed( );
	}

	// update network
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CINSObjective::PlayerExit( CINSPlayer *pPlayer )
{
	PlayerLeft( pPlayer, true );
}

//=========================================================
//=========================================================
void CINSObjective::PlayerRemoved( void )
{
	if( !IsPlayTeam( m_iCaptureTeam ) )
		return;

	// reset capture team when necessary
	if( HasCapturePlayers( ) )
		return;

	// stop the paused capture
	StopPausedCapture( );

	if( IsEnemyPlayersEmpty( ) )
	{
		// reset capture team
		m_iCaptureTeam = TEAM_NEUTRAL;
	}
	else
	{
		// copy the enemy players to the capturing players
		// and clear the enemy players
		m_CapturePlayers.AddVectorToTail( m_EnemyPlayers );
		m_EnemyPlayers.Purge( );

		// set new capture team
		if( m_iCaptureTeam != TEAM_NEUTRAL )
		{
			m_iCaptureTeam = FlipPlayTeam( m_iCaptureTeam );

			// attempt capture
			AttemptTimedCapture( );
		}
	}
}

//=========================================================
//=========================================================
void CINSObjective::PlayerAdd( CINSPlayer *pPlayer, CUtlVector<CINSPlayer*> &PlayerList )
{
	PlayerList.AddToTail( pPlayer );
}

//=========================================================
//=========================================================
void CINSObjective::PlayerRemove( CINSPlayer *pPlayer, CUtlVector<CINSPlayer*> &PlayerList )
{
	PlayerList.FindAndRemove( pPlayer );
}

//=========================================================
//=========================================================
bool CINSObjective::CanPlayerCapture( CINSPlayer *pPlayer )
{
	return ( !pPlayer->IsObserver( ) && pPlayer->GetHealth( ) > 0 && !pPlayer->IsDead( ) );
}

//=========================================================
//=========================================================
bool CINSObjective::IsPlayersEmpty( const CUtlVector<CINSPlayer*> &PlayerList ) const
{
	return ( PlayerList.Count( ) == 0 );
}

//=========================================================
//=========================================================
void CINSObjective::PlayerAwardPoints( CINSPlayer *pPlayer, bool bFullCapture )
{
	CINSSquad *pSquad = pPlayer->GetSquad( );
	Assert( pSquad );

	if( pSquad )
	{
		int iPts=INSRules( )->ObjectiveCapturePoints( !bFullCapture && pSquad->IsFollowingAttackOrders( this ) );
		pPlayer->IncrementStat( PLAYERSTATS_GAMEPTS, iPts );
		pPlayer->SendStatNotice(iPts,"Capture");
	}
}

//=========================================================
//=========================================================
void CINSObjective::UpdateAllRequiredPlayers( void )
{
	for( int i = 0; i < INSRules( )->GetObjectiveCount( ); i++ )
	{
		CINSObjective *pObjective = INSRules( )->GetObjective( i );

		if( pObjective->IsCapturable( ) )
			pObjective->UpdateRequiredPlayers( );
	}
}

//=========================================================
//=========================================================
void CINSObjective::UpdateRequiredPlayers(void)
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		int iReqPercent = GetReqPercent( );
		Assert( iReqPercent != 0 && iReqPercent <= 100 && iReqPercent >= 0 );

		CTeam *pCapturingTeam = GetGlobalTeam( i );
		Assert( pCapturingTeam );

		int iRequiredValue, iNumPlayers;

		if( ( iNumPlayers = pCapturingTeam->GetNumPlayers( ) ) != 0 )
		{
			iRequiredValue = RoundFloatToInt( ( float ) iNumPlayers * ( float )( iReqPercent / 100.0f ) );

			if( iRequiredValue < 1 )
				iRequiredValue = 1;
		}
		else
		{
			iRequiredValue = 0;
		}

		m_iRequiredPlayers[ TeamToPlayTeam( i ) ] = iRequiredValue;
	}

	if( !IsCapturing( ) && IsPlayTeam( GetCaptureTeam( ) ) )
		AttemptTimedCapture( );

	NetworkStateChanged( );
}

//=========================================================
//=========================================================
bool CINSObjective::EnoughPlayersForCapture(void)
{
	return ( m_CapturePlayers.Count( ) >= GetRequiredPlayers( ) );
}

//=========================================================
//=========================================================
int CINSObjective::GetRequiredPlayers( int iTeamID )
{
	if( !IsPlayTeam( iTeamID ) )
		return 0;

	return m_iRequiredPlayers[ TeamToPlayTeam( iTeamID ) ];
}

//=========================================================
//=========================================================
int CINSObjective::GetRequiredPlayers( void )
{
	return GetRequiredPlayers( m_iCaptureTeam );
}

//=========================================================
//=========================================================
int CINSObjective::CountCapturePlayers( void )
{
	return m_CapturePlayers.Count( );
}

//=========================================================
// * ... CAPTURING
//=========================================================

//=========================================================
//=========================================================
void CINSObjective::StartTimedCapture( void )
{
#ifdef _DEBUG

	if( OBJDEBUG )
		Msg( "%s: CaptureStartTimed\n", GetName( ) );

#endif

	// send to the capturing players team, minus the capturing people apart from when paused
	if( !IsCapturePaused( ) )
	{
		CReliablePlayTeamRecipientFilter filter;
		SendObjectiveUpdate( filter, OBJ_CAPTURE_START );
	}

	// init capture progress
	m_iCaptureProgress = IsCapturePaused( ) ? GetPausedCaptureProgress( ) : 0;

	StopPausedCapture( );

	// update the capture speed
	UpdateCaptureSpeed( );

	// set the next think
	SetThink( &CINSObjective::CaptureStepThink );
	SetNextStepThink( );

	// update network
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
void CINSObjective::SetNextStepThink( void )
{
	SetNextThink( gpGlobals->curtime + m_flCaptureProgressStep );
}

//=========================================================
//=========================================================
bool CINSObjective::IsTimedCaptureAllowed( void )
{
	// only allow a timed capture when it allow it and
	// there are enough players
	return ( CaptureAllowed( m_iCaptureTeam ) && EnoughPlayersForCapture( ) );
}

//=========================================================
//=========================================================
void CINSObjective::StopTimedCapture( void )
{
	// reset timer
	m_iCaptureProgress = OBJ_CAPTUREINVALID;

	// update capture speed
	UpdateCaptureSpeed( );
}

//=========================================================
//=========================================================
void CINSObjective::AttemptTimedCapture( void )
{
	// don't allow a capture to start when we are already capturing
	// or it is not allowed
	if( IsCapturing( ) || !IsTimedCaptureAllowed( ) )
		return;

	StartTimedCapture( );
}

//=========================================================
//=========================================================
void CINSObjective::CaptureStepThink( void )
{
	// ensure we can continue capturing
	if( !CheckTimedCapture( ) )
	{
		SetThink( NULL );
		return;
	}

	m_iCaptureProgress++;

#ifdef _DEBUG

	if( OBJDEBUG )
		Msg( "%s: %i Capturing\n", GetName( ), m_iCaptureProgress );

#endif

	// check if we can capture
	if( m_iCaptureProgress >= OBJ_PROGRESS_MAX )
	{
		Captured( m_iCaptureTeam, true );

		SetThink( NULL );

		return;
	}

	// set next think
	SetNextStepThink( );

	// update network
	NetworkStateChanged( );
}

//=========================================================
//=========================================================
bool CINSObjective::CheckTimedCapture( void )
{
	if( !IsTimedCaptureAllowed( ) )
	{
		StopTimedCapture( );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CINSObjective::UpdateCaptureSpeed( void )
{
	m_flCaptureProgressStep = 0.0f;

	if( !IsCapturing( ) )
		return;

	CPlayTeam *pTeam = GetGlobalPlayTeam( m_iCaptureTeam );

	if( !pTeam )
	{
		AssertMsg( false, "CINSObjective::UpdateCaptureSpeed, Capture Team Failed" );
		return;
	}

	int iDoubledRequiredPlayers = GetRequiredPlayers( ) * 2.0f;

	if( iDoubledRequiredPlayers == 0 )
	{
		AssertMsg( false, "CINSObjective::UpdateCaptureSpeed, Invalid Number of Required Players" );
		return;
	}

	int iPlayersCapturing = m_CapturePlayers.Count( );

	if( iPlayersCapturing == 0 )
	{
		AssertMsg( false, "CINSObjective::UpdateCaptureSpeed, Invalid Number of Capture Players" );
		return;
	}

	float flTimeMod;
	static float flTimeModDelta = OBJ_PROGRESS_TIMEMOD_MAX - OBJ_PROGRESS_TIMEMOD_MIN;
	
	if( iPlayersCapturing <= 1 )
		flTimeMod = OBJ_PROGRESS_TIMEMOD_MIN;
	else
		flTimeMod = 1.0f - ( ( ( min( ( iPlayersCapturing / ( float )iDoubledRequiredPlayers ), 1.0f ) ) + ( iPlayersCapturing / ( float )pTeam->GetNumPlayers( ) ) ) * 0.5f );

	flTimeMod = OBJ_PROGRESS_TIMEMOD_MIN + ( flTimeModDelta * flTimeMod );

	m_flCaptureProgressStep = ( m_pParent->GetCapTime( ) * flTimeMod ) / OBJ_PROGRESS_MAX;
}

//=========================================================
//=========================================================
bool CINSObjective::IsCapturing( void ) const
{
	return ( m_iCaptureProgress != OBJ_CAPTUREINVALID );
}

//=========================================================
//=========================================================
bool CINSObjective::IsCapturePaused( void ) const
{
	const PausedCaptureData_t *pPausedData = GetPausedCapture( );
	return ( pPausedData && pPausedData->m_bPaused );
}

//=========================================================
//=========================================================
int CINSObjective::GetPausedCaptureProgress( void ) const
{
	const PausedCaptureData_t *pPausedData = GetPausedCapture( );
	return ( pPausedData ) ? pPausedData->m_iProgress : 0;
}

//=========================================================
//=========================================================
void CINSObjective::ResetPausedCapture( void )
{
	for( int i = 0; i < MAX_PLAY_TEAMS; i++ )
	{
		m_CapturePaused[ i ].m_bPaused = false;
		m_CapturePaused[ i ].m_iProgress = 0;
	}
}

//=========================================================
//=========================================================
void CINSObjective::StartPausedCapture( int iValue )
{
	PausedCaptureData_t *pPausedData = GetPausedCapture( );
	Assert( pPausedData );

	pPausedData->m_bPaused = true;
	pPausedData->m_iProgress = iValue;
}

//=========================================================
//=========================================================
void CINSObjective::StopPausedCapture( void )
{
	PausedCaptureData_t *pPausedData = GetPausedCapture( );
	Assert( pPausedData );

	pPausedData->m_bPaused = false;

	if( !IsKOTH( ) )
		pPausedData->m_iProgress = 0;
}

//=========================================================
//=========================================================
CINSObjective::PausedCaptureData_t *CINSObjective::GetPausedCapture( void )
{
	if( !IsPlayTeam( m_iCaptureTeam ) )
		return NULL;

	return &m_CapturePaused[ TeamToPlayTeam( m_iCaptureTeam ) ];
}

//=========================================================
//=========================================================
const CINSObjective::PausedCaptureData_t *CINSObjective::GetPausedCapture( void ) const
{
	if( !IsPlayTeam( m_iCaptureTeam ) )
		return NULL;

	return &m_CapturePaused[ TeamToPlayTeam( m_iCaptureTeam ) ];
}

//=========================================================
// * SPAWN MANAGEMENT
//=========================================================

//=========================================================
//=========================================================
bool CINSObjective::FindSpawnPoints( void )
{
	// find the spawnpoints
	for( int i = 0; i < CSpawnPoint::CountSpawns( ); i++ )
	{
		CSpawnPoint *pSpawnPoint = CSpawnPoint::GetSpawn( i );

		if( !pSpawnPoint )
			continue;

		// find the parent and ensure its a valid spawn group
		CINSObjective *pParent = pSpawnPoint->GetParent( );

		if( !pParent )
			continue;

		// ensure the spawn points are the same
		if( pSpawnPoint->GetSpawnGroup( ) != pParent->GetSpawnGroup( pSpawnPoint->IsReinforcement( ) ) )
			continue;

		// add the spawn
		pParent->AddSpawn( pSpawnPoint, i );
	}

	return true;
}

//=========================================================
//=========================================================
void CINSObjective::AddSpawn( CSpawnPoint *pSpawnPoint, int iID )
{
	Assert( pSpawnPoint );

	CObjSpawns &ObjSpawn = GetObjSpawn( pSpawnPoint->IsReinforcement( ) );

	// there's no "control" entity that tells the code
	// what type of spawns the group has, so it has to work
	// it out from the first spawn found
	if( !ObjSpawn.IsSetup( ) )
		ObjSpawn.Setup( pSpawnPoint );

	// find the handler
	CSpawnGroupHandler *pHandler = ObjSpawn.GetSpawnHandler( );
	Assert( pHandler );

	if( !pHandler )
		return;

	// find the group
	CSpawnGroup *pGroup = pHandler->GetSpawnGroup( pSpawnPoint->GetTeamID( ), pSpawnPoint->GetSquadID( ) );
	Assert( pGroup );

	if( !pGroup )
		return;

	// then add the spawn to the group
	pGroup->AddSpawn( iID );
}

//=========================================================
//=========================================================
CSpawnGroup *CINSObjective::GetSpawnGroupPlayer( CINSPlayer *pPlayer )
{
	Assert( m_iCapturedTeam == pPlayer->GetTeamID( ) );
	return GetSpawnGroup( UseReinforcementSpawns( ), m_iCapturedTeam, pPlayer->GetSquadID( ) );
}

//=========================================================
//=========================================================
CSpawnGroup *CINSObjective::GetSpawnGroup( bool bReinforcementWave, int iTeamID, int iSquadID )
{
	CObjSpawns &ObjSpawn = GetObjSpawn( bReinforcementWave );

	CSpawnGroupHandler *pSpawnHandler = ObjSpawn.GetSpawnHandler( );

	if( !pSpawnHandler )
	{
		Assert( false );
		return NULL;
	}

	return pSpawnHandler->GetSpawnGroup( iTeamID, iSquadID );
}

//=========================================================
//=========================================================
CObjSpawns &CINSObjective::GetObjSpawn( bool bReinforcementWave )
{
	return ( bReinforcementWave ? m_ReinforcementSpawns : m_MainSpawns );
}

//=========================================================
//=========================================================
bool CINSObjective::UseReinforcementSpawns( void ) const
{
	if( !IsCaptured( ) )
	{
		Assert( false );
		return false;
	}

	if( !UsingReinforcementSpawns( ) )
		return false;

	if( IMCConfig( )->IsCapturingFallback( ) && IsCapturing( ) )
		return true;

	CPlayTeam *pTeam = GetGlobalPlayTeam( m_iCapturedTeam );
	return ( pTeam && pTeam->HadReinforcement( ) );
}

//=========================================================
//=========================================================
bool CINSObjective::CheckSpawns( void )
{
	if( !m_MainSpawns.IsValid( ) )
	{
		Warning( "objective %i has invalid main spawns!\n", m_iID );
		return false;
	}

	if( UsingReinforcementSpawns( ) && !m_ReinforcementSpawns.IsValid( ) )
	{
		Warning( "objective %i has invalid reinforcement spawns!\n", m_iID );
		return false;
	}

	return true;
}

//=========================================================
//=========================================================
bool CINSObjective::CheckSpawns( CObjSpawns &ObjSpawns )
{
	return ObjSpawns.IsValid( );
}

//=========================================================
//=========================================================
bool CINSObjective::CheckSpawnGroup( int iTeamID, int iSquadID )
{
	int iFixedTeamID = INVALID_TEAM;
	
	if( iTeamID != INVALID_TEAM )
		iFixedTeamID = PlayTeamToTeam( iTeamID );

	if( IsHidden( ) )
	{
		if( iFixedTeamID != INVALID_TEAM && m_iCapturedTeam != iFixedTeamID )
			return false;
	}

	if( !IMCConfig( )->IsMovingSpawns( ) )
	{
		int iT1Spawn, iT2Spawn;
		INSRules( )->GetStartSpawns( iT1Spawn, iT2Spawn );

		if( m_iID != iT1Spawn && m_iID != iT2Spawn )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
void CINSObjective::PrintSpawnStatus( void )
{
	Msg( "----------------------------------\n" );
	Msg( "Objective: %s\n", GetName( ) );
	Msg( "----------------------------------\n" );

	PrintSpawnStatus( "Main", m_MainSpawns );

	if( UsingReinforcementSpawns( ) )
	{
		Msg( "----------------------------------\n" );

		PrintSpawnStatus( "Reinforcement", m_ReinforcementSpawns );
	}

	Msg( "----------------------------------\n\n" );
}

//=========================================================
//=========================================================
void CINSObjective::PrintSpawnStatus( const char *pszName, CObjSpawns &ObjSpawns )
{
	Msg( "Spawns: %s\n", pszName );

	CSpawnGroupHandler *pSpawnGroupHandler = ObjSpawns.GetSpawnHandler( );

	if( !pSpawnGroupHandler )
	{
		Msg( "None\n" );
		return;
	}

	pSpawnGroupHandler->Print( );
}

//=========================================================
// * WEAPONCACHE MANAGEMENT
//=========================================================

//=========================================================
//=========================================================
void CINSObjective::AddWeaponCache(CINSWeaponCache *pWeaponCache)
{
	pWeaponCache->SetObjParent( this );

	m_WeaponCaches.AddToTail(pWeaponCache);
}

//=========================================================
//=========================================================
void CINSObjective::CreateWeaponCaches(void)
{
	CreateWeaponCaches(this, m_WeaponCaches, m_pWeaponCacheRandomData);
}

//=========================================================
//=========================================================
void CINSObjective::CreateWeaponCaches( CINSObjective *pObjective, CUtlVector< CINSWeaponCache* > &WeaponCaches, const CWeaponCacheRandomData *pWeaponCacheRandomData )
{
	// NOTE: this function is used by the gamerules to create the 
	// global weaponcaches

	if( !pWeaponCacheRandomData )
		return;

	int iWeaponCacheCount = WeaponCaches.Count( );

	if( iWeaponCacheCount == 0 )
		return;

	// remove all the old ones
	for( int i = 0; i < iWeaponCacheCount; i++ )
	{
		CINSWeaponCache *pWeaponCache = WeaponCaches[ i ];

		if( pWeaponCache )
			pWeaponCache->Remove( );
	}

	// create some new ones
	int iQuota = RoundFloatToInt( iWeaponCacheCount * ( pWeaponCacheRandomData->UseRandom( ) ? random->RandomFloat( pWeaponCacheRandomData->GetMin( ), pWeaponCacheRandomData->GetMax( ) ) : 1.0f ) );

	if( pObjective )
		pObjective->SetCacheCount( iQuota );

	int iWeaponCacheID = random->RandomInt( 0, iWeaponCacheCount - 1 );

	while( iQuota )
	{
		Assert( WeaponCaches.IsValidIndex( iWeaponCacheID ) );

		CINSWeaponCache *pWeaponCache = WeaponCaches[ iWeaponCacheID ];

		if( pWeaponCache )
			pWeaponCache->Create( );

		iQuota--;

		if( iWeaponCacheID >= ( iWeaponCacheCount - 1 ) )
		{
			iWeaponCacheID = 0;
			continue;
		}

		iWeaponCacheID++;
	}
}

//=========================================================
//=========================================================
void CINSObjective::CacheDestroyed( CINSWeaponCache *pWeaponCache, CINSPlayer *pInflictor )
{
	m_iCacheCount--;

	bool bAllowCapture = ( m_iCapturedTeam != TEAM_NEUTRAL && m_iCapturedTeam != pInflictor->GetTeamID( ) );

	// give the player a little points when working towards a capture
	if( bAllowCapture )
		PlayerAwardPoints( pInflictor, false );

	// only capture when there are no more left
	if( m_iCacheCount > 0 )
		return;

	// capture it
	Captured( pInflictor->GetTeamID( ) , false );

	// award points
	PlayerAwardPoints( pInflictor, true );
}

//=========================================================
// * GENERAL
//=========================================================

//=========================================================
//=========================================================
bool CINSObjective::IsValid( void ) const
{
	return ( HasParent( ) && GetOrderID( ) != INVALID_OBJECTIVE );
}

//=========================================================
//=========================================================
void CINSObjective::SetOrderID( int iOrderID )
{
	m_iOrderID = iOrderID;
}

//=========================================================
//=========================================================
void CINSObjective::SetupObjectiveMarkers( void )
{
	CINSRules *pRules = INSRules( );

	if( !pRules )
		return;

	CBaseEntity *pObjFind = NULL;

	// find all markers and assign
	while( ( pObjFind = gEntList.FindEntityByClassname( pObjFind, "ins_objmarker" ) ) != NULL )
	{
		CINSObjMarker *pObjMarker = ( CINSObjMarker* )pObjFind;
		Assert( pObjMarker );

		CINSObjective *pObjective = pRules->GetUnorderedObjective( pObjMarker->GetParentObj( ) );

		if( pObjective )
			pObjective->SetMarker( pObjMarker->GetAbsOrigin( ) );

		// no point it hanging around
		UTIL_Remove( pObjMarker );
	}

	// objectives that don't have a marker, use their centre
	for( int i = 0; i < pRules->GetObjectiveCount( ); i++ )
	{
		CINSObjective *pObjective = pRules->GetObjective( i );
		Assert( pObjective );

		if( pObjective && !pObjective->HasMarker( ) )
			pObjective->SetMarker( pObjective->WorldSpaceCenter( ) );
	}
}

//=========================================================
//=========================================================
void CINSObjective::SetMarker( const Vector &MarkerOrigin )
{
	m_vecOrigin = MarkerOrigin;
}

//=========================================================
//=========================================================
bool CINSObjective::HasMarker(void) const
{
	return ( m_vecOrigin != vec3_origin );
}

//=========================================================
//=========================================================
void CINSObjective::SetCutUp( bool bState )
{
	Assert( IsCapturable( ) );
	m_bCutUp = bState;
}

//=========================================================
//=========================================================
void CINSObjective::SetOrdersAllowed( bool bState )
{
	m_bOrdersAllowed = bState;
}

//=========================================================
//=========================================================
void CINSObjective::SendObjectiveUpdate( IRecipientFilter &filter, int iMsgType )
{
	UserMessageBegin( filter, "ObjMsg" );

		WRITE_BYTE( GetOrderID( ) );			// what objective this has happened on
		WRITE_BYTE( iMsgType );					// whats happened

		if( iMsgType == OBJ_CAPTURE_START )
			WRITE_BYTE( m_iCaptureTeam );

		if( iMsgType == OBJ_CAPTURED )
			WRITE_BYTE( m_iCapturedTeam );

	MessageEnd( );
}

//=========================================================
//=========================================================
const char *CINSObjective::GetTitle( void ) const
{
	return GetName( );
}

//=========================================================
// * IMC ACCESS
//=========================================================

//=========================================================
//=========================================================
CIMCObjective *CINSObjective::GetParent( ) const
{
	Assert( HasParent( ) );
	return m_pParent;
}

//=========================================================
//=========================================================
bool CINSObjective::HasParent( void ) const
{
	return ( m_pParent != NULL );
}

//=========================================================
//=========================================================
int CINSObjective::GetSpawnGroup( bool bReinforcement ) const
{
	return ( bReinforcement ? GetParent( )->GetSpawnGroup( ) : GetParent( )->GetReinforcementSpawnGroup( ) );
}

//=========================================================
//=========================================================
const char *CINSObjective::GetName( void ) const
{
	return GetParent( )->GetName( );
}

//=========================================================
//=========================================================
int CINSObjective::GetPhonetic( void ) const
{
	return GetParent( )->GetPhonetic( );
}

//=========================================================
//=========================================================
const Color &CINSObjective::GetColor( void ) const
{
	return GetParent( )->GetColor( );
}

//=========================================================
//=========================================================
int CINSObjective::GetInitialTeam(void) const
{
	return GetParent( )->GetInitialTeam( );
}

//=========================================================
//=========================================================
bool CINSObjective::UsingReinforcementSpawns( void ) const
{
	return GetParent( )->HasReinforcementSpawns( );
}

//=========================================================
//=========================================================
bool CINSObjective::IsMixedSpawns( void ) const
{
	return GetParent( )->IsMixedSpawns( );
}

//=========================================================
//=========================================================
bool CINSObjective::IsKOTH( void ) const
{
	return GetParent( )->IsKOTH( );
}

//=========================================================
//=========================================================
bool CINSObjective::IsStartSpawn(void) const
{
	return GetParent( )->IsStartingSpawn( );
}

//=========================================================
//=========================================================
bool CINSObjective::IsHidden( void ) const
{
	return GetParent( )->IsHidden( );
}

//=========================================================
//=========================================================
int CINSObjective::GetReqPercent(void) const
{
	return GetParent( )->GetReqPercent( );
}

//=========================================================
//=========================================================
float CINSObjective::GetCapTime(void) const
{
	return GetParent( )->GetCapTime( );
}

//=========================================================
//=========================================================
int CINSObjective::GetInvincibilityTime( void ) const
{
	return GetParent( )->GetInvincibilityTime( );
}

//=========================================================
// * MISC
//=========================================================

//=========================================================
//=========================================================
bool CINSObjective::HideBrush( void ) const
{
#ifdef _DEBUG

	return false;

#else

	return !m_bShowBrush;

#endif
}

//=========================================================
//=========================================================
CObjCapturingRecipientFilter::CObjCapturingRecipientFilter( CUtlVector<CINSPlayer*> &CapturingPlayers )
{
	MakeReliable( );

	for( int i = 0; i < CapturingPlayers.Count( ); i++ )
		AddRecipient( CapturingPlayers[ i ] );
}

//=========================================================
//=========================================================
#ifdef TESTING

void CINSObjective::DumpLog( void )
{
	const CUtlVector< CINSObjective* > &Objectives = INSRules( )->GetObjectives( );

	Warning( "\n\n\n*** Dumping Status of Valid Objectives\n\n" );

	int iT1ObjSpawn, iT2ObjSpawn;
	iT1ObjSpawn = iT2ObjSpawn = INVALID_OBJECTIVE;

	if( INSRules( )->IsModeRunning( ) )
		INSRules( )->RunningMode( )->GetCurrentSpawns( iT1ObjSpawn, iT2ObjSpawn );

	for( int i = 0; i < Objectives.Count( ); i++ )
	{
		CINSObjective *pObjective = Objectives[ i ];

		// EXAMPLE:
		// Name: Upper Bridge (2)

		// Captured Team: US Marines
		// Capturing Team: Iraqi Insurgents

		// Capture Players: Pongles
		// Enemy Players: Tarky, Argyll, Optical

		char szBuffer[ 256 ];

		// name
		Q_snprintf( szBuffer, sizeof( szBuffer ), "Name: %s (%i)\n", pObjective->GetName( ), pObjective->GetID( ) );
		Warning( szBuffer );

		// if its a current spawn, print it
		if( pObjective->GetOrderID( ) == iT1ObjSpawn )
			Msg( "SpawnObj: Team One\n" );
		else if( pObjective->GetOrderID() == iT2ObjSpawn )
			Msg( "SpawnObj: Team Two\n" );

		// captured team
		int iCapturedTeam = pObjective->GetCapturedTeam( );
		Q_snprintf( szBuffer, sizeof( szBuffer ), "Captured Team: %s\n",
			( iCapturedTeam != TEAM_NEUTRAL ) ? GetGlobalTeam( iCapturedTeam )->GetName( ) : "Neutral" );
		Msg( szBuffer );

		// capturing team
		int iCapturingTeam = ( pObjective->IsCapturing( ) ? pObjective->GetCaptureTeam( ) : TEAM_NEUTRAL );
		Q_snprintf( szBuffer, sizeof( szBuffer ), "Capturing Team: %s\n",
			( iCapturingTeam != TEAM_NEUTRAL ) ? GetGlobalTeam( iCapturingTeam )->GetName( ) : "None");
		Msg( szBuffer );

		// capture players
		Msg( "Capture Players: " );
		pObjective->WriteList( pObjective->GetCapturePlayers( ), szBuffer, sizeof( szBuffer ) );
		Msg( szBuffer );
		Msg( "\n" );

		// enemy players
		Msg( "Enemy Players: " );
		pObjective->WriteList( pObjective->GetEnemyPlayers( ), szBuffer, sizeof( szBuffer ) );
		Msg( szBuffer );
		Msg( "\n\n" );
	}

	Warning( "\n*** End of Dump\n\n" );
}

void CINSObjective::WriteList( const CUtlVector<CINSPlayer*> &PlayerList, char *pszBuffer, int iBufferLength )
{
	if( iBufferLength == 0 )
		return;

	pszBuffer[ 0 ] = '\0';

	int iNumPlayers = PlayerList.Count( );

	if( iNumPlayers == 0 )
	{
		Q_strncpy( pszBuffer, "None", iBufferLength );
		return;
	}

	for( int j = 0; j < iNumPlayers; j++ )
	{
		CBasePlayer *pPlayer = PlayerList[ j ];
		char szNameBuffer[ MAX_PLAYER_NAME_LENGTH * 2 ];

		Assert( pPlayer );
		Q_strcpy( szNameBuffer, pPlayer->GetPlayerName( ) );

		if( j != ( iNumPlayers - 1 ) )
		{
			Q_strncat( szNameBuffer, ", ", sizeof( szNameBuffer ), COPY_ALL_CHARACTERS );
		}
		else
		{
			char szNum[ 32 ];
			Q_snprintf( szNum, sizeof( szNum ), " (%i)", iNumPlayers );
			Q_strncat( szNameBuffer, szNum, sizeof( szNameBuffer ), COPY_ALL_CHARACTERS );
		}

		Q_strncat( pszBuffer, szNameBuffer, iBufferLength, COPY_ALL_CHARACTERS );
	}
}

#endif
