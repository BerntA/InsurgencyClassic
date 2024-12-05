//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"

#include "hud_comms.h"

#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

#include "inshud.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define COMMS_BG_HEADER "bg_header"
#define COMMS_BG_SUB "bg_sub"

#define COMMS_BG_WIDE 128
#define COMMS_BG_TALL 32

#define COMMS_BG_YGAP_SECTION -4
#define COMMS_BG_YGAP_OPTION -6

#define COMMS_TEXT_XPOS 36
#define COMMS_TEXT_YPOS 12

#define INVALID_COMOPTION -1

//=========================================================
//=========================================================
CommsGroupCreator_t g_CommsGroupHelpers[ COMGROUP_COUNT ];

//=========================================================
//=========================================================
CommsTex_t::CommsTex_t( )
{
	m_iSelectedID = m_iUnselectedID = -1;
}

//=========================================================
//=========================================================
void CommsGroup_t::AddOption( const char *pszName, bool bAddQuotes )
{
	Assert( pszName );

	if( !pszName )
		return;

	const char *pszNewName = NULL;
	char szName[ COMMS_OPTION_LENGTH ];

	if( bAddQuotes )
	{
		Q_snprintf( szName, sizeof( szName ), "\"%s\"", pszName );
		pszNewName = szName;
	}
	else
	{
		pszNewName = pszName;
	}

	int iOptionID = m_Options.AddToTail( );
	Assert( m_Options.IsValidIndex( iOptionID ) );

	CommsOption_t &Option = m_Options[ iOptionID ];

	localize( )->ConvertANSIToUnicode( pszNewName, Option.m_wszName, sizeof( Option.m_wszName ) );
	Option.m_iNameLength = Q_strlen( pszNewName );
}

//=========================================================
//=========================================================
void ICommsGroup::Init( CHUDComms *pParent )
{
	m_pParent = pParent;
}

//=========================================================
//=========================================================
void ICommsGroup::LoadIcon( const char *pszName, CommsTex_t &IconTex )
{
	char szBuffer[ 64 ];

	Q_strncpy( szBuffer, "icon_", sizeof( szBuffer ) );
	Q_strncat( szBuffer, pszName, sizeof( szBuffer ), COPY_ALL_CHARACTERS );

	m_pParent->LoadTex( szBuffer, IconTex );
}

//=========================================================
//=========================================================
CHUDComms *g_pHUDComms = NULL;

CHUDComms *HUDComms( void )
{
	Assert( g_pHUDComms );
	return g_pHUDComms;
}

//=========================================================
//=========================================================
CHUDComms::CHUDComms( const char *pszElementName ) :
	CHudElement( pszElementName ), BaseClass( NULL, "HudComms" )
{
	g_pHUDComms = this;

	Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	HScheme Scheme = scheme( )->LoadSchemeFromFile( "resource/ClientScheme.res", "ClientScheme" );
	SetScheme( Scheme );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );

	memset( m_pGroups, NULL, sizeof( m_pGroups ) );
	m_bIconsLoaded = false;

	m_iSelectedID = INVALID_COMOPTION;	
}

DECLARE_HUDELEMENT( CHUDComms );

//=========================================================
//=========================================================
CHUDComms::~CHUDComms( )
{
	g_pHUDComms = NULL;

}

//=========================================================
//=========================================================
void CHUDComms::Init( void )
{
	// init groups
	for( int i = 0; i < COMGROUP_COUNT; i++ )
	{
		CommsGroupCreator_t Creator = g_CommsGroupHelpers[ i ];
		Assert( Creator );

		m_pGroups[ i ] = ( Creator )( );
		ICommsGroup *pGroup = m_pGroups[ i ];
		Assert( pGroup );

		pGroup->Init( this );
	}

	// init bg's
	LoadTex( COMMS_BG_HEADER, m_BGHeader );
	LoadTex( COMMS_BG_SUB, m_BGSub );
}

//=========================================================
//=========================================================
void CHUDComms::LevelInit( void )
{
	if( m_bIconsLoaded )
		return;

	for( int i = 0; i < COMGROUP_COUNT; i++ )
		m_pGroups[ i ]->SetupIcons( );

	m_bIconsLoaded = true;
}

//=========================================================
//=========================================================
void CHUDComms::Reset( void )
{
	Close( );
}

//=========================================================
//=========================================================
void CHUDComms::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// find size and positions
	m_iBGWide = scheme( )->GetProportionalScaledValue( COMMS_BG_WIDE );
	m_iBGTall = scheme( )->GetProportionalScaledValue( COMMS_BG_TALL );

	m_iBGYGapSection = scheme( )->GetProportionalScaledValue( COMMS_BG_YGAP_SECTION );
	m_iBGYGapOption = scheme( )->GetProportionalScaledValue( COMMS_BG_YGAP_OPTION );

	m_iTextXPos = scheme( )->GetProportionalScaledValue( COMMS_TEXT_XPOS );
	m_iTextYPos = scheme( )->GetProportionalScaledValue( COMMS_TEXT_YPOS );

	// find font
	m_Font = pScheme->GetFont( "CommsOption", true );
}

//=========================================================
//=========================================================
void CHUDComms::Toggle( void )
{
	if( ShouldDraw( ) )
		Close( );
	else
		Activate( );
}

//=========================================================
//=========================================================
void CHUDComms::Activate( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->IsAlive( ) )
		return;

	Rebuild( );

	m_iSelectedID = 0;

	ControlTakeFocus( );
}

//=========================================================
//=========================================================
void CHUDComms::Close( void )
{
	m_iSelectedID = INVALID_COMOPTION;
}

//=========================================================
//=========================================================
bool CHUDComms::HasValidSelection( void ) const
{
	return ( m_iSelectedID != INVALID_COMOPTION );
}

//=========================================================
//=========================================================
void CHUDComms::Rebuild( void )
{
	// purgeold groups
	m_Groups.Purge( );

	// add all groups
	int iGroupCount, iOptionCount;
	iGroupCount = iOptionCount = 0;

	for( int i = 0; i < COMGROUP_COUNT; i++ )
	{
		ICommsGroup *pGroup = m_pGroups[ i ];

		// ... don't add if not visible
		if( !pGroup || !pGroup->IsVisible( ) )
			continue;

		// ... reset
		pGroup->Reset( );

		// ... setup
		int iGroupID = m_Groups.AddToTail( );
		CommsGroup_t &Group = m_Groups[ iGroupID ];
		Group.m_iGroupID = i;

		pGroup->SetupGroup( Group );

		// ... increment total options
		iGroupCount++;
		iOptionCount += Group.m_Options.Count( );
	}

	// store away count
	m_iOptionCount = iOptionCount;

	// work out start ypos
	iGroupCount = min( iGroupCount, 1 );
	m_iStartYPos = ( GetTall( ) * 0.5f ) - ( ( ( iOptionCount * m_iBGTall ) + ( ( iGroupCount - 1 ) * m_iBGYGapSection ) + ( ( iOptionCount - 1 ) * m_iBGYGapOption ) ) * 0.5f );
}

//=========================================================
//=========================================================
bool CHUDComms::ShouldDraw( void )
{
	return ( CHudElement::ShouldDraw( ) && HasValidSelection( ) );
}

//=========================================================
//=========================================================
void CHUDComms::Paint( void )
{
	int iYPos = m_iStartYPos;
	int iOptionCount = 0;

	for( int i = 0; i < m_Groups.Count( ); i++ )
	{
		CommsGroup_t &Group = m_Groups[ i ];

		CUtlVector< CommsOption_t > &Options = Group.m_Options;

		for( int j = 0; j < Options.Count( ); j++ )
		{
			CommsOption_t &Option = Options[ j ];

			bool bSelected = ( iOptionCount == m_iSelectedID );

			if( j == 0 )
				DrawGroup( Group.m_Icon, Option, bSelected, iYPos );
			else
				DrawOption( Option, bSelected, iYPos );

			iYPos += m_iBGTall;

			if( j != ( Options.Count( ) - 1 ) )
				 iYPos += m_iBGYGapOption;

			iOptionCount++;
		}

		iYPos += m_iBGYGapSection;
	}
}

//=========================================================
//=========================================================
void CHUDComms::DrawGroup( const CommsTex_t &Icon, const CommsOption_t &Option, bool bSelected, int iYPos )
{
	Draw( m_BGHeader, bSelected, iYPos );
	Draw( Icon, bSelected, iYPos );
	DrawOptionText( Option, bSelected, iYPos );
}

//=========================================================
//=========================================================
void CHUDComms::DrawOption( const CommsOption_t &Option, bool bSelected, int iYPos )
{
	Draw( m_BGSub, bSelected, iYPos );
	DrawOptionText( Option, bSelected, iYPos );
}

//=========================================================
//=========================================================
void CHUDComms::DrawOptionText( const CommsOption_t &Option, bool bSelected, int iYPos )
{
	static Color DrawColorSelected = COLOR_WHITE;
	static Color DrawColorUnselected = Color( 255, 255, 255, 144 );

	surface( )->DrawSetTextFont( m_Font );
	surface( )->DrawSetTextPos( m_iTextXPos, iYPos + m_iTextYPos  );
	surface( )->DrawSetTextColor( bSelected ? DrawColorSelected : DrawColorUnselected );
	surface( )->DrawPrintText( Option.m_wszName, Option.m_iNameLength );	
}

//=========================================================
//=========================================================
void CHUDComms::Draw( const CommsTex_t &Tex, bool bSelected, int iYPos )
{
	static Color DrawColor = COLOR_WHITE;

	Assert( Tex.m_iSelectedID != -1 && Tex.m_iUnselectedID != -1 );

	surface( )->DrawSetTexture( bSelected ? Tex.m_iSelectedID : Tex.m_iUnselectedID );
	surface( )->DrawSetColor( DrawColor );
	surface( )->DrawTexturedRect( 0, iYPos, m_iBGWide, m_iBGTall + iYPos );
}

//=========================================================
//=========================================================
bool CHUDComms::IsControlActive( void )
{
	return HasValidSelection( );
}

//=========================================================
//=========================================================
void CHUDComms::Scroll( int iType )
{
	m_iSelectedID += ( iType == INSHUD_SCROLLUP ) ? -1 : 1;

	if( m_iSelectedID < 0 )
		m_iSelectedID = m_iOptionCount - 1;
	else if( m_iSelectedID >= m_iOptionCount )
		m_iSelectedID = 0;
}

//=========================================================
//=========================================================
void CHUDComms::Selection( void )
{
	// find what we're selecting
	int iOptionTotal = 0;

	for( int i = 0; i < m_Groups.Count( ); i++ )
	{
		CommsGroup_t &Group = m_Groups[ i ];
		CUtlVector< CommsOption_t > &Options = Group.m_Options;

		int iOptionCount = Options.Count( );

		iOptionTotal += iOptionCount;

		if( iOptionTotal >= ( m_iSelectedID + 1 ) )
		{
			int iOptionID = m_iSelectedID - ( iOptionTotal - iOptionCount );
			Assert( Options.IsValidIndex( iOptionID ) );

			m_pGroups[ Group.m_iGroupID ]->Selected( iOptionID );

			break;
		}
	}
}

//=========================================================
//=========================================================
void CHUDComms::DoControlClose( void )
{
	Close( );
}

//=========================================================
//=========================================================
void CHUDComms::PlayerDeath( void )
{
	m_iSelectedID = INVALID_COMOPTION;
}

//=========================================================
//=========================================================
void CHUDComms::LoadTex( const char *pszName, CommsTex_t &CommsTex )
{
	LoadTex( pszName, CommsTex, true );
	LoadTex( pszName, CommsTex, false );
}

//=========================================================
//=========================================================
void CHUDComms::LoadTex( const char *pszName, CommsTex_t &CommsTex, bool bSelection )
{
	char szBuffer[ 64 ];
	FindPath( pszName, bSelection, szBuffer, sizeof( szBuffer ) );

	int &iTexID = bSelection ? CommsTex.m_iSelectedID : CommsTex.m_iUnselectedID;

	iTexID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( iTexID, szBuffer, false, false );
}

//=========================================================
//=========================================================
void CHUDComms::FindPath( const char *pszName, bool bSelected, char *pszBuffer, int iLength )
{
	Q_snprintf( pszBuffer, iLength, "hud/comms/%s_%s", pszName, bSelected ? "selected" : "unselected" );
}

//=========================================================
//=========================================================
void CC_Comms( void )
{
	HUDComms( )->Toggle( );
}

ConCommand comms( "comms", CC_Comms );
