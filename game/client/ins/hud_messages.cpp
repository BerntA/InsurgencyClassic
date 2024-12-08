//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <vgui/vgui.h>
#include <vgui/ivgui.h>
#include "iclientmode.h"
#include "hud_messages.h"
#include "hud_macros.h"

#include <vgui/ilocalize.h>

#include "c_ins_obj.h"
#include "ins_obj_shared.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "c_playerresource.h"

#include "ins_gamerules.h"

#include "clientmode_shared.h"
#include "basic_colors.h"
#include "stats_protocal.h"
#include "vguicenterprint.h"
#include "text_message.h"
#include "ins_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CHudChatInputLine : public Panel
{
	DECLARE_CLASS_SIMPLE( CHudChatInputLine, Panel );

public:
	CHudChatInputLine( Panel *parent, char const *pszPanelName );

	void ClearEntry( bool bResetType );
	void SetEntry( const wchar_t *pwcEntry );

	vgui::Panel *GetInputPanel( void );

	void Start( int iType );
	void Send( void );
	void Stop( void );

protected:
	virtual void ApplySchemeSettings( IScheme *pScheme );
	virtual void PerformLayout( void );

	virtual VPANEL GetCurrentKeyFocus( void );

	MESSAGE_FUNC_PARAMS( OnTextChanged, "TextChanged", pData );

private:
	void RecalculateSizes( void );
	bool SetMessageType( int iType );
	bool SetMessageType( void );

private:
	Label *m_pPrompt;
	CHudChatEntry *m_pInput;

	int m_iType;
};

//=========================================================
//=========================================================
class CHudChatEntry : public vgui::TextEntry
{
	DECLARE_CLASS_SIMPLE( CHudChatEntry, TextEntry );

public:
	CHudChatEntry( CHudChatInputLine *pParent, char const *pszPanelName );

private:
	void ApplySchemeSettings( IScheme *pScheme );
	void OnKeyCodeTyped( KeyCode Code );

private:
	CHudChatInputLine *m_pInputLine;

	char m_szLastMsg[ MAX_CHATMSG_LENGTH ];
};

//=========================================================
//=========================================================
enum ChatShowTypes_t
{
	CHATSHOWTYPE_ALL = 0,
	CHATSHOWTYPE_TEAM,
	CHATSHOWTYPE_SQUAD
};

ConVar shownamechange( "cl_shownamechange", "1" );
ConVar showdisconnects( "cl_showdisconnects", "1" );
ConVar chatshowtypes( "cl_chatshowtypes", "0", FCVAR_ARCHIVE, "Determines which Chat Messages to Show", true, CHATSHOWTYPE_ALL, true, CHATSHOWTYPE_SQUAD );

//=========================================================
//=========================================================
void __MsgFunc_TextMsg( bf_read &msg )
{
	char szString[2048];
	int msg_dest = msg.ReadByte();

	wchar_t szBuf[5][128];
	wchar_t outputBuf[256];

	for ( int i=0; i<5; ++i )
	{
		msg.ReadString( szString, sizeof(szString) );
		char *tmpStr = hudtextmessage->LookupString( szString, &msg_dest );
		const wchar_t *pBuf = g_pVGuiLocalize->Find( tmpStr );
		if ( pBuf )
		{
			// Copy pBuf into szBuf[i].
			int nMaxChars = sizeof( szBuf[i] ) / sizeof( wchar_t );
			wcsncpy( szBuf[i], pBuf, nMaxChars );
			szBuf[i][nMaxChars-1] = 0;
		}
		else
		{
			if ( i )
			{
				StripEndNewlineFromString( tmpStr );  // these strings are meant for subsitution into the main strings, so cull the automatic end newlines
			}
			g_pVGuiLocalize->ConvertANSIToUnicode( tmpStr, szBuf[i], sizeof(szBuf[i]) );
		}
	}

	int len;
	switch ( msg_dest )
	{
		case HUD_PRINTCENTER:
		{
			g_pVGuiLocalize->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			internalCenterPrint->Print( ConvertCRtoNL( outputBuf ) );
			break;
		}

		case HUD_PRINTNOTIFY:
		{
			szString[0] = 1;  // mark this message to go into the notify buffer
			g_pVGuiLocalize->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString+1, sizeof(szString)-1 );
			len = strlen( szString );
			if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
			{
				Q_strncat( szString, "\n", sizeof(szString), 1 );
			}
			Msg( "%s", ConvertCRtoNL( szString ) );
			break;
		}

		case HUD_PRINTTALK:
		{
			g_pVGuiLocalize->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
			len = strlen( szString );
			if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
			{
				Q_strncat( szString, "\n", sizeof(szString), 1 );
			}

			CColoredString String;
			UTIL_ParseChatMessage( String, 0, SAYTYPE_SERVER, false, ConvertCRtoNL( szString ) );

			if( g_pPlayerChat )
				g_pPlayerChat->PrintChat( String, SAYTYPE_SERVER );

			break;
		}

		case HUD_PRINTCONSOLE:
		{
			g_pVGuiLocalize->ConstructString( outputBuf, sizeof(outputBuf), szBuf[0], 4, szBuf[1], szBuf[2], szBuf[3], szBuf[4] );
			g_pVGuiLocalize->ConvertUnicodeToANSI( outputBuf, szString, sizeof(szString) );
			len = strlen( szString );
			if ( len && szString[len-1] != '\n' && szString[len-1] != '\r' )
			{
				Q_strncat( szString, "\n", sizeof(szString), 1 );
			}
			Msg( "%s", ConvertCRtoNL( szString ) );
			break;
		
		}
	}
}

//=========================================================
//=========================================================
void __MsgFunc_TextMsgFast( bf_read &msg )
{
	char szString[ 2048 ];
	int iMsgDest = msg.ReadByte( );

	msg.ReadString( szString, sizeof( szString ) );

	switch( iMsgDest )
	{
		case HUD_PRINTCENTER:
		{
			internalCenterPrint->Print( szString );

			break;
		}

		case HUD_PRINTCONSOLE:
		{
			Msg( szString );

			break;
		}

		case HUD_PRINTTALK:
		{
			CColoredString String;
			UTIL_ParseChatMessage( String, 0, SAYTYPE_SERVER, false, szString );

			if( g_pPlayerChat )
				g_pPlayerChat->PrintChat( String, SAYTYPE_SERVER );

			break;
		}
	}
}

//=========================================================
//=========================================================
CHudMessages *g_pPlayerChat = NULL;

CHudMessages::CHudMessages( const char *pszElementName ) :
	CHudMessagesBase( pszElementName, "HudMessages" ),
	CDeadHUDHelper( this )
{
	// set global var
	g_pPlayerChat = this;

	// register
	GetINSHUDHelper( )->RegisterChatMessages( this );

	// temp hooks
	HOOK_MESSAGE( TextMsg );
	HOOK_MESSAGE( TextMsgFast );

	// listen to certain game events
	gameeventmanager->AddListener( this, "player_connect", false );
	gameeventmanager->AddListener( this, "player_changename", false );
	gameeventmanager->AddListener( this, "player_death", false );
	gameeventmanager->AddListener( this, "player_disconnect", false );
	gameeventmanager->AddListener( this, "player_team", false );
}

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudMessages );
DECLARE_HUD_MESSAGE( CHudMessages, FFMsg );
DECLARE_HUD_MESSAGE( CHudMessages, PlayerLogin );

//=========================================================
//=========================================================
CHudMessages::~CHudMessages( )
{
	g_pPlayerChat = NULL;
}

//=========================================================
//=========================================================
void CHudMessages::Init( void )
{
	BaseClass::Init( );

	m_pInputLine = new CHudChatInputLine( this, "ChatInput" );

	HOOK_HUD_MESSAGE( CHudMessages, FFMsg );
	HOOK_HUD_MESSAGE( CHudMessages, PlayerLogin );
}

//=========================================================
//=========================================================
void CHudMessages::ApplySettings( KeyValues *pResourceData )
{
	BaseClass::ApplySettings( pResourceData );

	// setup deadhelper
	DeadInit( pResourceData );
}

//=========================================================
//=========================================================
void CHudMessages::StartMessageMode( int iType )
{
	SETUP_PANEL( this );

	m_pInputLine->Start( iType );
}

//=========================================================
//=========================================================
vgui::Panel *CHudMessages::GetInputPanel( void )
{
	return m_pInputLine->GetInputPanel( );
}

//=========================================================
//=========================================================
void CHudMessages::PrintChat( CColoredString &String, int iType )
{
	if( !IsValidSayType( iType ) )
		return;

	// print message
	Print( String );

	// print to console
	Msg( "%s\n", String.Get( ) );
}

//=========================================================
//=========================================================
void CHudMessages::PrintChat( const char *pszMessage )
{
	CColoredString ServerChat;
	ServerChat.Add( pszMessage );

	PrintChat( ServerChat, SAYTYPE_SERVER );
}

//=========================================================
//=========================================================
void CHudMessages::PrintRadio( const char *pszMessage, int iEntID )
{
	CColoredString RadioString;

	// setup vars
	const char *pszName = NULL;
	int iTeamID, iTeamColor;
	
	pszName = g_PR->GetPlayerName( iEntID );
	iTeamID = g_PR->GetTeamID( iEntID );

	if( !pszName || !IsPlayTeam( iTeamID ) )
	{
		Assert( false );
		return;
	}

	iTeamColor = ( iTeamID == TEAM_ONE ) ? CLOOKUP_TEAM_ONE : CLOOKUP_TEAM_TWO;

	// write
	RadioString.Add( "(RADIO) ", COLOR_DGREY );
	RadioString.Add( pszName, iTeamColor );
	RadioString.Add( ": ", iTeamColor );
	RadioString.Add( pszMessage, COLOR_ORANGE );

	// print
	Print( RadioString );
}

//=========================================================
//=========================================================
bool CHudMessages::IsValidSayType( int iType )
{
	// ensure valid value
	if( iType < 0 || iType >= SAYTYPE_COUNT )
		return false;

	// ensure valid type
	int iShowTypesValue = chatshowtypes.GetInt( );

	switch( iShowTypesValue )
	{
		case CHATSHOWTYPE_ALL:
		{
			return true;
		}

		case CHATSHOWTYPE_TEAM:
		{
			if( iType == SAYTYPE_TEAM )
				return true;

			break;
		}

		case CHATSHOWTYPE_SQUAD:
		{
			if( iType == SAYTYPE_SQUAD )
				return true;

			break;
		}
	}

	return false;
}

//=========================================================
//=========================================================
void CHudMessages::Update( void )
{
	BaseClass::Update( );

	RecaculateInput( );
}

//=========================================================
//=========================================================
void CHudMessages::FireGameEvent( IGameEvent *pEvent )
{
	C_PlayerResource *pPR = g_PR;

	if( !pPR )
		return;

	// find generic data
	int iUserID, iPlayerID, iTeamID, iTeamColorID;
	iPlayerID = 0;
	iTeamID = INVALID_TEAM;
	iTeamColorID = CLOOKUP_INVALID;

	iUserID = pEvent->GetInt( "userid" );
	
	if( iUserID != 0 )
	{
		iPlayerID = engine->GetPlayerForUserID( iUserID );
		iTeamID = pPR->GetTeamID( iPlayerID );
		iTeamColorID = UTIL_TeamColorLookup( iTeamID );
	}

	// create the string
	CColoredString EventString;
	EventString.Add( "* " );

	// handle the event
	const char *pszEvent = pEvent->GetName( );

	if( FStrEq( pszEvent, "player_connect" ) )
	{
		EventString.Add( pEvent->GetString( "name" ) );
		EventString.Add( " has Connected" );
	}
	else if( FStrEq( pszEvent, "player_changename" ) )
	{
		if( !shownamechange.GetBool( ) )
			return;

		EventString.Add( pEvent->GetString( "oldname" ), iTeamColorID );
		EventString.Add( " is now known as " );
		EventString.Add( pEvent->GetString( "newname" ), iTeamColorID );
	}
	else if( FStrEq( pszEvent, "player_disconnect" ) )
	{
		if( !showdisconnects.GetBool( ) )
			return;

		EventString.Add( pPR->GetPlayerName( iPlayerID ), iTeamColorID );
		EventString.Add( " has Disconnected" );
	}
	else if( FStrEq( pszEvent, "player_death" ) )
	{
		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer( );

		if( ( iPlayerID == pLocalPlayer->entindex( ) ) || ( iTeamID != pLocalPlayer->GetTeamID( ) ) )
			return;

		// ... find type
		int iType = pEvent->GetInt( "type" );

		// ... handle by type
		EventString.Add( pPR->GetPlayerName( iPlayerID ), iTeamColorID );

		switch( iType )
		{
			case PDEATHTYPE_SOULSTOLEN:
			{
				EventString.Add( " had their Soul Stolen" );
				break;
			}

			case PDEATHTYPE_SELF:
			{
				EventString.Add( " Commited Suicide" );
				break;
			}

			case PDEATHTYPE_TELEFRAG:
			{
				EventString.Add( " was Telefragged" );
				break;
			}

			case PDEATHTYPE_FF:
			{
				EventString.Add( " was TK'ed" );
				break;
			}

			default:
			{
				EventString.Add( " was KIA" );
				break;
			}
		}

		// ... add extra info for friendly kill
		if( iType == PDEATHTYPE_FF )
		{
			int iAttackerID = engine->GetPlayerForUserID( pEvent->GetInt( "attacker" ) );

			if( iAttackerID == 0 )
				return;

			EventString.Add( " by " );
			EventString.Add( pPR->GetPlayerName( iAttackerID ), iTeamColorID );
		}
	}
	else if( FStrEq( pszEvent, "player_team" ) )
	{
		int iOldTeamID, iTeamID;
		iOldTeamID = pEvent->GetInt( "oldteam" );
		iTeamID = pEvent->GetInt( "team" );

		C_Team *pTeam = GetGlobalTeam( iTeamID );

		if( !pTeam )
			return;

		EventString.Add( pPR->GetPlayerName( iPlayerID ), UTIL_TeamColorLookup( iOldTeamID ) );
		EventString.Add( " has joined the " );
		EventString.Add( pTeam->GetName( ), UTIL_TeamColorLookup( iTeamID ) );
	}
	else
	{
		return;
	}

	// ... print it
	Print( EventString );
}

//=========================================================
//=========================================================
void CHudMessages::RecaculateInput( void )
{
	int iXPos, iYPos, iWide, iTall;
	GetBounds( iXPos, iYPos, iWide, iTall );

	m_pInputLine->SetBounds( iXPos, iYPos + iTall + 1, iWide - 2, m_iFontHeight );
}

//=========================================================
//=========================================================
void CHudMessages::OnThink( void )
{
	BaseClass::OnThink( );

	DeadUpdate( );
}

//=========================================================
//=========================================================
void CHudMessages::MsgFunc_FFMsg( bf_read &msg )
{
	C_PlayerResource *pPR = g_PR;

	if( !pPR )
		return;

	CColoredString FFMessage;

	int iPlayerID = msg.ReadShort( );

	if( iPlayerID == 0 )
		return;

	FFMessage.Add( pPR->GetPlayerName( iPlayerID ), UTIL_TeamColorLookup( pPR->GetTeamID( iPlayerID ) ) );
	FFMessage.Add( " Attacked a Teammate" );

	Print( FFMessage );
}

//=========================================================
//=========================================================
void CHudMessages::MsgFunc_PlayerLogin( bf_read &msg )
{
	CColoredString StatsMessage;

	// get type
	int iType = msg.ReadByte( );

	// find message
	const char *pszMessage;

	if( iType == SAC_PLAYERTYPE_VALID || iType == SAC_PLAYERTYPE_DEVELOPER )
		pszMessage = "You are Logged into the Stats Server";
	else
		pszMessage = "You are not Logged into the Stats Server";

	// print message
	StatsMessage.Add( pszMessage );
	Print( StatsMessage );
}

//=========================================================
//=========================================================
CHudChatInputLine::CHudChatInputLine( Panel *pParent, char const *pszPanelName ) : 
	Panel( pParent, pszPanelName )
{
	// create controls
	m_pPrompt = new Label( this, "ChatInputPrompt", L" " );

	m_pInput = new CHudChatEntry( this, "ChatInput" );	
	m_pInput->SetMaximumCharCount( MAX_CHATMSG_LENGTH - 1 );
	m_pInput->AddActionSignalTarget( this );

	// setup panel
	SetMouseInputEnabled( false );
	SetVisible( false );

	MakePopup( );
}

//=========================================================
//=========================================================
VPANEL CHudChatInputLine::GetCurrentKeyFocus( void )
{
	return m_pInput->GetVPanel( );
}

//=========================================================
//=========================================================
void CHudChatInputLine::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// setup panels
	SetPaintBackgroundEnabled( false );
	
	// setup controls
	HFont hFont = pScheme->GetFont( "ChatFont" );

	m_pPrompt->SetFont( hFont );
	m_pPrompt->SetPaintBackgroundEnabled( false );
	m_pPrompt->SetContentAlignment( Label::a_west );
	m_pPrompt->SetTextInset( 2, 0 );
	m_pPrompt->SetFgColor( pScheme->GetColor( "ChatInput.PromptColor", GetFgColor( ) ) );

	m_pInput->SetFont( hFont );
	m_pInput->SetFgColor( pScheme->GetColor( "ChatInput.InputColor", GetFgColor( ) ) );
	m_pInput->SetBgColor( Color( 0, 0, 0, 0 ) );
}

//=========================================================
//=========================================================
void CHudChatInputLine::PerformLayout( void )
{
	RecalculateSizes( );
}

//=========================================================
//=========================================================
#define ST_SWITCH_GLOBAL 'g'
#define ST_SWITCH_TEAM 't'
#define ST_SWITCH_SQUAD 's'

void CHudChatInputLine::OnTextChanged( KeyValues *pData )
{
	char szEntryBuffer[ 32 ];
	m_pInput->GetText( szEntryBuffer, sizeof( szEntryBuffer ) );

	int iEntryLength = Q_strlen( szEntryBuffer );

	if( iEntryLength != 2 || szEntryBuffer[ 0 ] != '/' )
		return;

	char cSecondChar = szEntryBuffer[ 1 ];

	bool bResult = false;

	if( cSecondChar == ST_SWITCH_GLOBAL )
		bResult = SetMessageType( SAYTYPE_GLOBAL );
	if( cSecondChar == ST_SWITCH_TEAM )
		bResult = SetMessageType( SAYTYPE_TEAM );
	else if( cSecondChar == ST_SWITCH_SQUAD )
		bResult = SetMessageType( SAYTYPE_SQUAD );
}

//=========================================================
//=========================================================
bool CHudChatInputLine::SetMessageType( int iType )
{
	if( iType == 0 )
	{
		iType = SAYTYPE_GLOBAL;

		switch( chatshowtypes.GetInt( ) )
		{
			case CHATSHOWTYPE_ALL:
			{
				iType = SAYTYPE_GLOBAL;
				break;
			}

			case CHATSHOWTYPE_TEAM:
			{
				iType = SAYTYPE_TEAM;
				break;
			}

			case CHATSHOWTYPE_SQUAD:
			{
				iType = SAYTYPE_SQUAD;
				break;
			}
		}
	}

	if( !CHudMessages::IsValidSayType( iType ) )
		return false;

	const char *pszPromptText = NULL;
	m_iType = iType;

	switch( m_iType )
	{
		case SAYTYPE_GLOBAL:
		{
			pszPromptText = "Say: ";
			break;
		}

		case SAYTYPE_TEAM:
		{
			pszPromptText = "Say Team: ";
			break;
		}

		case SAYTYPE_SQUAD:
		{
			pszPromptText = "Say Squad: ";
			break;
		}
	}

	m_pPrompt->SetText( pszPromptText );
	RecalculateSizes( );

	ClearEntry( false );

	return true;
}

//=========================================================
//=========================================================
bool CHudChatInputLine::SetMessageType( void )
{
	return SetMessageType( 0 );
}

//=========================================================
//=========================================================
void CHudChatInputLine::RecalculateSizes( void )
{
	int iWide, iTall;
	GetSize( iWide, iTall );

	int iContentWide, iContentTall;
	m_pPrompt->GetContentSize( iContentWide, iContentTall );
	m_pPrompt->SetBounds( 0, 0, iContentWide, iTall );

	m_pInput->SetBounds( iContentWide + 2, 0, iWide - iContentWide - 2 , iTall );
}

//=========================================================
//=========================================================
void CHudChatInputLine::ClearEntry( bool bResetType )
{
	SetEntry( L"" );

	if( bResetType )
		SetMessageType( );
}

//=========================================================
//=========================================================
void CHudChatInputLine::SetEntry( const wchar_t *pwcEntry )
{
	m_pInput->SetText( pwcEntry );
	m_pInput->SelectNoText( );
}

//=========================================================
//=========================================================
vgui::Panel *CHudChatInputLine::GetInputPanel( void )
{
	return m_pInput;
}

//=========================================================
//=========================================================
void CHudChatInputLine::Start( int iType )
{
	// setup type
	if( !SetMessageType( iType ) )
		return;

	// setup panel
	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( false );
	SetVisible( true );
	surface( )->CalculateMouseVisible( );
	MoveToFront( );
	RequestFocus( );
	SetVisible( true );
	SetEnabled( true );
}

//=========================================================
//=========================================================
void CHudChatInputLine::Stop( void )
{
	SetKeyBoardInputEnabled( false );
	SetVisible( false );
	SetEnabled( false );
}

//=========================================================
//=========================================================
void CHudChatInputLine::Send( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	char szText[ MAX_CHATMSG_LENGTH ];
	m_pInput->GetText( szText, MAX_CHATMSG_LENGTH );

	GetINSHUDHelper( )->SendChat( szText, m_iType );
}

//=========================================================
//=========================================================
CHudChatEntry::CHudChatEntry( CHudChatInputLine *pParent, char const *pszPanelName )
		: BaseClass( pParent, pszPanelName )
{
	SetCatchEnterKey( true );
	SetAllowNonAsciiCharacters( false );

	m_pInputLine = pParent;
	m_szLastMsg[ 0 ] = '\0';
}

//=========================================================
//=========================================================
void CHudChatEntry::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetPaintBorderEnabled( false );
}

//=========================================================
//=========================================================
void CHudChatEntry::OnKeyCodeTyped( KeyCode Code )
{
	if( !m_pInputLine )
		return;

	switch( Code )
	{
		case KEY_ENTER:
		case KEY_PAD_ENTER:
		case KEY_ESCAPE:
		{
			if( Code != KEY_ESCAPE )
			{
				m_pInputLine->Send( );
				GetText( m_szLastMsg, MAX_CHATMSG_LENGTH );
			}

			m_pInputLine->Stop( );
			m_pInputLine->ClearEntry( true );
			
			return;
		}

		case KEY_UP:
		{
			SetText( m_szLastMsg );

			return;
		}

		case KEY_TAB:
		{
			return;
		}
	}

	BaseClass::OnKeyCodeTyped( Code );
}