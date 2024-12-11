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

#include "view.h"
#include "iviewrender.h"
#include "view_shared.h"
#include "ivieweffects.h"
#include "iefx.h"
#include "dlight.h"
#include "model_types.h"
#include "datacache/imdlcache.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
class CHUDUnitOrders : public IINSHUDElement, public IINSUnitOrder, public IINSPostRenderListener, public IINSControlListener
{
public:
	CHUDUnitOrders( );

private:
	void Init( void );

	void Reset( void );

	void StartOrder( void );

	void PostRender( void );

	bool IsControlActive( void );
	void DoControlClose( void );
	void Selection( void );
	void Scroll( int iType );

	void UpdateModel( void );

private:
	bool m_bActive;

	C_BaseAnimating *m_pCursorModel;

	int m_iUnitOrderID;
	Vector m_vecLastPosition, m_vecLastPositionNormal;
};

//=========================================================
//=========================================================
DECLARE_INSHUDELEMENT( CHUDUnitOrders );

CHUDUnitOrders::CHUDUnitOrders( )
{
	Reset( );

	m_pCursorModel = NULL;
}

//=========================================================
//=========================================================
void CHUDUnitOrders::Init( void )
{
	GetINSHUDHelper( )->RegisterUnitOrder( this );
}

//=========================================================
//=========================================================
void CHUDUnitOrders::Reset( void )
{
	m_bActive = false;

	m_iUnitOrderID = ORDERTYPE_UNIT_SECURE;
	m_vecLastPosition.Init( );
	m_vecLastPositionNormal.Init( );
}

//=========================================================
//=========================================================
void CHUDUnitOrders::StartOrder( void )
{
	Reset( );

	m_bActive = true;

	ControlTakeFocus( );

	UpdateModel( );
}

//=========================================================
//=========================================================
void CHUDUnitOrders::PostRender( void )
{
	if( !m_bActive )
		return;

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pLocalPlayer || !m_pCursorModel )
		return;

	Vector vecStart, vecEnd;
	VectorMA( MainViewOrigin( ), 1500, MainViewForward( ), vecEnd );
	VectorMA( MainViewOrigin( ), 10, MainViewForward( ), vecStart );

	trace_t tr;
	UTIL_TraceLine( vecStart, vecEnd, MASK_SOLID, pLocalPlayer, COLLISION_GROUP_NONE, &tr );

	if( !tr.DidHitWorld( ) || tr.surface.flags & SURF_SKY )
		return;

	vecEnd = tr.endpos;

	// store last pos
	m_vecLastPosition = vecEnd;
	m_vecLastPositionNormal = tr.plane.normal;

	// make a light so the model is well lit.
	dlight_t *pDL = effects->CL_AllocDlight( 0 );

	pDL->origin = vecEnd + Vector( -100, 0, 100 );
	pDL->die = gpGlobals->curtime + 0.0001f; // go away immediately so it doesn't light the world too
	pDL->color.r = pDL->color.g = pDL->color.b = 250;
	pDL->radius = 400;

	// move player model in front of our view
	m_pCursorModel->SetAbsOrigin( vecEnd );
	m_pCursorModel->SetAbsAngles( QAngle( 0, 180, 0 ) );

	// draw it
	const CViewSetup *pViewSetup = view->GetPlayerViewSetup( );

	CViewSetup view;
	view.x = pViewSetup->x;
	view.y = pViewSetup->y;
	view.width = pViewSetup->width;
	view.height = pViewSetup->height;

	view.m_bOrtho = false;
	view.fov = 54;

	view.origin = pViewSetup->origin;
	view.angles = pViewSetup->angles;
	view.zNear = VIEW_NEARZ;
	view.zFar = 1000;

	Frustum dummyFrustum;
	render->Push3DView(view, 0, NULL, dummyFrustum);
	m_pCursorModel->DrawModel( STUDIO_RENDER );
	render->PopView( dummyFrustum );
}

//=========================================================
//=========================================================
bool CHUDUnitOrders::IsControlActive( void )
{
	return m_bActive;
}

//=========================================================
//=========================================================
void CHUDUnitOrders::DoControlClose( void )
{
	Reset( );
}

//=========================================================
//=========================================================
#define POSITION_WALLNORMAL_ADJUST 20.0f

void CHUDUnitOrders::Selection( void )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );

	if( !pPlayer )
		return;

	// find normal of plane and move out a bit
	Vector vecPosition = m_vecLastPosition + ( m_vecLastPositionNormal * POSITION_WALLNORMAL_ADJUST );
	
	trace_t tr;
	UTIL_TraceLine( m_vecLastPosition, vecPosition, MASK_SOLID, pPlayer, COLLISION_GROUP_NONE, &tr );

	if( tr.DidHitWorld( ) )
		vecPosition = m_vecLastPosition + ( m_vecLastPositionNormal * POSITION_WALLNORMAL_ADJUST * tr.fraction );

	// send off as finish
	GetINSHUDHelper( )->UnitOrder( m_iUnitOrderID, vecPosition );
}

//=========================================================
//=========================================================
void CHUDUnitOrders::Scroll( int iType )
{
	m_iUnitOrderID += ( iType == INSHUD_SCROLLUP ) ? -1 : 1;

	if( m_iUnitOrderID < 0 )
		m_iUnitOrderID = ORDERTYPE_UNIT_COUNT - 1;
	else if( m_iUnitOrderID >= ORDERTYPE_UNIT_COUNT )
		m_iUnitOrderID = 0;

	UpdateModel( );
}

//=========================================================
//=========================================================
void CHUDUnitOrders::UpdateModel( void )
{
	MDLCACHE_CRITICAL_SECTION( );

	if( !m_pCursorModel )
	{
		m_pCursorModel = new C_BaseAnimating( );
		m_pCursorModel->AddEffects( EF_NODRAW );

		// don't let the renderer draw the model normally
		m_pCursorModel->InitializeAsClientEntity( CUnitOrder::ModelName( m_iUnitOrderID ), RENDER_GROUP_OPAQUE_ENTITY );
	}

	if( m_pCursorModel )
		m_pCursorModel->SetModel( CUnitOrder::ModelName( m_iUnitOrderID ) );
}