//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "keyvalues.h"
#include "deadhelper.h"
#include "c_playerresource.h"
#include <vgui_controls/editablepanel.h>
#include <vgui/isurface.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
CDeadHUDHelper::CDeadHUDHelper( vgui::EditablePanel *pParent )
{
	m_pParent = pParent;

	m_bIsDead = false;

	m_iDeadXPos = m_iDeadYPos = 0;
	m_iRestoreXPos = m_iRestoreYPos = 0;
}

//=========================================================
//=========================================================
void CDeadHUDHelper::DeadInit( KeyValues *pResourceData )
{
	// find position
	AlignedPositionData_t XData, YData;
	UTIL_FindAlignment( pResourceData->GetString( "xpos_dead", NULL ), pResourceData->GetString( "ypos_dead", NULL ), XData, YData );

	// load position
	int iScreenWide, iScreenTall;
	surface( )->GetScreenSize( iScreenWide, iScreenTall );

	m_iDeadXPos = UTIL_TranslateAlignment( XData, iScreenWide );
	m_iDeadYPos = UTIL_TranslateAlignment( YData, iScreenTall );

	// restore position
	int iXPos, iYPos;
	m_pParent->GetPos( iXPos, iYPos );

	m_iRestoreXPos = iXPos;
	m_iRestoreYPos = iYPos;
}

//=========================================================
//=========================================================
void CDeadHUDHelper::DeadUpdate( void )
{
	if( !engine->IsInGame( ) )
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer( );
	C_PlayerResource *pPR = g_PR;

	if( !pPlayer || !pPR )
		return;

	bool bIsDead = !pPR->IsAlive( pPlayer->entindex( ) );

	if( m_bIsDead == bIsDead )
		return;

	if( bIsDead )
	{
		m_pParent->SetPos( m_iDeadXPos, m_iDeadYPos );

		OnDeadSize( );
	}
	else
	{
		m_pParent->SetPos( m_iRestoreXPos, m_iRestoreYPos );
	}

	m_bIsDead = bIsDead;
}