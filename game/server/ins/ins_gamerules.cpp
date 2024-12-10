//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"
#include "viewport_panel_names.h"
#include "voice_gamemgr.h"
#include "team_lookup.h"
#include "imc_config.h"
#include "team_spawnpoint.h"
#include "view_team.h"
#include "play_team_shared.h"
#include "ins_obj.h"
#include "ins_objmarker.h"
#include "ins_squad_shared.h"
#include "basic_colors.h"
#include "script_check_shared.h"
#include "keyvalues.h"
#include "filesystem.h"
#include "ins_player.h"
#include "ins_weaponcache.h"
#include "voicemgr.h"
#include "ins_imcsync.h"
#include "ins_utils.h"
#include "game.h"
#include "ins_profilemanager.h"
#include "ins_area.h"
#include "movevars_shared.h"
#include "grenadedef.h"
#include "eventqueue.h"
#include "gameinterface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define INVALID_VIEWPOINT -1
#define VIEWPOINT_ROTATE 3.5f

//=========================================================
//=========================================================
const char *g_pszErrorMessages[ MAX_GMWARNS ] = {
	// GRWARN_IMC_INVALIDFILE
		"IMC does not Exist",
	// GRWARN_IMC_INVALIDFORMAT
		"IMC is an Invalid Format (either Legacy or Corrupt)",
	// GRWARN_IMC_INVALIDTEAM
		"IMC has an Invalid TeamType",
	// GRWARN_IMC_MISSINGDATA
		"IMC has Missing Data",
	// GRWARN_IMC_INVALIDGLOBALOBJ
		"IMC has an Invalid Objective",
	// GRWAWN_IMC_NOOBJECTIVES
		"IMC has No Objectives",
	// GRWAWN_IMC_NOPROFILES
		"IMC has No Profiles",
	// GRWARN_IMC_INVALIDPROFILE
		"IMC has Loaded an Invalid Profile",
	// GRWARN_IMC_BOUNDS
		"IMC has a Value out of Bounds",
	// GRWARN_IMC_NOSQUADS
		"IMC has a Team without any Squads",
	// GRWARN_IMC_INVALIDSQUAD
		"IMC has a Team with an Invalid Squad",
	// GRWARN_IMC_NOSLOTS
		"IMC has a Squad without any Slots",
	// GRWARN_IMC_INVALIDOBJECTIVES
		"IMC has an Invalid Objective",
	// GRWARN_IMC_INVALIDCACHES
		"IMC has an Invalid WeaponCache",
	// GRWARN_IMC_NONMOVINGFALLBACK
		"IMC has a Non-Moving Fallback Spawn",

	// GMWARN_GENERAL_NOTENOUGHOBJS,
		"Not Enough Objectives",
	// GMWARN_GENERAL_INVALID_TEAM_STARTS
		"Invalid Team Starts",
	// GMWARN_GENERAL_INVALIDOBJS
		"The IMC defines Objectives not included in the Map",
	// GMWARN_GENERAL_FAILEDSETUP
		"The IMC failed Setup",
	// GMWARN_GENERAL_INVALIDCACHE
		"The IMC defines a WeaponCache not included in the Map",

	// GMWARN_FF_NO_HIDDEN_OBJS
		"The Lowest and Highest Objectives must be Hidden",	
	// GMWARN_FF_INCORRECT_HIDDEN
		"Only the Lowest and Highest Objectives may be Hidden",
	// GMWARN_FF_NEUTRAL_HIDDEN_OBJS		
		"The Lowest and Highest Objectives must not be Neutral",
	// GMWARN_FF_BAD_HIDDEN_OBJ_TEAMS
		"The Lowest and Highest Objectives must be Assigned to Opposite Teams",
	
	// GMWARN_PUSH_NO_HIDDEN_OBJ		
		"Either the Lowest or Highest Objective must be Hidden",
	// GMWARN_PUSH_BOTH_HIDDEN
		"Both the Lowest and Highest Objective cannot be Hidden",
	// GMWARN_PUSH_NEUTRAL_OBJS
		"No Objectives can be Neutral",	
	// GMWARN_PUSH_BAD_HIDDEN_OBJ_TEAMS
		"The Lowest and Highest Objectives must be Assigned to Opposite Teams",
	// GMWARN_PUSH_MISPLACED_HIDDEN_OBJ
		"There can Only be One Hidden Objective Placed",
	// GMWARN_PUSH_BAD_DEFENDER_TEAM
		"The Non-Hidden Objectives are not a Consistant Team",

	// GMWARN_PB_MOVINGSPAWNS
		"Cannot have Moving Spawns",
	// GMWARN_PB_HIDDENOBJS
		"No Hidden Objs",
	// GMWARN_PB_TEAMNEUTRAL
		"No Neutral Objs",
	// GMWARN_PB_TEAMSAME	
		"Teams have same Team",
	// GMWARN_PB_NOPOWERBALL
		"Cannot find Powerball Entity"
};

//=========================================================
//=========================================================
const char *g_pszGameTypeClassnames[ MAX_GAMETYPES ];

//=========================================================
//=========================================================

// PNOTE: "weapon_" is handled in code because it's conditional

static const char *g_pszRemoveGroups[ ] = {
	"prop_",
	"grenade_",
	"phys_",
	"physics_",
	NULL,
};

static const char *g_pszRemoveEnts[ ] =
{
	"ins_ragdoll",
	"func_breakable",
	"func_breakable_surf",
	"ins_breakable",
	"func_physbox_multiplayer",
	NULL,
};

//=========================================================
//=========================================================
static ConVar *sv_lan = NULL;

//=========================================================
//=========================================================
extern ConVar alltalk;

class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	bool CanPlayerHearPlayer(CBasePlayer* pListener, CBasePlayer* pTalker, bool& bProximity) OVERRIDE
	{
		CINSPlayer *pINSListener = ToINSPlayer( pListener );
		CINSPlayer *pINSTalker = ToINSPlayer( pTalker );

		// check dead states first
		if( !g_pGameRules->PlayerCanCommunicate( pListener, pTalker ) )
			return false;

		bool bGlobalTalk = alltalk.GetBool( );

		if( OnSameTeam( pINSListener, pINSTalker ) )
		{
			// when they're on the same team, just ensure we're not wanting to 
			// send to their squad only
			if( ( pINSTalker->GetVoiceType( ) == PVOICETYPE_SQUAD ) && !OnSameSquad( pINSListener, pINSTalker ) )
				return false;
		}
		else if( !bGlobalTalk || pINSTalker->GetVoiceType( ) != PVOICETYPE_ALL )
		{
			// global talk disabled or the talker isn't broadcasting
			return false;
		}

		return true;
	}
};

CVoiceGameMgrHelper g_VoiceGameMgrHelper;
IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;

//=========================================================
//=========================================================
/*void CC_INS_ChangeLevel(void)
{
	if( args.ArgC() < 2 )
		return;

	int iProfileID = 0;

	if( args.ArgC() == 3 )
		iProfileID = atoi( args[2] );

	CINSRules::ChangeLevel( args[1], iProfileID );
}

static ConCommand insmap( "ins_changelevel", CC_INS_ChangeLevel, "Change the Map (with Additional Profile Specifier)" );*/

//=========================================================
//=========================================================
void CC_INS_ShowProfiles( void )
{
	if( !IMCConfig( ) )
		return;

	Msg( "Profiles for: %s\n", gpGlobals->mapname );
	Msg( "-----------------\n" );

	for( int i = 0; i < IMCConfig( )->GetProfileCount( ); i++ )
	{
		const ProfileDataChunk_t &Data = IMCConfig( )->GetProfile( i );
		Msg( "%i - %s (%s)\n", i, Data.m_szProfileName, g_pszGameTypes[ Data.m_iGameModeID ] );
	}

	Msg( "-----------------\n" );
}

static ConCommand insprofiles( "ins_showprofiles", CC_INS_ShowProfiles, "Show the Profiles for the Current Map" );

//=========================================================
//=========================================================
CINSMapEntityFilter::CINSMapEntityFilter( )
{
	m_iIterator = 0;
}

//=========================================================
//=========================================================
void CINSMapEntityFilter::Setup( void )
{
	m_iIterator = g_MapEntityRefs.Head( );
}

//=========================================================
//=========================================================
bool CINSMapEntityFilter::ShouldCreateEntity( const char *pszClassname )
{
	// recreate when on a remove list
	if( UTIL_FindPrefixInList( g_pszRemoveGroups, pszClassname ) || UTIL_FindInList( g_pszRemoveEnts, pszClassname ) )
	{
		return true;
	}
	else
	{
		// increment our iterator since it's not going to call CreateNextEntity for this ent
		if( m_iIterator != g_MapEntityRefs.InvalidIndex( ) )
			m_iIterator = g_MapEntityRefs.Next( m_iIterator );

		return false;
	}
}
 
//=========================================================
//=========================================================
CBaseEntity* CINSMapEntityFilter::CreateNextEntity( const char *pszClassname ) 
{
	if( m_iIterator == g_MapEntityRefs.InvalidIndex( ) )
	{
		// this shouldn't be possible. when we loaded the map, it should have used 
		// CINSMapEntityFilter, which should have built the g_MapEntityRefs list
		// with the same list of entities we're referring to here
		Assert( false );
		return NULL;
	}
	else
	{
		CMapEntityRef &ref = g_MapEntityRefs[ m_iIterator ];
		m_iIterator = g_MapEntityRefs.Next( m_iIterator );	// seek to the next entity.

		if( ref.m_iEdict == -1 || engine->PEntityOfEntIndex( ref.m_iEdict ) )
		{
			// doh! The entity was deleted and its slot was reused
			// just use any old edict slot. this case sucks because we lose the baseline.
			return CreateEntityByName( pszClassname );
		}
		else
		{
			// cool, the slot where this entity was is free again
			// now create an entity with this specific index
			return CreateEntityByName( pszClassname, ref.m_iEdict );
		}
	}
}

//=========================================================
//=========================================================

ConVar autoteambalance( "mp_autoteambalance", "1", FCVAR_NOTIFY | FCVAR_REPLICATED, "", true, 0, true, 1 );
ConVar limitteams( "mp_limitteams", "2", FCVAR_NOTIFY | FCVAR_REPLICATED, "Minimum Difference between Team Sizes", true, 0, true, MAX_SQUAD_SLOTS );
ConVar timelimit( "mp_timelimit", "0", FCVAR_NOTIFY | FCVAR_REPLICATED, "game time per map in minutes" );
ConVar tkpunish( "mp_tkpunish", "1", FCVAR_NOTIFY, "Toggles Team-Killing Punishment", true, 0, true, 1 );

ConVar deadchat( "ins_deadchat", "0", FCVAR_NOTIFY, "Determines whether Dead Players can Chat to Alive Players", true, 0, true, 1 );

ConVar clanmode( "ins_clanmode", "0", FCVAR_NOTIFY, "Defines whether Clan Mode is Active", true, 0, true, 1);
ConVar clanleaderpass( "ins_clanleaderpass", "0", FCVAR_NEVER_AS_STRING | FCVAR_UNLOGGED, "Defines the Password for Clan Leaders" );

ConVar strictnaming( "ins_strictnaming", "1", FCVAR_REPLICATED, "When Enabled, the Player can only Change his name Once per Round", true, 0, true, 1 );
ConVar cachesabotage( "ins_cachesabotage", "1", FCVAR_NOTIFY, "When Enabled, Allows a Team to Sabotage their own Caches", true, 0, true, 1 );

ConVar forcesquadopen( "ins_forcesquadopen", "0", FCVAR_NOTIFY, "Forces all Squads to be Open", true, 0, true, 1 );

ConVar forceautoassign( "ins_forceautoassign", "0", FCVAR_NOTIFY, "Forces a Player to Auto-Assign when Joining", true, 0, true, 1 );

ConVar suppresskillhint( "ins_suppresskillhint", "0", FCVAR_NOTIFY, "Supress the Kill Hint from being Sent", true, 0, true, 1 );

ConVar tkremove( "ins_tkremove", "4", FCVAR_NOTIFY, "How Many TKs before Kicking", true, 0, true, 25 );
ConVar tkdetect( "ins_tkdetect", "1", FCVAR_NOTIFY, "Auto Kick TKs", true, 0, true, 1 );

#ifdef TESTING

ConVar noragdolls( "ins_noragdolls", "0", FCVAR_NOTIFY, "Defines whether Ragdolls are Created", true, 0, true, 1 );

#endif

ConVar decalfrequency( "ins_decalfrequency", "10", FCVAR_NOTIFY );

//=========================================================
//=========================================================
int CINSRules::m_iError = 0;

char CINSRules::m_szServerString[ 64 ];

//=========================================================
//=========================================================
CINSRules::CINSRules( )
{
	// setup voice helper
	GetVoiceGameMgr( )->Init( g_pVoiceGameMgrHelper, gpGlobals->maxClients );
}

//=========================================================
//=========================================================
void CINSRules::InitRules( void )
{
	// init server string
	m_szServerString[ 0 ] = '\0';

	// find sv_lan
	sv_lan = cvar->FindVar("sv_lan");

	// init obj system
	m_bUseObjs = false;

	memset( m_pUnsortedObjectives, NULL, sizeof( m_pUnsortedObjectives ) );

	m_flErrorTime = 0.0f;

	m_iT1ObjOrientation = T1ORIENTATION_INVALID;

	memset( &m_iStartObjs, INVALID_OBJECTIVE, sizeof( m_iStartObjs ) );

	// init game status
	m_iCurrentMode = GRMODE_IDLE;
	m_iLastMode = GRMODE_NONE;

	for( int i = 0; i < GRMODE_COUNT; i++ )
	{
		ModeHelper_t ModeHelper = g_ModeHelper[ i ];

		if( !ModeHelper )
		{
			AssertMsg( false, "Missing Mode Class" );
			continue;
		}

		m_pModes[ i ] = ModeHelper( );
	}

	// init squad
	UpdateSquadOrderType( );
	m_bForcedSquadsOpen = false;

	// init players
	m_iConnectedPlayers = 0;

	// init viewpoints
	m_iCurrentViewpoint = INVALID_VIEWPOINT;
	m_flViewpointRotateTime = 0.0f;

	// init misc
	m_bVirginWorld = true;
	m_bIgnoreNextDeath = false;
	m_bTimeLimitPassed = false;
	m_bClanMode = false;

	m_bAwardRoundPoints = false;

	m_pFallbackSpawn = NULL;

	// create the proxy
	CreateProxy( );

	// setup server config
	ExecServerConfig( );
}

//=========================================================
//=========================================================
void CINSRules::CreateProxy( void )
{
	CreateEntityByName( "insrules" );
}

//=========================================================
//=========================================================
void CINSRules::Precache( void )
{
	Assert( IMCConfig( ) );

	// set the server name etc
	Q_strncpy( m_szServerString, IMCConfig( )->GetGameName( ), sizeof( m_szServerString ) );

	if( IMCConfig( )->GetTheaterID( ) != THEATERTYPE_UNKNOWN )
	{
		Q_strncat( m_szServerString, "- ", sizeof( m_szServerString ), COPY_ALL_CHARACTERS );
		Q_strncat( m_szServerString, IMCConfig( )->GetTheaterName( ), sizeof( m_szServerString ), COPY_ALL_CHARACTERS );
	}

	// precache teams
	for( int i = 0; i < MAX_TEAMS; i++ )
	{
		CTeam *pTeam = GetGlobalTeam( i );
		Assert( pTeam );

		if( pTeam )
			pTeam->Precache( );
	}

	// precache player stuff
	CINSPlayer::LoadData( );

	// precache ammo stuff
	GetAmmoDef( )->Precache( );
	
	// precache sounds
	g_VoiceMgr.Precache( );

	// precache unitorders
	CUnitOrder::Precache( );

	// basic explosion sound
	CBaseEntity::PrecacheScriptSound( INS_EXPLOSION_DEFAULTSOUND );
}

//=========================================================
//=========================================================
void CINSRules::Think( void )
{
	if( CurrentMode( )->ShouldThink( ) )
	{
		// update voice manager
		GetVoiceGameMgr( )->Update( gpGlobals->frametime );

		// check view-points for rotation
		if( m_flViewpointRotateTime != 0.0f && gpGlobals->curtime >= m_flViewpointRotateTime )
			UpdateCurrentViewpoint( );

		// repeat error messages
		EchoRepeatingError( );
	}

	// check if we can change
	if( ShouldMapRotate( ) )
	{
		CGameRules::ChangeLevel( );
		return;
	}
	
	// current mode think
	HandleModeThink( );
}

//=========================================================
//=========================================================
void CINSRules::LevelInitPreEntity( void )
{
	BaseClass::LevelInitPreEntity( );

	// load stats -- we dont use statsman stuff
	//GetINSStats( )->Init( );

	// setup script checking
	g_ScriptCheckShared.Calculate( );

	// sync the IMC with the client
	CIMCSync::Create( );

	// setup teams
	CViewTeam::Create( TEAM_UNASSIGNED );
	CViewTeam::Create( TEAM_SPECTATOR );

	// finalise IMC
	IMCConfig( )->Finalise( );

	// set gravity
	sv_gravity.SetValue( IMCConfig( )->GetGravity( ) );
}

//=========================================================
//=========================================================
void CINSRules::LevelInitPostEntity( void )
{
	// fire the IMC output on the profilemanager
	CINSProfileManager *pProfileManager = ( CINSProfileManager* )gEntList.FindEntityByClassname( NULL, "ins_profilemanager" );

	if( pProfileManager )
		pProfileManager->FireIMCOutput( );

	// setup objectives
	int iObjError = SetupObjectives( );

	if( iObjError != 0 )
		m_iError = iObjError;

	// setup viewpoints
	UpdateCurrentViewpoint( );

	// find fallback spawn
	m_pFallbackSpawn = gEntList.FindEntityByClassname( NULL, "info_player_start" );
}

//=========================================================
//=========================================================
void CINSRules::LevelShutdown( void )
{
	m_szServerString[ 0 ] = '\0';
}

//=========================================================
//=========================================================
void CINSRules::LevelShutdownPreEntity( void )
{
	// send off stats
	//GetINSStats( )->LogoutPlayers( );

	// clear spawn points
	UTIL_CleanSpawnPoints( );

	m_pFallbackSpawn = NULL;

	// reset IMC
	IMCConfig( )->Reset( );

	// remove entities from list
	g_INSAreas.RemoveAll( );

	// remove debug lines
#ifdef _DEBUG

	UTIL_ClearLineList( );

#endif
}

//=========================================================
//=========================================================
const char *CINSRules::GetGameTypeName( void ) const
{
	return g_pszGameTypes[ GetGameType( ) ];
}

//=========================================================
//=========================================================
const char *CINSRules::GetGameDescription( void )
{
	if( !CIMCConfig::IsValidIMCConfig( ) )
		return GetGameVersion( );

	CIMCConfig *pConfig = CIMCConfig::IMCConfig( );

	if( !pConfig || !pConfig->IsInit( ) )
		return GetGameVersion( );

	return m_szServerString;
}

//=========================================================
//=========================================================
const char *CINSRules::GetGameVersion( void )
{
#ifndef TESTING

#pragma message ( "CINSRules::GetGameVersion - Remember to Update!" )

#endif

	static const char *pszVersion = "Insurgency 1.2";
	return pszVersion;
}

//=========================================================
//=========================================================
void CINSRules::SetMode( int iMode )
{
	if( m_iCurrentMode == iMode )
		return;

	m_iLastMode = m_iCurrentMode;
	m_iCurrentMode = iMode;

#ifdef TESTING

	char szMessage[ 128 ];

	Q_snprintf( szMessage, sizeof( szMessage ), "Entering \"%s\" Status\n", g_pszGRModes[ iMode ] );
	Msg( szMessage );

#endif

	int iInitRet = CurrentMode( )->Init( );

	if( iInitRet != GRMODE_NONE )
		SetMode( iInitRet );
}

//=========================================================
//=========================================================
void CINSRules::HandleModeThink( void )
{
	int iNewMode = CurrentMode( )->Think( );

	if( iNewMode != GRMODE_NONE )
		SetMode( iNewMode );
}

//=========================================================
//=========================================================
void CINSRules::RoundReset( void )
{
	// make smoke grenades shutup
	for( int i = 0; i < GRENADE_COUNT; i++ )
	{
		const CGrenadeData &GrenadeData = CGrenadeDef::GetGrenadeData( i );

		if( GrenadeData.m_bPrecacheDetonateSound )
			CBaseEntity::StopSound( 0, CHAN_STATIC, GrenadeData.m_szDetonationSound );
	}

	// update area if needed
	if( m_bForcedSquadsOpen != forcesquadopen.GetBool( ) )
	{
		m_bForcedSquadsOpen = forcesquadopen.GetBool( );

		UpdateSquadsArea( );
	}

	// cleanup
	CleanUpEntities( );

	// reset the objectives
	if( IsUsingObjectives( ) )
	{
		for( int i = 0; i < INSRules( )->GetObjectiveCount( ); i++ )
		{
			CINSObjective *pObjective = INSRules( )->GetObjective( i );

			pObjective->Reset( );
			pObjective->CreateWeaponCaches( );
		}

		// create the caches
		CreateWeaponCaches( );
	}

	if (m_bAwardRoundPoints)
	{
		m_bAwardRoundPoints=false;

/*
		for( int i = 1; i <= MAX_PLAY_TEAMS; i++ )
		{
			CPlayTeam* pTeam=GetGlobalPlayTeam(i);
			const CUtlVector<CINSSquad*> &aSquads=pTeam->GetSquads();
			int iMax=0;
			CINSSquad* pSquad=NULL;
			for (int j=0;j<aSquads.Count();j++)
			{
				int s=aSquads[j]->GetScore();
				if (s>iMax)
				{
					iMax=s;
					pSquad=aSquads[j];
				}
				else if (s==iMax)
				{
					pSquad=NULL;
				}
			}
			if (pSquad)
			{
				for (int j=0;j<MAX_SQUAD_SLOTS;j++)
				{
					CINSPlayer* pPlayer=pSquad->GetPlayer(j);
					if (pPlayer)
					{
						pPlayer->IncrementStat( PLAYERSTATS_GAMEPTS, MORALE_BEST_SQUAD );
						pPlayer->SendStatNotice(MORALE_BEST_SQUAD,"Leading Squad");
					}
				}
			}
		}
*/
		CUtlVector<CINSPlayer*> apMaxPlayers;
		int iMax=0;
		for ( int i = 0; i < MAX_PLAYERS; i++ )
		{
			CINSPlayer* pPlayer=ToINSPlayer(UTIL_PlayerByIndex(i));
			if (!pPlayer) continue;
			if (pPlayer->GetMorale()>iMax)
			{
				iMax=pPlayer->GetMorale();
				apMaxPlayers.RemoveAll();
				apMaxPlayers.AddToTail(pPlayer);
			}
			else if (pPlayer->GetMorale()==iMax)
			{
				apMaxPlayers.AddToTail(pPlayer);
			}
		}
		for ( int i = 0; i < apMaxPlayers.Count(); i++ )
		{
			apMaxPlayers[i]->IncrementStat( PLAYERSTATS_GAMEPTS, MORALE_MOST_SKILLED );
			apMaxPlayers[i]->SendStatNotice(MORALE_MOST_SKILLED,"Most Skilled");
		}
	}
}

//=========================================================
//=========================================================
void CINSRules::RoundUnfrozen( void )
{
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "round_unfrozen", true );

	if( pEvent )
		gameeventmanager->FireEvent( pEvent );
}

//=========================================================
//=========================================================
void CINSRules::SwapPlayTeams( void )
{
	CUtlVector< CINSPlayer* > TeamPlayers[ MAX_PLAY_TEAMS ];

	// record current teams
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CPlayTeam *pTeam = GetGlobalPlayTeam( i );

		CUtlVector< CINSPlayer* > &Players = TeamPlayers[ TeamToPlayTeam( i ) ];
		Players = pTeam->GetPlayerList( );

		for( int j = 0; j < Players.Count( ); j++ )
		{
			CINSPlayer *pPlayer = Players[ j ];

			if( !pPlayer )
			{
				AssertMsg( false, "CINSRules::SwapPlayTeams, Team has Invalid Player" );
				continue;
			}

			pPlayer->RemoveFromTeam( );
		}
	}

	// add to new team
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CUtlVector< CINSPlayer* > &Players = TeamPlayers[ TeamToPlayTeam( FlipPlayTeam( i ) ) ];

		for( int j = 0; j < Players.Count( ); j++ )
		{
			CINSPlayer *pPlayer = Players[ j ];

			if( !pPlayer )
			{
				AssertMsg( false, "CINSRules::SwapPlayTeams, Invalid Player" );
				continue;
			}

			pPlayer->ChangeTeam( i );
		}
	}
}

//=========================================================
//=========================================================
void CINSRules::UpdateSquadsArea( void )
{
	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
		GetGlobalPlayTeam( i )->UpdateSquadArea( );
}

//=========================================================
//=========================================================
bool CINSRules::ClientConnected( edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
{
	GetVoiceGameMgr( )->ClientConnected( pEntity );

	PlayerConnected( );

	return true;
}

//=========================================================
//=========================================================
void CINSRules::ClientSettingsChanged( CBasePlayer *pPlayer )
{
	ToINSPlayer( pPlayer )->ClientSettingsChanged( );
}

//=========================================================
//=========================================================
bool CINSRules::ClientCommand(CBasePlayer* pBasePlayer, const CCommand& args)
{
	CINSPlayer *pPlayer = ToINSPlayer(pBasePlayer);
	if (!pPlayer)
		return false;
	
	// firstly, send to the voice manager
	if (GetVoiceGameMgr()->ClientCommand(pPlayer, args))
		return true;

	// secondly, send to the player
	if (pPlayer->ClientCommand(args))
		return true;

	const char* pszCommand = args[0];

	if( FStrEq( pszCommand, GRCMD_TEAMSETUP ) )
	{
		// GRCMD_TEAMSETUP "teamid"
		if (args.ArgC() != 2)
			return true;

		SetupPlayerTeam(pPlayer, atoi(args[1]));
		pPlayer->ShowViewPortPanel( PANEL_CHANGETEAM, false );
		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_SQUADSETUP ) )
	{
		// GRCMD_SQUADSETUP "encodedsquad" "whendie"
		if( args.ArgC() != 3 )
			return true;

		if( !pPlayer->OnPlayTeam( ) )
			return true;

		EncodedSquadData_t EncodedSquadData = atoi( args[1] );
		SetupPlayerSquad( pPlayer, false, &EncodedSquadData, atoi( args[2] ) ? true : false );

		// only force a close when running
		pPlayer->ShowViewPortPanel( PANEL_SQUADSELECT, false );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_FULLSETUP ) )
	{
		// GRCMD_TEAMSETUP "teamid" "encodedsquad"
		if( args.ArgC() != 3 )
			return true;

		SetupPlayerTeam( pPlayer, atoi( args[1] ) );

		EncodedSquadData_t EncodedSquadData = atoi( args[1] );
		SetupPlayerSquad( pPlayer, false, &EncodedSquadData, false );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_INVALIDSCRIPTS ) )
	{
		// GRCMD_INVALIDSCRIPTS "playercrc32"
		if( args.ArgC() != 2 )
			return true;
		
		unsigned int iPlayerCRC32 = atoi( args[1] );

		Warning( "*** Invalid Scripts Detected: %s has %u and needs %u\n",
			pPlayer->GetPlayerName( ), iPlayerCRC32, g_ScriptCheckShared.GetScriptCRC32( ) );

		PlayerKick( pPlayer );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_MODIFYLAYOUT ) )
	{
		// GRCMD_MODIFYLAYOUT "layouttype" "id"
		if( args.ArgC() != 3 )
			return true;

		pPlayer->CustomiseLayout( atoi( args[1] ), atoi( args[2] ) );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_CREINFORCE ) )
	{
		// GRCMD_CREINFORCE

		// need to be running around and a commander
		if( !pPlayer->IsCommander( ) || !pPlayer->IsRunningAround( ) )
			return true;

		CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

		if( !pTeam )
			return true;

		// find squad
		int iSquadID = pPlayer->GetSquadID( );

		// do reinforcement
		if( iSquadID == INVALID_SQUAD )
			return true;

		int iEDRet = pTeam->DoEmergencyReinforcement( iSquadID );

		if( iEDRet == EDRET_INVALID )
			return true;

		// send voice
		KeyValues *pData = new KeyValues( "voicedata" );
		pData->SetInt( "rtype", iEDRet );
		UTIL_SendVoice( VOICEGRP_REINFORCE, pPlayer, pData );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_OBJORDERS ) )
	{
		// GRCMD_OBJORDERS "objective"
		if( args.ArgC() != 2 )
			return false;
		
		// ensure they are valid and a commander
		if( !pPlayer->IsRunningAround( ) || !pPlayer->IsCommander( ) )
			return true;

		int iObjectiveID = atoi( args[1] );

		// ensure valid obj
		if( !IsValidObjective( iObjectiveID ) )
			return false;

		// do a bunch of checks
		CINSObjective *pObjective = GetObjective( iObjectiveID );

		if( !pObjective || !pObjective->IsCapturable( ) )
			return true;

		// get player data
		int iTeamID = pPlayer->GetTeamID( );
		CINSSquad *pSquad = pPlayer->GetSquad( );

		if( !pSquad )
		{
			Assert( false );
			return true;
		}

		// make sure it's valid
		if( !IsValidObjOrder( iTeamID, pObjective ) )
			return true;

		// assign orders
		if( !pSquad->AssignObjOrders( pObjective ) )
			return true;

		// get order type and ensure validity
		int iOrderType = UTIL_CaculateObjType( iTeamID, iObjectiveID );

		if( iOrderType == ORDERTYPE_OBJ_NONE )
		{
			Assert( false );
			return true;
		}

		// send voice data
		KeyValues *pData = new KeyValues( "voicedata" );
		pData->SetInt( "order", iOrderType );
		pData->SetInt( "obj", iObjectiveID );
		UTIL_SendVoice( VOICEGRP_ORDER_OBJ, pPlayer, pData );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_UNITORDERS ) )
	{
		// GRCMD_UNITORDERS "type" "x" "y" "z"
		if( args.ArgC() < 5 )
			return true;

		// ensure they are valid and a commander
		if( !pPlayer->IsRunningAround( ) || !pPlayer->IsCommander( ) )
			return true;

		int iOrderType;
		Vector vecPosition;
		
		iOrderType = atoi( args[1] );
		vecPosition.x = atof( args[2] );
		vecPosition.y = atof( args[3] );
		vecPosition.z = atof( args[3] );

		if( !CUnitOrder::IsValidOrder( iOrderType ) )
			return true;
		
		CPlayTeam *pTeam = pPlayer->GetPlayTeam( );
		CINSSquad *pSquad = pPlayer->GetSquad( );

		if( !pTeam || !pSquad )
		{
			Assert( false );
			return true;
		}

		// assign unit orders
		pSquad->AssignUnitOrders( iOrderType, vecPosition );

		// send voice data
		KeyValues *pData = new KeyValues( "voicedata" );
		pData->SetInt( "order", iOrderType );
		pData->SetInt( "posx", RoundFloatToInt( vecPosition.x ) );
		pData->SetInt( "posy", RoundFloatToInt( vecPosition.y ) );
		pData->SetInt( "posz", RoundFloatToInt( vecPosition.z ) );

		const char *pszPlaceName = UTIL_FindPlaceName( vecPosition );

		if( pszPlaceName && *pszPlaceName )
			pData->SetString( "area", pszPlaceName );
	
		UTIL_SendVoice( VOICEGRP_ORDER_UNIT, pPlayer, pData );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_PLAYERORDERS ) )
	{
		// GRCMD_PLAYERORDERS "type"
		if( args.ArgC() != 2 )
			return true;

		// need to be running around and a command
		if( !pPlayer->IsCommander( ) || !pPlayer->IsRunningAround( ) )
			return true;

		// find squad, assign and send voice
		CINSSquad *pSquad = pPlayer->GetSquad( );

		if( pSquad )
			pSquad->AssignPlayerOrders( atoi( args[1] ) );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_REGLEADER ) )
	{
		if( args.ArgC() != 2 )
			return true;

		// ensure its in clan mode
		if( !m_bClanMode )
			return true;

		// check that a clan leader hasn't already registered
		CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

		if( !pTeam )
			return true;

		CINSPlayer *pClanPlayer = pTeam->GetClanLeader( );
		const char *pszReturnMsg;

		if( pClanPlayer )
		{
			pszReturnMsg = "Clan-Leader has already Registered\n";
		}
		else
		{
			pTeam->RegisterClanLeader( pPlayer );
			pszReturnMsg = "You have been Registered as the Clan-Leader\n";
		}

		ClientPrint( pPlayer, HUD_PRINTCONSOLE, pszReturnMsg );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_CLANREADY ) )
	{
		// ensure its in clan mode
		if( !m_bClanMode )
			return true;

		// check that its the clan leader
		CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

		if( !pTeam || pTeam->GetClanLeader( ) != pPlayer )
			return true;

		// update gamerules
		RunningMode( )->UpdateClanStatus( pTeam );

		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_ACCEPTPROMO ) )
	{
		pPlayer->GetSquad()->PromotionResponse(pPlayer,true);
		return true;
	}
	else if( FStrEq( pszCommand, GRCMD_DECLINEPROMO ) )
	{
		pPlayer->GetSquad()->PromotionResponse(pPlayer,false);
		return true;
	}

#ifdef _DEBUG

	else if( FStrEq( pszCommand, "forcewin" ) )
	{
		// GRCMD_FORCEWIN "team" "wintype" ( "winentity" )
		if( args.ArgC() < 3 )
			return true;

		if( !IsModeRunning( ) )
			return true;

		int iTeamID, iWinType;
		iTeamID = atoi( args[1] );
		iWinType = atoi( args[2] );

		int iWinningEntity = -1;

		if( args.ArgC() == 4 )
			iWinningEntity = atoi( args[3] );

		CBaseEntity *pWinningEntity = NULL;

		if( iWinningEntity != -1 )
		{
			if( iWinType == ENDGAME_WINCONDITION_OBJ )
				pWinningEntity = GetObjective( iWinningEntity );
			else if( iWinType == ENDGAME_WINCONDITION_DEATH )
				pWinningEntity = UTIL_PlayerByIndex( iWinningEntity );
		}

		RunningMode( )->RoundWon( iTeamID, iWinType, pWinningEntity, true );

		return true;
	}

#endif

	return false;
}

//=========================================================
//=========================================================
void CINSRules::ClientDisconnected( edict_t *pClient )
{
	if( !pClient )
		return;

	CINSPlayer *pPlayer = ToINSPlayer( CBaseEntity::Instance( pClient ) );

	if( !pPlayer )
		return;

	// store the players old team
	CPlayTeam *pOldPlayTeam = pPlayer->GetPlayTeam( );

	// update stats
	//GetINSStats( )->LogoutPlayer( pPlayer );

	// he's not connected anymore
	pPlayer->SetConnected( false );
	PlayerDisconnected( );

	// destroy all of the players weapons and items
	pPlayer->RemoveAllItems( );

	// remove from team etc
	pPlayer->ExecuteRemoveCommon( );

	// ensure that the player wasn't the last one standing
	if( pOldPlayTeam && IsModeRunning( ) )
		RunningMode( )->CheckDeathRoundWin( pOldPlayTeam );

	// kill off view model entities
	pPlayer->DestroyViewModels( );

	// check we don't have to start waiting
	HandlePlayerCount( );
}

//=========================================================
//=========================================================
CBaseEntity *CINSRules::GetPlayerSpawnSpot( CBasePlayer *pPlayer )
{
	CINSPlayer *pINSPlayer = ToINSPlayer( pPlayer );

	// spawn in a viewpoint when not a normal spawn
	if( pINSPlayer->LastSpawnType( ) != PSPAWN_NONE )
		return GetViewpoint( );

	// otherwise spawn in an objective
	return GetObjectiveSpawnPoint( pINSPlayer );
}

//=========================================================
//=========================================================
void CINSRules::EchoSpawnError( CINSPlayer *pPlayer ) const
{
#ifdef TESTING

	ClientPrint( pPlayer, HUD_PRINTCENTER, "Spawned in Invalid Spawn" );

#endif
}

//=========================================================
//=========================================================
void CINSRules::PlayerInitialSpawn( CINSPlayer *pPlayer )
{
#ifndef STATS_PROTECTION
	if( forceautoassign.GetBool( ) )
		SetupPlayerTeam( pPlayer, TEAMSELECT_AUTOASSIGN );
	else
		pPlayer->ShowViewPortPanel( PANEL_CHANGETEAM, true );
#endif
}

//=========================================================
//=========================================================
CBaseEntity *CINSRules::GetLastSpawnOption( CINSPlayer *pPlayer ) const
{
	EchoSpawnError( pPlayer );

	return GetLastSpawnOption( );
}

//=========================================================
//=========================================================
CBaseEntity *CINSRules::GetLastSpawnOption( void ) const
{
	// PNOTE: no point in it doing anything clever in this function
	// because it should never reach here for properly configured maps

	if( m_pFallbackSpawn )
		return m_pFallbackSpawn;

	CSpawnPoint *pSpawnPoint = CSpawnPoint::GetFirstSpawn( );

	if( !pSpawnPoint )
		return pSpawnPoint;

	return CBaseEntity::Instance( INDEXENT( 0 ) );
}

//=========================================================
//=========================================================
void CINSRules::PlayerSpawn( CBasePlayer *pPlayer )
{
	// this world has been touched
	m_bVirginWorld = false;

	// tell the current mode
	CurrentMode( )->PlayerSpawn( ToINSPlayer( pPlayer ) );

	// if an invalid IMC, tell the player
	if( m_iError != 0 )
	{
		if( IMCConfig( )->UsingDefaults( ) )
			ClientPrint( pPlayer, HUD_PRINTCENTER, "Game using Invalid IMC" );
		else
			ClientPrint( pPlayer, HUD_PRINTCENTER, "Game using an Invalid IMC in Context" );
	}
}

//=========================================================
//=========================================================
int CINSRules::PlayerSpawnType( CINSPlayer *pPlayer )
{
	if( pPlayer->OnPlayTeam( ) && CurrentMode( )->PlayerViewpointSpawn( ) )
		return PSPAWN_VIEWPOINT;

	int iSpawnType = pPlayer->GetTeam( )->PlayerSpawnType( );
	
	if( iSpawnType != PSPAWN_NONE )
		return iSpawnType;

	if( IsModeRunning( ) )
	{
		if( !pPlayer->OnPlayTeam( ) )
			return PSPAWN_OBSERVER;

		// spawn in a viewpoint when changing team
		if( pPlayer->HasChangedTeam( ) )
			return PSPAWN_VIEWPOINT;

		// allow a spawn when waiting for players or had not a reinforcement
		if( RunningMode( )->IsWaitingForPlayers( ) || !pPlayer->GetPlayTeam( )->HadReinforcement( ) )
			return PSPAWN_NONE;

		// check this is a reinforcement
		CPlayTeam *pTeam = pPlayer->GetPlayTeam( );

		if( !pTeam )
			return PSPAWN_OBSERVER;

		if( pTeam->IsDeploying( ) )
			return PSPAWN_NONE;

		return PSPAWN_OBSERVER;
	}

	return PSPAWN_VIEWPOINT;
}

//=========================================================
//=========================================================
bool CINSRules::PlayerCanCommunicate( CBasePlayer *pListener, CBasePlayer *pSpeaker )
{
	return ( deadchat.GetBool( ) ? true : ( pListener->IsAlive( ) == pSpeaker->IsAlive( ) ) );
}

//=========================================================
//=========================================================
void CINSRules::PlayerKilled( CBasePlayer *pVictim, const CTakeDamageInfo &info )
{
	if( !IsModeRunning( ) )
	{
		Assert( false );	
		return;
	}

	// find	the	killer and the scorer
	CINSPlayer *pINSVictim = ToINSPlayer( pVictim );

	CBaseEntity	*pInflictor, *pKiller;
	pInflictor = info.GetInflictor( );
	pKiller = info.GetAttacker( );

	CINSPlayer *pScorer = NULL;
	
	if( pInflictor )
		pScorer = ToINSPlayer( pInflictor->GetScorer( ) );

	// work out deathtype
	int iDeathType = PDEATHTYPE_KIA;

	if( ( info.GetDamageType( ) & DMG_DEVELOPER ) != 0 )
		iDeathType = PDEATHTYPE_SOULSTOLEN;
	else if( ( info.GetDamageType( ) & DMG_TELEFRAG ) != 0 )
		iDeathType = PDEATHTYPE_TELEFRAG;
	else if( !pScorer || pScorer == pVictim )
		iDeathType = PDEATHTYPE_SELF;
	else if( PlayerRelationship( pINSVictim, pScorer ) == GR_FRIEND )
		iDeathType = PDEATHTYPE_FF;

	// send off an event
	IGameEvent *pEvent = gameeventmanager->CreateEvent( "player_death", true );

	if( pEvent )
	{
		pEvent->SetInt( "userid", pVictim->GetUserID( ) );

		if( pKiller->IsPlayer( ) )
			pEvent->SetInt( "attacker", ( ( CBasePlayer* )info.GetAttacker( ) )->GetUserID( ) );
		else
			pEvent->SetInt( "attacker", 0 );

		pEvent->SetInt( "type", iDeathType );
		pEvent->SetInt( "nodeath", m_bIgnoreNextDeath );
		gameeventmanager->FireEvent( pEvent );
	}

	if( m_bIgnoreNextDeath )
	{
		pINSVictim->DeathIgnored( );

		m_bIgnoreNextDeath = false;
		return;
	}

	// fade the screen black
	pINSVictim->SetCanShowDeathMenu( );

	// add a death
	pINSVictim->BumpStat( PLAYERSTATS_DEATHS );

	// calculate scores for everyone
	CalculateScores( pINSVictim, pScorer );

	// allow the scorer to immediately paint a decal
	if( pScorer )
	{
		// ... and possibly award a kill
		if( pScorer != pVictim )
		{
			CINSPlayer *pINSScorer = ToINSPlayer( pScorer );
			pINSScorer->BumpStat( PLAYERSTATS_KILLS );

			if( PlayerRelationship( pScorer, pVictim ) == GR_FRIEND )
			{
				pINSScorer->BumpStat( PLAYERSTATS_FKILLS );
				pINSScorer->SendStatNotice(MORALE_TEAM_KILL,"Friendly Fire");

				INSRules( )->PlayerTK( pINSScorer );

				// punish?
				if( PunishTK( ) )
				{
					pINSScorer->PunishTK( );

					// send a hint about fate
					UTIL_SendHint( pINSScorer, HINT_TKPUNISH );
				}
			}

			CBaseCombatWeapon *pWeapon = dynamic_cast< CBaseCombatWeapon* >( pInflictor );

			if( pWeapon )
				pINSScorer->BumpWeaponStat( pWeapon->GetWeaponID( ), WEAPONSTATS_FRAGS );
		}
	}

	// setup killed info
	int iDistance, iInflictorType, iInflictorID;

	iDistance = 0;
	iInflictorType = INVALID_INFLICTORTYPE;
	iInflictorID = INVALID_INFLICTORID;

	if( pInflictor && !pInflictor->IsWorld( ) )
	{
		// ... find distance
		if( pInflictor->IsInflictorDistance( ) )
			iDistance = max( 0, RoundFloatToInt( UTIL_3DCaculateDistance( pInflictor, pVictim ) * METERS_PER_INCH ) );

		// ... get inflicter stuff
		iInflictorType = pInflictor->GetInflictorType( );
		iInflictorID = pInflictor->GetInflictorID( );
	}

	// send it off now, so when the "show it" is sent - no weird layout stuff occurs
	pINSVictim->SendDeathInfo( iDeathType, pScorer, iDistance, iInflictorType, iInflictorID, info.GetDamageType( ) );

	// inform the others
	CPlayTeam *pVictimTeam = pINSVictim->GetPlayTeam( );

	// check that their entire team isn't dead
	if( pVictimTeam )
	{
		pVictimTeam->PlayerKilled( pINSVictim );

		RunningMode( )->CheckDeathRoundWin( pVictimTeam, pScorer );
	}
}

//=========================================================
//=========================================================
#define KILLPOINTS_KILL 1
#define KILLPOINTS_SERIOUS 2
#define KILLPOINTS_MODERATE 1

#define KILLPOINTS_MAX 3

void CINSRules::CalculateScores( CINSPlayer *pVictim, CINSPlayer *pScorer )
{

	// add kill points
	if( pScorer )
		pScorer->IncrementStat( PLAYERSTATS_KILLPTS, KILLPOINTS_KILL * CalculateScoreFactor( pVictim, pScorer->GetTeamID( ) ) );

	// loop through all damage taken
	int m_iScoreTotals[ MAX_PLAYERS + 1 ];
	memset( &m_iScoreTotals, 0, sizeof( m_iScoreTotals ) );

	for( int i = 0; i < pVictim->GetDamageTakenCount( ); i++ )
	{
		const DamageTaken_t &DamageTaken = pVictim->GetDamageTaken( i );

		CINSPlayer *pPlayer = ToINSPlayer( DamageTaken.m_hPlayer );

		if( !pPlayer )
			continue;

		// find the score total
		int &iScoreTotal = m_iScoreTotals[ pPlayer->entindex( ) ];
		
		// work out the score they hope to get
		int iScore = ( ( DamageTaken.m_iDamageType == DAMAGEINFO_SERIOUS ) ? KILLPOINTS_SERIOUS : KILLPOINTS_MODERATE ) * CalculateScoreFactor( pVictim, DamageTaken.m_iTeamID );

		// limit score
		if( ( iScoreTotal + iScore ) >= KILLPOINTS_MAX )
		{
			iScore = KILLPOINTS_MAX - iScoreTotal;

			if( iScore <= 0 )
				continue;
		}

		// increment player and total score
		pPlayer->IncrementStat( PLAYERSTATS_KILLPTS, iScore );
		iScoreTotal += iScore;
	}
}

//=========================================================
//=========================================================
int CINSRules::CalculateScoreFactor( CINSPlayer *pVictim, int iScorerTeamID )
{
	return ( TeamRelationship( pVictim, iScorerTeamID ) == GR_ENEMY ) ? 1 : -1;
}

//=========================================================
//=========================================================
void CINSRules::PlayedFinishedDeathThink( CINSPlayer *pPlayer )
{
	if( pPlayer->HasChangedSquad( ) && IsModeRunning( ) && RunningMode( )->IsWaitingForPlayers( ) )
		pPlayer->Spawn( );
}

//=========================================================
//=========================================================
void CINSRules::PlayerKick( CBasePlayer *pPlayer )
{
	const char *pszPlayerName = pPlayer->GetPlayerName( );

	if( !pszPlayerName || *pszPlayerName == 0 )
		return;

	char szKickString[ 64 ];
	Q_snprintf( szKickString, sizeof( szKickString ), "kick %s\n", pszPlayerName );
	engine->ServerCommand( szKickString );
}

//=========================================================
//=========================================================
void CINSRules::PlayerKickBots( bool bCheckLAN )
{
#ifdef TETING

	if( bCheckLAN )
		return;

#endif

	if( bCheckLAN && sv_lan->GetBool( ) )
		return;

	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CINSPlayer *pPlayer = ToINSPlayer( UTIL_PlayerByIndex( i ) );

		if( pPlayer && pPlayer->IsBot( ) )
			INSRules( )->PlayerKick( pPlayer );
	}
}

//=========================================================
//=========================================================
bool CINSRules::PlayerLiveScores( void ) const
{
	return ( !IsModeRunning( ) || ( IsModeRunning( ) && ( RunningMode( )->IsEnding( ) || RunningMode( )->IsWarmingup( ) ) ) );
}

//=========================================================
//=========================================================
void CINSRules::PlayerConnectionUpdate( bool bConnected )
{
	if( bConnected )
		m_iConnectedPlayers++;
	else
		m_iConnectedPlayers--;
}

//=========================================================
//=========================================================
int CINSRules::PlayerCalculateMorale( const CGamePlayerStats &Stats )
{
	// see 'http://forums.insmod.net/index.php?showtopic=370' for original formula
	return ( Stats.m_iKillPoints * MORALE_KILL ) + ( Stats.m_iFriendlyKills * MORALE_TEAM_KILL ) + Stats.m_iGamePoints;
}

//=========================================================
//=========================================================
void CINSRules::PlayerTK( CINSPlayer *pPlayer )
{
	bool bKickPlayer = false;

	const CGamePlayerStats &PlayerStats = pPlayer->GetStats( );

	if( tkdetect.GetBool( ) && PlayerStats.m_iFriendlyKills > 0 && pPlayer->GetMorale( ) < 0 )
	{
		// PNOTE: comment back in for 1.2
		// bKickPlayer = true;
	}
	else
	{
		int iTKRemove = tkremove.GetInt( );

		if( iTKRemove > 0 && PlayerStats.m_iFriendlyKills >= iTKRemove )
			bKickPlayer = true;
	}

	if( bKickPlayer )
		PlayerKick( pPlayer );
}

//=========================================================
//=========================================================
void CINSRules::PlayerViewpointAdd( CINSPlayer *pPlayer )
{
	m_ViewpointPlayers.AddToTail( pPlayer );

	if( m_flViewpointRotateTime == 0.0f )
		UpdateViewpointRotateTime( );
}

//=========================================================
//=========================================================
void CINSRules::PlayerViewpointRemove( CINSPlayer *pPlayer )
{
	m_ViewpointPlayers.FindAndRemove( pPlayer );

	if( m_ViewpointPlayers.Count( ) == 0 )
		m_flViewpointRotateTime = 0.0f;
}

//=========================================================
//=========================================================
void CINSRules::RagdollSpawn( CINSRagdoll *pRagdoll )
{
	CurrentMode( )->RagdollSpawn( pRagdoll );
}

//=========================================================
//=========================================================
CBaseEntity *CINSRules::GetViewpoint( void )
{
	if( CViewPoint::CountPoints( ) == 0 )
		return GetLastSpawnOption( );

	Assert( m_iCurrentViewpoint != INVALID_VIEWPOINT );

	return CViewPoint::GetPoint( m_iCurrentViewpoint );
}

//=========================================================
//=========================================================
void CINSRules::UpdateCurrentViewpoint( void )
{
	int iViewpointCount = CViewPoint::CountPoints( );

	if( iViewpointCount == 0 )
		return;

	// find new point
	if( m_iCurrentViewpoint != INVALID_VIEWPOINT )
	{
		if( m_iCurrentViewpoint >= iViewpointCount - 1 )
			m_iCurrentViewpoint = 0;
		else
			m_iCurrentViewpoint++;
	}
	else
	{
		m_iCurrentViewpoint = random->RandomInt( 0, iViewpointCount - 1 );
	}

	// update network
	NetworkStateChanged( );

	// update time
	if( m_iConnectedPlayers > 0 )
	{
		// set new time
		UpdateViewpointRotateTime( );

		// update players
		for( int i = 0; i < m_ViewpointPlayers.Count( ); i++ )
		{
			CINSPlayer *pPlayer = m_ViewpointPlayers[ i ];

			if( pPlayer )
				SendCurrentViewpoint( pPlayer );
		}
	}
}

//=========================================================
//=========================================================
void CINSRules::UpdateViewpointRotateTime( void )
{
	m_flViewpointRotateTime = gpGlobals->curtime + VIEWPOINT_ROTATE;
}

//=========================================================
//=========================================================
void CINSRules::SendCurrentViewpoint( CINSPlayer *pPlayer )
{
	SendViewpoint( pPlayer, m_iCurrentViewpoint, true );
}

//=========================================================
//=========================================================
void CINSRules::SendViewpoint( CINSPlayer *pPlayer, int iViewpointID, bool bHacked )
{
	CBaseEntity *pViewPoint = CViewPoint::GetPoint( iViewpointID );

	Vector ViewPointOrigin = pViewPoint->GetAbsOrigin( );
	QAngle ViewPointAngle = pViewPoint->GetLocalAngles( );

	if( bHacked )
	{
		/*CSingleUserRecipientFilter filter( pPlayer );
		filter.MakeReliable( );

		UserMessageBegin( filter, "ViewPoint" );
			WRITE_FLOAT( ViewPointOrigin[ 0 ] );
			WRITE_FLOAT( ViewPointOrigin[ 1 ] );
			WRITE_FLOAT( ViewPointOrigin[ 2 ] );

			WRITE_FLOAT( ViewPointAngle[ 0 ] );
			WRITE_FLOAT( ViewPointAngle[ 1 ] );
			WRITE_FLOAT( ViewPointAngle[ 2 ] );
		MessageEnd();*/
	}
	else
	{
		pPlayer->SetAbsOrigin( ViewPointOrigin );
		pPlayer->SnapEyeAngles( ViewPointAngle, FIXANGLE_ABSOLUTE );
	}
}

//=========================================================
//=========================================================
const Vector &CINSRules::GetCurrentViewpointOrigin( void )
{
	return GetViewpoint( )->GetAbsOrigin( );
}

//=========================================================
//=========================================================
const QAngle &CINSRules::GetCurrentViewpointAngle( void )
{
	return GetViewpoint( )->GetAbsAngles( );
}

//=========================================================
//=========================================================
bool CINSRules::ShouldMapRotate( void )
{
	// must make sure it's allowed first
	if( !CurrentMode( )->AllowGameEnd( ) )
		return false;

	// check for having passed the timelimit
	float flTimeLimit = timelimit.GetFloat( );

	if( flTimeLimit > 0.0f && gpGlobals->curtime >= ( flTimeLimit * 60.0f ) )
		return true;

	// check if the game has ended
	if( CurrentMode( )->ShouldGameEnd( ) )
		return true;

	return false;
}

//=========================================================
//=========================================================
#define ERROR_REPEAT 2.5f

void CINSRules::EchoRepeatingError( void )
{
	if( m_iError <= 0 || m_flErrorTime >= gpGlobals->curtime )
		return;

	m_flErrorTime = engine->IsDedicatedServer( ) ? FLT_MAX : ( gpGlobals->curtime + ERROR_REPEAT );

	EchoError( IMCConfig( )->UsingDefaults( ) ? "imc" : "imc context", g_pszErrorMessages[ m_iError - 1 ] );
}

//=========================================================
//=========================================================
void CINSRules::ResetError( void )
{
	m_iError = 0;
}

//=========================================================
//=========================================================
void CINSRules::SetError( int iID )
{
	m_iError = iID;
}

//=========================================================
//=========================================================
void CINSRules::EchoError( const char *pszContext, const char *pszError )
{
	char szWarning[ 128 ];
	Q_snprintf( szWarning, sizeof( szWarning ), "*** %s error: %s\n", pszContext, pszError );

	Warning( szWarning );
}

//=========================================================
//=========================================================
void CINSRules::ExecServerConfig( void )
{
	const char *pszCFGFile = NULL;

	if( engine->IsDedicatedServer( ) )
		pszCFGFile = servercfgfile.GetString( );
	else
		pszCFGFile = lservercfgfile.GetString( );

	if( pszCFGFile && *pszCFGFile != '\0' )
	{
		Msg( "Executing Server Config\n" );

		char szCommand[ 64 ];
		Q_snprintf( szCommand, sizeof( szCommand ), "exec %s\n", pszCFGFile );
		engine->ServerCommand( szCommand );
	}
}

//=========================================================
//=========================================================
void CINSRules::CleanUpEntities( void )
{
	if( m_bVirginWorld )
		return;

	// remove all entities 
	CBaseEntity *pCurrent = gEntList.FirstEnt( );

	while( pCurrent )
	{
		CBaseCombatWeapon *pWeapon = dynamic_cast< CBaseCombatWeapon* >( pCurrent );

		if ( pWeapon )
		{
			// only remove weapons without owners
			if( pWeapon->GetOwner( ) != NULL )
				UTIL_Remove( pCurrent );
		}
		else
		{
			const char *pszClassname = pCurrent->GetClassname( );

			if( UTIL_FindPrefixInList( g_pszRemoveGroups, pszClassname ) || UTIL_FindInList( g_pszRemoveEnts, pszClassname ) )
				UTIL_Remove( pCurrent );
		}

		pCurrent = gEntList.NextEnt( pCurrent );
	}

	// really remove the entities so we can have access to their slots below
	gEntList.CleanupDeleteList( );

	// cancel all queued events
	g_EventQueue.Clear( );

	// respawn them
	m_EntityFilter.Setup( );
	MapEntity_ParseAllEntities( engine->GetMapEntitiesString( ), &m_EntityFilter, true );

	// we have our flower back :)
	m_bVirginWorld = true;

	// tell the current mode
	CurrentMode( )->EntitiesCleaned( );
}

//=========================================================
//=========================================================
bool CINSRules::ShouldForceDeathFade( void ) const
{
	return ( IsModeRunning( ) && ( RunningMode( )->GetDeadCamMode( ) != OBS_ALLOWMODE_ALL || RunningMode( )->IsEnding( ) ) );
}

//=========================================================
//=========================================================
bool CINSRules::DetonationsAllowed( void ) const
{
	return ( IsModeRunning( ) ? RunningMode( )->DetonationsAllowed( ) : false );
}

//=========================================================
//=========================================================
void CINSRules::UpdateClanMode( void )
{
	bool bOldClanMode = clanmode.GetBool( );
	m_bClanMode = clanmode.GetBool( );

	if( m_bClanMode != bOldClanMode )
	{
		for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
			GetGlobalPlayTeam( i )->RegisterClanLeader( NULL );
	}
}

//=========================================================
//=========================================================
bool CINSRules::ChangeLevel( const char *pszMapName, int iProfileID )
{
	if( !pszMapName )
		return false;

	if( iProfileID < 0 || iProfileID >= MAX_IMC_PROFILES )
		return false;

	CIMCConfig::SetNextProfile( iProfileID );

	engine->ChangeLevel( pszMapName, NULL );

	return true;
}

//=========================================================
//=========================================================
bool CINSRules::PunishTK( void ) const
{
	return tkpunish.GetBool( );
}

//=========================================================
//=========================================================
bool CINSRules::CanSabotageCaches( void ) const
{
	return cachesabotage.GetBool( );
}

//=========================================================
//=========================================================
void CINSRules::DrawBadSpawns( void )
{
	for( int i = 0; i < m_BadSpawns.Count( ); i++ )
		DrawSpawnDebug_Entity( COLOR_BLACK, m_BadSpawns[ i ], true );
}

//=========================================================
//=========================================================
bool CINSRules::AllowBotAdd( void ) const
{
#ifdef TESTING

	return true;

#endif

	if( sv_lan->GetBool( ) )
		return true;

	return ( CurrentMode( )->AllowBotAdd( ) );
}

//=========================================================
//=========================================================
CON_COMMAND( ins_serverversion, "prints the server version" )
{
	Msg( "Server is Running: %s\n", CINSRules::GetGameVersion( ) );
}
