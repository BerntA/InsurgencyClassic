//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "grenade_c4.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( c4_grenade, CC4Grenade );

//=========================================================
//=========================================================
CC4Grenade::CC4Grenade( )
{
}

//=========================================================
//=========================================================
void CC4Grenade::GetDetonatorBounds( Vector &vecMins, Vector &vecMaxs ) const
{
	vecMins = Vector( -6, -6, -2 );
	vecMaxs = Vector(  6,  6,  2 );
}

//=========================================================
//=========================================================
int CC4Grenade::GetExplosionDamageFlags( void ) const
{
	return DMG_ENGINEER;
}