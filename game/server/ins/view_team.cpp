//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//
//=============================================================================//

#include "cbase.h"
#include "view_team.h"
#include "ins_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( vteam_manager, CViewTeam )

//=========================================================
//=========================================================
IMPLEMENT_SERVERCLASS_ST( CViewTeam, DT_ViewTeam )

END_SEND_TABLE( )

//=========================================================
//=========================================================
void CViewTeam::Create( int iTeamID )
{
	CViewTeam *pTeam = ( CViewTeam* )CreateEntityByName( "vteam_manager" );
	pTeam->Init( iTeamID );
}

//=========================================================
//=========================================================
int CViewTeam::PlayerSpawnType( void ) const
{
	return ( ( GetTeamID( ) == TEAM_SPECTATOR ) ? PSPAWN_OBSERVER : PSPAWN_VIEWPOINT );
}