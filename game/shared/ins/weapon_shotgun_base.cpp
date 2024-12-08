//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "weapon_shotgun_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define SHOTGUN_PELLETS 5

//=========================================================
//=========================================================
int CWeaponShotgunBase::ForcedFiremode( void ) const
{
	return ( 1 << FIREMODE_SEMI );
}

//=========================================================
//=========================================================
int CWeaponShotgunBase::GetShots( void ) const
{
	return SHOTGUN_PELLETS;
}

//=========================================================
//=========================================================
int CWeaponShotgunBase::GetShellType( void ) const
{
	return INS_SHELL_SHOTGUN;
}