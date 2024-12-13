// Insurgency Team (C) 2007
// First revision

#include "cbase.h"

#include "insvgui.h"
#include "ins_panel.h"
#include "ins_imagebutton.h"
#include "gameuipanel.h"

#include "vgui/ivgui.h"

#include <game/client/iviewport.h>
#include "basic_colors.h"

#include <keyvalues.h>
#include <vgui_controls/label.h>
#include "vgui/ilocalize.h"
#include <vgui_controls/sectionedlistpanel.h>
#include <c_playerresource.h>

#include "ins_allegiance_content.h"
#include "tier0/memdbgon.h"

using namespace vgui;

#define SECTION_HEADER 0

class CMiniScoreBoardPanel : public Panel
{
	DECLARE_CLASS_SIMPLE( CMiniScoreBoardPanel, Panel );

	int m_iTeam;
	vgui::SectionedListPanel *m_pPlayerList;
	int			m_iPlayerIndexSymbol;

	void InitScoreboardSections( void )
	{
		IGameResources *gr = GameResources( );

		if ( !gr )
			return;

		// setup the team name
		wchar_t *teamName = g_pVGuiLocalize->Find( gr->GetTeamName( m_iTeam ) );
		wchar_t name[64];

		if (!teamName)
		{
			g_pVGuiLocalize->ConvertANSIToUnicode(gr->GetTeamName(m_iTeam), name, sizeof(name));
			teamName = name;
		}

		m_pPlayerList->SetFgColor( Color( 137, 137, 137, 255 ) );
		m_pPlayerList->AddSection( SECTION_HEADER, "" );
		m_pPlayerList->SetSectionAlwaysVisible( SECTION_HEADER, true );
		m_pPlayerList->AddColumnToSection( SECTION_HEADER, "name", "", 0, scheme( )->GetProportionalScaledValue(200) );

		m_pPlayerList->SetSectionFgColor( SECTION_HEADER, COLOR_GREY );

		m_pPlayerList->SetFgColor( Color( 137, 137, 137, 255 ) );
		
		m_pPlayerList->SetFgColor( Color( 137, 137, 137, 255 ) );

		m_pPlayerList->AddSection( 1, "", StaticPlayerSortFunc );
		m_pPlayerList->SetSectionAlwaysVisible( 1, true );

		m_pPlayerList->AddColumnToSection( 1, "name", "", 0, scheme()->GetProportionalScaledValue(80));
		m_pPlayerList->AddColumnToSection( 1, "score", "#PlayerScore", 0, scheme()->GetProportionalScaledValue(25));
		m_pPlayerList->AddColumnToSection( 1, "ping", "Ping", 0, scheme()->GetProportionalScaledValue(10));

		m_pPlayerList->SetItemFgColor( SECTION_HEADER, Color( 137, 137, 137, 255 ) );
		m_pPlayerList->SetItemFgColor( 1, Color( 137, 137, 137, 255 ) );

		m_iPlayerIndexSymbol = KeyValuesSystem( )->GetSymbolForString( "playerIndex" );
	}

	bool GetPlayerScoreInfo( int playerIndex, KeyValues *kv )
	{
		C_PlayerResource* pPR = g_PR;
		if (!pPR )
			return false;

		kv->SetInt("score", pPR->GetMorale( playerIndex ) );
		kv->SetInt("ping", pPR->GetPing( playerIndex ) ) ;
		kv->SetString("name", pPR->GetPlayerName( playerIndex ) );
		kv->SetInt("playerIndex", playerIndex);
		kv->SetInt("kills", pPR->GetFrags( playerIndex ));
		kv->SetInt("deaths", pPR->GetDeaths( playerIndex ));

		return true;
	}


	void FillScoreBoard( void )
	{
		IGameResources *gr = GameResources( );
		if(!gr)
			return;

		wchar_t tn[256];
		char cstring[256];

		sprintf( cstring, "%s - %i", gr->GetTeamName( m_iTeam ), gr->GetTeamScore( m_iTeam ) );
		g_pVGuiLocalize->ConvertANSIToUnicode( cstring, tn, sizeof( tn ) );

		m_pPlayerList->ModifyColumn( 0, "name", tn );
	}

	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
	{
		KeyValues *it1 = list->GetItemData(itemID1);
		KeyValues *it2 = list->GetItemData(itemID2);
		Assert(it1 && it2);

		// first compare score
		int v1 = it1->GetInt("score");
		int v2 = it2->GetInt("score");
		if (v1 > v2)
			return true;
		else if (v1 < v2)
			return false;

		// first compare frags
		v1 = it1->GetInt("frags");
		v2 = it2->GetInt("frags");
		if (v1 > v2)
			return true;
		else if (v1 < v2)
			return false;

		// next compare deaths
		v1 = it1->GetInt("deaths");
		v2 = it2->GetInt("deaths");
		if (v1 > v2)
			return false;
		else if (v1 < v2)
			return true;

		// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
		return itemID1 < itemID2;
	}

public:
	CMiniScoreBoardPanel( int iTeam, Panel *parent, const char *panelName ) : Panel( parent, panelName )
	{
		m_iTeam = iTeam;
		SetPaintBackgroundEnabled( false );

		ivgui( )->AddTickSignal( GetVPanel( ), 1000 );

		SetKeyBoardInputEnabled(false);

		// set the scheme before any child control is created
		SetScheme("ClientScheme");

		m_pPlayerList = new SectionedListPanel(this, "PlayerList");
		m_pPlayerList->SetVerticalScrollbar( true );
		m_pPlayerList->SetVisible( true );
		m_pPlayerList->SetBorder( NULL );

		Setup( );
	}

	int FindItemIDForPlayerIndex( int playerIndex )
	{
		for (int i = 0; i <= m_pPlayerList->GetHighestItemID( ); i++)
		{
			if (m_pPlayerList->IsItemIDValid( i ))
			{
				KeyValues *kv = m_pPlayerList->GetItemData( i );
				kv = kv->FindKey( m_iPlayerIndexSymbol );
				if (kv && kv->GetInt( ) == playerIndex)
					return i;
			}
		}
		return -1;
	}

	virtual void OnTick( void )
	{
		FillScoreBoard( );

		// walk all the players and make sure they're in the scoreboard
		for ( int i = 1; i < gpGlobals->maxClients; i++ )
		{
			IGameResources *gr = GameResources( );

			if ( gr && gr->IsConnected( i ) && gr->GetTeam( i ) == m_iTeam )
			{
				// add the player to the list
				KeyValues *playerData = new KeyValues( "data" );
				GetPlayerScoreInfo( i, playerData );

				const char *oldName = playerData->GetString( "name","" );
				int bufsize = strlen(oldName) * 2 + 1;
				char *newName = (char *)_alloca( bufsize );

				UTIL_MakeSafeName( oldName, newName, bufsize );

				playerData->SetString( "name", newName );

				int itemID = FindItemIDForPlayerIndex( i );
				int playerTeam = gr->GetTeam( i );
				int sectionID = 1;

				if (itemID == -1)
				{
					// add a new row
					itemID = m_pPlayerList->AddItem( sectionID, playerData );
				}
				else
				{
					// modify the current row
					m_pPlayerList->ModifyItem( itemID, sectionID, playerData );
				}

				// set the row color based on the players team
				m_pPlayerList->SetItemFgColor( itemID, Color( 137, 137, 137, 255 ) );

				playerData->deleteThis( );
			}
			else
			{
				// remove the player
				int itemID = FindItemIDForPlayerIndex( i );
				if (itemID != -1)
				{
					m_pPlayerList->RemoveItem( itemID );
				}
			}
		}
	}

	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		// light up scoreboard a bit
		SetBgColor( Color( 0,0,0,0) );
	}

	void Setup( void )
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

		if(!pPlayer)
			return;

		m_pPlayerList->DeleteAllItems( );
		m_pPlayerList->RemoveAllSections( );

		m_pPlayerList->SetBorder( NULL );

		// add all the sections
		InitScoreboardSections( );

		// setup should only be called when data needs to be displayed so ...
		FillScoreBoard( );
	}

	virtual void PerformLayout( void )
	{
		int w = 0, t = 0;
		GetSize( w, t );

		Setup( );

		m_pPlayerList->SetSize( w, t );
		m_pPlayerList->SetPos( 0, 0);
	}
};

Panel *CreateMiniscoreBoardPanel( int iTeam, Panel *pParent, const char *pszPanelName )
{
	return new CMiniScoreBoardPanel( iTeam, pParent, pszPanelName );
}


SquadButton::SquadButton( const char* pszImage, const char* pszImageArmed, const char* pszImageDepressed, const char* pszImageDisabled, Panel *parent, const char *panelName, Panel *pActionSignalTarget, const char *pCmd ) :
ImageButton( pszImage, pszImageArmed, pszImageDepressed, pszImageDisabled, parent, panelName, pActionSignalTarget, pCmd )
{
	m_pScoreboards[0] = m_pScoreboards[1] = NULL;
	m_pSituationPanel = NULL;
	m_pDescriptionPanel = NULL;
}

void SquadButton::OnCursorEntered( void )
{
	SetVisibleContent( true );
}

void SquadButton::OnCursorExited( void )
{
	SetVisibleContent( false );
}

void SquadButton::SetParentPanels( Panel *pScoreBoards[2], Panel *pSituation, Panel *pDescriptionPanel )
{
	m_pScoreboards[0] = pScoreBoards[0]; m_pScoreboards[1] = pScoreBoards[1];
	m_pSituationPanel = pSituation;
	m_pDescriptionPanel = pDescriptionPanel;
}

void SquadButton::SetVisibleContent( bool bState )
{
	if(!m_pDescriptionPanel || !m_pSituationPanel || !m_pScoreboards[0] || !m_pScoreboards[1])
		return;

	m_pScoreboards[0]->SetVisible( !bState );
	m_pScoreboards[1]->SetVisible( !bState );

	m_pSituationPanel->SetVisible( bState );
	m_pDescriptionPanel->SetVisible( bState );
}