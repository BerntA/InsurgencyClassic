//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "action_helper.h"

#include "ins_player_shared.h"
#include "weapon_bipod_base.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
extern ConVar hideactionindicators;

//=========================================================
//=========================================================
CUtlVector< IActionListener* > g_ActionListeners;

//=========================================================
//=========================================================
IActionListener::IActionListener( )
{
	g_ActionListeners.AddToTail( this );
}

//=========================================================
//=========================================================
IActionListener::~IActionListener( )
{
	g_ActionListeners.FindAndRemove( this );
}

//=========================================================
//=========================================================
CActionHelper g_ActionHelper;

CActionHelper::CActionHelper( )
{
	Reset( );
}

//=========================================================
//=========================================================
const char *CActionHelper::Action( int iActionID ) const
{
	static const char *pszActionText[ ACTION_COUNT ] = {
		"Open Door",			// ACTION_DOOR
		"Deploy Bipod",			// ACTION_BIPOD 
		"Pickup Weapon",		// ACTION_WEAPON
		"Open Weapon Cache",	// ACTION_WCACHE
		"Use Weapon Cache"		// ACTION_AMMOBOX
	};

	return pszActionText[ iActionID ];
}

//=========================================================
//=========================================================
void CActionHelper::Reset( void )
{
	m_iCurrentID = ACTION_INVALID;
}

//=========================================================
//=========================================================
void CActionHelper::LevelInitPreEntity( void )
{
	Reset( );
}

//=========================================================
//=========================================================
void CActionHelper::Update( float flFrameTime )
{
	if( m_flNextUpdate > gpGlobals->curtime )
		return;

	m_flNextUpdate = gpGlobals->curtime + 0.25f;

	// update current action
	int iActionID = ACTION_INVALID;

	if( CanUpdate( ) )
		iActionID = Update( );

	if( m_iCurrentID == iActionID )
		return;

	m_iCurrentID = iActionID;

	// tell the listener
	for( int i = 0; i < g_ActionListeners.Count( ); i++ )
		g_ActionListeners[ i ]->OnAction( iActionID );
}

//=========================================================
//=========================================================
bool CActionHelper::CanUpdate( void ) const
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer )
		return false;

	if( pPlayer->GetPlayerFlags( ) & ( FL_PLAYER_IRONSIGHTS | FL_PLAYER_BIPOD ) )
		return false;

	return !hideactionindicators.GetBool( );
}

//=========================================================
//=========================================================
int CActionHelper::Update( void )
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer( );

	if( !pPlayer->IsRunningAround( ) )
		return ACTION_INVALID;

	// try use entity
	C_BaseEntity *pUseEntity = pPlayer->FindUseEntity( );

	if( pUseEntity )
		return pUseEntity->ActionType( );

	// try and find bipod
	C_BaseCombatWeapon *pWeapon = pPlayer->GetActiveWeapon( );

	if( pWeapon && ( pWeapon->GetWeaponClass( ) == WEAPONCLASS_LMG || pWeapon->GetWeaponClass( ) == WEAPONCLASS_SNIPER ) )
	{
		C_WeaponBipodBase *pBipod = ( C_WeaponBipodBase* )pWeapon;

		if( pBipod->CanBipod( ) )
			return ACTION_BIPOD;
	}

	return ACTION_INVALID;
}

//=========================================================
//=========================================================
void CActionHelper::PlayerDeath( void )
{
	Reset( );
}