//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_ammobox.h"
#include "weapon_ballistic_base.h"
#include "ins_player.h"
#include "ins_recipientfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS( ins_ammobox, CINSAmmoBox );

//=========================================================
//=========================================================
CINSAmmoBox::CINSAmmoBox( )
{
}

//=========================================================
//=========================================================
void CINSAmmoBox::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( !pActivator || !pActivator->IsPlayer( ) )
		return;

	GiveAmmo( ToINSPlayer( pActivator ) );
}

//=========================================================
//=========================================================
#define GIVEAMMO_WEAPONCLIPS_PRIMARY 4
#define GIVEAMMO_WEAPONCLIPS_WEAPON 2

void CINSAmmoBox::GiveAmmo( CINSPlayer *pPlayer )
{
	// makes the player have ...
	// at least 4 clips for his primary weapon
	// at least 2 clips for his secondary weapon
	// and for each grenade he has 2 ammo

	bool bAddedPrimary, bAddedSecondary;
	bAddedPrimary = GiveClips( pPlayer, pPlayer->GetPrimaryWeapon( ), GIVEAMMO_WEAPONCLIPS_PRIMARY );
	bAddedSecondary = GiveClips( pPlayer, pPlayer->GetSecondaryWeapon( ), GIVEAMMO_WEAPONCLIPS_WEAPON );

	if( bAddedPrimary || bAddedSecondary )
		pPlayer->EmitSound( "HL2Player.PickupWeapon" );
}

//=========================================================
//=========================================================
bool CINSAmmoBox::GiveClips( CINSPlayer *pPlayer, CBaseCombatWeapon *pWeapon, int iMaximum )
{
	if( !pWeapon )
		return false;

	if( pWeapon->GetAmmoCount( ) >= iMaximum )
		return false;

	pWeapon->GiveClip( 2 );

	return true;
}