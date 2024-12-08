//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include <vgui/ischeme.h>
#include <vgui/isurface.h>
#include <vgui/ilocalize.h>

#include "insvgui.h"
#include "hud_boxed.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
enum ActionMenuNumbers_t
{
	AMN_ONE = 0,
	AMN_TWO,
	AMN_THREE,
	AMN_FOUR,
	AMN_FIVE,
	AMN_SIX,
	AMN_SEVEN,
	AMN_EIGHT,
	AMN_NINE,
	AMN_COUNT
};

//=========================================================
//=========================================================
#define ACTIONMENU_SECTIONINVALID -1
#define ACTIONMENU_ITEMINVALID ACTIONMENU_SECTIONINVALID

#define MAX_AMELEMENT_LENGTH 64
#define MAX_AMITEMEXT_LENGTH 8

//=========================================================
//=========================================================
#define NUMBERS_PATH "HUD/actionmenu/number"

#define NUMBERS_WIDE 8
#define NUMBERS_TALL 8
#define NUMBERS_XPOS 3
#define NUMBERS_YPOS 20
#define NUMBERS_YGAP 12

#define TITLE_XPOS 8
#define TITLE_YPOS 5

#define TEXT_XPOS 14
#define TEXT_EXT_GAP 2

//=========================================================
//=========================================================
struct MenuElement_t
{
	MenuElement_t( );

	wchar_t m_wszText[ MAX_AMELEMENT_LENGTH ];
	int m_iLength;
};

struct MenuItem_t : public MenuElement_t
{
	MenuItem_t( );

	wchar_t m_wszExtText[ MAX_AMITEMEXT_LENGTH ];
	int m_iExtLength;

	bool m_bAction;
	int m_iID;
	int m_iSectionID;
};

struct MenuSection_t : public MenuElement_t
{
	MenuSection_t( );

	int m_iSectionID;
	bool m_bShowHeader;
	int m_iItems;
};

//=========================================================
//=========================================================
MenuElement_t::MenuElement_t( )
{
	m_iLength = 0;
}

//=========================================================
//=========================================================
MenuItem_t::MenuItem_t( )
{
	m_iExtLength = 0;
}

//=========================================================
//=========================================================
MenuSection_t::MenuSection_t( )
{
	m_iItems = 0;
}

//=========================================================
//=========================================================
class CHudActionMenu : public CHUDBoxed, public IINSMenuManager
{
	DECLARE_CLASS_SIMPLE( CHudActionMenu, CHUDBoxed );

public:
	CHudActionMenu( const char *pszName );

private:
	MenuSection_t *FindSection( int iSectionID ) const;

private:
	void Init( void );
	void Reset( void );
	void LevelShutdown( void );

	void ApplySchemeSettings( IScheme *pScheme );

	bool ShouldDraw( void );
	void Paint( void );

	void OnKeyCodePressed( vgui::KeyCode Code );

	void ResetMenu( void );
	void ResetItems( void );

	void ShowMenu( int iID );
	void UpdateMenu( void );
	void CloseMenu( void );

	int GetActiveMenu( void ) const;

	int AddSection( int iSectionID, const char *pszName );
	int AddItem( int iSectionID, const char *pszName, const char *pszExtText, bool bHasAction );
	int GetSection( int iItemID );

	void UpdatePages( void );
	void NextPage( void );
	bool IsPageBroken( int iNumberID, int iLastSectionID, int iSectionID, int iItemID );

	void SetupElement( MenuElement_t *pElement, const char *pszText );

	void DrawSection( int &iYPos, MenuSection_t *pSection );
	void DrawItem( int &iYPos, int iNumberID, MenuItem_t *pItem, bool bHasAction );
	void DrawElement( int iXPos, int &iYPos, Color &DrawColor, MenuElement_t *pElement );

private:
	IINSMenu *m_pCurrentMenu;
	int m_iCurrentMenuID;

	vgui::HFont m_Elements, m_ElementsSmall;
	int m_iNumbersTexID[ AMN_COUNT + 1 ];
	int m_iNumbersWide, m_iNumbersTall;
	int m_iNumbersXPos, m_iNumbersYPos, m_iNumbersYGap;
	int m_iTitleXPos, m_iTitleYPos;
	int m_iTextXPos, m_iTextExtGap;

	CUtlLinkedList< MenuSection_t* > m_Sections;
	CUtlVector< MenuItem_t* > m_Items;
	CUtlVector< MenuItem_t* > m_SortedItems;

	MenuItem_t *m_pNext;

	int m_iCurrentPageID;
	CUtlVector< int > m_Pages;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudActionMenu );

CHudActionMenu::CHudActionMenu( const char *pszName )
	: CHUDBoxed( pszName, "HudActionMenu" )
{
	GetINSVGUIHelper( )->RegisterMenuManager( this );

	// setup menus
	m_pCurrentMenu = NULL;
	m_iCurrentMenuID = INSMENU_INVALID;

	// setup next item
	m_pNext = new MenuItem_t;
	SetupElement( m_pNext, "Next" );

	// setup panel
	SetAllEdges( );
	MakePopup( );
}

//=========================================================
//=========================================================
void CHudActionMenu::Init( void )
{
	BaseClass::Init( );

	char szPath[ 128 ];

	for( int i = 0; i <= AMN_COUNT; i++ )
	{
		Q_snprintf( szPath, sizeof( szPath ), "%s_%02i", NUMBERS_PATH, i );

		m_iNumbersTexID[ i ] = vgui::surface()->CreateNewTextureID( );
		vgui::surface( )->DrawSetTextureFile( m_iNumbersTexID[ i ], szPath, false, false );
	}
}

//=========================================================
//=========================================================
void CHudActionMenu::Reset( void )
{
	if( m_pCurrentMenu && m_pCurrentMenu->IgnoreResetHUD( ) )
		return;

	ResetMenu( );
}

//=========================================================
//=========================================================
void CHudActionMenu::LevelShutdown( void )
{
	CloseMenu( );
}

//=========================================================
//=========================================================
void CHudActionMenu::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_Elements = pScheme->GetFont( "Default" );
	m_ElementsSmall = pScheme->GetFont( "DefaultSmall" );

	m_iNumbersWide = scheme( )->GetProportionalScaledValue( NUMBERS_WIDE );
	m_iNumbersTall = scheme( )->GetProportionalScaledValue( NUMBERS_TALL );
	m_iNumbersXPos = scheme( )->GetProportionalScaledValue( NUMBERS_XPOS );
	m_iNumbersYPos = scheme( )->GetProportionalScaledValue( NUMBERS_YPOS );
	m_iNumbersYGap = scheme( )->GetProportionalScaledValue( NUMBERS_YGAP );

	m_iTitleXPos = scheme( )->GetProportionalScaledValue( TITLE_XPOS );
	m_iTitleYPos = scheme( )->GetProportionalScaledValue( TITLE_YPOS );

	m_iTextXPos = scheme( )->GetProportionalScaledValue( TEXT_XPOS );
	m_iTextExtGap = scheme( )->GetProportionalScaledValue( TEXT_EXT_GAP );
}

//=========================================================
//=========================================================
bool CHudActionMenu::ShouldDraw( void )
{
	return HasActiveMenu( );
}

//=========================================================
//=========================================================
void CHudActionMenu::Paint( void )
{
	const char *pszTitle = m_pCurrentMenu->GetTitle( );

	if( pszTitle && pszTitle[ 0 ] )
	{
		wchar_t wszBuffer[ 128 ];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszTitle, wszBuffer, sizeof( wszBuffer ) );

		surface( )->DrawSetTextColor( COLOR_GREY );
		surface( )->DrawSetTextFont( m_Elements );
		surface( )->DrawSetTextPos( m_iTitleXPos, m_iTitleYPos );
		surface( )->DrawPrintText( wszBuffer, Q_strlen( pszTitle ) );
	}

	if( !m_Pages.IsValidIndex( m_iCurrentPageID ) )
		return;

	int iYPos = m_iNumbersYPos;
	int iLastSectionID = ACTIONMENU_SECTIONINVALID;

	for( int i = m_Pages[ m_iCurrentPageID ], iNumberID = 0; i < m_SortedItems.Count( ) && iNumberID < AMN_COUNT + 1; i++, iNumberID++ )
	{
		MenuItem_t *pItem = m_SortedItems[ i ];

		if( !pItem )
			continue;

		int iSectionID = pItem->m_iSectionID;

		if( IsPageBroken( iNumberID, iLastSectionID, pItem->m_iSectionID, i ) )
		{
			DrawItem( iYPos, ACTIONMENU_ITEMINVALID, m_pNext, true );
			return;
		}

		if( iLastSectionID != iSectionID )
		{
			MenuSection_t *pSection = FindSection( pItem->m_iSectionID );
			Assert( pSection );

			if( pSection && pSection->m_bShowHeader )
				DrawSection( iYPos, pSection );
		}

		DrawItem( iYPos, iNumberID, pItem, pItem->m_bAction );

		iLastSectionID = iSectionID;
	}
}

//=========================================================
//=========================================================
void CHudActionMenu::OnKeyCodePressed( KeyCode Code )
{
	if( HasActiveMenu( ) )
	{
		if( Code == KEY_ESCAPE )
			CloseMenu( );

		if( Code == KEY_0 )
			NextPage( );

		if( Code >= KEY_1 && Code <= KEY_9 )
		{
			int iNumberID = AMN_ONE;

			if( Code != KEY_0 )
				iNumberID = ( Code - KEY_1 );

			int iActionItemID = m_Pages[ m_iCurrentPageID ] + iNumberID;

			if( m_SortedItems.IsValidIndex( iActionItemID ) )
			{
				if( m_pCurrentMenu->Action( m_SortedItems[ iActionItemID ]->m_iID ) )
					CloseMenu( );
			}
		}
	}

	BaseClass::OnKeyCodePressed( Code );
}

//=========================================================
//=========================================================
void CHudActionMenu::ResetMenu( void )
{
	ResetItems( );

	m_pCurrentMenu = NULL;
	m_iCurrentMenuID = INSMENU_INVALID;

	SetKeyBoardInputEnabled( false );
	SetVisible( false );
	SetEnabled( false );
}

//=========================================================
//=========================================================
void CHudActionMenu::ResetItems( void )
{
	m_Sections.RemoveAll( );
	m_Items.RemoveAll( );
	m_SortedItems.RemoveAll( );
}

//=========================================================
//=========================================================
void CHudActionMenu::ShowMenu( int iID )
{
	m_pCurrentMenu = GetINSVGUIHelper( )->GetMenu( iID );
	Assert( m_pCurrentMenu );

	if( !m_pCurrentMenu )
		return;

	m_iCurrentMenuID = iID;

	UpdateMenu( );

	SetKeyBoardInputEnabled( true );
	SetMouseInputEnabled( false );
	MoveToFront( );
	RequestFocus( );
	SetVisible( true );
	SetEnabled( true );
}

//=========================================================
//=========================================================
void CHudActionMenu::UpdateMenu( void )
{
	if( !m_pCurrentMenu )
		return;

	ResetItems( );

	m_pCurrentMenu->Setup( );

	UpdatePages( );
}

//=========================================================
//=========================================================
void CHudActionMenu::CloseMenu( void )
{
	if( !m_pCurrentMenu )
		return;

	IINSMenu *pClosedMenu = m_pCurrentMenu;

	ResetMenu( );

	pClosedMenu->Closed( );
}

//=========================================================
//=========================================================
int CHudActionMenu::GetActiveMenu( void ) const
{
	return m_iCurrentMenuID;
}

//=========================================================
//=========================================================
int CHudActionMenu::AddSection( int iSectionID, const char *pszName )
{
	MenuSection_t *pMenuSection = new MenuSection_t;
	pMenuSection->m_iSectionID = iSectionID;
	pMenuSection->m_bShowHeader = ( pszName != NULL );

	if( pMenuSection->m_bShowHeader )
		SetupElement( pMenuSection, pszName );

	return ( m_Sections.AddToTail( pMenuSection ) != m_Sections.InvalidIndex( ) );
}

//=========================================================
//=========================================================
int CHudActionMenu::AddItem( int iSectionID, const char *pszName, const char *pszExtText, bool bHasAction )
{
	if( !FindSection( iSectionID ) )
		return ACTIONMENU_ITEMINVALID;

	MenuItem_t *pMenuItem = new MenuItem_t;
	SetupElement( pMenuItem, pszName );

	if( pszExtText )
	{
		g_pVGuiLocalize->ConvertANSIToUnicode( pszExtText, pMenuItem->m_wszExtText, sizeof( pMenuItem->m_wszExtText ) );
		pMenuItem->m_iExtLength = Q_strlen( pszExtText );
	}
	else
	{
		pMenuItem->m_iExtLength = 0;
	}

	pMenuItem->m_bAction = bHasAction;
	pMenuItem->m_iSectionID = iSectionID;
	pMenuItem->m_iID = m_Items.AddToTail( pMenuItem );

	if( pMenuItem->m_iID == m_Items.InvalidIndex( ) )
		return ACTIONMENU_ITEMINVALID;

	return pMenuItem->m_iID;
}

//=========================================================
//=========================================================
int CHudActionMenu::GetSection( int iItemID )
{
	return m_Items[ iItemID ]->m_iSectionID;
}

//=========================================================
//=========================================================
MenuSection_t *CHudActionMenu::FindSection( int iSectionID ) const
{
	for( int i = 0; i < m_Sections.Count( ); i++ )
	{
		MenuSection_t *pSection = m_Sections[ i ];

		if( pSection && pSection->m_iSectionID == iSectionID )
			return pSection;
	}

	return NULL;
}

//=========================================================
//=========================================================
void CHudActionMenu::SetupElement( MenuElement_t *pElement, const char *pszText )
{
	g_pVGuiLocalize->ConvertANSIToUnicode( pszText, pElement->m_wszText, sizeof( pElement->m_wszText ) );
	pElement->m_iLength = Q_strlen( pszText );
}

//=========================================================
//=========================================================
void CHudActionMenu::DrawSection( int &iYPos, MenuSection_t *pSection )
{
	DrawElement( m_iTextXPos - m_iNumbersXPos, iYPos, COLOR_GREY, pSection );
}

//=========================================================
//=========================================================
void CHudActionMenu::DrawItem( int &iYPos, int iNumberID, MenuItem_t *pItem, bool bHasAction )
{
	int iXPos = 0;

	if( bHasAction )
	{
		surface( )->DrawSetTexture( m_iNumbersTexID[ iNumberID + 1 ] );
		surface( )->DrawSetColor( COLOR_WHITE );
		surface( )->DrawTexturedRect( m_iNumbersXPos, iYPos, m_iNumbersXPos + m_iNumbersWide, iYPos + m_iNumbersTall );

		iXPos = m_iTextXPos;
	}
	else
	{
		iXPos = m_iTextXPos - m_iNumbersXPos;
	}

	if( pItem->m_iExtLength != 0 )
	{
		int iExtXPos = iXPos + UTIL_ComputeStringWidth( m_Elements, pItem->m_wszText ) + m_iTextExtGap;

		surface()->DrawSetTextColor( COLOR_GREY );
		surface()->DrawSetTextFont( m_ElementsSmall );
		surface()->DrawSetTextPos( iExtXPos, iYPos );
		surface()->DrawPrintText( pItem->m_wszExtText, pItem->m_iExtLength );
	}

	DrawElement( iXPos, iYPos, COLOR_WHITE, pItem );
}

//=========================================================
//=========================================================
void CHudActionMenu::DrawElement( int iXPos, int &iYPos, Color &DrawColor, MenuElement_t *pElement )
{
	surface()->DrawSetTextColor( DrawColor );
	surface()->DrawSetTextFont( m_Elements );
	surface()->DrawSetTextPos( iXPos, iYPos );
	surface()->DrawPrintText( pElement->m_wszText, pElement->m_iLength );

	iYPos += m_iNumbersYGap;
}



//=========================================================
//=========================================================
typedef MenuItem_t *MenuItemPtr_t;

int SortItems( const MenuItemPtr_t *pLeft, const MenuItemPtr_t *pRight )
{
	MenuItem_t *pItemLeft = *pLeft;
	MenuItem_t *pItemRight = *pRight;

	// first compare sections
	if( pItemLeft->m_iSectionID > pItemRight->m_iSectionID )
		return 1;
	else if( pItemLeft->m_iSectionID < pItemRight->m_iSectionID )
		return -1;

	// next compare items
	if( pItemLeft->m_iID > pItemRight->m_iID )
		return 1;
	else if( pItemLeft->m_iID < pItemRight->m_iID )
		return -1;

	return 0;
}

//=========================================================
//=========================================================
void CHudActionMenu::UpdatePages( void )
{
	m_Pages.RemoveAll( );

	// add the first page
	m_iCurrentPageID = m_Pages.AddToTail( 0 );

	// add the items
	m_SortedItems.AddVectorToTail( m_Items );

	// sort them
	m_SortedItems.Sort( SortItems );

	// work out pages
	int iNumberID = 0;
	int iLastSectionID = ACTIONMENU_SECTIONINVALID;

	for( int i = 0; i < m_SortedItems.Count( ); i++, iNumberID++ )
	{
		MenuItem_t &Item = *m_SortedItems[ i ];

		if( IsPageBroken( iNumberID, iLastSectionID, Item.m_iSectionID, i ) )
		{
			iNumberID = 0;
			m_Pages.AddToTail( i );
		}

		iLastSectionID = Item.m_iSectionID;
	}
}

//=========================================================
//=========================================================
void CHudActionMenu::NextPage( void )
{
	if( m_iCurrentPageID + 1 >= m_Pages.Count( ) )
		return;

	m_iCurrentPageID++;
}

//=========================================================
//=========================================================
bool CHudActionMenu::IsPageBroken( int iNumberID, int iLastSectionID, int iSectionID, int iItemID )
{
	if( iNumberID >= AMN_COUNT )
		return true;

	if( iLastSectionID != iSectionID && iNumberID >= ( AMN_COUNT * 0.5f ) )
		return true;

	MenuSection_t *pSection = FindSection( iSectionID );

	if( !pSection || pSection->m_iItems >= ( AMN_COUNT - iNumberID ) )
		return true;

	return false;
}