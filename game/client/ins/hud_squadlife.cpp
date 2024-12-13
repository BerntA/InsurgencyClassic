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
#include "c_playerresource.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define REINFORCEMENTS_XGAP 6

//=========================================================
//=========================================================
class CHUDSquadLife : public CHUDIconText, public IINSReinforcement
{
	DECLARE_CLASS_SIMPLE( CHUDSquadLife, CHUDIconText );

public:
	CHUDSquadLife( const char *pszElementName );

private:
	void Init( void );

	void FireGameEvent( IGameEvent *pEvent );

	const char *IconPath( void ) const { return "reinforcements"; }
	int IconID( void ) const { return m_iIconID; }
	int TextXGap( void ) const { return REINFORCEMENTS_XGAP; }
	void Update( void );
	bool OnThinkUpdate( void ) const { return true; }

	void ReinforcementDeployed( int iRemaining );

private:
	int m_iIconID;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHUDSquadLife );

//=========================================================
//=========================================================
CHUDSquadLife::CHUDSquadLife( const char *pszElementName )
	: CHUDIconText( pszElementName, "HudReinforcements" )
{
	gameeventmanager->AddListener( this, "player_death", false );
}

//=========================================================
//=========================================================
void CHUDSquadLife::Init( void )
{
	BaseClass::Init( );

	m_iIconID = LoadIcon( "icon" );
}

//=========================================================
//=========================================================
void CHUDSquadLife::FireGameEvent( IGameEvent *pEvent )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	const char *pszEventName = pEvent->GetName( );

	if( Q_strcmp( pszEventName, "player_death" ) == 0 )
	{
		int iPlayerID = engine->GetPlayerForUserID( pEvent->GetInt( "userid" ) );

		int iLocalTeamID, iLocalSquadID;
		iLocalTeamID = pPlayer->GetTeamID( );
		iLocalSquadID = pPlayer->GetSquadID( );

		if(g_PR->GetTeamID( iPlayerID ) == iLocalTeamID && g_PR->GetSquadID( iPlayerID ) == iLocalSquadID )
			HighlightText( );

		return;
	}
	
	BaseClass::FireGameEvent( pEvent );
}

//=========================================================
//=========================================================
void CHUDSquadLife::Update( void )
{
	char szBuffer[ ICONTEXT_TEXT_LENGTH ];

	int iLeft, iPool;

	if( !GetINSHUDHelper( )->SquadLife( iLeft, iPool ) )
		return;

	Q_snprintf( szBuffer, sizeof( szBuffer ), "%i / %i", iLeft, iPool );

	SetText( szBuffer, false );
}

//=========================================================
//=========================================================
void CHUDSquadLife::ReinforcementDeployed( int iRemaining )
{
	HighlightText( );
}