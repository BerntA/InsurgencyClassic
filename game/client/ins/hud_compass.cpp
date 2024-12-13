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
#include <vgui_controls/panel.h>

#include "inshud.h"
#include "basic_colors.h"
#include "ins_player_shared.h"
#include "ins_squad_shared.h"
#include "play_team_shared.h"
#include "view.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define COMPASS_PATH "hud/compass/"

//=========================================================
//=========================================================
#define COMPASS_BG_TEX "bg"

#define COMPASS_BG_XPOS 0
#define COMPASS_BG_YPOS 0
#define COMPASS_BG_WIDTH 512
#define COMPASS_BG_TALL 64
#define COMPASS_BG_SCALE 4
#define COMPASS_BG_LINEGAP 2

#define COMPASS_BG_INTERNAL_LENGTH 380

#define COMPASS_NUMBERS_YPOS 42

#define COMPASS_NUMBERS_BIG_WIDE 32
#define COMPASS_NUMBERS_BIG_INTERNAL_TENS_WIDE 19
#define COMPASS_NUMBERS_BIG_INTERNAL_ONES_WIDE 16
#define COMPASS_NUMBERS_BIG_TALL 8

#define COMPASS_NUMBERS_SMALL_WIDE 16
#define COMPASS_NUMBERS_SMALL_INTERNAL_TENS_WIDE 9
#define COMPASS_NUMBERS_SMALL_INTERNAL_ONES_WIDE 11
#define COMPASS_NUMBERS_SMALL_TALL 4

#define COMPASS_NUMBERS_SIZE 12
#define COMPASS_NUMBERS_RANGE 180

#define COMPASS_NUMBERS_LINES 3
#define COMPASS_NUMBERS_LINE_TEX "seperator"
#define COMPASS_NUMBERS_LINE_WIDE 1
#define COMPASS_NUMBERS_LINE_TALL 3

#define COMPASS_INDICATOR_BG_TEX "indicator"
#define COMPASS_INDICATOR_BG_SIZE_MAX 16
#define COMPASS_INDICATOR_BG_SIZE_MIN 4
#define COMPASS_INDICATOR_BG_ALPHA_MAX 255
#define COMPASS_INDICATOR_BG_ALPHA_MIN 128

enum CompassIcons_t
{
	COMPASS_ICON_SECURE = 0,
	COMPASS_ICON_DEFEND,
	COMPASS_ICON_UNIT,
	COMPASS_ICON_MEDIC,
	COMPASS_ICON_LEADER,
	COMPASS_ICON_COUNT
};

static const char *g_pszCompassIconPaths[ COMPASS_ICON_COUNT ] = {
	"secure",		// COMPASS_ICON_SECURE
	"defend",		// COMPASS_ICON_DEFEND
	"maneuver",		// COMPASS_ICON_UNIT
	"medic_needed",	// COMPASS_ICON_MEDIC
	"officer"		// COMPASS_ICON_LEADER
};

#define COMPASS_ICON_SIZE 32

#define COMPASS_ICON_PATH "indicators"

//=========================================================
//=========================================================
#define COMPASSPOINTS 10

struct CompassPoint_t
{
	int m_iX, m_iY;
};

typedef CompassPoint_t CompassPoints_t[ COMPASSPOINTS ];

CompassPoints_t g_CompassPoints = {
	{ 0, 103 },
	{ 117, 69 },
	{ 181, 55 },
	{ 270, 39 },
	{ 336, 29 },
	{ 415, 18 },
	{ 535, 9 },
	{ 592, 6 },
	{ 647, 3 },
	{ 767, 2 },
};

//=========================================================
//=========================================================
class NumberData_t
{
public:
	void InitTex( int iID, int iTexID )
	{
		m_iTexID[ iID ] = iTexID;
	}

	void InitSize( int iWide, int iTall, int iWideTens, int iWideOnes )
	{
		m_iWide = scheme( )->GetProportionalScaledValue( iWide );
		m_iTall = scheme( )->GetProportionalScaledValue( iTall );
		m_iInnerWideTens = scheme( )->GetProportionalScaledValue( iWideTens );
		m_iInnerWideOnes = scheme( )->GetProportionalScaledValue( iWideOnes );
	}

public:
	int m_iTexID[ COMPASS_NUMBERS_SIZE ];

	int m_iWide, m_iTall;
	int m_iInnerWideTens, m_iInnerWideOnes;
};

//=========================================================
//=========================================================
enum NumberAlignment_t
{
	NUMBERALIGN_NONE = 0,
	NUMBERALIGN_MIDDLE,
	NUMBERALIGN_RIGHT
};

//=========================================================
//=========================================================
class CHUDCompass : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHUDCompass, Panel );

public:
	CHUDCompass( const char *pszElementName );

private:
	void Init( void );

	void ApplySchemeSettings( IScheme *pScheme );

	void Paint( void );

	float DrawIndicator( int iIndicatorTypeID, const Vector &vecOrigin );
	inline float DrawIndicator( int iIndicatorTypeID, CBaseEntity *pEntity );

private:
	void CreatePath( const char *pszPrefix, const char *pszSuffix, char *pszBuffer, int iLength );

	bool FindYPos( int iXPos, int &iYPos );
	float GetIndicatorGapFraction( int iTexGap, float flSize );

	float WrapYaw( float flBadYaw );

	void DrawNumber( int iClock, NumberData_t &NumberData, int iXPos, int iAlignment );
	int NumberInnerWide( int iClock, NumberData_t &NumberData );

private:
	CompassPoints_t m_ScaledCompassPoints;

	int m_iBGID, m_iBGWide, m_iBGTall;
	int m_iBGInnerLength, m_iBGInnerLengthHalf;
	int m_iBGLineGap;

	int m_iIndiPointerID, m_iIndiPointerSizeMax, m_iIndiPointerSizeMin;
	int m_iIndiSize;

	NumberData_t m_BigNumbers, m_SmallNumbers;
	int m_iNumbersYPos;
	int m_iSeperatorID, m_iSeperatorWide, m_iSeperatorTall;

	int m_iIconTexIDs[ COMPASS_ICON_COUNT ];
	int m_iIconSize;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHUDCompass );

//=========================================================
//=========================================================
CHUDCompass::CHUDCompass( const char *pszElementName ) :
	CHudElement( pszElementName ), BaseClass( NULL, "HudCompass" )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );
}

//=========================================================
//=========================================================
void CHUDCompass::Init( void )
{
	char szBuffer[ 128 ];

	// create bg tex
	m_iBGID = surface( )->CreateNewTextureID( );

	CreatePath( NULL, COMPASS_BG_TEX, szBuffer, sizeof( szBuffer ) );
	surface( )->DrawSetTextureFile( m_iBGID, szBuffer, false, false );

	// create indicator tex
	m_iIndiPointerID = surface( )->CreateNewTextureID( );

	CreatePath( NULL, COMPASS_INDICATOR_BG_TEX, szBuffer, sizeof( szBuffer ) );
	surface( )->DrawSetTextureFile( m_iIndiPointerID, szBuffer, false, false );

	// create number tex

	// ... numbers
	char szNumberSuffix[ 32 ];

	for( int i = 0; i < 2; i++ )
	{
		NumberData_t &NumberData = ( ( i == 0 ) ? m_BigNumbers : m_SmallNumbers );

		for( int j = 0; j < COMPASS_NUMBERS_SIZE; j++ )
		{
			int iTexID = surface( )->CreateNewTextureID( );

			Q_snprintf( szNumberSuffix, sizeof( szNumberSuffix ), "%s%02i_0", ( ( i == 0 ) ? "" : "s" ), j + 1 );
			CreatePath( NULL, szNumberSuffix, szBuffer, sizeof( szBuffer ) );

			surface( )->DrawSetTextureFile( iTexID, szBuffer, false, false );

			NumberData.InitTex( j, iTexID );
		}
	}

	// ... seperators
	m_iSeperatorID = surface( )->CreateNewTextureID( );

	CreatePath( NULL, COMPASS_NUMBERS_LINE_TEX, szBuffer, sizeof( szBuffer ) );
	surface( )->DrawSetTextureFile( m_iSeperatorID, szBuffer, false, false );

	// ... icons
	for( int i = 0; i < COMPASS_ICON_COUNT; i++ )
	{
		CreatePath( COMPASS_ICON_PATH, g_pszCompassIconPaths[ i ], szBuffer, sizeof( szBuffer ) );

		m_iIconTexIDs[ i ] = surface( )->CreateNewTextureID( );
		surface( )->DrawSetTextureFile( m_iIconTexIDs[ i ], szBuffer, false, false );
	}
}

//=========================================================
//=========================================================
void CHUDCompass::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_iBGWide = scheme( )->GetProportionalScaledValue( COMPASS_BG_WIDTH );
	m_iBGTall = scheme( )->GetProportionalScaledValue( COMPASS_BG_TALL );
	m_iBGInnerLength = scheme( )->GetProportionalScaledValue( COMPASS_BG_INTERNAL_LENGTH );
	m_iBGInnerLengthHalf = RoundFloatToInt( m_iBGInnerLength * 0.5f );
	m_iBGLineGap = scheme( )->GetProportionalScaledValue( COMPASS_BG_LINEGAP );

	m_iIndiPointerSizeMax = scheme( )->GetProportionalScaledValue( COMPASS_INDICATOR_BG_SIZE_MAX );
	m_iIndiPointerSizeMin = scheme( )->GetProportionalScaledValue( COMPASS_INDICATOR_BG_SIZE_MIN );

	m_BigNumbers.InitSize( COMPASS_NUMBERS_BIG_WIDE, COMPASS_NUMBERS_BIG_TALL, COMPASS_NUMBERS_BIG_INTERNAL_TENS_WIDE, COMPASS_NUMBERS_BIG_INTERNAL_ONES_WIDE );
	m_SmallNumbers.InitSize( COMPASS_NUMBERS_SMALL_WIDE, COMPASS_NUMBERS_SMALL_TALL, COMPASS_NUMBERS_SMALL_INTERNAL_TENS_WIDE, COMPASS_NUMBERS_SMALL_INTERNAL_ONES_WIDE );
	m_iNumbersYPos = scheme( )->GetProportionalScaledValue( COMPASS_NUMBERS_YPOS );

	for( int i = 0; i < COMPASSPOINTS; i++ )
	{
		m_ScaledCompassPoints[ i ].m_iX = RoundFloatToInt( g_CompassPoints[ i ].m_iX * ( ( float )m_iBGWide / ( COMPASS_BG_WIDTH * COMPASS_BG_SCALE ) ) );
		m_ScaledCompassPoints[ i ].m_iY = RoundFloatToInt( g_CompassPoints[ i ].m_iY * ( ( float )m_iBGTall / ( COMPASS_BG_TALL * COMPASS_BG_SCALE ) ) );
	}

	m_iSeperatorWide = scheme( )->GetProportionalScaledValue( COMPASS_NUMBERS_LINE_WIDE );
	m_iSeperatorTall = scheme( )->GetProportionalScaledValue( COMPASS_NUMBERS_LINE_TALL );

	m_iIconSize = scheme( )->GetProportionalScaledValue( COMPASS_ICON_SIZE );
}

//=========================================================
//=========================================================
void CHUDCompass::Paint( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer || !pPlayer->IsRunningAround( ) )
		return;

	// draw the background
	surface( )->DrawSetTexture( m_iBGID );
	surface( )->DrawSetColor( COLOR_WHITE );
	surface( )->DrawTexturedRect( 0, 0, m_iBGWide, m_iBGTall );

	// draw the indicated
	float flObjOrdersYaw = -1.0f;

	// draw obj orders
	if( pPlayer->HasObjOrders( ) )
	{
		const CObjOrder *pObjOrder = pPlayer->GetObjOrders( );
		Assert( pObjOrder );

		C_INSObjective *pObjective = pObjOrder->Objective( );

		if( pObjective && pObjective != pPlayer->GetCurrentObj( ) )
			flObjOrdersYaw = DrawIndicator( ( pObjOrder->OrderType( ) == ORDERTYPE_OBJ_ATTACK ) ? COMPASS_ICON_SECURE : COMPASS_ICON_DEFEND, pObjective->GetMarker( ) );
	}

	// draw the squad leader
	CINSSquad *pSquad = pPlayer->GetSquad( );
	
	if( pSquad )
	{
		CINSPlayer *pCommander = pSquad->GetCommander( );

		if( pCommander && pCommander != pPlayer )
			DrawIndicator( COMPASS_ICON_LEADER, pCommander );
	}

	// ... draw unit orders
	if( pPlayer->HasUnitOrders( ) )
	{
		const CUnitOrder *pUnitOrder = pPlayer->GetUnitOrders( );
		Assert( pUnitOrder );

		DrawIndicator( COMPASS_ICON_UNIT, pUnitOrder->Position( ) );
	}

	// ... when a medic, always draw wounded players
	if( pPlayer->IsMedic( ) )
	{
		C_PlayTeam *pTeam = pPlayer->GetPlayTeam( );
		Assert( pTeam );

		for( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			if( pTeam->NeedsHelp( i ) && i != pPlayer->entindex( ) )
				DrawIndicator( COMPASS_ICON_MEDIC, pPlayer );
		}
	}

	// draw the numbers
	if( flObjOrdersYaw >= 0.0f )
	{
		static int iAngleDelta = 360 / COMPASS_NUMBERS_SIZE;
		static int iNumberRange = COMPASS_NUMBERS_RANGE / iAngleDelta;

		int iXPosDelta = m_iBGInnerLength / iNumberRange;
		int iLineXPosDelta = iXPosDelta / ( COMPASS_NUMBERS_LINES + 1 );

		// get view start and end yaw
		float flYawStart, flYawEnd;
		flYawStart = WrapYaw( flObjOrdersYaw - 90.0f );
		flYawEnd = WrapYaw( flObjOrdersYaw + 90.0f );

		// work out what o'clock wer are at
		int iStartClock, iDrawClock;
		iStartClock = iDrawClock = floor( flYawStart / iAngleDelta );

		if( iDrawClock == 0 )
			iDrawClock = 12;

		// are we drawing arrows?
		bool bDrawArrows = ( ( ( int )flYawStart != flYawStart ) || ( int )flYawStart % iAngleDelta != 0 );

		// work out how many needs drawing
		int iInnerNumDraw = iNumberRange - 2;

		// ... when we aren't perfectly between sectors, draw an extra two sectors
		if( bDrawArrows )
			iInnerNumDraw += 2;

		// set draw color
		surface( )->DrawSetColor( COLOR_WHITE );

		int iNumberXPos = m_iBGLineGap;
		int iReducedStartXDelta = 0;

		// draw first number
		DrawNumber( iDrawClock, m_BigNumbers, 0, NUMBERALIGN_NONE );

		iReducedStartXDelta = iXPosDelta * ( 1.0f - ( ( flYawStart - ( iStartClock * iAngleDelta ) ) / iAngleDelta ) );
		iNumberXPos += iReducedStartXDelta;

		// draw all the numbers inbetween
		for( int i = 0; i < iInnerNumDraw; i++ )
		{
			// increment (and rotate if needed draw clock)
			iDrawClock++;

			if( iDrawClock > 12 )
				iDrawClock = 1;

			// work out where we are
			bool bNotFirstEdge, bNotEndEdge;
			bNotFirstEdge = ( i != 0 );
			bNotEndEdge = ( i != ( iInnerNumDraw - 1 ) );

			// increment xpos
			if( bNotFirstEdge )
				iNumberXPos += iXPosDelta;

			if( bNotEndEdge )
			{
				int iLineXPos = iNumberXPos;

				for( int j = 0; j < COMPASS_NUMBERS_LINES; j++ )
				{
					iLineXPos += iLineXPosDelta;

					surface( )->DrawSetTexture( m_iSeperatorID );
					surface( )->DrawTexturedRect( iLineXPos, m_iNumbersYPos, iLineXPos + m_iSeperatorWide, m_iNumbersYPos + m_iSeperatorTall );
				}
			}

			// draw it
			DrawNumber( iDrawClock, ( ( iDrawClock == 12 && bNotFirstEdge && bNotEndEdge ) ? m_BigNumbers : m_SmallNumbers ), iNumberXPos, NUMBERALIGN_MIDDLE );
		}

		// draw the last number
		DrawNumber( iDrawClock, m_BigNumbers, iNumberXPos + ( iXPosDelta - iReducedStartXDelta ) + m_iBGLineGap, NUMBERALIGN_RIGHT );
	}
}

//=========================================================
//=========================================================
bool CHUDCompass::FindYPos( int iXPos, int &iYPos )
{
	if( iXPos > m_iBGInnerLengthHalf )
		iXPos = m_iBGInnerLengthHalf - abs( m_iBGInnerLengthHalf - iXPos );

	int iXSegment = 0;

	// find a new segment
	for( int i = 0; i < COMPASSPOINTS; i++ )
	{
		if( i == COMPASSPOINTS - 1 )
			return false;

		iXSegment = i;

		if( iXPos < m_ScaledCompassPoints[ i ].m_iX )
			break;
	}

	// find position
	CompassPoint_t &CP1 = m_ScaledCompassPoints[ iXSegment ];
	CompassPoint_t &CP2 = m_ScaledCompassPoints[ iXSegment + 1 ];

	iYPos = RoundFloatToInt( CP1.m_iY + ( ( float )( CP2.m_iY - CP1.m_iY ) / ( float )( CP2.m_iX - CP1.m_iX ) ) * ( iXPos - CP1.m_iX ) );

	return true;
}

//=========================================================
//=========================================================
float CHUDCompass::DrawIndicator( int iIconID, const Vector &vecOrigin )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );
	Assert( pPlayer );

	// find rotation
	float flYaw;
	GetINSHUDHelper( )->GetRotation( vecOrigin - pPlayer->GetAbsOrigin( ), &flYaw );

	if( flYaw < 270.0f && flYaw > 90.0f )
		return flYaw;

	// work out where we are on the bg
	float flAdjYaw = WrapYaw( flYaw + 90.0f );
	float flFraction = 1.0f - ( abs( flAdjYaw - 90.0f ) / 90.0f );

	float flSize = m_iIndiPointerSizeMin + ( ( m_iIndiPointerSizeMax - m_iIndiPointerSizeMin ) * flFraction );

	int iXPos1, iXPos2, iCenterXPos, iYPos1, iYPos2;
	float flHalfIPSize = flSize * 0.5f;

	iCenterXPos = m_iBGInnerLength * ( 1.0f - ( flAdjYaw / 180.0f ) );

	iXPos1 = iCenterXPos - flHalfIPSize;
	iXPos2 = iCenterXPos + flHalfIPSize;

	iYPos1 = iYPos2 = 0;

	FindYPos( iXPos1, iYPos1 );
	FindYPos( iXPos2, iYPos2 );

	// draw the top part of the indicator
	float flXTex1, flXTex2;
	flXTex1 = flXTex2 = 0.0f;

	if( iYPos1 > iYPos2 )
		flXTex2 = GetIndicatorGapFraction( iYPos1 - iYPos2, flSize );
	else if( iYPos2 > iYPos1 )
		flXTex2 = GetIndicatorGapFraction( iYPos2 - iYPos1, flSize );
	
	Vertex_t Points[ 4 ] =
	{
		Vertex_t( Vector2D( iXPos1, iYPos1 ), Vector2D( 0, flXTex1 ) ),  // top left
		Vertex_t( Vector2D( iXPos2, iYPos2 ), Vector2D( 1, flXTex2 ) ), // top right
		Vertex_t( Vector2D( iXPos2, iYPos2 + flSize ), Vector2D( 1, 1 ) ), // bottom right
		Vertex_t( Vector2D( iXPos1, iYPos1 + flSize ), Vector2D( 0, 1 ) ) // bottom left
	};

	Color IndiColor = COLOR_WHITE;
	IndiColor[ 3 ] = COMPASS_INDICATOR_BG_ALPHA_MIN + ( ( COMPASS_INDICATOR_BG_ALPHA_MAX - COMPASS_INDICATOR_BG_ALPHA_MIN ) * flFraction );

	surface( )->DrawSetTexture( m_iIndiPointerID );
	surface( )->DrawSetColor( IndiColor );
	surface( )->DrawTexturedPolygon( 4, Points );

	// draw icon
	int iIconSize = RoundFloatToInt( m_iIconSize * flFraction );
	int iIconXPos = iCenterXPos - RoundFloatToInt( iIconSize * 0.5f );

	int iCenterYPos = 0;
	FindYPos( iCenterXPos, iCenterYPos );

	iCenterYPos += flSize;

	surface( )->DrawSetTexture( m_iIconTexIDs[ iIconID ] );
	surface( )->DrawSetColor( IndiColor );
	surface( )->DrawTexturedRect( iIconXPos, iCenterYPos, iIconXPos + iIconSize, iCenterYPos + iIconSize );

	return flYaw;
}

//=========================================================
//=========================================================
float CHUDCompass::DrawIndicator( int iIndicatorTypeID, CBaseEntity *pEntity )
{
	Assert( pEntity );
	return DrawIndicator( iIndicatorTypeID, pEntity->GetAbsOrigin( ) );
}

//=========================================================
//=========================================================
float CHUDCompass::GetIndicatorGapFraction( int iTexGap, float flSize )
{
	return ( iTexGap ) / flSize;
}

//=========================================================
//=========================================================
float CHUDCompass::WrapYaw( float flBadYaw )
{
	if( flBadYaw < 0.0f )
		flBadYaw = 360 - abs( flBadYaw );
	else if( flBadYaw > 360.0f )
		flBadYaw = flBadYaw - 360.0f;

	return flBadYaw;
}

//=========================================================
//=========================================================
void CHUDCompass::CreatePath( const char *pszPrefix, const char *pszSuffix, char *pszBuffer, int iLength )
{
	Q_strncpy( pszBuffer, COMPASS_PATH, iLength );

	if( pszPrefix )
	{
		Q_strncat( pszBuffer, pszPrefix, iLength, COPY_ALL_CHARACTERS );
		Q_strncat( pszBuffer, "/", iLength, COPY_ALL_CHARACTERS );
	}

	Q_strncat( pszBuffer, pszSuffix, iLength, COPY_ALL_CHARACTERS );
}

//=========================================================
//=========================================================
void CHUDCompass::DrawNumber( int iClock, NumberData_t &NumberData, int iXPos, int iAlignment )
{
	int iDrawXPos = iXPos;

	switch( iAlignment )
	{
		case NUMBERALIGN_MIDDLE:
		{
			iDrawXPos -= NumberInnerWide( iClock, NumberData ) * 0.5f;
			break;
		}

		case NUMBERALIGN_RIGHT:
		{
			iDrawXPos -= NumberInnerWide( iClock, NumberData );
			break;
		}
	}
	
	surface( )->DrawSetTexture( NumberData.m_iTexID[ iClock - 1 ] );
	surface( )->DrawTexturedRect( iDrawXPos, m_iNumbersYPos, iDrawXPos + NumberData.m_iWide, m_iNumbersYPos + NumberData.m_iTall );
}

//=========================================================
//=========================================================
int CHUDCompass::NumberInnerWide( int iClock, NumberData_t &NumberData )
{
	return ( iClock >= 10 ? NumberData.m_iInnerWideTens : NumberData.m_iInnerWideOnes );
}
