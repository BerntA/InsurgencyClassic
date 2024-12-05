#include "cbase.h"

#include "insvgui.h"
#include "ins_panel.h"
#include "ins_imagebutton.h"
#include "gameuipanel.h"

#include "ins_allegiance_content.h"

#include "tier0/memdbgon.h"

#ifdef INSVGUI_FINAL_CHANGETEAM

CONTROL_SIZE(BUTTON_SQUAD1,93,266,191,61);
CONTROL_SIZE(BUTTON_SQUAD2,93,331,191,61);
CONTROL_SIZE(BUTTON_CELL1,93,481,191,61);
CONTROL_SIZE(BUTTON_CELL2,93,544,191,61);
CONTROL_SIZE(BUTTON_RANDOM,98,613,186,39);
CONTROL_SIZE(BUTTON_SPECTATE,98,659,186,39);

CONTROL_SIZE(SCOREBOARD1,335,236,272,238);
CONTROL_SIZE(SCOREBOARD2,618,236,272,238);

CONTROL_SIZE(SITUATION,346,570,294,51);
CONTROL_SIZE(SQUADDESC,346,236,544,238);


class CAllegiance : public CINSPanel {
	DECLARE_CLASS_SIMPLE( CAllegiance, CINSPanel );

public:
	CAllegiance(void*);

	void PerformLayout();

	void Reset();

	void OnCommand( const char *pszCommand );

private:
	ImageButton* m_pSquad1;
	ImageButton* m_pSquad2;
	ImageButton* m_pCell1;
	ImageButton* m_pCell2;
	ImageButton* m_pRandom;
	ImageButton* m_pSpectate;

	Panel* m_pScoreboards[2];

	Label* m_pSituation;
	Label* m_pSquadDescription;
};

CREATE_INSVIEWPORT_PANEL( CAllegiance );

CAllegiance::CAllegiance(void*) : CINSPanel( NULL, PANEL_CHANGETEAM, "allegiance" )
{
	m_pScoreboards[0] = CreateMiniscoreBoardPanel( TEAM_ONE, this, "Scoreboard1");
	m_pScoreboards[0]->SetVisible( true );
	m_pScoreboards[1] = CreateMiniscoreBoardPanel( TEAM_TWO, this, "Scoreboard2" );
	m_pScoreboards[1]->SetVisible( true );

	m_pSituation = new Label( this, "Situation", "" ); 
	m_pSquadDescription = new Label( this, "Desc", "" );

	m_pSquad1 = new SquadButton("VGUI/allegiance/squad1_unselected","VGUI/allegiance/squad1_hover","VGUI/allegiance/squad1_unselected","VGUI/allegiance/squad1_unselected",this,"Squad1Button",this,"squad1");
	m_pSquad2 = new SquadButton("VGUI/allegiance/squad2_unselected","VGUI/allegiance/squad2_hover","VGUI/allegiance/squad2_unselected","VGUI/allegiance/squad2_unselected",this,"Squad2Button",this,"squad2");
	m_pCell1 = new SquadButton("VGUI/allegiance/cell1_unselected","VGUI/allegiance/cell1_hover","VGUI/allegiance/cell1_unselected","VGUI/allegiance/cell1_unselected",this,"Cell1Button",this,"cell1");
	m_pCell2 = new SquadButton("VGUI/allegiance/cell2_unselected","VGUI/allegiance/cell2_hover","VGUI/allegiance/cell2_unselected","VGUI/allegiance/cell2_unselected",this,"Cell2Button",this,"cell2");
	m_pRandom = new SquadButton("VGUI/allegiance/random_unselected","VGUI/allegiance/random_hover","VGUI/allegiance/random_unselected","VGUI/allegiance/random_unselected",this,"RandomButton",this,"random");
	m_pSpectate = new SquadButton("VGUI/allegiance/spectator_unselected","VGUI/allegiance/spectator_hover","VGUI/allegiance/spectator_unselected","VGUI/allegiance/spectator_unselected",this,"SpectateButton",this,"spectate");


	EnableNextButton( false );
}

void CAllegiance::PerformLayout( void )
{
	BaseClass::PerformLayout( );

	SCALE_CONTROL( m_pSquad1, BUTTON_SQUAD1 );
	m_pSquad1->SetScaledImageSize(IntegerScale(256),IntegerScale(64));
	SCALE_CONTROL( m_pSquad2, BUTTON_SQUAD2 );
	m_pSquad2->SetScaledImageSize(IntegerScale(256),IntegerScale(64));
	SCALE_CONTROL( m_pCell1, BUTTON_CELL1 );
	m_pCell1->SetScaledImageSize(IntegerScale(256),IntegerScale(64));
	SCALE_CONTROL( m_pCell2, BUTTON_CELL2 );
	m_pCell2->SetScaledImageSize(IntegerScale(256),IntegerScale(64));
	SCALE_CONTROL( m_pRandom,BUTTON_RANDOM );
	m_pRandom->SetScaledImageSize(IntegerScale(256),IntegerScale(64));
	SCALE_CONTROL( m_pSpectate, BUTTON_SPECTATE );
	m_pSpectate->SetScaledImageSize(IntegerScale(256),IntegerScale(64));
	SCALE_CONTROL( m_pScoreboards[0], SCOREBOARD1 );
	SCALE_CONTROL( m_pScoreboards[1], SCOREBOARD2 );
	SCALE_CONTROL( m_pSituation, SITUATION );
	SCALE_CONTROL( m_pSquadDescription, SQUADDESC );
}

void CAllegiance::Reset()
{

}

#include "insvgui_utils.h"

TeamSelectionData_t g_selectionData;

void CAllegiance::OnCommand( const char *pszCommand )
{
	if( FStrEq( pszCommand, "squad1" ) )
	{
		// close the panel
		ShowPanel( false );

		CINSVGUIHelper::JoinTeam( TEAM_TWO );

		g_selectionData.m_iSquadID = 0;
	}
	if( FStrEq( pszCommand, "squad2" ) )
	{
		// close the panel
		ShowPanel( false );

		CINSVGUIHelper::JoinTeam( TEAM_TWO );

		g_selectionData.m_iSquadID = 1;
	}
	if( FStrEq( pszCommand, "cell1" ) )
	{
		// close the panel
		ShowPanel( false );

		CINSVGUIHelper::JoinTeam( TEAM_ONE );

		g_selectionData.m_iSquadID = 1;
	}
	if( FStrEq( pszCommand, "cell2" ) )
	{
		// close the panel
		ShowPanel( false );

		CINSVGUIHelper::JoinTeam( TEAM_ONE );

		g_selectionData.m_iSquadID = 0;
	}
	if( FStrEq( pszCommand, "random" ) )
	{
		// close the panel
		ShowPanel( false );

		CINSVGUIHelper::JoinTeam( 2 );

		g_selectionData.m_iSquadID = -1; // Random
	}
	if( FStrEq( pszCommand, "spectate" ) )
	{
		// close the panel
		ShowPanel( false );
		
		CINSVGUIHelper::JoinTeam( 3 );

		// join the server!
		g_pGameUIPanel->JoinServer( );
	}
}

#endif