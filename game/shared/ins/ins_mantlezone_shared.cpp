//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#ifdef GAME_DLL
#include "ins_obj.h"
#include "hint_helper.h"
#endif
#include "ins_mantlezone_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_mantle, CMantleZone );

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( MantleZone, DT_MantleZone )

BEGIN_NETWORK_TABLE( CMantleZone, DT_MantleZone )
#ifdef GAME_DLL
#else
#endif
END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef GAME_DLL

BEGIN_DATADESC( CMantleZone )
END_DATADESC( )

//=========================================================
//=========================================================
void CMantleZone::PlayerStartTouch( CINSPlayer *pPlayer )
{
	m_Players.AddToTail( pPlayer );

	if( !m_pfnThink )
	{
		SetThink( &CMantleZone::PlayerThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

//=========================================================
//=========================================================
void CMantleZone::PlayerEndTouch( CINSPlayer *pPlayer )
{
	m_Players.FindAndRemove( pPlayer );

	if( m_Players.Count( ) <= 0 )
		SetThink( NULL );
}

//=========================================================
//=========================================================
void CMantleZone::PlayerThink( void )
{
}

#endif