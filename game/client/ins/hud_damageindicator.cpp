//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"

#include <vgui/isurface.h>
#include <vgui_controls/panel.h>

#include "inshud.h"
#include "view.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define DAMAGEINDI_TEX_PATH "hud/dmgindi/dmg"
#define DAMAGEINDI_TEX_WIDE 192
#define DAMAGEINDI_TEX_TALL 384

// TODO: have fp's to control fade - so falling can be solid for a bit and then fate thereafter
#define DAMAGEINDI_DRAWTIME_NORMAL 0.5f
#define DAMAGEINDI_DRAWTIME_FALL 1.5f

//=========================================================
//=========================================================
enum DamageLocations_t
{
	DAMAGELOC_INVALID = -1,
	DAMAGELOC_LEFT = 0,
	DAMAGELOC_RIGHT,
	DAMAGELOC_BOTTOM,
	DAMAGELOC_COUNT
};

//=========================================================
//=========================================================
float g_DamageAngles[ DAMAGELOC_COUNT ][ 2 ] = {
	{ 45.0f, 135.0f },		// DAMAGELOC_LEFT
	{ 225.0f, 315.0f },		// DAMAGELOC_RIGHT
	{ 135.0f, 225.0f },		// DAMAGELOC_BOTTOM
};

//=========================================================
//=========================================================
typedef int DamagePositions_t[ DAMAGELOC_COUNT ][ 2 ];

#define DAMAGEPOS_INTERNAL_WIDE 139
#define DAMAGEPOS_INTERNAL_TALL 264
#define DAMAGEPOS_INTERNAL_YGAP ( DAMAGEINDI_TEX_TALL - DAMAGEPOS_INTERNAL_TALL )
#define DAMAGEPOS_INTERNAL_INWARD 0.2f

//=========================================================
//=========================================================
#ifdef TESTING

ConVar fdrawdindi( "cl_fdrawdindi", "0" );

#endif

//=========================================================
//=========================================================
class DamageDrawData_t
{
public:
	void Init( float flTime, bool bFall )
	{
		m_flTime = flTime;
		m_bFall = bFall;
	}

	void Reset( void )
	{
		m_flTime = 0.0f;
		m_bFall = false;
	}

public:
	float m_flTime;
	bool m_bFall;
};

//=========================================================
//=========================================================
class CHudDamageIndicator : public CHudElement, public Panel, public IINSDamageListener
{
	DECLARE_CLASS_SIMPLE( CHudDamageIndicator, Panel );

public:
	CHudDamageIndicator( const char *pElementName );

private:
	void Init( void );
	void Reset( void );

	void ApplySchemeSettings( IScheme *pScheme );

	void DamageTaken( int iAmount, int iBits, Vector &vecFrom );
	void GetAdjustedDimensions( int iLocation, int iWide, int iTall, int &iAdjustedWide, int &iAdjustedTall );
	float GetDrawTime( bool bFallDamage );

	bool ShouldDraw( void );
	void Paint( void );
	void DrawDamageIndicator( int iLocation );

private:
	CMaterialReference m_Tex;
	int m_iTexWide, m_iTexTall;

	DamagePositions_t m_Positions;

	DamageDrawData_t m_DrawData[ DAMAGELOC_COUNT ];
	int m_iLastUpdate;
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT( CHudDamageIndicator );

//=========================================================
//=========================================================
CHudDamageIndicator::CHudDamageIndicator( const char *pElementName )
	: CHudElement( pElementName ), BaseClass( NULL, "HudDamageIndicator" )
{
	Panel *pParent = g_pClientMode->GetViewport( );
	SetParent( pParent );

	SetPaintBackgroundEnabled( false );
	SetHiddenBits( HIDEHUD_DEAD );
}

//=========================================================
//=========================================================
void CHudDamageIndicator::Reset( void )
{
	for( int i = 0; i < DAMAGELOC_COUNT; i++ )
		m_DrawData[ i ].Reset( );

	m_iLastUpdate = DAMAGELOC_INVALID;
}

//=========================================================
//=========================================================
void CHudDamageIndicator::Init( void )
{
	m_Tex.Init( DAMAGEINDI_TEX_PATH, TEXTURE_GROUP_VGUI ); 
}

//=========================================================
//=========================================================
void CHudDamageIndicator::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	// resize to screensize
	int iScreenWide, iScreenTall;
	surface( )->GetScreenSize( iScreenWide, iScreenTall );
	SetSize( iScreenWide, iScreenTall );

	// calc size
	m_iTexWide = scheme( )->GetProportionalScaledValue( DAMAGEINDI_TEX_WIDE );
	m_iTexTall = scheme( )->GetProportionalScaledValue( DAMAGEINDI_TEX_TALL );

	// calc positions
	float flInternalWide, flInternalTall;

	flInternalWide = scheme( )->GetProportionalScaledValue( DAMAGEPOS_INTERNAL_WIDE );
	flInternalTall = scheme( )->GetProportionalScaledValue( DAMAGEPOS_INTERNAL_TALL );

	for( int i = 0; i < DAMAGELOC_COUNT; i++ )
	{
		if( i == DAMAGELOC_BOTTOM )
		{
			m_Positions[ i ][ 0 ] = ( iScreenWide * 0.5f ) - scheme( )->GetProportionalScaledValue( DAMAGEPOS_INTERNAL_YGAP ) - ( flInternalTall * 0.5f );
			m_Positions[ i ][ 1 ] = iScreenTall - ( flInternalWide * ( 1.0f + DAMAGEPOS_INTERNAL_INWARD ) );
		}
		else
		{
			m_Positions[ i ][ 1 ] = ( iScreenTall * 0.5f ) - ( flInternalTall * 0.5f );

			if( i == DAMAGELOC_LEFT )
				m_Positions[ i ][ 0 ] = flInternalWide * DAMAGEPOS_INTERNAL_INWARD;
			else
				m_Positions[ i ][ 0 ] = iScreenWide - m_iTexWide - ( flInternalWide * DAMAGEPOS_INTERNAL_INWARD );
		}
	}
}

//=========================================================
//=========================================================
void CHudDamageIndicator::DamageTaken( int iAmount, int iBits, Vector &vecFrom )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	bool bFallDamage = ( ( iBits & DMG_FALL ) != 0 );

	if( !pPlayer || iAmount <= 0 || ( ( iBits & DMG_VALIDFORCE ) == 0 && !bFallDamage ) )
		return;

	// see which effect to play
	int iDamageLocation;

	if( !bFallDamage )
	{
		iDamageLocation = DAMAGELOC_INVALID;

		// now handle directional indicators
		Vector vecDelta = ( vecFrom - MainViewOrigin( ) );
		VectorNormalize( vecDelta );

		// see which quandrant the effect is in
		float flAngle;
		GetINSHUDHelper( )->GetRotation( vecDelta, &flAngle );

		for( int i = 0; i < DAMAGELOC_COUNT; i++ )
		{
			if( flAngle > g_DamageAngles[ i ][ 0 ] && flAngle < g_DamageAngles[ i ][ 1 ] )
			{
				iDamageLocation = i;
				break;
			}
		}

		// ensure valid loc
		if( iDamageLocation == DAMAGELOC_INVALID )
			return;
	}
	else
	{
		iDamageLocation = DAMAGELOC_BOTTOM;
	}

	// set new draw time
	m_DrawData[ iDamageLocation ].Init( gpGlobals->curtime + GetDrawTime( bFallDamage ), bFallDamage );
	m_iLastUpdate = iDamageLocation;
}

//=========================================================
//=========================================================
void CHudDamageIndicator::GetAdjustedDimensions( int iLocation, int iWide, int iTall, int &iAdjustedWide, int &iAdjustedTall )
{
	bool bXFlip = ( iLocation == DAMAGELOC_LEFT || iLocation == DAMAGELOC_RIGHT );
	iAdjustedWide = bXFlip ? iWide : iTall;
	iAdjustedTall = bXFlip ? iTall : iWide;
}

//=========================================================
//=========================================================
float CHudDamageIndicator::GetDrawTime( bool bFallDamage )
{
	return ( bFallDamage ? DAMAGEINDI_DRAWTIME_FALL : DAMAGEINDI_DRAWTIME_NORMAL );
}

//=========================================================
//=========================================================
bool CHudDamageIndicator::ShouldDraw( void )
{
	if( !CHudElement::ShouldDraw( ) )
		return false;

#ifdef TESTING 

	if( fdrawdindi.GetBool( ) )
		return true;

#endif

	return ( m_iLastUpdate != DAMAGELOC_INVALID && m_DrawData[ m_iLastUpdate ].m_flTime > gpGlobals->curtime );
}

//=========================================================
//=========================================================
void CHudDamageIndicator::Paint( void )
{
	for( int i = 0; i < DAMAGELOC_COUNT; i++ )
	{
		float &flDrawTime = m_DrawData[ i ].m_flTime;

	#ifdef TESTING

		if( !fdrawdindi.GetBool( ) && gpGlobals->curtime > flDrawTime )
			continue;

	#else

		if( gpGlobals->curtime > flDrawTime )
			continue;

	#endif

		int iAlpha = min( RoundFloatToInt( 255 * ( ( flDrawTime - gpGlobals->curtime ) / GetDrawTime( m_DrawData[ i ].m_bFall ) ) ), 255 );

		static int TexUVs[ DAMAGELOC_COUNT ][ 4 ][ 2 ] = {
			{ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } },
			{ { 1, 0 }, { 0, 0 }, { 0, 1 }, { 1, 1 } },
			{ { 1, 1 }, { 1, 0 }, { 0, 0 }, { 0, 1 } }
		};

		int iWide, iTall;
		GetAdjustedDimensions( i, m_iTexWide, m_iTexTall, iWide, iTall );

		IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, m_Tex );

		CMeshBuilder meshBuilder;
		meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

		meshBuilder.Color4ub( 255, 255, 255, iAlpha );
		meshBuilder.TexCoord2f( 0, TexUVs[ i ][ 0 ][ 0 ], TexUVs[ i ][ 0 ][ 1 ] );
		meshBuilder.Position3f( m_Positions[ i ][ 0 ], m_Positions[ i ][ 1 ], 0 );
		meshBuilder.AdvanceVertex( );

		meshBuilder.Color4ub( 255, 255, 255, iAlpha );
		meshBuilder.TexCoord2f( 0, TexUVs[ i ][ 1 ][ 0 ], TexUVs[ i ][ 1 ][ 1 ] );
		meshBuilder.Position3f( m_Positions[ i ][ 0 ] + iWide, m_Positions[ i ][ 1 ], 0 );
		meshBuilder.AdvanceVertex( );

		meshBuilder.Color4ub( 255, 255, 255, iAlpha );
		meshBuilder.TexCoord2f( 0, TexUVs[ i ][ 2 ][ 0 ], TexUVs[ i ][ 2 ][ 1 ] );
		meshBuilder.Position3f( m_Positions[ i ][ 0 ] + iWide, m_Positions[ i ][ 1 ] + iTall, 0 );
		meshBuilder.AdvanceVertex( );

		meshBuilder.Color4ub( 255, 255, 255, iAlpha );
		meshBuilder.TexCoord2f( 0, TexUVs[ i ][ 3 ][ 0 ], TexUVs[ i ][ 3 ][ 1 ] );
		meshBuilder.Position3f( m_Positions[ i ][ 0 ], m_Positions[ i ][ 1 ] + iTall, 0 );
		meshBuilder.AdvanceVertex( );

		meshBuilder.End( );
		pMesh->Draw( );
	}
}