//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"
#include <vgui_controls/animationcontroller.h>
#include <vgui/isurface.h>
#include "hudelement.h"
#include "pain_helper.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
class CHudDamageIndicator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE( CHudDamageIndicator, vgui::Panel );

public:
	CHudDamageIndicator( const char *pElementName );
	void Init( void );
	void Reset( void );
	virtual bool ShouldDraw( void );

	// Handler for our message
	void MsgFunc_Damage( bf_read &msg );
	void MsgFunc_Pain( bf_read &msg );

private:
	virtual void Paint(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CPanelAnimationVarAliasType( float, m_flDmgX, "dmg_xpos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgY, "dmg_ypos", "80", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgWide, "dmg_wide", "30", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgTall1, "dmg_tall1", "300", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flDmgTall2, "dmg_tall2", "240", "proportional_float" );

	CPanelAnimationVar( Color, m_DmgColorLeft, "DmgColorLeft", "255 0 0 0" );
	CPanelAnimationVar( Color, m_DmgColorRight, "DmgColorRight", "255 0 0 0" );

	void DrawDamageIndicator(int side);
	void GetDamagePosition( const Vector &vecDelta, float *flRotation );

	CMaterialReference m_WhiteAdditiveMaterial;
};

DECLARE_HUDELEMENT( CHudDamageIndicator );
DECLARE_HUD_MESSAGE( CHudDamageIndicator, Damage );
DECLARE_HUD_MESSAGE( CHudDamageIndicator, Pain );

#define ANGLE_ANY	0.0f

struct DamageAnimation_t
{
	const char *name;
	float angleMinimum;
	float angleMaximum;
};

//=========================================================
//=========================================================
static DamageAnimation_t g_DamageAnimations[] =
{
	{ "HudTakeDamageLeft",		45.0f,		135.0f },
	{ "HudTakeDamageRight",		225.0f,		315.0f },
	{ "HudTakeDamageBehind",	135.0f,		225.0f },

	// fall through to front damage
	{ "HudTakeDamageFront",		ANGLE_ANY,	ANGLE_ANY },
	{ NULL },
};


//=========================================================
//=========================================================
CHudDamageIndicator::CHudDamageIndicator( const char *pElementName ) : CHudElement( pElementName ), BaseClass(NULL, "HudDamageIndicator")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );

	m_WhiteAdditiveMaterial.Init( "vgui/white_additive", TEXTURE_GROUP_VGUI ); 

	SetHiddenBits( HIDEHUD_SPECTATOR );
}

//=========================================================
//=========================================================
void CHudDamageIndicator::Reset( void )
{
	m_DmgColorLeft[3] = 0;
	m_DmgColorRight[3] = 0;
}

//=========================================================
//=========================================================
void CHudDamageIndicator::Init( void )
{
	HOOK_HUD_MESSAGE( CHudDamageIndicator, Damage );
	HOOK_HUD_MESSAGE( CHudDamageIndicator, Pain );
}

//=========================================================
//=========================================================
bool CHudDamageIndicator::ShouldDraw( void )
{
	if ( !CHudElement::ShouldDraw() )
		return false;

	if ( !m_DmgColorLeft[3] && !m_DmgColorRight[3] )
		return false;

	return true;
}

//=========================================================
//=========================================================
void CHudDamageIndicator::DrawDamageIndicator(int side)
{
	IMesh *pMesh = materials->GetDynamicMesh( true, NULL, NULL, m_WhiteAdditiveMaterial );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	int insetY = (m_flDmgTall1 - m_flDmgTall2) / 2;

	int x1 = m_flDmgX;
	int x2 = m_flDmgX + m_flDmgWide;
	int y[4] = { m_flDmgY, m_flDmgY + insetY, m_flDmgY + m_flDmgTall1 - insetY, m_flDmgY + m_flDmgTall1 };
	int alpha[4] = { 0.0f, 1.0f, 1.0f, 0.0f };

	int r, g, b, a;
	if (side == 1)
	{
		r = m_DmgColorRight[0], g = m_DmgColorRight[1], b = m_DmgColorRight[2], a = m_DmgColorRight[3];

		// realign x coords
		x1 = GetWide() - x1;
		x2 = GetWide() - x2;

		meshBuilder.Color4ub( r, g, b, a * alpha[0]);
		meshBuilder.TexCoord2f( 0,0,0 );
		meshBuilder.Position3f( x1, y[0], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[3] );
		meshBuilder.TexCoord2f( 0,0,1 );
		meshBuilder.Position3f( x1, y[3], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[2] );
		meshBuilder.TexCoord2f( 0,1,1 );
		meshBuilder.Position3f( x2, y[2], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[1] );
		meshBuilder.TexCoord2f( 0,1,0 );
		meshBuilder.Position3f( x2, y[1], 0 );
		meshBuilder.AdvanceVertex();
	}
	else
	{
		r = m_DmgColorLeft[0], g = m_DmgColorLeft[1], b = m_DmgColorLeft[2], a = m_DmgColorLeft[3];

		meshBuilder.Color4ub( r, g, b, a * alpha[0] );
		meshBuilder.TexCoord2f( 0,0,0 );
		meshBuilder.Position3f( x1, y[0], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[1] );
		meshBuilder.TexCoord2f( 0,1,0 );
		meshBuilder.Position3f( x2, y[1], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[2] );
		meshBuilder.TexCoord2f( 0,1,1 );
		meshBuilder.Position3f( x2, y[2], 0 );
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub( r, g, b, a * alpha[3] );
		meshBuilder.TexCoord2f( 0,0,1 );
		meshBuilder.Position3f( x1, y[3], 0 );
		meshBuilder.AdvanceVertex();
	}

	meshBuilder.End();
	pMesh->Draw();
}

//=========================================================
//=========================================================
void CHudDamageIndicator::MsgFunc_Damage( bf_read &msg )
{
	int iDmgTake = msg.ReadByte();
	int iDamageBits = msg.ReadLong();

	Vector vecFrom = vec3_origin;

	if(iDamageBits & DMG_VALIDFORCE)
	{
		vecFrom.x = msg.ReadFloat();
		vecFrom.y = msg.ReadFloat();
		vecFrom.z = msg.ReadFloat();
	}

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if(!pPlayer || iDamageBits == 0)
		return;

	// handle damage bits etc
	int iPainEffect;

	if(iDamageBits & DMG_BLAST || iDamageBits & DMG_FALL)
		iPainEffect = PAINTYPE_CMAJOR;
	else if(iDamageBits & (DMG_BULLET | DMG_BUCKSHOT | DMG_SLASH))
		iPainEffect = PAINTYPE_PMAJOR;
	else
		iPainEffect = PAINTYPE_PMINOR;

	g_PainHelper.CreatePain(iPainEffect, (void*)iDmgTake);

	// fiddle with viewangles when blasted
	if(iDamageBits & DMG_BLAST)
	{
		float flDamageFraction = iDmgTake / 100.0f;

		float flXMax = 8.0f * flDamageFraction;
		float flYMax = 6.5f * flDamageFraction;

		QAngle AngTmp = QAngle(random->RandomFloat(-flXMax, flXMax), random->RandomFloat(-flYMax, flYMax), 0) * 8;
		pPlayer->SnapEyeAngles(AngTmp);
	}

	// now handle directional indicators
	Vector vecDelta = (vecFrom - MainViewOrigin());
	VectorNormalize( vecDelta );

	if ( iDmgTake > 0 && vecFrom != vec3_origin)
	{
		// see which quandrant the effect is in
		float angle;
		GetDamagePosition( vecDelta, &angle );

		// see which effect to play
		DamageAnimation_t *dmgAnim = g_DamageAnimations;
		for ( ; dmgAnim->name != NULL; ++dmgAnim )
		{
			if ( dmgAnim->angleMinimum && angle < dmgAnim->angleMinimum )
				continue;

			if ( dmgAnim->angleMaximum && angle > dmgAnim->angleMaximum )
				continue;

			// we have a match, break
			break;
		}

		if ( dmgAnim->name )
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( dmgAnim->name );
		}
	}
}

//=========================================================
//=========================================================
void CHudDamageIndicator::GetDamagePosition( const Vector &vecDelta, float *flRotation )
{
	float flRadius = 360.0f;

	// Player Data
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();

	Vector forward, right, up(0,0,1);
	AngleVectors (playerAngles, &forward, NULL, NULL );
	forward.z = 0;
	VectorNormalize(forward);
	CrossProduct( up, forward, right );
	float front = DotProduct(vecDelta, forward);
	float side = DotProduct(vecDelta, right);
	float xpos = flRadius * -side;
	float ypos = flRadius * -front;

	// Get the rotation (yaw)
	*flRotation = atan2(xpos, ypos) + M_PI;
	*flRotation *= 180 / M_PI;

	float yawRadians = -(*flRotation) * M_PI / 180.0f;
	float ca = cos( yawRadians );
	float sa = sin( yawRadians );
				 
	// Rotate it around the circle
	xpos = (int)((ScreenWidth() / 2) + (flRadius * sa));
	ypos = (int)((ScreenHeight() / 2) - (flRadius * ca));
}

//=========================================================
//=========================================================
void CHudDamageIndicator::Paint()
{
	// draw side damage indicators
	DrawDamageIndicator(0);
	DrawDamageIndicator(1);
}

//=========================================================
//=========================================================
void CHudDamageIndicator::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	int wide, tall;
	surface()->GetScreenSize(wide, tall);
	SetSize(wide, tall);
}

//=========================================================
//=========================================================
void CHudDamageIndicator::MsgFunc_Pain(bf_read &msg)
{
	g_PainHelper.CreatePain( msg.ReadByte( ) - 1 );
}