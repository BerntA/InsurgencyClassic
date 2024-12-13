//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud_messages_base.h"
#include "hud_macros.h"
#include "basic_colors.h"
#include "ins_gamerules.h"
#include "deadhelper.h"
#include "inshud.h"

#include "c_ins_obj.h"
#include "play_team_shared.h"
#include "ins_player_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CHudMessagesGame;

//=========================================================
//=========================================================
#define GAMELINE_BG_WIDE 8
#define GAMELINE_BG_TALL 16

#define GAMELINE_BGOFFSET_TOP 1
#define GAMELINE_BGOFFSET_BOTTOM 1

//=========================================================
//=========================================================
enum BGTypes_t
{
	BGTYPE_LEFT = 0,
	BGTYPE_MIDDLE,
	BGTYPE_RIGHT,
	BGTYPE_COUNT
};

//=========================================================
//=========================================================
class CHudMessagesGameLine : public CHudMessagesLine
{
	DECLARE_CLASS_SIMPLE( CHudMessagesGameLine, CHudMessagesLine );

public:
	CHudMessagesGameLine( CHudMessagesBase *pParent );

private:
	void Init( void );

	void Paint( void );

	void PerformLayout( void );

	int GetHeightForce( void ) const;
	int CalcTextWidth( void );

private:
	CHudMessagesGame *m_pGameParent;
};

//=========================================================
//=========================================================
class CHudMessagesGame : public CHudMessagesBase, public CDeadHUDHelper, public IINSReinforcement
{
	DECLARE_CLASS_SIMPLE( CHudMessagesGame, CHudMessagesBase );

public:
	CHudMessagesGame( const char *pszElementName );

	void Init( void );

	void MsgFunc_ObjMsg( bf_read &msg );

	int BGTexID( int iType ) const;
	int BGWide( void ) const { return m_iBGWide; }
	int BGTall( void ) const { return m_iBGTall; }

private:
	void ApplySettings( KeyValues *pResourceData );

	void OnThink( void );

	CHudMessagesLine *CreateNewLine( void );
	int LineCount( void ) { return 4; }

	const char *FontName( void ) const { return "GameMessagesFont"; }
	bool FontScaled( void ) const { return true; }

	void PrintObjMessage( C_INSObjective *pObjective, int iMsgType, int iTeamID );

	bool DrawBackwards( void ) const { return false; }
	bool LineRightAlign( void ) const { return true; }
	bool CalculateLineWidth( void ) const { return true; }

	void ReinforcementDeployed( int iRemaining );

	void ApplySchemeSettings( IScheme *pScheme );
	void LoadTex( const char *pszName, int &iTexID );

private:
	int m_iBGTexID[ BGTYPE_COUNT ];

	int m_iBGWide, m_iBGTall;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudMessagesGame );
DECLARE_HUD_MESSAGE( CHudMessagesGame , ObjMsg );

//=========================================================
//=========================================================
CHudMessagesGame::CHudMessagesGame( const char *pszElementName ) :
	CHudMessagesBase( pszElementName, "HudGameMessages" ),
	CDeadHUDHelper( this )
{
	memset( &m_iBGTexID, 0, sizeof( m_iBGTexID ) );
}

//=========================================================
//=========================================================
void CHudMessagesGame::Init( void )
{
	static const char *pszBGPaths[ BGTYPE_COUNT ] = {
		"left",		// BGTYPE_LEFT
		"middle",	// BGTYPE_MIDDLE
		"right"		// BGTYPE_RIGHT
	};

	BaseClass::Init( );

	// load bg elements
	for( int i = 0; i < BGTYPE_COUNT; i++ )
		LoadTex( pszBGPaths[ i ], m_iBGTexID[ i ] );

	// hook objmsg
	HOOK_HUD_MESSAGE( CHudMessagesGame, ObjMsg );
}

//=========================================================
//=========================================================
void CHudMessagesGame::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_iBGWide = scheme( )->GetProportionalScaledValue( GAMELINE_BG_WIDE );
	m_iBGTall = scheme( )->GetProportionalScaledValue( GAMELINE_BG_TALL );
}

//=========================================================
//=========================================================
void CHudMessagesGame::LoadTex( const char *pszName, int &iTexID )
{
	char szBuffer[ 64 ];

	Q_strncpy( szBuffer, "HUD/gmessages/", sizeof( szBuffer ) );
	Q_strncat( szBuffer, pszName, sizeof( szBuffer ), COPY_ALL_CHARACTERS );

	iTexID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( iTexID, szBuffer, false, false );
}

//=========================================================
//=========================================================
void CHudMessagesGame::MsgFunc_ObjMsg( bf_read &msg )
{
	int iObjID = msg.ReadByte( );

	C_INSObjective *pObjective = C_INSObjective::GetObjective( iObjID );
	Assert( pObjective );

	if( !pObjective )
		return;

	int iMsgType = msg.ReadByte( );
	int iTeamID = msg.ReadByte( );

	PrintObjMessage( pObjective, iMsgType, iTeamID );
}

//=========================================================
//=========================================================
int CHudMessagesGame::BGTexID( int iType ) const
{
	return m_iBGTexID[ iType ];
}

//=========================================================
//=========================================================
void CHudMessagesGame::ApplySettings( KeyValues *pResourceData )
{
	BaseClass::ApplySettings( pResourceData );

	DeadInit( pResourceData );
}

//=========================================================
//=========================================================
void CHudMessagesGame::OnThink( void )
{
	BaseClass::OnThink( );

	CDeadHUDHelper::DeadUpdate( );
}

//=========================================================
//=========================================================
CHudMessagesLine *CHudMessagesGame::CreateNewLine( void )
{
	return new CHudMessagesGameLine( this );
}

//=========================================================
//=========================================================
void CHudMessagesGame::PrintObjMessage( C_INSObjective *pObjective, int iMsgType, int iTeamID )
{
	C_Team *pCapturedTeam = GetGlobalTeam( iTeamID );

	if( !pCapturedTeam )
		return;

	bool bPlayTeam = IsPlayTeam( iTeamID );
	bool bLocalTeam = ( bPlayTeam && GetLocalTeam( ) == pCapturedTeam );
	const Color &ObjColor = ( !bPlayTeam ? pObjective->GetColor( ) : INSRules( )->TeamColor( pCapturedTeam ) );

	char szObjBuffer[ 64 ];
	Q_snprintf( szObjBuffer, sizeof( szObjBuffer ), "Obj. %c ", pObjective->GetPhonetischLetter( ) );

	CColoredString ObjMessage;

	ObjMessage.Add( szObjBuffer, ObjColor );

	switch( iMsgType )
	{
		case OBJ_CAPTURE_START:
		{
			ObjMessage.Add( bLocalTeam ? "is being secured" : "is under attack" );
			break;
		}

		case OBJ_CAPTURED:
		{
			ObjMessage.Add( bLocalTeam ? "has been secured" : "has been lost" );
			break;
		}

		default:
		{
			// if you hit this assert then something went wrong when sending
			// the message to update the objective status
			Assert( false );
			return;
		}
	}

	Print( ObjMessage );
}

//=========================================================
//=========================================================
#define REINFORCEMENTS_LOWMARK 4

void CHudMessagesGame::ReinforcementDeployed( int iRemaining )
{
	char szBuffer[ 64 ];
	szBuffer[ 0 ] = '\0';

	C_PlayTeam *pTeam = GetLocalPlayTeam( );

	if( !pTeam )
		return;

	bool bLimitedWaves = !pTeam->IsUnlimitedWaves( );

	if( bLimitedWaves && iRemaining == 0 )
	{
		Q_strncpy( szBuffer, "Last Reinforcement Deployed!", sizeof( szBuffer ) );
	}
	else
	{
		Q_strncat( szBuffer, "Reinforcements Deployed!", sizeof( szBuffer ) );

		if( bLimitedWaves && pTeam->GetNumWaves( ) >= REINFORCEMENTS_LOWMARK && iRemaining <= REINFORCEMENTS_LOWMARK )
		{
			char szLeftBuffer[ 32 ];
			Q_snprintf( szLeftBuffer, sizeof( szLeftBuffer ), " %i Left", iRemaining );

			Q_strncat( szBuffer, szLeftBuffer, sizeof( szBuffer ) );
		}
	}

	CColoredString ReinforceMessage;
	ReinforceMessage.Add( szBuffer );

	Print( ReinforceMessage );
}

//=========================================================
//=========================================================
CHudMessagesGameLine::CHudMessagesGameLine( CHudMessagesBase *pParent )
	: CHudMessagesLine( pParent )
{
	m_pGameParent = NULL;
}

//=========================================================
//=========================================================
void CHudMessagesGameLine::Init( void )
{
	BaseClass::Init( );

	m_pGameParent = ( CHudMessagesGame* )m_pParent;
}

//=========================================================
//=========================================================
void CHudMessagesGameLine::Paint( void )
{
	int iTexWide, iTexTall;
	iTexWide = m_pGameParent->BGWide( );
	iTexTall = m_pGameParent->BGTall( );

	Color DrawColor( 255, 255, 255, GetAlpha( ) );

	surface( )->DrawSetTexture( m_pGameParent->BGTexID( BGTYPE_LEFT ) );
	surface( )->DrawSetColor( DrawColor );
	surface( )->DrawTexturedRect( 0, 0, iTexWide, iTexTall );

	surface( )->DrawSetTexture( m_pGameParent->BGTexID( BGTYPE_MIDDLE ) );
	surface( )->DrawSetColor( DrawColor );
	surface( )->DrawTexturedRect( iTexWide, 0, m_iLineWidth - iTexWide, iTexTall );

	surface( )->DrawSetTexture( m_pGameParent->BGTexID( BGTYPE_RIGHT ) );
	surface( )->DrawSetColor( DrawColor );
	surface( )->DrawTexturedRect( m_iLineWidth - iTexWide, 0, m_iLineWidth, iTexTall );

	BaseClass::Paint( );
}

//=========================================================
//=========================================================
int CHudMessagesGameLine::GetHeightForce( void ) const
{
	return m_pGameParent->BGTall( );
}

//=========================================================
//=========================================================
int CHudMessagesGameLine::CalcTextWidth( void )
{
	return ( BaseClass::CalcTextWidth( ) + m_pGameParent->BGTall( ) );
}

//=========================================================
//=========================================================
void CHudMessagesGameLine::PerformLayout( void )
{
	BaseClass::PerformLayout( );

	SetDrawOffsets( m_pGameParent->BGTall( ) * 0.5f, ( GetHeightForce( ) * 0.5f ) - ( surface( )->GetFontTall( _font ) * 0.5f ) );
}