//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
void CIdleMode::HandlePlayerCount( void )
{
	INSRules( )->SetMode( GRMODE_SQUAD );
}

//=========================================================
//=========================================================
int CIdleMode::Init( void )
{
	INSRules( )->PlayerKickBots( true );

	return GRMODE_NONE;
}