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
#include "ins_utils.h"
#include "ins_obj_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define ACTIONBAR_BG_TEX "hud/actionbar/bg"
#define ACTIONBAR_BG_WIDTH 512
#define ACTIONBAR_BG_TALL 16

#define ACTIONBAR_HEAVYFONT_YPOS 1
#define ACTIONBAR_LIGHTFONT_YPOS 0

//=========================================================
//=========================================================
enum BarActions_t
{
	BARACTION_PLAYER = 0,
	BARACTION_OBJ,
	BARACTION_COUNT
};

//=========================================================
//=========================================================
class CHUDActionBar;

class IBarAction
{
public:
	void Create( CHUDActionBar *pHUDActionBar );
	CHUDActionBar *HUDActionBar( void );

	virtual const char *TexturePath( void ) const { return NULL; }

	virtual void Init( void ) { }
	virtual void Resize( void ) { }

	virtual bool IsActive( void ) const = 0;
	virtual void Paint( void ) = 0;

private:
	CHUDActionBar *m_pHUDActionBar;
};

//=========================================================
//=========================================================
typedef IBarAction *( *BarActionCreator_t )( void );

BarActionCreator_t g_BarActionHelpers[ BARACTION_COUNT ];

class CBarActionHelper
{
public:
	CBarActionHelper( int iID, BarActionCreator_t BarActionHelper )
	{
		g_BarActionHelpers[ iID ] = BarActionHelper;
	}
};

#define DECLARE_BARACTION( id, baraction ) \
	IBarAction *CreateBarAction__##id( void ) { \
		return new baraction; } \
		CBarActionHelper g_BarActionHelper__##id( id, CreateBarAction__##id );

//=========================================================
//=========================================================
void IBarAction::Create( CHUDActionBar *pHUDActionBar )
{
	m_pHUDActionBar = pHUDActionBar;
}

CHUDActionBar *IBarAction::HUDActionBar( void )
{
	Assert( m_pHUDActionBar );
	return m_pHUDActionBar;
}

//=========================================================
//=========================================================
class CHUDActionBar : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDActionBar, Panel );

public:
	CHUDActionBar( const char *pszElementName );

	HFont GetHeavyFont( void ) const { return m_hHeavyFont; }
	HFont GetLightFont( void ) const { return m_hLightFont; }
	int GetHeavyFontYPos( void ) const { return m_iHeavyFontYPos; }
	int GetLightFontYPos( void ) const { return m_iLightFontYPos; }

	void LoadTexture( IBarAction *pBarAction, const char *pszPath, int &iTexID );

private:
	void Init( void );

	void ApplySchemeSettings( IScheme *pScheme );

	void Paint( void );

private:
	bool m_bInit;
	IBarAction *m_pBarActions[ BARACTION_COUNT ];

	int m_iBGID, m_iBGWide, m_iBGTall;

	HFont m_hHeavyFont, m_hLightFont;
	int m_iHeavyFontYPos, m_iLightFontYPos;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHUDActionBar );

//=========================================================
//=========================================================
CHUDActionBar::CHUDActionBar( const char *pszElementName ) :
	CHudElement( pszElementName ), BaseClass( NULL, "HudActionBar" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );

	m_bInit = false;
}

//=========================================================
//=========================================================
void CHUDActionBar::Init( void )
{
	for( int i = 0; i < BARACTION_COUNT; i++ )
	{
		if( !m_bInit )
		{
			BarActionCreator_t Creator = g_BarActionHelpers[ i ];
			Assert( Creator );

			m_pBarActions[ i ] = ( Creator )( );
			m_pBarActions[ i ]->Create( this );
		}

		m_pBarActions[ i ]->Init( );
	}

	m_bInit = true;

	m_iBGID = surface( )->CreateNewTextureID( );
	surface( )->DrawSetTextureFile( m_iBGID, ACTIONBAR_BG_TEX, false, false );
}

//=========================================================
//=========================================================
void CHUDActionBar::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_iBGWide = scheme( )->GetProportionalScaledValue( ACTIONBAR_BG_WIDTH );
	m_iBGTall = scheme( )->GetProportionalScaledValue( ACTIONBAR_BG_TALL );

	m_hHeavyFont = pScheme->GetFont( "ActionBarHeavy", true );
	m_hLightFont = pScheme->GetFont( "ActionBarLight", true );

	m_iHeavyFontYPos = scheme( )->GetProportionalScaledValue( ACTIONBAR_HEAVYFONT_YPOS );
	m_iLightFontYPos = scheme( )->GetProportionalScaledValue( ACTIONBAR_LIGHTFONT_YPOS );

	for( int i = 0; i < BARACTION_COUNT; i++ )
		m_pBarActions[ i ]->Resize( );
}

//=========================================================
//=========================================================
void CHUDActionBar::Paint( void )
{
	for( int i = 0; i < BARACTION_COUNT; i++ )
	{
		IBarAction *pAction = m_pBarActions[ i ];
		Assert( pAction );

		if( pAction->IsActive( ) )
		{
			surface( )->DrawSetTexture( m_iBGID );
			surface( )->DrawSetColor( COLOR_WHITE );
			surface( )->DrawTexturedRect( 0, 0, m_iBGWide, m_iBGTall );

			pAction->Paint( );

			return;
		}
	}
}

//=========================================================
//=========================================================
void CHUDActionBar::LoadTexture( IBarAction *pBarAction, const char *pszPath, int &iTexID )
{
	iTexID = surface( )->CreateNewTextureID( );

	const char *pszBarPath = pBarAction->TexturePath( );

	if( !pszBarPath )
	{
		Assert( false );
		return;
	}

	char szPath[ 256 ];
	Q_snprintf( szPath, sizeof( szPath ), "HUD/actionbar/%s/%s", pszBarPath, pszPath );

	surface( )->DrawSetTextureFile( iTexID, szPath, false, false );
}

//=========================================================
//=========================================================
class CPlayerAction : public IBarAction
{
private:
	int m_iLastEntIndex;
	float m_flLastChangeTime;

public:
	void Init( void )
	{
		m_iLastEntIndex = 0;
		m_flLastChangeTime = 0.0f;
	}

	void Resize( void ) { }

	bool IsActive( void ) const
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
		return ( pPlayer->GetIDTarget( ) != 0 || m_flLastChangeTime != 0.0f );
	}

	void Paint( void )
	{
		C_INSPlayer *pLocalPlayer = C_INSPlayer::GetLocalPlayer( );
		Assert( pLocalPlayer );

		// get the target's entindex
		int iEntIndex = pLocalPlayer->GetIDTarget( );

		// didn't find one?
		if( iEntIndex == 0 )
		{
			// check to see if we should clear our ID
			if( m_flLastChangeTime != 0.0f && ( gpGlobals->curtime > ( m_flLastChangeTime + 0.5f ) ) )
			{
				m_flLastChangeTime = 0.0f;
				m_iLastEntIndex = 0;
			}
			else
			{
				// keep reusing the old one
				iEntIndex = m_iLastEntIndex;
			}
		}
		else
		{
			m_flLastChangeTime = gpGlobals->curtime;
		}

		// we got a valid entindex?
		if( iEntIndex == 0 )
			return;

		C_INSPlayer *pViewingPlayer;
		pViewingPlayer = ToINSPlayer( cl_entitylist->GetEnt( iEntIndex ) );

		if( !pViewingPlayer )
			return;

		// work out what needs to be shown
		Color DrawColor;

		if( pLocalPlayer->GetSquadID( ) == pLocalPlayer->GetSquadID( ) )
			DrawColor = GetINSHUDHelper( )->GetBrightHealthColor( pLocalPlayer->GetHealthType( ) );
		else
			DrawColor = COLOR_LGREY;

		wchar_t wszPlayerName[ MAX_PLAYER_NAME_LENGTH ];
		char szPlayerTitle[ MAX_PLAYER_TITLE_LENGTH ], szPlayerInfo[ 256 ];

		C_INSPlayer::ParsePlayerName( iEntIndex, true, szPlayerTitle, MAX_PLAYER_TITLE_LENGTH );
		Q_snprintf( szPlayerInfo, sizeof( szPlayerInfo ), "%s - %im", szPlayerTitle, RoundFloatToInt( UTIL_2DCaculateDistance( pLocalPlayer, pViewingPlayer ) * METERS_PER_INCH ) );
		localize( )->ConvertANSIToUnicode( szPlayerInfo, wszPlayerName, sizeof( wszPlayerName ) );
	
		// draw it
		HFont HeavyFont = HUDActionBar( )->GetHeavyFont( );

		surface( )->DrawSetTextFont( HeavyFont );
		surface( )->DrawSetTextPos( ( HUDActionBar( )->GetWide( ) - UTIL_ComputeStringWidth( HeavyFont, szPlayerInfo ) ) * 0.5f, HUDActionBar( )->GetLightFontYPos( ) );
		surface( )->DrawSetTextColor( DrawColor );
		surface( )->DrawPrintText( wszPlayerName, Q_strlen( szPlayerInfo ) );

		// remember it
		m_iLastEntIndex = iEntIndex;
	}
};

DECLARE_BARACTION( BARACTION_PLAYER, CPlayerAction );

//=========================================================
//=========================================================
#define CAPTURE_TYPE_XPOS 4
#define CAPTURE_OBJ_XGAP 3

#define CAPTURE_PROGRESS_TEX_PATH "progress"
#define CAPTURE_PROGRESS_TEX_WIDE 512
#define CAPTURE_PROGRESS_TEX_TALL 8
#define CAPTURE_PROGRESS_WIDE 262
#define CAPTURE_PROGRESS_YPOS 3
#define CAPTURE_PROGRESS_XGAP 10

class CObjectiveAction : public IBarAction
{
private:
	int m_iCaptureTypeXPos;

	int m_iCaptureObjXGap;

	int m_iCaptureProgressTexID, m_iCaptureProgressTexWide, m_iCaptureProgressTexTall;
	int m_iCaptureProgressWide;
	int m_iCaptureProgressYPos;
	int m_iCaptureProgressXGap;

public:
	virtual const char *TexturePath( void ) const { return "objbar"; }

	void Init( void )
	{
		HUDActionBar( )->LoadTexture( this, CAPTURE_PROGRESS_TEX_PATH, m_iCaptureProgressTexID );
	}

	void Resize( void )
	{
		m_iCaptureTypeXPos = scheme( )->GetProportionalScaledValue( CAPTURE_TYPE_XPOS );
		m_iCaptureObjXGap = scheme( )->GetProportionalScaledValue( CAPTURE_OBJ_XGAP );

		m_iCaptureProgressTexWide = scheme( )->GetProportionalScaledValue( CAPTURE_PROGRESS_TEX_WIDE );
		m_iCaptureProgressTexTall = scheme( )->GetProportionalScaledValue( CAPTURE_PROGRESS_TEX_TALL );
		m_iCaptureProgressWide = scheme( )->GetProportionalScaledValue( CAPTURE_PROGRESS_WIDE );
		m_iCaptureProgressYPos = scheme( )->GetProportionalScaledValue( CAPTURE_PROGRESS_YPOS );
		m_iCaptureProgressXGap = scheme( )->GetProportionalScaledValue( CAPTURE_PROGRESS_XGAP );
	}

	bool IsActive( void ) const
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
		return ( pPlayer && pPlayer->GetCurrentObj( ) != NULL );
	}

	void Paint( void )
	{
		C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
		Assert( pPlayer );

		CINSObjective *pObjective = pPlayer->GetCurrentObj( );
		Assert( pObjective );
		
		int iPlayerTeamID, iCapturingTeamID, iCapturedTeamID;
		iPlayerTeamID = pPlayer->GetTeamID( );
		iCapturingTeamID = pObjective->GetCaptureTeam( );
		iCapturedTeamID = pObjective->GetCapturedTeam( );

		const char *pszCaptureType = NULL;
		int iScrollProgress = OBJ_PROGRESS_MAX;

		if( iPlayerTeamID == iCapturedTeamID )
		{
			pszCaptureType = "captured";
		}
		else if( iCapturingTeamID != iPlayerTeamID )
		{
			pszCaptureType = "blocked";
		}
		else if( pObjective->IsCapturing( ) )
		{
			pszCaptureType = "securing";
			iScrollProgress = pObjective->GetCaptureProgress( );
		}

		if( !pszCaptureType )
			return;

		// work out what text needs painting
		HFont HeavyFont, LightFont;
		HeavyFont = HUDActionBar( )->GetHeavyFont( );
		LightFont = HUDActionBar( )->GetLightFont( );

		int iHeavyFontYPos, iLightFontYPos;
		iHeavyFontYPos = HUDActionBar( )->GetHeavyFontYPos( );
		iLightFontYPos = HUDActionBar( )->GetLightFontYPos( );

		// ... create capture type
		wchar_t wszCaptureTypeText[ 32 ];
		int iCaptureTypeStringLength, iCaptureTypeLength;

		localize( )->ConvertANSIToUnicode( pszCaptureType, wszCaptureTypeText, sizeof( wszCaptureTypeText ) );
		iCaptureTypeStringLength = Q_strlen( pszCaptureType );
		iCaptureTypeLength = UTIL_ComputeStringWidth( LightFont, pszCaptureType );

		// ... create capture obj
		char szCaptureObjText[ 32 ];
		wchar_t wszCaptureObjText[ 32 ];
		int iCaptureObjStringLength;

		Q_snprintf( szCaptureObjText, sizeof( szCaptureObjText ), "Objective %s", pObjective->GetPhonetischName( ) );

		localize( )->ConvertANSIToUnicode( szCaptureObjText, wszCaptureObjText, sizeof( wszCaptureObjText ) );
		iCaptureObjStringLength = Q_strlen( szCaptureObjText );

		// paint the text
		surface( )->DrawSetTextColor( COLOR_WHITE );

		// ... draw type
		surface( )->DrawSetTextFont( LightFont );
		surface( )->DrawSetTextPos( m_iCaptureTypeXPos, iLightFontYPos );
		surface( )->DrawPrintText( wszCaptureTypeText, iCaptureTypeStringLength );

		// ... draw obj
		int iCaptureObjXPos = m_iCaptureTypeXPos + iCaptureTypeLength + m_iCaptureObjXGap;

		surface( )->DrawSetTextFont( HeavyFont );
		surface( )->DrawSetTextPos( iCaptureObjXPos, iHeavyFontYPos );
		surface( )->DrawPrintText( wszCaptureObjText, iCaptureObjStringLength );

		// paint the progress
		if( iScrollProgress > 0 )
		{
			float flScrollFraction = ( float )iScrollProgress / OBJ_PROGRESS_MAX;
			int iProgressXPos = iCaptureObjXPos + UTIL_ComputeStringWidth( HeavyFont, wszCaptureObjText ) + m_iCaptureProgressXGap;

			TexCoord_t CaptureProgressTexCoords;

			CreateTexCoord( CaptureProgressTexCoords,
				0, 0,
				m_iCaptureProgressWide * flScrollFraction, m_iCaptureProgressTexTall,
				m_iCaptureProgressTexWide, m_iCaptureProgressTexTall );

			surface( )->DrawSetTexture( m_iCaptureProgressTexID );
			surface( )->DrawTexturedSubRect( iProgressXPos, m_iCaptureProgressYPos, iProgressXPos + ( m_iCaptureProgressWide * flScrollFraction ), m_iCaptureProgressYPos + m_iCaptureProgressTexTall,
				CaptureProgressTexCoords[ 0 ], CaptureProgressTexCoords[ 1 ], CaptureProgressTexCoords[ 2 ], CaptureProgressTexCoords[ 3 ] );
		}
	}
};

DECLARE_BARACTION( BARACTION_OBJ, CObjectiveAction );