//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "hud_selectionbox.h"

#include "ins_player_shared.h"

#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
void CC_CmdRadio( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	// only allow when valid player and a commander
	if( !pPlayer || !pPlayer->IsCommander( ) )
		return;

	HUDSelectionBox( )->Activate( SELECTIONBOX_CMDRADIO );
}

ConCommand cmdradio( "cmdradio", CC_CmdRadio );

//=========================================================
//=========================================================
class CHudCmdRadio : public ISelectionBox
{
public:
	CHudCmdRadio( ) { }

private:
	void Init( void ) { }

	bool Activate( void )
	{
		/*for( int i = 0; i < GRPCMD_COUNT; i++ )
			Manager( )->AddElement( g_pszCmdGroupText[ i ] );*/

		return true;
	}

	void Selected( void )
	{
		GetINSHUDHelper( )->CmdRadio( Manager( )->GetActiveElementID( ) );
	}
};

DECLARE_SELECTIONBOX( SELECTIONBOX_CMDRADIO, CHudCmdRadio );