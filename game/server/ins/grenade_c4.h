//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef	GRENADE_C4_H
#define	GRENADE_C4_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
#include "weapon_grenade_frag_base.h"

//=========================================================
//=========================================================
class CC4Grenade : public CFragGrenade
{
	DECLARE_CLASS( CC4Grenade, CFragGrenade );

public:
	CC4Grenade( );

private:
	void GetDetonatorBounds( Vector &vecMins, Vector &vecMaxs ) const;

	int GetExplosionDamageFlags( void ) const;
};

#endif //GRENADE_C4_H
