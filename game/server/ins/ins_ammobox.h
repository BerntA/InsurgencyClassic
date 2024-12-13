//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_AMMOBOX_H
#define INS_AMMOBOX_H
#ifdef _WIN32
#pragma once
#endif

#include "props.h"

//=========================================================
//=========================================================
class CINSPlayer;
class CBaseCombatWeapon;

//=========================================================
//=========================================================
class CINSAmmoBox : public CBaseProp
{
	DECLARE_CLASS( CINSAmmoBox, CBaseProp );
	DECLARE_SERVERCLASS( );

public:
	CINSAmmoBox( );

private:
	int	ObjectCaps( void );
	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void GiveAmmo( CINSPlayer *pPlayer );
	bool GiveClips( CINSPlayer *pPlayer, CBaseCombatWeapon *pWeapon, int iMaximum );
};

#endif // INS_AMMOBOX_H
