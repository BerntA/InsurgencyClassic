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

#include <vgui/isurface.h>
#include <vgui/ilocalize.h>
#include <vgui_controls/panel.h>

#include "inshud.h"
#include "basic_colors.h"
#include "ins_player_shared.h"
#include "play_team_shared.h"
#include "ins_squad_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define HEADER_BG_TEX "hud/header/bg"
#define HEADER_BG_WIDTH 256
#define HEADER_BG_TALL 32

#define HEADER_UNLIMITED_TEX "hud/header/unlimited"
#define HEADER_UNLIMITED_WIDTH 8
#define HEADER_UNLIMITED_TALL 8
#define HEADER_UNLIMITED_XGAP -2

#define ELEMENT_YPOS 1
#define ORDER_TYPE_XPOS 5
#define ORDER_OBJ_XGAP 3
#define WAVE_XPOS 103
#define TIMER_XPOS 124

#define TIMER_FLASHTIME 0.35f

//=========================================================
//=========================================================
class CHUDHeader : public CHudElement, public vgui::Panel, public IINSOrderListener, public IINSTeamListener, public IINSObjListener
{
	DECLARE_CLASS_SIMPLE( CHUDHeader, Panel );

public:
	CHUDHeader( const char *pszElementName );

private:
	void Init( void );

	void ApplySchemeSettings( IScheme *pScheme );

	void Paint( void );

	void ObjOrder( void );
	void ObjUpdate( C_INSObjective *pObjective );
	void Update( const CObjOrder *pOrders );

	void TeamUpdate( C_PlayTeam *pTeam );

private:
	int m_iBGID, m_iBGWide, m_iBGTall;

	int m_iUnlimitedID, m_iUnlimitedWide, m_iUnlimitedTall;

	HFont m_hLightFont, m_hHeavyFont;

	int m_iElementYPos;

	wchar_t m_wszOrderTypeString[ 32 ];
	int m_iOrderTypeStringLength;
	int m_iOrderTypeXPos;

	wchar_t m_wszOrderObjString[ 32 ];
	int m_iOrderObjStringLength;
	int m_iOrderObjXGap, m_iOrderObjXPos;

	bool m_bWavesUnlimited;
	wchar_t m_wszWaveString[ 8 ];
	int m_iWaveStringLength;
	int m_iWaveXPos;
	int m_iWaveUnlimitedXPos;

	int m_iTimerXPos;
	float m_flTimerNextFlashState;
	bool m_bTimerDraw;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHUDHeader );

//=========================================================
//=========================================================
CHUDHeader::CHUDHeader( const char *pszElementName ) :
	CHudElement( pszElementName ), BaseClass( NULL, "HudHeader" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );
}

//=========================================================
//=========================================================
void CHUDHeader::Init( void )
{
	m_iBGID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( m_iBGID, HEADER_BG_TEX, false, false );

	m_iUnlimitedID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( m_iUnlimitedID, HEADER_UNLIMITED_TEX, false, false );

	m_iOrderTypeStringLength = 0;
	m_iOrderObjStringLength = 0;

	m_flTimerNextFlashState = 0.0f;
	m_bTimerDraw = false;
}

//=========================================================
//=========================================================
void CHUDHeader::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_iBGWide = scheme( )->GetProportionalScaledValue( HEADER_BG_WIDTH );
	m_iBGTall = scheme( )->GetProportionalScaledValue( HEADER_BG_TALL );

	m_iUnlimitedWide = scheme( )->GetProportionalScaledValue( HEADER_UNLIMITED_WIDTH );
	m_iUnlimitedTall = scheme( )->GetProportionalScaledValue( HEADER_UNLIMITED_WIDTH );

	m_hLightFont = pScheme->GetFont( "HeaderLight", true );
	m_hHeavyFont = pScheme->GetFont( "HeaderHeavy", true );

	m_iElementYPos = scheme( )->GetProportionalScaledValue( ELEMENT_YPOS );

	m_iOrderTypeXPos = scheme( )->GetProportionalScaledValue( ORDER_TYPE_XPOS );
	m_iOrderObjXGap = scheme( )->GetProportionalScaledValue( ORDER_OBJ_XGAP );

	m_iWaveXPos = scheme( )->GetProportionalScaledValue( WAVE_XPOS );
	m_iWaveUnlimitedXPos = m_iWaveXPos + scheme( )->GetProportionalScaledValue( HEADER_UNLIMITED_XGAP );

	m_iTimerXPos = scheme( )->GetProportionalScaledValue( TIMER_XPOS );
}

//=========================================================
//=========================================================
void CHUDHeader::Paint( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->IsRunningAround( ) )
		return;

	// draw background
	surface( )->DrawSetTexture( m_iBGID );
	surface( )->DrawSetColor( COLOR_WHITE );
	surface( )->DrawTexturedRect( 0, 0, m_iBGWide, m_iBGTall );

	// draw objective text
	surface( )->DrawSetTextColor( COLOR_WHITE );

	// ... draw type
	surface( )->DrawSetTextFont( m_hLightFont );
	surface( )->DrawSetTextPos( m_iOrderTypeXPos, m_iElementYPos );
	surface( )->DrawPrintText( m_wszOrderTypeString, m_iOrderTypeStringLength );

	// ... draw objective 
	if( m_iOrderObjStringLength != 0 )
	{
		surface( )->DrawSetTextFont( m_hHeavyFont );
		surface( )->DrawSetTextPos( m_iOrderObjXPos, m_iElementYPos );
		surface( )->DrawPrintText( m_wszOrderObjString, m_iOrderObjStringLength );
	}

	// draw waves
	if( m_bWavesUnlimited )
	{
		surface( )->DrawSetTexture( m_iUnlimitedID );
		surface( )->DrawSetColor( COLOR_WHITE );
		surface( )->DrawTexturedRect( m_iWaveUnlimitedXPos, m_iElementYPos, m_iWaveUnlimitedXPos + m_iUnlimitedWide, m_iElementYPos + m_iUnlimitedTall );
	}
	else
	{
		surface( )->DrawSetTextFont( m_hHeavyFont );
		surface( )->DrawSetTextPos( m_iWaveXPos, m_iElementYPos );
		surface( )->DrawPrintText( m_wszWaveString, m_iWaveStringLength );
	}

	// draw timer
	char szTimer[ 8 ];
	int iTimerType = GetINSHUDHelper( )->CreateRoundTimer( szTimer, sizeof( szTimer ) );

	if( iTimerType != ROTIMERTYPE_FLASH )
	{
		m_flTimerNextFlashState = 0.0f;
		m_bTimerDraw = true;
	}
	else
	{
		if( m_flTimerNextFlashState >= gpGlobals->curtime )
		{
			m_flTimerNextFlashState = gpGlobals->curtime + TIMER_FLASHTIME;
			m_bTimerDraw = !m_bTimerDraw;
		}
	}

	if( m_bTimerDraw )
	{
		wchar_t wszTimer[ 8 ];
		localize( )->ConvertANSIToUnicode( szTimer, wszTimer, sizeof( wszTimer ) );

		if( iTimerType != ROTIMERTYPE_NORMAL )
			surface( )->DrawSetTextColor( ( iTimerType == ROTIMERTYPE_NONE ) ? COLOR_DGREY : COLOR_RED );

		surface( )->DrawSetTextFont( m_hHeavyFont );
		surface( )->DrawSetTextPos( m_iTimerXPos, m_iElementYPos );
		surface( )->DrawPrintText( wszTimer, Q_strlen( szTimer ) );
	}
}

//=========================================================
//=========================================================
void CHUDHeader::ObjOrder( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	const CObjOrder *pObjOrders = pPlayer->GetObjOrders( );

	if( pObjOrders )
		Update( pObjOrders );
}

//=========================================================
//=========================================================
void CHUDHeader::ObjUpdate( C_INSObjective *pObjective )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	const CObjOrder *pObjOrders = pPlayer->GetObjOrders( );

	if( pObjOrders && pObjOrders->HasOrders( ) && pObjOrders->Objective( ) == pObjective )
		Update( pObjOrders );
}

//=========================================================
//=========================================================
void CHUDHeader::Update( const CObjOrder *pObjOrders )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	// find the type
	const char *pszType = NULL;

	if( pObjOrders->HasOrders( ) )
	{
		pszType = g_pszObjOrderTypeNames[ pObjOrders->OrderType( ) ];

		// set objective
		char szObjBuffer[ 32 ];
		Q_snprintf( szObjBuffer, sizeof( szObjBuffer ), "Objective %s", pObjOrders->Objective( )->GetPhonetischName( ) );
		localize( )->ConvertANSIToUnicode( szObjBuffer, m_wszOrderObjString, sizeof( m_wszOrderObjString ) );
		m_iOrderObjStringLength = Q_strlen( szObjBuffer );
		m_iOrderObjXPos = m_iOrderTypeXPos + UTIL_ComputeStringWidth( m_hLightFont, pszType ) + m_iOrderObjXGap;
	}
	else
	{
		C_INSSquad *pSquad = pPlayer->GetSquad( );

		if( pSquad && !pSquad->HasCommander( ) )
			pszType = "No Commander";
		else
			pszType = "Unassigned Orders";

		// don't draw obj
		m_iOrderObjStringLength = 0;
	}

	// set type
	localize( )->ConvertANSIToUnicode( pszType, m_wszOrderTypeString, sizeof( m_wszOrderTypeString ) );
	m_iOrderTypeStringLength = Q_strlen( pszType );
}

//=========================================================
//=========================================================
void CHUDHeader::TeamUpdate( C_PlayTeam *pTeam )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
	{
		Assert( false );
		return;
	}

	int iSquadID = pPlayer->GetSquadID( );

	if( iSquadID == INVALID_SQUAD )
		return;

	m_bWavesUnlimited = pTeam->IsUnlimitedWaves( );
	m_iWaveStringLength = 0;

	if( !m_bWavesUnlimited )
	{
		char szBuffer[ 8 ];
		Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", pTeam->GetReinforcementsLeft( iSquadID ) );
		localize( )->ConvertANSIToUnicode( szBuffer, m_wszWaveString, sizeof( m_wszWaveString ) );
		m_iWaveStringLength = Q_strlen( szBuffer );
	}
}