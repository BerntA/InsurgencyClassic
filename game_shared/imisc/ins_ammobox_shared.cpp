//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"

#ifdef GAME_DLL

#include "ins_ammobox.h"

#else

#include "c_ins_ammobox.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( INSAmmoBox, DT_INSAmmoBox )

BEGIN_NETWORK_TABLE( CINSAmmoBox, DT_INSAmmoBox )
END_NETWORK_TABLE( )

//=========================================================
//=========================================================
int	CINSAmmoBox::ObjectCaps( void )
{
	return ( BaseClass::ObjectCaps( ) | FCAP_IMPULSE_USE );
}