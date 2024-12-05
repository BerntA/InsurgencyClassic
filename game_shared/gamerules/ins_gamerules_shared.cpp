//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "ins_gamerules.h"
#include "gameeventdefs.h"
#include "in_buttons.h"
#include "play_team_shared.h"
#include "basic_colors.h"
#include "script_check_shared.h"
#include "takedamageinfo.h"
#include "imc_config.h"
#include "weapon_ins_base.h"

#ifdef GAME_DLL

#include "ins_stats_shared.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
const char *g_pszGameTypes[ MAX_GAMETYPES ] = { 
	"Battle",		// GAMETYPE_BATTLE
	"Firefight", 	// GAMETYPE_FIREFIGHT
	"Push",			// GAMETYPE_PUSH
	"Strike",		// GAMETYPE_STRIKE
	"TDM",			// GAMETYPE_TDM
	"PowerBall"		// GAMETYPE_POWERBALL
};

//=========================================================
//=========================================================
const char *g_pszGRModes[ GRMODE_COUNT ] = {
	"Idle",				// GRMODE_IDLE
	"Squad Choice",		// GRMODE_SQUAD
	"Running"			// GRMODE_RUNNING
};

//=========================================================
//=========================================================
#ifdef TESTING

#define DEFAULT_MOTDMSG "Testing DLL"

#else

#define DEFAULT_MOTDMSG "Welcome to our Server"

#endif

//=========================================================
//=========================================================
ConVar motdmsg( "motdmsg", DEFAULT_MOTDMSG, FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines a MOTD Message" );

ConVar allowspectators( "mp_allowspectators", "1.0", FCVAR_REPLICATED, "Toggles Whether the Server Allows Spectator Mode or Not", true, 0, true, 1 );
ConVar friendlyfire( "mp_friendlyfire", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Defines Friendly-Fire Status", true, 0, true, 1 );
ConVar chattime( "mp_chattime", "3", FCVAR_NOTIFY | FCVAR_REPLICATED, "Length of Time (in Seconds) for Players to Commune When the Game is Over", true, 2, true, 10 );

ConVar dmgfactor( "ins_dmgfactor", "1", FCVAR_REPLICATED | FCVAR_NOTIFY, "Factor Reduction of Damage in FF", true, 0, true, 1 );
ConVar locksquads( "ins_locksquads", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Lock Squads During a Running Game", true, 0, true, 1 );
ConVar firetype( "ins_firetype", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "Defines if you can Fire either Always, Never or During Warmup", true, FIRETYPE_ALWAYS, true, FIRETYPE_NEVER );

#ifdef TESTING

ConVar ventrequired( "ins_ventrequired", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "When Enabled a Player Cannot Join a Team without Vent Running", true, 0, true, 1 );

#endif

//=========================================================
//=========================================================
ModeHelper_t g_ModeHelper[ GRMODE_COUNT ];

//=========================================================
//=========================================================
bool CINSRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	if( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap( collisionGroup0, collisionGroup1 );
	}

	if ( ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT ) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}

//=========================================================
//=========================================================
void CINSRules::PlayerThink(CBasePlayer *pPlayer)
{
	if( !IsModeRunning( ) )
		return;

	CINSPlayer *pINSPlayer = ToINSPlayer( pPlayer );

	if( !pINSPlayer || !pINSPlayer->IsRunningAround( ) )
		return;

	// update weapon states
	pINSPlayer->UpdateWeaponStates( );

	// clear any buttons etc
	if( !AllowAttacking( ) )
	{
		const int iClearFlags = IN_ATTACK | IN_SPECIAL1 | IN_SPECIAL2 | IN_RELOAD;

		// clear any attack flags
		pPlayer->m_afButtonPressed &= ~iClearFlags;
		pPlayer->m_nButtons &= ~iClearFlags;
		pPlayer->m_afButtonReleased &= ~iClearFlags;
	}
}

//=========================================================
//=========================================================
bool CINSRules::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
	// don't take damage 
	if( firetype.GetInt( ) == FIRETYPE_NODAMAGE )
		return false;

	// player can always can damage themselves
	if( pPlayer == pAttacker )
		return true;

	// handle friendly fire etc
	if( PlayerRelationship( pPlayer, pAttacker ) == GR_FRIEND )
		return friendlyfire.GetBool( );

	return true;
}

//=========================================================
//=========================================================
bool CINSRules::PlayerAdjustDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker, CTakeDamageInfo &info )
{
	if( !FPlayerCanTakeDamage( pPlayer, pAttacker ) )
		return false;

	// handle friendly fire scale
	if( PlayerRelationship( pPlayer, pAttacker ) == GR_FRIEND )
		info.ScaleDamage( dmgfactor.GetFloat( ) );

	return true;
}

//=========================================================
//=========================================================
int	CINSRules::PlayerRelationship( CBasePlayer *pPlayer, CBaseEntity *pTarget )
{
	if( !pPlayer || !pTarget )
		return GR_ENEMY;

	if( !pTarget->IsPlayer( ) )
		return GR_ENEMY;

	if( OnSameTeam( ToINSPlayer( pPlayer ), ToINSPlayer( pTarget ) ) )
		return GR_FRIEND;

	return GR_ENEMY;
}

//=========================================================
//=========================================================
int	CINSRules::TeamRelationship( CBasePlayer *pPlayer, int iTeamID )
{
	if( !pPlayer )
		return GR_ENEMY;

	if( OnSameTeam( ToINSPlayer( pPlayer ), iTeamID ) )
		return GR_FRIEND;

	return GR_ENEMY;
}

//=========================================================
//=========================================================
bool CINSRules::CanChangeTeam( CINSPlayer *pPlayer )
{
	if( !pPlayer )
		return false;

	switch( m_iCurrentMode )
	{
		case GRMODE_RUNNING:
		{
			if( RunningMode( )->IsEnding( ) )
				return false;

			// don't allow team changes while restarting
			if( pPlayer->OnPlayTeam( ) && RunningMode( )->IsRestarting( ) )
				return false;

			// only allow when they can can or the game is waiting for players
			if( RunningMode( )->IsWaitingForPlayers( ) || pPlayer->CanChangeTeam( ) )
				return true;

			break;
		}

		case GRMODE_SQUAD:
		{
			return ( !pPlayer->OnPlayTeam( ) );
		}

		case GRMODE_IDLE:
		{
			return true;
		}
	}

	return false;
}

//=========================================================
//=========================================================
bool CINSRules::CanChangeSquad( CINSPlayer *pPlayer )
{
	// ensure sane data
	if( !pPlayer )
		return false;

	// can only change when the game is running, squads are not locked, when on a play team and can change team
	return ( IsModeRunning( ) && pPlayer->OnPlayTeam( ) && CanChangeTeam( pPlayer ) && !locksquads.GetBool( ) );
}

//=========================================================
//=========================================================
int CINSRules::DefaultFOV( void ) const
{
	return 85;	
}

//=========================================================
//=========================================================
int CINSRules::DefaultWeaponFOV( void ) const
{
	return INS_WEAPONFOV_DEFAULT;	
}

//=========================================================
//=========================================================
#define PLAYER_PRONEFORWARD 6

int CINSRules::GetProneForward( void ) const
{
	return PLAYER_PRONEFORWARD;
}

//=========================================================
//=========================================================
int CINSRules::GetMode( void ) const
{
	return m_iCurrentMode;
}

bool CINSRules::IsModeIdle( void ) const
{
	return ( m_iCurrentMode == GRMODE_IDLE );
}

bool CINSRules::IsModeSquad( void ) const
{
	return ( m_iCurrentMode == GRMODE_SQUAD );
}

bool CINSRules::IsModeRunning( void ) const
{
	return ( m_iCurrentMode == GRMODE_RUNNING );
}

//=========================================================
//=========================================================
CModeBase *CINSRules::CurrentMode( void ) const
{
	Assert( m_iCurrentMode >= 0 && m_iCurrentMode < GRMODE_COUNT );
	return m_pModes[ m_iCurrentMode ];
}

//=========================================================
//=========================================================
CRunningMode *CINSRules::RunningMode( void ) const
{
	if( !IsModeRunning( ) )
	{
		Assert( false );
		return NULL;
	}

	return ( CRunningMode* )CurrentMode( );
}

//=========================================================
//=========================================================
CSquadMode *CINSRules::SquadMode( void ) const
{
	if( !IsModeSquad( ) )
	{
		Assert( false );
		return NULL;
	}

	return ( CSquadMode* )CurrentMode( );
}

//=========================================================
//=========================================================
bool CINSRules::AllowAttacking( void )
{
	if( RunningMode( )->IsFrozen( ) )
		return false;

	switch( firetype.GetInt( ) )
	{
	case FIRETYPE_ALWAYS:
		return true;

	case FIRETYPE_NEVER:
		return false;

	case FIRETYPE_WARMUP:
		return ( !( IsModeRunning( ) && RunningMode( )->IsWarmingup( ) ) );
	}

	return true;
}

//=========================================================
//=========================================================
int CINSRules::GetDeadCamMode( void ) const
{
	return ( IsModeRunning( ) ? RunningMode( )->GetDeadCamMode( ) : OBS_ALLOWMODE_ALL );
}

//=========================================================
//=========================================================
int CINSRules::GetDeadCamTargets( void ) const
{
	return ( IsModeRunning( ) ? RunningMode( )->GetDeadCamTargets( ) : OBS_ALLOWTARGETS_ALL );
}

//=========================================================
//=========================================================
bool CINSRules::IsRoundCold( void ) const
{
	if( !IsModeRunning( ) )
		return true;

	if( RunningMode( )->IsFrozen( ) || RunningMode( )->IsWaitingForPlayers( ) )
		return true;

	return false;
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void SendProxy_ScriptCheck( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = g_ScriptCheckShared.GetScriptCRC32( );
}

void SendProxy_Stats( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = GetINSStats( )->GetType( );
}

void SendProxy_GameStatus( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->GetMode( );
}

void *SendProxy_SendSquadModeDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->ClearAllRecipients( );

	if( INSRules( )->IsModeSquad( ) )
	{
		pRecipients->SetAllRecipients( );

		return ( void* )pVarData;
	}

	return NULL;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendSquadModeDataTable );

void *SendProxy_SendRunningModeDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	pRecipients->ClearAllRecipients( );

	if( INSRules( )->IsModeRunning( ) )
	{
		pRecipients->SetAllRecipients( );

		return ( void* )pVarData;
	}

	return NULL;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendRunningModeDataTable );

bool SetupSquadOrderProxy( int iTeamID, CSendProxyRecipients *pRecipients )
{
	pRecipients->ClearAllRecipients( );

	if( !INSRules( )->IsModeSquad( ) )
		return false;

	AddTeamRecipients( GetGlobalTeam( iTeamID ), pRecipients );
	return true;
}

void *SendProxy_SendSquadOrderT1DataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	if( !SetupSquadOrderProxy( TEAM_ONE, pRecipients ) )
		return NULL;

	return ( void* )pVarData;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendSquadOrderT1DataTable );

void *SendProxy_SendSquadOrderT2DataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID )
{
	if( !SetupSquadOrderProxy( TEAM_TWO, pRecipients ) )
		return NULL;

	return ( void* )pVarData;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER( SendProxy_SendSquadOrderT2DataTable );

int ArrayLength_POrderArray( int iTeamID )
{
	return INSRules( )->SquadMode( )->GetOrderLength( iTeamID );
}

int SendProxyArrayLength_T1_POrderArray( const void *pStruct, int objectID )
{
	return ArrayLength_POrderArray( TEAM_ONE );
}

int SendProxyArrayLength_T2_POrderArray( const void *pStruct, int objectID )
{
	return ArrayLength_POrderArray( TEAM_TWO );
}

int POrderList( int iTeamID, int iElement )
{
	return INSRules( )->SquadMode( )->GetOrderElement( iTeamID, iElement );
}

void SendProxy_T1_POrderList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = POrderList( TEAM_ONE, iElement );
}

void SendProxy_T2_POrderList( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = POrderList( TEAM_TWO, iElement );
}

void SendProxy_RunningStatus( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->GetStatus( );
}

void SendProxy_RoundStartTime( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Float = INSRules( )->RunningMode( )->GetRoundStartTime( );
}

void SendProxy_RoundLength( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->GetRoundLength( ) + 1;
}

void SendProxy_RoundExtended( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->IsRoundExtended( ) ? 1 : 0;
}

void SendProxy_RunningDIF( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->IsDeathInfoFull( ) ? 1 : 0;
}

void SendProxy_FrozenTime( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->GetFrozenTime( );
}

void SendProxy_DeadCamMode( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->GetDeadCamMode( );
}

void SendProxy_DeadCamTargets( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	pOut->m_Int = INSRules( )->RunningMode( )->GetDeadCamTargets( );
}

void SendProxy_CurrentViewpointOrigin( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	const Vector &vecCurrentViewpoint = INSRules( )->GetCurrentViewpointOrigin( );

	pOut->m_Vector[ 0 ] = vecCurrentViewpoint[ 0 ];
	pOut->m_Vector[ 1 ] = vecCurrentViewpoint[ 1 ];
	pOut->m_Vector[ 2 ] = vecCurrentViewpoint[ 2 ];
}

void SendProxy_CurrentViewpointAngle( const SendProp *pProp, const void *pStruct, const void *pData, DVariant *pOut, int iElement, int objectID )
{
	const QAngle &angCurrentViewpoint = INSRules( )->GetCurrentViewpointAngle( );

	pOut->m_Vector[ 0 ] = anglemod( angCurrentViewpoint.x );
	pOut->m_Vector[ 1 ] = anglemod( angCurrentViewpoint.y );
	pOut->m_Vector[ 2 ] = anglemod( angCurrentViewpoint.z );
}

#else

void RecvProxy_ScriptCheck( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->SetScriptsCRC32( pData->m_Value.m_Int );
}

void RecvProxy_Stats( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->SetStatsType( pData->m_Value.m_Int );
}

void RecvProxy_GameStatus( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->SetMode( pData->m_Value.m_Int );
}

void ArrayLength_POrderArray( int iTeamID, int iLength )
{
	INSRules( )->SquadModeUpdate( )->SetOrderLength( iTeamID, iLength );
}

void RecvProxyArrayLength_T1_POrderArray( void *pStruct, int objectID, int currentArrayLength )
{
	ArrayLength_POrderArray( TEAM_ONE, currentArrayLength );
}

void RecvProxyArrayLength_T2_POrderArray( void *pStruct, int objectID, int currentArrayLength )
{
	ArrayLength_POrderArray( TEAM_TWO, currentArrayLength );
}

void POrderList( int iTeamID, int iElement, int iValue )
{
	INSRules( )->SquadModeUpdate( )->SetOrder( iTeamID, iElement, iValue );
}

void RecvProxy_T1_POrderList(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	POrderList( TEAM_ONE, pData->m_iElement, pData->m_Value.m_Int );
}

void RecvProxy_T2_POrderList(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	POrderList( TEAM_TWO, pData->m_iElement, pData->m_Value.m_Int );
}

void RecvProxy_RunningStatus( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetStatus( pData->m_Value.m_Int );
}

void RecvProxy_RoundStartTime( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetRoundStartTime( pData->m_Value.m_Float );
}

void RecvProxy_RoundLength( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetRoundLength( pData->m_Value.m_Int - 1 );
}

void RecvProxy_RoundExtended( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetRoundExtended( pData->m_Value.m_Int ? true : false );
}

void RecvProxy_RunningDIF( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetDeathInfoFull( pData->m_Value.m_Int ? true : false );
}

void RecvProxy_FrozenTime( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetFrozenTimeLength( pData->m_Value.m_Int );
}

void RecvProxy_DeadCamMode( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetDeadCamMode( pData->m_Value.m_Int );
}

void RecvProxy_DeadCamTargets( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->RunningModeUpdate( )->SetDeadCamTargets( pData->m_Value.m_Int );
}

void RecvProxy_CurrentViewpointOrigin( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->SetCurrentViewpointOrigin( pData->m_Value.m_Vector[ 0 ], pData->m_Value.m_Vector[ 1 ], pData->m_Value.m_Vector[ 2 ] );
}

void RecvProxy_CurrentViewpointAngle( const CRecvProxyData *pData, void *pStruct, void *pOut )
{
	INSRules( )->SetCurrentViewpointAngles( pData->m_Value.m_Vector[ 0 ], pData->m_Value.m_Vector[ 1 ], pData->m_Value.m_Vector[ 2 ] );
}

#endif

BEGIN_NETWORK_TABLE_NOBASE( CINSRules, DT_SquadOrderT1 )

#ifdef GAME_DLL

	SendPropArray2( SendProxyArrayLength_T1_POrderArray,
		SendPropInt( "porder_element", 0, 4, 10, SPROP_UNSIGNED, SendProxy_T1_POrderList ), 
		MAX_PLAYERS, 
		0, 
		"porder" ),

#else

	RecvPropArray2( RecvProxyArrayLength_T1_POrderArray,
		RecvPropInt( "porder_element", 0, SIZEOF_IGNORE, 0, RecvProxy_T1_POrderList ), 
		MAX_PLAYERS, 
		0, 
		"porder" ),

#endif

END_NETWORK_TABLE( )

BEGIN_NETWORK_TABLE_NOBASE( CINSRules, DT_SquadOrderT2 )

#ifdef GAME_DLL

	SendPropArray2(SendProxyArrayLength_T2_POrderArray,
		SendPropInt( "porder_element", 0, 4, 10, SPROP_UNSIGNED, SendProxy_T2_POrderList ), 
		MAX_PLAYERS, 
		0, 
		"porder" ),

#else

	RecvPropArray2( RecvProxyArrayLength_T2_POrderArray,
		RecvPropInt( "porder_element", 0, SIZEOF_IGNORE, 0, RecvProxy_T2_POrderList ), 
		MAX_PLAYERS, 
		0, 
		"porder" ),

#endif

END_NETWORK_TABLE( )

BEGIN_NETWORK_TABLE_NOBASE( CINSRules, DT_SquadMode )

#ifdef GAME_DLL

	SendPropDataTable( "OrderT1", 0, &REFERENCE_SEND_TABLE( DT_SquadOrderT1 ), SendProxy_SendSquadOrderT1DataTable ),
	SendPropDataTable( "OrderT2", 0, &REFERENCE_SEND_TABLE( DT_SquadOrderT2 ), SendProxy_SendSquadOrderT2DataTable ),

#else

	RecvPropDataTable( "OrderT1", 0, 0, &REFERENCE_RECV_TABLE( DT_SquadOrderT1 ) ),
	RecvPropDataTable( "OrderT2", 0, 0, &REFERENCE_RECV_TABLE( DT_SquadOrderT2 ) ),

#endif

END_NETWORK_TABLE( )

BEGIN_NETWORK_TABLE_NOBASE( CINSRules, DT_RunningMode )

#ifdef GAME_DLL

	SendPropInt( "running_status", 0, SIZEOF_IGNORE, GAMERUNNING_MAXBITS, SPROP_UNSIGNED, SendProxy_RunningStatus ),
	SendPropFloat( "round_start", 0, SIZEOF_IGNORE, 32, SPROP_UNSIGNED, 0.0f, HIGH_DEFAULT, SendProxy_RoundStartTime ),
	SendPropInt( "round_length", 0, SIZEOF_IGNORE, 16, 0, SendProxy_RoundLength ),
	SendPropInt( "round_extended", 0, SIZEOF_IGNORE, 1, SPROP_UNSIGNED, SendProxy_RoundExtended ),
	SendPropInt( "running_dif", 0, SIZEOF_IGNORE, 1, SPROP_UNSIGNED, SendProxy_RunningDIF ),
	SendPropInt( "frozen_time", 0, SIZEOF_IGNORE, FROZENLENGTH_BITS, SPROP_UNSIGNED, SendProxy_FrozenTime ),

	SendPropInt( "deadcam_mode", 0, SIZEOF_IGNORE, 2, SPROP_UNSIGNED, SendProxy_DeadCamMode ),
	SendPropInt( "deadcam_targets", 0, SIZEOF_IGNORE, 2, SPROP_UNSIGNED, SendProxy_DeadCamTargets ),

#else

	RecvPropInt( "running_status", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_RunningStatus ),
	RecvPropFloat( "round_start", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_RoundStartTime ),
	RecvPropInt( "round_length", 0, SIZEOF_IGNORE, 0, RecvProxy_RoundLength ),
	RecvPropInt( "round_extended", 0, SIZEOF_IGNORE, 0, RecvProxy_RoundExtended ),
	RecvPropInt( "running_dif", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_RunningDIF ),
	RecvPropInt( "frozen_time", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_FrozenTime ),

	RecvPropInt( "deadcam_mode", 0, SIZEOF_IGNORE, 0, RecvProxy_DeadCamMode ),
	RecvPropInt( "deadcam_targets", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_DeadCamTargets ),

#endif

END_NETWORK_TABLE( )

BEGIN_NETWORK_TABLE_NOBASE( CINSRules, DT_INSRules )

#ifdef GAME_DLL

	// Scripts
	SendPropInt( "scriptcheck", 0, SIZEOF_IGNORE, 32, SPROP_UNSIGNED, SendProxy_ScriptCheck ),

	// Using Status
	SendPropInt( "stats", 0, SIZEOF_IGNORE, 2, SPROP_UNSIGNED, SendProxy_Stats ),

	// Game Status
	SendPropInt( "game_status", 0, SIZEOF_IGNORE, 4, SPROP_UNSIGNED, SendProxy_GameStatus ),

	// Squad Mode
	SendPropDataTable( "SquadMode", 0, &REFERENCE_SEND_TABLE( DT_SquadMode ), SendProxy_SendSquadModeDataTable ),

	// Running Mode
	SendPropDataTable( "RunningMode", 0, &REFERENCE_SEND_TABLE( DT_RunningMode ), SendProxy_SendRunningModeDataTable ),

	// Current Viewpoint
	SendPropVector( "vp_origin", 0, SIZEOF_IGNORE, -1, SPROP_COORD, 0.0f, HIGH_DEFAULT, SendProxy_CurrentViewpointOrigin ),
	SendPropQAngles( "vp_angle", 0, SIZEOF_IGNORE, 32, 0, SendProxy_CurrentViewpointAngle ),

#else

	// Scripts
	RecvPropInt( "scriptcheck", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_ScriptCheck ),

	// Using Stats
	RecvPropInt( "stats", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_Stats ),

	// Game Status
	RecvPropInt( "game_status", 0, SIZEOF_IGNORE, SPROP_UNSIGNED, RecvProxy_GameStatus ),

	// Squad Mode
	RecvPropDataTable( "SquadMode", 0, 0, &REFERENCE_RECV_TABLE( DT_SquadMode ) ),

	// Running Mode
	RecvPropDataTable( "RunningMode", 0, 0, &REFERENCE_RECV_TABLE( DT_RunningMode ) ),

	// Current Viewpoint
	RecvPropVector( "vp_origin", 0, SIZEOF_IGNORE, SPROP_COORD, RecvProxy_CurrentViewpointOrigin ),
	RecvPropQAngles( "vp_angle", 0, SIZEOF_IGNORE, 0, RecvProxy_CurrentViewpointAngle ),

#endif

END_NETWORK_TABLE( )

LINK_ENTITY_TO_CLASS( insrules, CINSRulesProxy );

IMPLEMENT_NETWORKCLASS_ALIASED( INSRulesProxy, DT_INSRulesProxy )

#ifdef CLIENT_DLL

BEGIN_RECV_TABLE( CINSRulesProxy, DT_INSRulesProxy )

	RecvPropDataTable( "insrules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_INSRules ) )

END_RECV_TABLE()

#else

BEGIN_SEND_TABLE( CINSRulesProxy, DT_INSRulesProxy )

	SendPropDataTable( "insrules_data", 0, &REFERENCE_SEND_TABLE( DT_INSRules ) )

END_SEND_TABLE( )

#endif