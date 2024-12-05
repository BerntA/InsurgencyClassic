//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//
#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"

#include <vgui/isurface.h>
#include <vgui_controls/panel.h>

#include "ins_player_shared.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define PATH_SCOPETEX "hud/scope/crosshair"

//=========================================================
//=========================================================
class CHudSniper : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudSniper, Panel );

public:
	CHudSniper( const char *pElementName );
	
private:
	void Init( void );

	bool ShouldDraw( void );
	void Paint( void );
	void OnScreenSizeChanged(int iOldWide, int iOldTall);

	void UpdateSize( void );

private:
	int m_iTexID, m_iWide, m_iTall;
};

//=========================================================
//=========================================================
CHudSniper::CHudSniper( const char *pElementName )
	: CHudElement( pElementName ), vgui::Panel( NULL, "HudSniper" )
{
	Panel *pPanel = g_pClientMode->GetViewport( );
	SetParent( pPanel );

	SetPaintBackgroundEnabled( false );
	SetPaintBorderEnabled( false );

}

DECLARE_HUDELEMENT( CHudSniper );

//=========================================================
//=========================================================
void CHudSniper::Init( void )
{
	m_iTexID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( m_iTexID, PATH_SCOPETEX, false, false );

	UpdateSize( );
}

//=========================================================
//=========================================================
void CHudSniper::OnScreenSizeChanged( int iOldWide, int iOldTall )
{
	UpdateSize( );
}

//=========================================================
//=========================================================
void CHudSniper::UpdateSize( void )
{
	surface( )->GetScreenSize( m_iWide, m_iTall );
	SetBounds( 0, 0, m_iWide, m_iTall );
}

//=========================================================
//=========================================================
bool CHudSniper::ShouldDraw( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return false;

	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon( );

	if( !pWeapon )
		return false;

	if( pWeapon->GetWeaponClass( ) != WEAPONCLASS_SNIPER )
		return false;
	
	if( ( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS ) == 0 )
		return false;
	
	if( pWeapon->ShouldDrawViewModel( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CHudSniper::Paint( void )
{
	surface( )->DrawSetColor( COLOR_WHITE );
	surface( )->DrawSetTexture( m_iTexID );
	surface( )->DrawTexturedRect( 0, 0, m_iWide, m_iTall );
}