//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "insvgui.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "vguicenterprint.h"
#include "hud_macros.h"
#include "pain_helper.h"
#include "ins_player_shared.h"
#include "c_playerresource.h"
#include "inshud.h"
#include "clientmode_hl2mpnormal.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
void __MsgFunc_VGUIHide( bf_read &msg )
{
	GetINSVGUIHelper( )->MassHide( );
}

//=========================================================
//=========================================================
void __MsgFunc_DeathInfo( bf_read &msg )
{
	GetINSVGUIHelper( )->DeathInfoMsg( msg );
}

//=========================================================
//=========================================================
CINSViewportHelper *CINSViewportHelper::m_sHelpers = NULL;

CINSViewportHelper::CINSViewportHelper( CreatePanel_t pfnCreate )
{
	if( m_sHelpers == NULL )
	{
		m_pNext = m_sHelpers;
		m_sHelpers = this;
	}
	else
	{
		CINSViewportHelper *pPrev = m_sHelpers;
		CINSViewportHelper *pCurrent = m_sHelpers->m_pNext;

		while( pCurrent != NULL )
		{
			pPrev = pCurrent;
			pCurrent = pCurrent->m_pNext;
		}

		pPrev->m_pNext = this;
		m_pNext = pCurrent;
	}

	Assert( pfnCreate );
	m_pfnCreate = pfnCreate;
}

//=========================================================
//=========================================================
void CINSViewportHelper::CreateAllElements( CBaseViewport *pViewport )
{
	Assert( pViewport );

	if( !pViewport )
		return;

	CINSViewportHelper *pHelper = m_sHelpers;

	while( pHelper )
	{
		CreatePanel_t pfnCreate = pHelper->m_pfnCreate;
		IViewPortPanel *pPanel = ( pfnCreate )( pViewport );

		Assert( pPanel );

		if( pPanel )
			pViewport->AddNewPanel( pPanel, "??");

		pHelper = pHelper->GetNext();
	}
}

//=========================================================
//=========================================================
bool MenuDataLess( const int &iLeft, const int &iRight )
{
	return ( iLeft > iRight );
}

//=========================================================
//=========================================================
static CINSMenuHelper *g_pMenuHelperList = NULL;

CINSMenuHelper::CINSMenuHelper( int iID, MenuCreatorHelper_t Helper )
{
	m_pNext = g_pMenuHelperList;
	g_pMenuHelperList = this;

	m_iID = iID;
	m_Helper = Helper;
}

//=========================================================
//=========================================================
bool IINSMenuManager::HasActiveMenu( void ) const
{
	return ( GetActiveMenu( ) != INSMENU_INVALID );
}

//=========================================================
//=========================================================
IINSSquadSelection::IINSSquadSelection( )
{
	GetINSVGUIHelper( )->RegisterSquadSelection( this );
}

//=========================================================
//=========================================================
IINSDeathInfo::IINSDeathInfo( )
{
	GetINSVGUIHelper( )->RegisterDeathInfo( this );
}

//=========================================================
//=========================================================
CINSVGUIHelper::CINSVGUIHelper( )
	: m_Menus( MenuDataLess )
{
	m_bValidDeathData = false;

	HOOK_MESSAGE( DeathInfo );

	// for IINSDeathInfo:
	gameeventmanager->AddListener( this, "player_team", false );
	gameeventmanager->AddListener( this, "player_squad", false );
	gameeventmanager->AddListener( this, "round_reset", false );
}

//=========================================================
//=========================================================
CINSVGUIHelper::~CINSVGUIHelper( )
{
	gameeventmanager->RemoveListener( this );
}

//=========================================================
//=========================================================
CINSVGUIHelper *CINSVGUIHelper::GetINSVGUIHelper( void )
{
	static CINSVGUIHelper *pVGUIHelper = NULL;

	if( !pVGUIHelper )
		pVGUIHelper = new CINSVGUIHelper;

	return pVGUIHelper;
}

//=========================================================
//=========================================================
void CINSVGUIHelper::Init( void )
{
	CINSMenuHelper *pCurrent = g_pMenuHelperList;

	while( pCurrent )
	{
		IINSMenu *pMenu = ( *pCurrent->m_Helper )( );
		AssertMsg( pMenu, "INSMenu Declared - but not Created" );

		m_Menus.Insert( pCurrent->m_iID, pMenu );

		pCurrent = pCurrent->m_pNext;
	}
}

//=========================================================
//=========================================================
void CINSVGUIHelper::RegisterMenuManager( IINSMenuManager *pMenuManager )
{
	m_pMenuManager = pMenuManager;
}

//=========================================================
//=========================================================
void CINSVGUIHelper::RegisterSquadSelection( IINSSquadSelection *pSquadSelection )
{
	m_pSquadSelection = pSquadSelection;
}

//=========================================================
//=========================================================
void CINSVGUIHelper::RegisterDeathInfo( IINSDeathInfo *pDeathInfo )
{
	m_pDeathInfo = pDeathInfo;
}

//=========================================================
//=========================================================
bool CINSVGUIHelper::CanShowTeamPanel( void )
{
	if( !gViewPortInterface )
		return false;

	C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer( );

	// ensure sane player
	if( !pLocalPlayer )
		return false;

	// ensure that we can actually show something
	if( !IsGameReady( ) || CINSStats::IsBlocked( pLocalPlayer ) )
		return false;

	// ensure the panel isn't being shown already
	IViewPortPanel *pSquadSelect = gViewPortInterface->FindPanelByName( PANEL_SQUADSELECT );
				
	if( pSquadSelect && pSquadSelect->IsVisible( ) )
		return false;

	// ask the gamerules
	if( INSRules( )->CanChangeTeam( pLocalPlayer ) )
		return true;

	internalCenterPrint->Print( "You Cannot Change your Team at this Time" );

	return false;
}

//=========================================================
//=========================================================
bool CINSVGUIHelper::CanShowSquadPanel( void )
{
	if( !gViewPortInterface )
		return false;

	// ensure the panel isn't being shown already
	IViewPortPanel *pChangeTeam = gViewPortInterface->FindPanelByName( PANEL_SQUADSELECT );

	if( pChangeTeam && pChangeTeam->IsVisible() )
		return false;

	// ask the gamerules
	if( INSRules( )->CanChangeSquad( C_INSPlayer::GetLocalPlayer( ) ) )
		return true;

	internalCenterPrint->Print( "You Cannot Change your Squad at this Time" );

	return false;
}

//=========================================================
//=========================================================
bool CINSVGUIHelper::ShowDeathPanel( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	// ensure sane player
	if( !pPlayer || !pPlayer->OnPlayTeam( ) )
		return false;

	// ensure we're running
	if( !IsGameReady( ) || !INSRules( )->IsModeRunning( ) )
		return false;

	// ensure we're on a play team and the game isn't restarting
	CRunningMode *pRunningMode = INSRules( )->RunningMode( );

	if( !pRunningMode || pRunningMode->GetStatus( GAMERUNNING_RESTARTING ) )
		return false;

	// show panel
	m_pDeathInfo->ShowDeathInfo( );

	return true;
}

//=========================================================
//=========================================================
void CINSVGUIHelper::FireGameEvent( IGameEvent *pEvent )
{
	// every event means the deathinfo should become invalid
	m_bValidDeathData = false;

	// handle event
	const char *pszEvent = pEvent->GetName( );

	// ... all our events have a 'userid'
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer( );
	int iPlayerID = engine->GetPlayerForUserID( pEvent->GetInt( "userid" ) );

	// ... don't continue if not valid player
	if( !pLocalPlayer || pLocalPlayer->entindex( ) != iPlayerID )
		return;

	if( FStrEq( pszEvent, "player_team" ) )
		m_pSquadSelection->TeamUpdate( pEvent->GetInt( "team" ) );
	else if( FStrEq( pszEvent, "player_squad" ) )
		m_pSquadSelection->SquadUpdate( );
}

//=========================================================
//=========================================================
bool CINSVGUIHelper::IsGameReady( void )
{
	return ( INSRules( ) && C_Team::ValidTeams( ) );
}

//=========================================================
//=========================================================
const char *CINSVGUIHelper::GetTeamName( int iTeamID )
{
	return ( g_PR ? g_PR->GetTeamName( iTeamID ) : "unknown" );
}

//=========================================================
//=========================================================
bool CINSVGUIHelper::IsGameRunning( void )
{
	return ( IsGameReady( ) && INSRules()->IsModeRunning( ) );
}

//=========================================================
//=========================================================
void CINSVGUIHelper::JoinFull( int iTeamSelectID, EncodedSquadData_t &EncodedSquadData )
{
	char szGame[ 128 ];

	Q_snprintf( szGame, sizeof( szGame ), "%s %i %i", GRCMD_FULLSETUP, iTeamSelectID, EncodedSquadData );
	engine->ServerCmd( szGame );
}

//=========================================================
//=========================================================
void CINSVGUIHelper::JoinTeam( int iTeamSelectID )
{
	// send team request
	char szFinish[32];
	sprintf( szFinish, "%s %i", GRCMD_TEAMSETUP, iTeamSelectID );
	engine->ServerCmd( szFinish );
}

//=========================================================
//=========================================================
void CINSVGUIHelper::JoinSquad( EncodedSquadData_t &EncodedSquadData, bool bWhenDie )
{
	char szJoin[ 128 ];

	Q_snprintf( szJoin, sizeof( szJoin ), "%s %i %i", GRCMD_SQUADSETUP, EncodedSquadData, bWhenDie ? 1 : 0 );
	engine->ServerCmd( szJoin );
}

//=========================================================
//=========================================================
IINSMenuManager *CINSVGUIHelper::GetMenuManager( void ) const
{
	Assert( m_pMenuManager );
	return m_pMenuManager;
}

//=========================================================
//=========================================================
IINSMenu *CINSVGUIHelper::GetMenu( int iID ) const
{
	int iMenuIndex = m_Menus.Find( iID );

	if( iMenuIndex == m_Menus.InvalidIndex( ) )
		return NULL;

	return m_Menus[ iMenuIndex ];
}

//=========================================================
//=========================================================
void CINSVGUIHelper::DeathInfoMsg( bf_read &Msg )
{
	C_RunningMode *pRunning = INSRules( )->RunningMode( );

	if( !pRunning )
		return;

	// we now have valid data
	m_bValidDeathData = true;

	// read in generic info
	m_DeathInfo.m_iType = Msg.ReadByte( );
	m_DeathInfo.m_iAttackerID = Msg.ReadByte( );

	// send off death effect
	if( m_DeathInfo.m_iType != PDEATHTYPE_SOULSTOLEN )
		g_PainHelper.CreatePain( PAINTYPE_DEATH );

	// handle remaining data depending on the type
	m_DeathInfo.m_bDeathInfoFull = pRunning->IsDeathInfoFull( );

	// set data
	if( m_DeathInfo.m_iType == PDEATHTYPE_KIA || m_DeathInfo.m_iType == PDEATHTYPE_FF )
	{
		if( m_DeathInfo.m_bDeathInfoFull )
		{
			m_DeathInfo.m_iDistance = Msg.ReadWord( );
			m_DeathInfo.m_iInflictorType = Msg.ReadByte( ) - 1;
			m_DeathInfo.m_iInflictorID = Msg.ReadByte( ) - 1;
		}
		else
		{
			m_DeathInfo.m_iDamageType = Msg.ReadWord( );
		}
	}

	// tell the HUD
	GetINSHUDHelper( )->SendDeathUpdate( );

	// update panel
	m_pDeathInfo->UpdateDeathInfo( );
}

//=========================================================
//=========================================================
void CINSVGUIHelper::FinishDeathInfo( void )
{
	engine->ServerCmd( PCMD_FINISHDI );
}

//=========================================================
//=========================================================
void CINSVGUIHelper::MassHide( void )
{
	if (gViewPortInterface == NULL)
		return;
	gViewPortInterface->HideAllPanels();
}

//=========================================================
//=========================================================
/*void CINSVGUIHelper::LoadRankIcons( const char *pszPath, RankIcons_t &RankIcons )
{
	char szPath[ 128 ], szRankPath[ 128 ];
	Q_strncpy( szPath, pszPath, sizeof( szPath ) ); 

	static const char *pszRankIcons[ RANK_COUNT ] = {
		"pvt",	// RANK_PRIVATE
		"lpv",	// RANK_LCORPORAL
		"cpl",	// RANK_CORPORAL
		"sgt",	// RANK_SERGEANT
		"lt"	// RANK_LIEUTENANT
	};

	for( int i = 0; i < RANK_COUNT; i++ )
	{
		Q_strncpy( szRankPath, szRankPath, sizeof( szRankPath ) );
		Q_strncat( szFullRankPath, pszRankIcons[ i ], sizeof( szFullRankPath ) );

		if(CheckVGUIMaterialExists(szFullRankPath))
		{
			m_iRankTexID[i] = vgui::surface()->CreateNewTextureID();
			vgui::surface()->DrawSetTextureFile(m_iRankTexID[i], szFullRankPath, false, false);
		}
	}
}*/

//=========================================================
//=========================================================
bool CINSVGUIHelper::CreateTeamPath( CTeamLookup *pTeam, int iType, char *pszBuffer, int iLength )
{
	if( !pTeam )
		return false;

	static const char *pszTeamVGUI[ VGUI_TEAMPATH_COUNT ] =  {
		"class",
		"endgame",
		"rankicons"
	};

	Q_snprintf( pszBuffer, iLength, "vgui/teams/%s/%s/", pTeam->GetFileName( ), pszTeamVGUI[ iType ] );

	return true;
}

//=========================================================
//=========================================================
bool CINSVGUIHelper::CreateTeamPath( C_PlayTeam *pTeam, int iType, char *pszBuffer, int iLength )
{
	if( !pTeam )
		return false;

	return CreateTeamPath( pTeam->GetTeamLookup( ), iType, pszBuffer, iLength );
}
