//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_touch.h"
#include "ins_player_shared.h"
#include "ins_area.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CUtlVector< CINSArea* > g_INSAreas;

//=========================================================
//=========================================================
BEGIN_DATADESC( CINSArea )

	DEFINE_KEYFIELD( m_strTitle, FIELD_STRING, "title" )

END_DATADESC()

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_area, CINSArea );

//=========================================================
//=========================================================
void CINSArea::Spawn( void )
{
	g_INSAreas.AddToTail( this );
}

//=========================================================
//=========================================================
void CINSArea::PlayerStartTouch( CINSPlayer *pPlayer )
{
	pPlayer->EnterCurrentArea( this );
}

//=========================================================
//=========================================================
void CINSArea::PlayerEndTouch( CINSPlayer *pPlayer )
{
	pPlayer->LeaveCurrentArea( );
}

//=========================================================
//=========================================================
const char *CINSArea::GetTitle( void ) const
{
	return STRING( m_strTitle );
}
