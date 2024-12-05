//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud_selectionbox.h"

#include "ins_player_shared.h"
#include "voice_commands_data.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
void CC_PlayerStatus( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( pPlayer )
		HUDSelectionBox( )->Activate( SELECTIONBOX_PLAYERSTATUS );
}

ConCommand playerstatus( "playerstatus", CC_PlayerStatus );

//=========================================================
//=========================================================
class CHudPlayerStatus : public ISelectionBox
{
public:
	CHudPlayerStatus( ) { }

private:
	void Init( void ) { }

	bool Activate( void )
	{
		for( int i = 0; i < PLAYERSTATUS_COUNT; i++ )
			Manager( )->AddElement( g_pszPSGroupText[ i ] );

		return true;
	}

	bool UpdatePerFrame( void ) const { return true; }

	void Selected( void )
	{
		GetINSHUDHelper( )->PlayerStatus( Manager( )->GetActiveElementID( ) );
	}
};

DECLARE_SELECTIONBOX( SELECTIONBOX_PLAYERSTATUS, CHudPlayerStatus );
