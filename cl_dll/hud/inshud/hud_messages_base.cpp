//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include <vgui/vgui.h>
#include <vgui/ivgui.h>
#include "iclientmode.h"
#include "hud_messages_base.h"
#include "hud_macros.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
void CMessagesColorLookup::FindColor( int iID, Color &StringColor ) const
{
	CColorLookup::FindColor( iID, StringColor );

	static Color DefaultColor( 245, 245, 245 );
	static Color ServerColor( 255, 180, 180 );
	static Color ThirdPersonColor( 255, 176, 0 );

	switch( iID )
	{
		case CLOOKUP_DEFAULT:
		{
			StringColor = DefaultColor;
			break;
		}
		
		case CLOOKUP_SERVER:
		{
			StringColor = ServerColor;
			break;
		}

		case CLOOKUP_THIRDPERSON:
		{
			StringColor = ThirdPersonColor;
			break;
		}
	}
}

//=========================================================
//=========================================================
CHudMessagesBase::CHudMessagesBase( const char *pszElementName, const char *pszPanelName )
	: CHudElement( pszElementName ), vgui::EditablePanel( NULL, pszPanelName )
{
	// set the parent
	vgui::Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetScheme( "ClientScheme" );

	// setup panel

	// ... setup visibilty
	SetVisible( false );
	SetPaintBackgroundEnabled( false );

	// .. add tick signal
	ivgui( )->AddTickSignal( GetVPanel( ), 100 );

	// init members
	m_hFont = INVALID_FONT;
	m_iFontHeight = 0;
}

//=========================================================
//=========================================================
void CHudMessagesBase::Init( void )
{
	for( int i = 0; i < LineCount( ); i++ )
	{
		CHudMessagesLine *pLine = CreateNewLine( );
		Assert( pLine );

		pLine->SetVisible( false );

		m_ChatLines.AddToTail( pLine );

		pLine->Init( );
	}
}

//=========================================================
//=========================================================
void CHudMessagesBase::LevelInit( const char *pszNewMap )
{
	Clear( );
}

//=========================================================
//=========================================================
void CHudMessagesBase::LevelShutdown( void )
{
	Clear( );
}

//=========================================================
//=========================================================
void CHudMessagesBase::Reset( void )
{
	Clear( );
}

//=========================================================
//=========================================================
void CHudMessagesBase::Clear( void )
{
	for( int i = 0; i < m_ChatLines.Count( ); i++ )
	{
		CHudMessagesLine *pLine = m_ChatLines[ i ];

		if( pLine && pLine->IsVisible( ) )
			pLine->Expire( );
	}
}

//=========================================================
//=========================================================
void CHudMessagesBase::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// setup fonts
	const char *pszFontName = FontName( );
	Assert( pszFontName );

	if( pszFontName )
		m_hFont = pScheme->GetFont( FontName( ), FontScaled( ) );

	// apply to lines
	for( int i = 0; i < m_ChatLines.Count( ); i++ )
	{
		CHudMessagesLine *pLine = m_ChatLines[ i ];

		if( pLine )
			pLine->SetFont( m_hFont );
	}

	m_iFontHeight = surface( )->GetFontTall( m_hFont ) + 2;

	// disable input
	SetKeyBoardInputEnabled( false );
	SetMouseInputEnabled( false );
}


//=========================================================
//=========================================================
typedef CHudMessagesLine *MessageLinePtr_t;

int SortLines( const MessageLinePtr_t *pLine1, const MessageLinePtr_t *pLine2 )
{
	CHudMessagesLine *pHudLine1 = *pLine1;
	CHudMessagesLine *pHudLine2 = *pLine2;

	// invisible at bottom
	if( pHudLine1->IsVisible( ) && !pHudLine2->IsVisible( ) )
		return -1;
	else if ( !pHudLine1->IsVisible( ) && pHudLine2->IsVisible( ) )
		return 1;

	// oldest start time at top
	if( pHudLine1->GetStartTime( ) < pHudLine2->GetStartTime( ) )
		return -1;
	else if ( pHudLine1->GetStartTime( ) > pHudLine2->GetStartTime( ) )
		return 1;

	// otherwise, compare counter
	if( pHudLine1->GetCount( ) < pHudLine2->GetCount( ) )
		return -1;
	else if ( pHudLine1->GetCount( ) > pHudLine2->GetCount( ) )
		return 1;

	return 0;
}

//=========================================================
//=========================================================
void CHudMessagesBase::OnTick( void )
{
	if( !IsVisible( ) )
		return;

	Update( );
}

//=========================================================
//=========================================================
void CHudMessagesBase::Update( void )
{
	// expire old lines
	for( int i = 0; i < m_ChatLines.Count( ); i++ )
	{
		CHudMessagesLine *pLine = m_ChatLines[ i ];

		if( pLine && pLine->IsVisible( ) && pLine->IsReadyToExpire( ) )
			pLine->Expire( );
	}

	// sort chat lines 
	m_ChatLines.Sort( SortLines );

	// start drawing
	int iWide, iHeight;
	int iCurrentY;

	GetSize( iWide, iHeight );
	iCurrentY = 0;

	if( DrawBackwards( ) )
	{
		// step backward from bottom
		iCurrentY = iHeight - m_iFontHeight - 1;

		// walk backward
		for( int i = m_ChatLines.Count( ) - 1; i >= 0 ; i-- )
		{
			CHudMessagesLine *pLine = m_ChatLines[ i ];

			if( !UpdateLine( pLine, iCurrentY ) )
				continue;
			
			iCurrentY -= ( FindLineHeight( pLine ) + pLine->GetYGap( ) ) * pLine->GetNumLines( );
		}
	}
	else
	{
		// step forward from top and Walk forward
		for( int i = 0; i < m_ChatLines.Count( ); i++ )
		{
			CHudMessagesLine *pLine = m_ChatLines[ i ];

			if( !UpdateLine( pLine, iCurrentY ) )
				continue;

			iCurrentY += ( FindLineHeight( pLine ) + pLine->GetYGap( ) ) * pLine->GetNumLines( );
		}
	}

	// move this panel to the back
	surface( )->MovePopupToBack( GetVPanel( ) );
}

//=========================================================
//=========================================================
void CHudMessagesBase::Print( CColoredString &Message )
{
	CHudMessagesLine *pUnusedLine = FindUnusedLine( );

	// find a line
	if( !pUnusedLine )
	{
		ExpireOldestLine( );
		pUnusedLine = FindUnusedLine( );
	}

	if( !pUnusedLine )
	{
		Assert( false );
		return;
	}

	// print it 
	pUnusedLine->Print( Message );

	// we're visible now
	SetVisible( true );

	// reorder etc
	Update( );
}

//=========================================================
//=========================================================
bool CHudMessagesBase::UpdateLine( CHudMessagesLine *pLine, int iCurrentY )
{
	if( !pLine )
		return false;

	int iWide, iTall;
	iWide = GetWide( );
	iTall = FindLineHeight( pLine );

	if( !pLine->IsVisible( ) )
	{
		pLine->SetSize( iWide, iTall );
		return false;
	}

	int iXPos = 0;

	if( LineRightAlign( ) )
		iXPos = GetWide( ) - pLine->LineWidth( );

	pLine->PerformFadeout( );
	pLine->SetSize( iWide, iTall * pLine->GetNumLines( ) );
	pLine->SetPos( iXPos, ( iCurrentY + iTall ) - ( iTall * pLine->GetNumLines( ) ) );

	return true;
}

//=========================================================
//=========================================================
void CHudMessagesBase::ExpireOldestLine( void )
{
	// find the oldest line
	float flOldestTime = FLT_MAX;
	CHudMessagesLine *pOldestLine = NULL;

	for( int i = 0; i < m_ChatLines.Count( ); i++ )
	{
		CHudMessagesLine *pLine = m_ChatLines[ i ];

		if( !pLine || !pLine->IsVisible( ) )
			continue;

		if( !pOldestLine )
		{
			pOldestLine = pLine;
			flOldestTime = pLine->GetStartTime( );
		}
		else if( pOldestLine->GetStartTime( ) < flOldestTime )
		{
			pOldestLine = pLine;
			flOldestTime = pLine->GetStartTime( );
		}
	}

	// if it's invalid - steal the first one
	if( !pOldestLine )
		pOldestLine = m_ChatLines[ 0 ];

	// expire it
	pOldestLine->Expire( );
}

//=========================================================
//=========================================================
CHudMessagesLine *CHudMessagesBase::CreateNewLine( void )
{
	return new CHudMessagesLine( this );
}

//=========================================================
//=========================================================
CHudMessagesLine *CHudMessagesBase::FindUnusedLine( void )
{
	for( int i = 0; i < m_ChatLines.Count( ); i++ )
	{
		CHudMessagesLine *pLine = m_ChatLines[ i ];

		if( pLine && !pLine->IsVisible( ) )
		{
			pLine->SetAlpha( 255 );
			return pLine;
		}
	}

	return NULL;
}

//=========================================================
//=========================================================
int CHudMessagesBase::FindLineHeight( const CHudMessagesLine *pLine ) const
{
	int iLineHeight = pLine->GetHeightForce( );

	if( iLineHeight == 0 )
		iLineHeight = m_iFontHeight;

	return iLineHeight;
}

//=========================================================
//=========================================================
CHudMessagesLine::CHudMessagesLine( CHudMessagesBase *pParent ) : 
	CINSRichText( pParent, "ScrollLine" )
{
	m_pParent = pParent;

	m_flExpireTime = 0.0f;
	m_flStartTime = 0.0f;

	m_iLineWidth = 0;

	SetVerticalScrollbar( false );
}

//=========================================================
//=========================================================
void CHudMessagesLine::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetBorder( NULL );

	SetBgColor( Color( 0, 0, 0, 0 ) );
	SetFgColor( Color( 0, 0, 0, 0 ) );
}

//=========================================================
//=========================================================
void CHudMessagesLine::PerformLayout( void )
{
	
}

//=========================================================
//=========================================================
void CHudMessagesLine::Print( CColoredString &Message )
{
	m_flStartTime = gpGlobals->curtime;
	m_flExpireTime = m_flStartTime + m_pParent->LineShowTime( );

	m_pParent->IncrementLineCounter( );
	m_iCount = m_pParent->GetLineCounter( );

	ClearText( );
	Message.Draw( this, m_pParent->ColorLookup( ) );

	if( m_pParent->CalculateLineWidth( ) )
		m_iLineWidth = CalcTextWidth( );

	SetVisible( true );
}

//=========================================================
//=========================================================
bool CHudMessagesLine::IsReadyToExpire( void )
{
	if( !engine->IsInGame( ) && !engine->IsConnected( ) )
		return true;

	if( gpGlobals->curtime >= m_flExpireTime )
		return true;

	return false;
}

//=========================================================
//=========================================================
void CHudMessagesLine::Expire( void )
{
	SetVisible( false );
}

//=========================================================
//=========================================================
void CHudMessagesLine::PerformFadeout( void )
{
	float flFadeTime = m_pParent->LineFadeTime( );

	if( gpGlobals->curtime <= m_flExpireTime && gpGlobals->curtime > ( m_flExpireTime - flFadeTime ) )
		SetAlpha( clamp( RoundFloatToInt( ( ( m_flExpireTime - gpGlobals->curtime ) / flFadeTime ) * 255.0f ), 0, 255 ) );
}