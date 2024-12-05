//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "clientmode_shared.h"

#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

#include "hudelement.h"
#include <vgui_controls/panel.h>

#include "inshud.h"
#include "basic_colors.h"

#include "weapon_ins_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================

// TODO: handle client latency issues due to UserMsg's
// coming 'in' before network tables

//=========================================================
//=========================================================
#define WEAPONINFO_SHOWTIME 2.5f

#define WEAPONINFO_ICON_NORMAL_SIZE 32
#define WEAPONINFO_ICON_NORMAL_TALL 32

#define WEAPONINFO_ICON_SLANTED_SIZE 64
#define WEAPONINFO_ICON_SLANTED_TALL 39

#define WEAPONINFO_ICON_YGAP 4

#define WEAPONINFO_AMMOLEFT_FONT "MagsLeft"
#define WEAPONINFO_AMMOLEFT_XPOS 5
#define WEAPONINFO_AMMOLEFT_YPOS 2

#define WEAPONINFO_AMMO_PATH "ammo"
#define WEAPONINFO_ROF_PATH "rof"

//=========================================================
//=========================================================
class CHudWeaponInfo : public CHudElement, public Panel, public IINSWeaponInfo, public IINSFireMode
{
	DECLARE_CLASS_SIMPLE( CHudWeaponInfo, Panel );

public:
	CHudWeaponInfo( const char *pszElementName );

	int CreateAmmoTexID( const char *pszName );
	void ResetInfo( void );
	void ShowAmmoInfo( float flTime, bool bAddDefault );
	void ShowAmmoInfo( void );
	void ShowROFInfo( float flTime, bool bAddDefault );
	void ShowROFInfo( void );

	void FireMode( int iFireMode );

private:
	void Init( void );

	void ApplySchemeSettings( IScheme *pScheme );

	void Paint( void );
	void DrawROF( int &iYPos );
	void DrawAmmo( int &iYPos );
	int DrawIcon( int iTexID, int &iYPos, bool bSlanted );

	void ShowInfo( float &flTimer, float flTime, bool bAddDefault );

	void CreateTexPath( const char *pszPath, const char *pszExtension, char *pszBuffer, int iLength );

private:
	int m_iROFTexIDs[ FIREMODE_COUNT ];

	int m_iIconNormalSize, m_iIconNormalTall;
	int m_iIconSlantedSize, m_iIconSlantedTall;
	int m_iIconYGap;

	float m_flAmmoShowTime;
	float m_flROFShowTime;

	HFont m_hAmmoFont;

	int m_iAmmoFontTall;
	int m_iAmmoFontXPos, m_iAmmoFontYPos;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudWeaponInfo );

CHudWeaponInfo::CHudWeaponInfo( const char *pElementName )
: CHudElement( pElementName ), BaseClass( NULL, "HudWeaponInfo" )
{
	Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );

	memset( m_iROFTexIDs, 0, sizeof( m_iROFTexIDs ) );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::Init( void )
{
	char szPathBuffer[ 64 ];

	// register
	GetINSHUDHelper( )->RegisterWeaponInfo( this );

	// load ROF
	static const char *pszROF[ FIREMODE_COUNT ] = {
		"semi",		// FIREMODE_SEMI
		"burst",	// FIREMODE_3RNDBURST
		"auto"		// FIREMODE_FULLAUTO
	};

	for( int i = 0; i < FIREMODE_COUNT; i++ )
	{
		CreateTexPath( WEAPONINFO_ROF_PATH, pszROF[ i ], szPathBuffer, sizeof( szPathBuffer ) );

		int &iROFTexID = m_iROFTexIDs[ i ];

		iROFTexID = surface( )->CreateNewTextureID( );
		surface( )->DrawSetTextureFile( iROFTexID, szPathBuffer, false, false );
	}
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_iIconNormalSize = scheme( )->GetProportionalScaledValue( WEAPONINFO_ICON_NORMAL_SIZE );
	m_iIconNormalTall = scheme( )->GetProportionalScaledValue( WEAPONINFO_ICON_NORMAL_TALL );

	m_iIconSlantedSize = scheme( )->GetProportionalScaledValue( WEAPONINFO_ICON_SLANTED_SIZE );
	m_iIconSlantedTall = scheme( )->GetProportionalScaledValue( WEAPONINFO_ICON_SLANTED_TALL );

	m_iIconYGap = scheme( )->GetProportionalScaledValue( WEAPONINFO_ICON_YGAP );

	m_hAmmoFont = pScheme->GetFont( WEAPONINFO_AMMOLEFT_FONT, true );

	m_iAmmoFontTall = surface( )->GetFontTall( m_hAmmoFont );

	m_iAmmoFontXPos = scheme( )->GetProportionalScaledValue( WEAPONINFO_AMMOLEFT_XPOS );
	m_iAmmoFontYPos = scheme( )->GetProportionalScaledValue( WEAPONINFO_AMMOLEFT_YPOS );
}

//=========================================================
//=========================================================
int CHudWeaponInfo::CreateAmmoTexID( const char *pszName )
{
	if( !pszName || *pszName == '\0' )
		return 0;

	int iAmmoTexID = 0;

	char szPathBuffer[ 256 ];
	CreateTexPath( WEAPONINFO_AMMO_PATH, pszName, szPathBuffer, sizeof( szPathBuffer ) );

	if( ::filesystem->FileExists( VarArgs( "materials/%s.vmt", szPathBuffer ) ) )
	{
		iAmmoTexID = surface( )->CreateNewTextureID( );
		surface( )->DrawSetTextureFile( iAmmoTexID, szPathBuffer, false, false );
	}

	return iAmmoTexID;
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ResetInfo( void )
{
	m_flAmmoShowTime = 0.0f;
	m_flROFShowTime = 0.0f;
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ShowAmmoInfo( float flTime, bool bAddDefault )
{
	ShowInfo( m_flAmmoShowTime, flTime, bAddDefault );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ShowAmmoInfo( void )
{
	ShowAmmoInfo( 0.0f, true );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ShowROFInfo( float flTime, bool bAddDefault )
{
	ShowInfo( m_flROFShowTime, flTime, bAddDefault );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ShowROFInfo( void )
{
	ShowROFInfo( 0.0f, true );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::FireMode( int iFireMode )
{
	ShowROFInfo( );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::Paint( void )
{
	int iYPos = GetTall( );

	if( m_flAmmoShowTime != 0.0f )
	{
		if( m_flAmmoShowTime > gpGlobals->curtime )
		{
			DrawAmmo( iYPos );

			iYPos -= m_iIconYGap;
		}
		else
		{
			m_flAmmoShowTime = 0.0f;
		}
	}

	if( m_flROFShowTime != 0.0f )
	{
		if( m_flROFShowTime > gpGlobals->curtime )
			DrawROF( iYPos );
		else
			m_flROFShowTime = 0.0f;
	}
}

//=========================================================
//=========================================================
void CHudWeaponInfo::DrawROF( int &iYPos )
{
	C_WeaponINSBase *pWeapon = GetINSActiveWeapon( );

	if( !pWeapon )
		return;

	int iFireMode = pWeapon->GetActiveFiremode( );

	if( iFireMode == INVALID_FIREMODE )
		return;

	DrawIcon( m_iROFTexIDs[ iFireMode ], iYPos, true );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::DrawAmmo( int &iYPos )
{
	C_WeaponINSBase *pWeapon = GetINSActiveWeapon( );

	if( !pWeapon )
		return;

	int iWeaponID = pWeapon->GetWeaponID( );

	if( iWeaponID == INVALID_WEAPON )
		return;

	if( WeaponIDToType( iWeaponID ) == WEAPONTYPE_MELEE )
		return;

	int iAmmoTexID = pWeapon->GetAmmoTexID( );

	if( iAmmoTexID == 0 )
		return;

	int iAmmoCount = pWeapon->GetAmmoCount( );

	// draw icons
	DrawIcon( iAmmoTexID, iYPos, false );

	// draw how many left
	int iWide, iTall;
	GetSize( iWide, iTall );

	char szAmmoLeft[ 4 ];
	Q_snprintf( szAmmoLeft, sizeof( szAmmoLeft ), "%i", iAmmoCount );

	int iNumChars = Q_strlen( szAmmoLeft );

	wchar_t wszAmmoLeft[ 4 ];
	localize( )->ConvertANSIToUnicode( szAmmoLeft, wszAmmoLeft, sizeof( wszAmmoLeft ) );

	int iAmmoFontWide = UTIL_ComputeStringWidth( m_hAmmoFont, wszAmmoLeft );

	surface( )->DrawSetTextFont( m_hAmmoFont );
	surface( )->DrawSetTextColor( COLOR_WHITE );
	surface( )->DrawSetTextPos( m_iIconNormalTall - iAmmoFontWide - m_iAmmoFontXPos, iTall - m_iAmmoFontTall - m_iAmmoFontYPos );

	surface( )->DrawPrintText( wszAmmoLeft, ( iNumChars > 3 ) ? 3 : iNumChars );
}

//=========================================================
//=========================================================
int CHudWeaponInfo::DrawIcon( int iTexID, int &iYPos, bool bSlanted )
{
	int iSize, iTall;
	iSize = bSlanted ? m_iIconSlantedSize : m_iIconNormalSize;
	iTall = bSlanted ? m_iIconSlantedTall : m_iIconNormalTall;

	int iStartYPos, iEndYPos;
	iStartYPos = iYPos - iTall;
	iEndYPos = iStartYPos + iSize;

	surface( )->DrawSetTexture( iTexID );
	surface( )->DrawSetColor( COLOR_WHITE );
	surface( )->DrawTexturedRect( 0, iStartYPos, iSize, iEndYPos );

	iYPos = iStartYPos;

	return iStartYPos;
}

//=========================================================
//=========================================================
void CHudWeaponInfo::CreateTexPath( const char *pszPath, const char *pszExtension, char *pszBuffer, int iLength )
{
	Q_snprintf( pszBuffer, iLength, "hud/wpninfo/%s/%s", pszPath, pszExtension );
}

//=========================================================
//=========================================================
void CHudWeaponInfo::ShowInfo( float &flTimer, float flTime, bool bAddDefault )
{
	flTimer = gpGlobals->curtime + flTime + ( bAddDefault ? WEAPONINFO_SHOWTIME : 0.0f );
}