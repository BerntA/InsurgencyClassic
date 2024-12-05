//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hud_macros.h"
#include "iclientmode.h"

#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

#include "hud_icontext.h"

#include "inshud.h"
#include "basic_colors.h"
#include "play_team_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CHUDPlayerStatus : public CHUDIconText
{
	DECLARE_CLASS_SIMPLE( CHUDPlayerStatus, CHUDIconText );

public:
	CHUDPlayerStatus( const char *pszElementName );

	void MsgFunc_PlayerStatus( bf_read &msg );

private:
	void Init( void );
	void LevelInit( void );

	const char *IconPath( void ) const { return "istatus"; }
	int IconID( void ) const;
	void Update( void );

	void SetStatus( int iType, int iID );
	void LoadIcons( void );

private:
	bool m_bLoadedIcons;

	int m_iIconIDs[ MAX_PLAY_TEAMS ][ RANK_COUNT ];
	int *m_pCurrentIconID;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHUDPlayerStatus );
DECLARE_HUD_MESSAGE( CHUDPlayerStatus, PlayerStatus );

//=========================================================
//=========================================================
CHUDPlayerStatus::CHUDPlayerStatus( const char *pszElementName )
	: CHUDIconText( pszElementName, "HudPlayerStatus" )
{	
	m_pCurrentIconID = NULL;
}

//=========================================================
//=========================================================
void CHUDPlayerStatus::Init( void )
{
	BaseClass::Init( );

	HOOK_HUD_MESSAGE( CHUDPlayerStatus, PlayerStatus );
}

//=========================================================
//=========================================================
void CHUDPlayerStatus::LevelInit( void )
{
	m_pCurrentIconID = NULL;
	m_bLoadedIcons = false;

	memset( &m_iIconIDs, 0, sizeof( m_iIconIDs ) );
}

//=========================================================
//=========================================================
void CHUDPlayerStatus::MsgFunc_PlayerStatus( bf_read &msg )
{
	int iType, iID;

	iType = msg.ReadByte( );
	iID = msg.ReadByte( );

	SetStatus( iType, iID );
}

//=========================================================
//=========================================================
int CHUDPlayerStatus::IconID( void ) const
{
	if( !m_pCurrentIconID )
		return 0;

	return *m_pCurrentIconID;
}

//=========================================================
//=========================================================
void CHUDPlayerStatus::Update( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	int iType, iID;
	pPlayer->GetStatus( iType, iID );

	SetStatus( iType, iID );
}

//=========================================================
//=========================================================
void CHUDPlayerStatus::LoadIcons( void )
{
	if( m_bLoadedIcons )
		return;

	char szFile[ 64 ];

	for( int i = TEAM_ONE; i <= TEAM_TWO; i++ )
	{
		CTeamLookup *pTeamLookup = GetGlobalPlayTeam( i )->GetTeamLookup( );
		Assert( pTeamLookup );

		for( int j = 0; j < RANK_COUNT; j++ )
		{
			Q_snprintf( szFile, sizeof( szFile ), "%s_%s", pTeamLookup->GetFileName( ), g_pszRankIcons[ j ] );
			m_iIconIDs[ TeamToPlayTeam( i ) ][ j ] = LoadIcon( szFile );
		}
	}

	m_bLoadedIcons = true;
}

//=========================================================
//=========================================================
void CHUDPlayerStatus::SetStatus( int iType, int iID )
{
	const StatusProgressiveData_t *pStatusData = UTIL_StatusProgressiveData( iType, iID );

	if( !pStatusData )
		return;

	SetText( pStatusData->pszText, true );

	// load if needed
	LoadIcons( );

	// reset current icon
	m_pCurrentIconID = NULL;

	// find player and set new icon
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer )
	{
		int iTeamID, iRankID;
		iTeamID = pPlayer->GetTeamID( );
		iRankID = pPlayer->GetRank( );

		if( IsPlayTeam( iTeamID ) && UTIL_ValidRank( iRankID ) )
			m_pCurrentIconID = &m_iIconIDs[ TeamToPlayTeam( iTeamID ) ][ iRankID ];
	}
}