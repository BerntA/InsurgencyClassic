//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>

#include <vgui_controls/Frame.h>
#include <game/client/iviewport.h>
#include <igameevents.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include <c_playerresource.h>

#include "insvgui.h"
#include "imc_config.h"

#include "ins_gamerules.h"

#include "c_ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

#ifndef INSVGUI_FINAL_SCOREBOARD

#define TYPE_NOTEAM			0	// NOTEAM must be zero :)
#define TYPE_TEAM			1	// a section for a single team	
#define TYPE_SPECTATORS		2	// a section for a spectator group
#define TYPE_BLANK			3
#define TYPE_UNASSIGNED		4

//-----------------------------------------------------------------------------
// Purpose: Game ScoreBoard
//-----------------------------------------------------------------------------
class CClientScoreBoardDialog : public vgui::Frame, public IViewPortPanel, public IGameEventListener2
{
private:
	DECLARE_CLASS_SIMPLE( CClientScoreBoardDialog, vgui::Frame );

protected:
// column widths at 640
	enum { NAME_WIDTH = 160, SCORE_WIDTH = 60, FRAGS_WIDTH = 60, DEATH_WIDTH = 60, PING_WIDTH = 80, VOICE_WIDTH = 0, FRIENDS_WIDTH = 0 };
	// total = 420

public:
	CClientScoreBoardDialog( IViewPort *pViewPort );
	virtual ~CClientScoreBoardDialog();

	virtual const char *GetName( void ) { return PANEL_SCOREBOARD; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void );
	virtual bool HasInputElements( void ) { return false; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }
 	
	// IGameEventListener interface:
	virtual void FireGameEvent( IGameEvent *event);
			

protected:
	// functions to override
	virtual bool GetPlayerScoreInfo(int playerIndex, KeyValues *outPlayerInfo);
	virtual void InitScoreboardSections();
	virtual void UpdateTeamInfo();
	virtual void UpdatePlayerInfo();
	
	virtual void AddHeader(); // add the start header of the scoreboard
	virtual int AddSection(int teamType, int teamNumber); // add a new section header for a team

	// sorts players within a section
	static bool StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// finds the player in the scoreboard
	int FindItemIDForPlayerIndex(int playerIndex);

	int m_iNumTeams;

	vgui::SectionedListPanel *m_pPlayerList;
	int				m_iSectionId; // the current section we are entering into

	int s_VoiceImage[5];
	int TrackerImage;
	int	m_HLTVSpectators;

	void MoveLabelToFront(const char *textEntryName);

private:
	int			m_iPlayerIndexSymbol;
	int			m_iDesiredHeight;
	IViewPort	*m_pViewPort;
	float		m_fNextUpdateTime;

    int iTeamSections[MAX_TEAMS];
    int numPlayersOnTeam[MAX_TEAMS];
    int teamLatency[MAX_TEAMS];

	int iTeamTranslation[MAX_TEAMS];

	// methods
	void FillScoreBoard();
	bool ShowKD();
};


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientScoreBoardDialog::CClientScoreBoardDialog(IViewPort *pViewPort)
	: Frame( NULL, PANEL_SCOREBOARD )
{
	m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");

	//memset(s_VoiceImage, 0x0, sizeof( s_VoiceImage ));
	TrackerImage = 0;
	m_pViewPort = pViewPort;

	// initialize dialog
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(false);
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );

	// set the scheme before any child control is created
	SetScheme("ClientScheme");

	m_pPlayerList = new SectionedListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(false);

	LoadControlSettings("resource/ui/basic/scoreboard.res");
	m_iDesiredHeight = GetTall();
	m_pPlayerList->SetVisible( false ); // hide this until we load the images in applyschemesettings

	m_HLTVSpectators = 0;

	// update scoreboard instantly if on of these events occure
	gameeventmanager->AddListener(this, "hltv_status", false );
	gameeventmanager->AddListener(this, "server_spawn", false );
}

CREATE_INSVIEWPORT_PANEL( CClientScoreBoardDialog );

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CClientScoreBoardDialog::~CClientScoreBoardDialog()
{
	gameeventmanager->RemoveListener(this);
}

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::Reset()
{
	// clear
	m_pPlayerList->DeleteAllItems();
	m_pPlayerList->RemoveAllSections();

	m_iSectionId = 0;
	m_fNextUpdateTime = 0;

	// add all the sections
	InitScoreboardSections( );
}

//-----------------------------------------------------------------------------
// Purpose: adds all the team sections to the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::InitScoreboardSections()
{
	iTeamTranslation[ TEAM_UNASSIGNED ] = TEAM_UNASSIGNED;
	iTeamTranslation[ TEAM_ONE ] = IMCConfig( )->IsSwitchedTeams( ) ? TEAM_TWO : TEAM_ONE;
	iTeamTranslation[ TEAM_TWO ] = IMCConfig( )->IsSwitchedTeams( ) ? TEAM_ONE : TEAM_TWO;
	iTeamTranslation[ TEAM_SPECTATOR ] = TEAM_SPECTATOR;

	AddHeader();
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	ImageList *imageList = new ImageList(false);
//	s_VoiceImage[0] = 0;	// index 0 is always blank
//	s_VoiceImage[CVoiceStatus::VOICE_NEVERSPOKEN] = imageList->AddImage(scheme()->GetImage("gfx/vgui/640_speaker1", true));
//	s_VoiceImage[CVoiceStatus::VOICE_NOTTALKING] = imageList->AddImage(scheme()->GetImage("gfx/vgui/640_speaker2", true));
//	s_VoiceImage[CVoiceStatus::VOICE_TALKING] = imageList->AddImage(scheme()->GetImage( "gfx/vgui/640_speaker3", true));
//	s_VoiceImage[CVoiceStatus::VOICE_BANNED] = imageList->AddImage(scheme()->GetImage("gfx/vgui/640_voiceblocked", true));
	
//	TrackerImage = imageList->AddImage(scheme()->GetImage("gfx/vgui/640_scoreboardtracker", true));

	// resize the images to our resolution
	for (int i = 0; i < imageList->GetImageCount(); i++ )
	{
		int wide, tall;
		imageList->GetImage(i)->GetSize(wide, tall);
		imageList->GetImage(i)->SetSize(scheme()->GetProportionalScaledValueEx( GetScheme(),wide), scheme()->GetProportionalScaledValueEx( GetScheme(),tall));
	}

	m_pPlayerList->SetImageList(imageList, false);
	m_pPlayerList->SetVisible( true );

	// light up scoreboard a bit
	SetBgColor( Color( 0,0,0,0) );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Reset();
		Update();

		Activate();
	}
	else
	{
		BaseClass::SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}



void CClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type, "hltv_status") == 0 )
	{
		// spectators = clients - proxies
		m_HLTVSpectators = event->GetInt( "clients" );
		m_HLTVSpectators -= event->GetInt( "proxies" );
	}

	else if ( Q_strcmp(type, "server_spawn") == 0 )
	{
		// We'll post the message ourselves instead of using SetControlString()
		// so we don't try to translate the hostname.
		const char *hostname = event->GetString( "hostname" );
		Panel *control = FindChildByName( "ServerName" );
		if ( control )
		{
			PostMessage( control, new KeyValues( "SetText", "text", hostname ) );
		}
		control->MoveToFront();
	}

	if( IsVisible() )
		Update();

}

bool CClientScoreBoardDialog::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);
		

}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::Update( void )
{
	// Set the title
	
	// Reset();
	m_pPlayerList->DeleteAllItems();
	
	FillScoreBoard();

	// grow the scoreboard to fit all the players
	int wide, tall;
	m_pPlayerList->GetContentSize(wide, tall);
	wide = GetWide();
	if (m_iDesiredHeight < tall)
	{
		SetSize(wide, tall);
		m_pPlayerList->SetSize(wide, tall);
	}
	else
	{
		SetSize(wide, m_iDesiredHeight);
		m_pPlayerList->SetSize(wide, m_iDesiredHeight);
	}

	MoveToCenterOfScreen();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: Sort all the teams
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdateTeamInfo()
{
  int sectionId;
    int i;
    char cstring[256];
    char plural[2];
    wchar_t tn[256];
    IGameResources *gr = GameResources();
    if ( !gr )
        return;

    for (i=0;i<MAX_TEAMS;i++)
    {
		int iTeamID = iTeamTranslation[ i ];

        sectionId = iTeamSections[iTeamID]; //get the section for the team

        if (numPlayersOnTeam[iTeamID] == 1)
            sprintf(plural,"");
        else
            sprintf(plural,"s");

        sprintf(cstring, "%s - (%i player%s)", gr->GetTeamName(iTeamID),  numPlayersOnTeam[iTeamID], plural);
		g_pVGuiLocalize->ConvertANSIToUnicode(cstring, tn, sizeof(tn));
        m_pPlayerList->ModifyColumn(sectionId, "name", tn);

        if ( numPlayersOnTeam[iTeamID] > 0 )
            teamLatency[iTeamID] /= numPlayersOnTeam[iTeamID];
        else
            teamLatency[iTeamID] = 0;

        wchar_t sz[6];
        swprintf(sz, L"%d", gr->GetTeamScore(iTeamID));
        m_pPlayerList->ModifyColumn(sectionId, "score", sz);
        if (teamLatency[iTeamID] < 1)
        {
            m_pPlayerList->ModifyColumn(sectionId, "ping", L"");
        }
        else
        {
            swprintf(sz, L"%i", teamLatency[iTeamID]);
            m_pPlayerList->ModifyColumn(sectionId, "ping", sz);
        }
        teamLatency[iTeamID] = 0;

        m_pPlayerList->SetSectionFgColor(sectionId, gr->GetTeamColor(iTeamID)); //mm colors
    }
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdatePlayerInfo()
{
	m_iSectionId = 0; // 0'th row is a header
	int selectedRow = -1;

	// walk all the players and make sure they're in the scoreboard
	for ( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		IGameResources *gr = GameResources();

		if ( gr && gr->IsConnected( i ) )
		{
			// add the player to the list
			KeyValues *playerData = new KeyValues("data");
			GetPlayerScoreInfo( i, playerData );
	
			const char *oldName = playerData->GetString("name","");
			int bufsize = strlen(oldName) * 2 + 1;
			char *newName = (char *)_alloca( bufsize );

			UTIL_MakeSafeName( oldName, newName, bufsize );

			playerData->SetString("name", newName);

            int itemID = FindItemIDForPlayerIndex( i );
            int playerTeam = gr->GetTeam(i);
            int sectionID = iTeamSections[  playerTeam  ];

            numPlayersOnTeam[ iTeamTranslation[ playerTeam ] ]++; //increment player count and add this player
            teamLatency[ iTeamTranslation[ playerTeam ] ]+=playerData->GetInt("ping");
           
            if ( gr->IsLocalPlayer( i ) )
            {
                selectedRow = itemID;
            }
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
            m_pPlayerList->SetItemFgColor( itemID, gr->GetTeamColor( playerTeam ) );

            playerData->deleteThis();
		}
		else
		{
			// remove the player
			int itemID = FindItemIDForPlayerIndex( i );
			if (itemID != -1)
			{
				m_pPlayerList->RemoveItem(itemID);
			}
		}
	}

	if ( selectedRow != -1 )
	{
		m_pPlayerList->SetSelectedItem(selectedRow);
	}
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::AddHeader()
{
    m_iSectionId = 0; //make a blank one
    m_pPlayerList->AddSection(m_iSectionId, "");
    m_pPlayerList->SetSectionAlwaysVisible(m_iSectionId);
    m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH) );

    m_iSectionId = 1;
    m_pPlayerList->AddSection(m_iSectionId, "");
    m_pPlayerList->SetSectionAlwaysVisible(m_iSectionId);
    m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH) );
    m_pPlayerList->AddColumnToSection(m_iSectionId, "score", "#PlayerScore", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH) );
	if (ShowKD())
	{
		m_pPlayerList->AddColumnToSection(m_iSectionId, "kills", "#PlayerKills", 0, scheme()->GetProportionalScaledValue(FRAGS_WIDTH) );
		m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "#PlayerDeaths", 0, scheme()->GetProportionalScaledValue(DEATH_WIDTH) );
	}
    m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValue(PING_WIDTH) );
    m_pPlayerList->AddColumnToSection(m_iSectionId, "voice", "#PlayerVoice", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, scheme()->GetProportionalScaledValue(VOICE_WIDTH) );

    m_iSectionId = 2; //first team;
	if (GameResources()->GetTeamScore(TEAM_ONE)>=GameResources()->GetTeamScore(TEAM_TWO))
	{
		iTeamSections[TEAM_ONE]  = AddSection(TYPE_TEAM, TEAM_ONE );
		iTeamSections[TEAM_TWO]   = AddSection(TYPE_TEAM, TEAM_TWO );
	}
	else
	{
		iTeamSections[TEAM_TWO]   = AddSection(TYPE_TEAM, TEAM_TWO );
		iTeamSections[TEAM_ONE]  = AddSection(TYPE_TEAM, TEAM_ONE );
	}
    iTeamSections[TEAM_SPECTATOR]   = AddSection(TYPE_SPECTATORS, TEAM_SPECTATOR );
    iTeamSections[TEAM_UNASSIGNED]  = AddSection(TYPE_UNASSIGNED, TEAM_UNASSIGNED );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::AddSection(int teamType, int teamNumber)
{
	teamNumber = iTeamTranslation[ teamNumber ];

    if ( teamType == TYPE_TEAM )
    {
        IGameResources *gr = GameResources();

        if ( !gr )
            return -1;

        // setup the team name
        wchar_t *teamName = g_pVGuiLocalize->Find( gr->GetTeamName(teamNumber) );
        wchar_t name[64];
       
        if (!teamName)
        {
			g_pVGuiLocalize->ConvertANSIToUnicode(gr->GetTeamName(teamNumber), name, sizeof(name));
            teamName = name;
        }

        m_pPlayerList->AddSection(m_iSectionId, "", StaticPlayerSortFunc);
		m_pPlayerList->SetSectionAlwaysVisible(m_iSectionId);
        m_pPlayerList->SetFgColor(gr->GetTeamColor(teamNumber));

        m_pPlayerList->AddColumnToSection(m_iSectionId, "name", teamName, 0, scheme()->GetProportionalScaledValue(NAME_WIDTH) );
        m_pPlayerList->AddColumnToSection(m_iSectionId, "score", "0", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH) );
		if (ShowKD())
		{
			m_pPlayerList->AddColumnToSection(m_iSectionId, "kills", "0", 0, scheme()->GetProportionalScaledValue(FRAGS_WIDTH) );
			m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "0", 0, scheme()->GetProportionalScaledValue(DEATH_WIDTH) );
		}
        m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue(PING_WIDTH) );
        m_pPlayerList->AddColumnToSection(m_iSectionId, "voice", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, scheme()->GetProportionalScaledValue(VOICE_WIDTH) );
    }
    else if ( teamType == TYPE_SPECTATORS )
    {
        m_pPlayerList->AddSection(m_iSectionId, "");
        m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "#Spectators", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
        m_pPlayerList->AddColumnToSection(m_iSectionId, "score", "", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH) );
		if (ShowKD())
		{
			m_pPlayerList->AddColumnToSection(m_iSectionId, "kills", "0", 0, scheme()->GetProportionalScaledValue(FRAGS_WIDTH) );
			m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "0", 0, scheme()->GetProportionalScaledValue(DEATH_WIDTH) );
		}
        m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue(PING_WIDTH) );
    }
    else if ( teamType == TYPE_UNASSIGNED )
    {
        m_pPlayerList->AddSection(m_iSectionId, "");
		m_pPlayerList->SetSectionAlwaysVisible(m_iSectionId);
        m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "#Unassigned", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH));
        m_pPlayerList->AddColumnToSection(m_iSectionId, "score", "", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH) );
		if (ShowKD())
		{
			m_pPlayerList->AddColumnToSection(m_iSectionId, "kills", "0", 0, scheme()->GetProportionalScaledValue(FRAGS_WIDTH) );
			m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "0", 0, scheme()->GetProportionalScaledValue(DEATH_WIDTH) );
		}
        m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "", 0, scheme()->GetProportionalScaledValue(PING_WIDTH) );
    }
    m_iSectionId++; //increment for next
    return m_iSectionId-1;
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
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

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	C_PlayerResource *pPR = g_PR;
	if (!pPR )
		return false;

	kv->SetInt("score", pPR->GetMorale( playerIndex ) );
	kv->SetInt("ping", pPR->GetPing( playerIndex ) ) ;
	kv->SetString("name", pPR->GetPlayerName( playerIndex ) );
	kv->SetInt("playerIndex", playerIndex);
	kv->SetInt("kills", pPR->GetFrags( playerIndex ));
	kv->SetInt("deaths", pPR->GetDeaths( playerIndex ));

//	kv->SetInt("voice",	s_VoiceImage[GetClientVoice()->GetSpeakerStatus( playerIndex - 1) ]);	

/*	// setup the tracker column
	if (g_pFriendsUser)
	{
		unsigned int trackerID = gEngfuncs.GetTrackerIDForPlayer(row);

		if (g_pFriendsUser->IsBuddy(trackerID) && trackerID != g_pFriendsUser->GetFriendsID())
		{
			kv->SetInt("tracker",TrackerImage);
		}
	}
*/

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::FillScoreBoard()
{
    for (int i = 0; i < MAX_TEAMS; i++)
    {
        numPlayersOnTeam[i] = 0; //clear!
        //clear anything else for the team
    }

	// update player info
	UpdatePlayerInfo();

	// update totals information
	UpdateTeamInfo();
} 

//-----------------------------------------------------------------------------
// Purpose: searches for the player in the scoreboard
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::FindItemIDForPlayerIndex(int playerIndex)
{
	for (int i = 0; i <= m_pPlayerList->GetHighestItemID(); i++)
	{
		if (m_pPlayerList->IsItemIDValid(i))
		{
			KeyValues *kv = m_pPlayerList->GetItemData(i);
			kv = kv->FindKey(m_iPlayerIndexSymbol);
			if (kv && kv->GetInt() == playerIndex)
				return i;
		}
	}
	return -1;
}




//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::MoveLabelToFront(const char *textEntryName)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->MoveToFront();
	}
}

bool CClientScoreBoardDialog::ShowKD()
{
	C_INSPlayer* pPlayer=C_INSPlayer::GetLocalPlayer();
	if (!pPlayer->OnPlayTeam())
		return true;

	return INSRules()->IsModeRunning()&&INSRules()->RunningMode()->IsEnding();
}

#endif