//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		SLAM 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//

#ifndef	WEAPON_C4_H
#define	WEAPON_C4_H

#include "weapon_exhaustible_base.h"

#ifdef CLIENT_DLL
#define CWeaponC4 C_WeaponC4
#endif

//=========================================================
//=========================================================
class CWeaponC4 : public CWeaponExhaustibleBase
{
public:
	DECLARE_CLASS(CWeaponC4, CWeaponExhaustibleBase);

public:
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CWeaponC4();

	int GetWeaponID(void) const;

	void Spawn(void);
	void Precache(void);

	void ItemPostFrame(void);
	void PrimaryAttack(void);

private:
	bool CanAttachC4(void);
	void AttachC4(void);

private:
	CNetworkVar(float, m_flDeployTime);
	bool m_bFirstDeploy;

private:
	CWeaponC4(const CWeaponC4 &);
};


#endif // WEAPON_C4_H
