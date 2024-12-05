//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_GRENADE_FRAG_BASE_H
#define WEAPON_GRENADE_FRAG_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_grenade_base.h"

#ifdef GAME_DLL

#include "grenade_thrown_base.h"

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

class CFragGrenade : public CGrenadeThrownBase
{
	DECLARE_CLASS( CFragGrenade, CGrenadeThrownBase );

private:
	void Detonate( void );
};

#endif

//=========================================================
//=========================================================
class CWeaponGrenadeFragBase : public CWeaponGrenadeBase
{
	DECLARE_CLASS( CWeaponGrenadeFragBase, CWeaponGrenadeBase );

private:

#ifdef GAME_DLL

	void OnGrenadeCooked( void );

#endif

	void FinishAttack( WeaponSound_t Sound );
};


#endif // WEAPON_GRENADE_FRAG_BASE_H
