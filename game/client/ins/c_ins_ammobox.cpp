//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_ins_ammobox.h"
#include "action_helper_types.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_ammobox, C_INSAmmoBox );

//=========================================================
//=========================================================
C_INSAmmoBox::C_INSAmmoBox( )
{
}

//=========================================================
//=========================================================
int	C_INSAmmoBox::ActionType( void ) const
{
	return ACTION_AMMOBOX;
}
