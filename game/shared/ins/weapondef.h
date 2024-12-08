//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#ifndef WEAPONDEF_H
#define WEAPONDEF_H

#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CWeaponDef : public CAutoGameSystem
{
public:
	static CWeaponDef *GetWeaponDef( void );

	bool Load( void );

private:
	bool Init( void );

public:
	int iDefaultViewmodelFOV, iDefaultViewmodelIronsightFOV;

	float flMaxRangeMod;

	float flAccuracyMod;
	float flIronSightsAccuracyMod;
	float flAngleExaggeration;

	float flRecoilModCrouchRunningIronsights, flRecoilModCrouchRunning, flRecoilModCrouchIronsights, flRecoilModCrouch;
	float flRecoilModProneIronsights, flRecoilModProne;
	float flRecoilModRunningIronsights, flRecoilModRunning, flRecoilModIronsights;

	float flRecoilModPush, flRecoilModPushIronsights, flRecoilModDecay, flRecoilModDecayIronsights;
	float flRecoilModBipodPunch, flRecoilModBipodDecayLength, iRecoilModBipodDecayReset;

	float flRecoilXMin, flRecoilXMax, flRecoilXMinBipod, flRecoilXMaxBipod;
	float flRecoilYMin, flRecoilYMax, flRecoilYMinBipod, flRecoilYMaxBipod;

	float flShake, flShakeIronsights, flShakeCrouched, flShakeCrouchedIronsights, flShakeProned, flShakePronedIronsights;
	float flShakeLowStamina, flShakeWounded, flShakeMoving, flShakeIronsightHold;
	float flShakeIronsightHoldTime;

	float flFreeaimDistance, flFreeaimScreenWeaponRelation;
	bool bFreeaimSWRFraction;

	float flVMOffsetStandMoveX, flVMOffsetStandMoveY, flVMOffsetStandMoveZ;
	float flVMOffsetCrouchX, flVMOffsetCrouchY, flVMOffsetCrouchZ;
	float flVMOffsetCrouchMoveX, flVMOffsetCrouchMoveY, flVMOffsetCrouchMoveZ;
	float flVMOffsetProneX, flVMOffsetProneY, flVMOffsetProneZ;

	float flDamageScale;

private:
	static CWeaponDef m_WeaponDef;

private:
	CWeaponDef( );
};

//=========================================================
//=========================================================
extern CWeaponDef *GetWeaponDef( void );

#endif // WEAPONDEF_H
 