//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "weapondef.h"
#include "keyvalues.h"
#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define PATH_WEAPONDEF "scripts/weapons/weapondata"

//=========================================================
//=========================================================
CWeaponDef CWeaponDef::m_WeaponDef;

//=========================================================
//=========================================================
CWeaponDef::CWeaponDef( )
{
}

//=========================================================
//=========================================================
bool CWeaponDef::Init( void )
{
	Load( );

	return true;
}

//=========================================================
//=========================================================
CWeaponDef *CWeaponDef::GetWeaponDef( void )
{
	return &CWeaponDef::m_WeaponDef;
}

//=========================================================
//=========================================================
bool CWeaponDef::Load( void )
{
	KeyValues *pWeaponDef = ReadEncryptedKVFile( ::filesystem, PATH_WEAPONDEF, GetEncryptionKey( ) );

	if( !pWeaponDef )
		return false;

	iDefaultViewmodelFOV = pWeaponDef->GetInt( "defaultfov_viewmodel", INS_WEAPONFOV_DEFAULT );
	iDefaultViewmodelIronsightFOV = pWeaponDef->GetInt( "defaultfov_viewmodel_ironsight", INS_WEAPONFOV_DEFAULT );

	flMaxRangeMod = pWeaponDef->GetFloat( "maxrange_mod", 3.0f );

	flAccuracyMod = pWeaponDef->GetFloat( "accuracy_mod", 1.0f );
	flIronSightsAccuracyMod = pWeaponDef->GetFloat( "accuracy_mod_ironsights", 1.0f );
	flAngleExaggeration = pWeaponDef->GetFloat( "angle_exaggeration", 1.0f );

	flRecoilModCrouchRunningIronsights = pWeaponDef->GetFloat( "recoilmod_crouch_running_ironsights" );
	flRecoilModCrouchRunning = pWeaponDef->GetFloat( "recoilmod_crouch_running" );
	flRecoilModCrouchIronsights = pWeaponDef->GetFloat( "recoilmod_crouch_ironsights" );
	flRecoilModCrouch = pWeaponDef->GetFloat( "recoilmod_crouch" );

	flRecoilModProneIronsights = pWeaponDef->GetFloat( "recoilmod_prone_ironsights" );
	flRecoilModProne = pWeaponDef->GetFloat( "recoilmod_prone" );

	flRecoilModRunningIronsights = pWeaponDef->GetFloat( "recoilmod_running_ironsights" );
	flRecoilModRunning = pWeaponDef->GetFloat( "recoilmod_running" );
	flRecoilModIronsights = pWeaponDef->GetFloat( "recoilmod_ironsights" );

	flRecoilModPush = pWeaponDef->GetFloat( "recoilmod_push" );
	flRecoilModPushIronsights = pWeaponDef->GetFloat( "recoilmod_push_ironsights" );
	flRecoilModDecay = pWeaponDef->GetFloat( "recoilmod_decay" );
	flRecoilModDecayIronsights = pWeaponDef->GetFloat( "recoilmod_decay_ironsights" );
	flRecoilModBipodPunch = pWeaponDef->GetFloat( "recoilmod_bipod_punch" );
	flRecoilModBipodDecayLength = pWeaponDef->GetFloat( "recoilmod_bipod_decay_length" );
	iRecoilModBipodDecayReset = pWeaponDef->GetFloat( "recoilmod_bipod_decay_reset" );

	flRecoilXMin = pWeaponDef->GetFloat( "recoil_x_min" );
	flRecoilXMax = pWeaponDef->GetFloat( "recoil_x_max" );
	flRecoilYMin = pWeaponDef->GetFloat( "recoil_y_min" );
	flRecoilYMax = pWeaponDef->GetFloat( "recoil_y_max" );

	flRecoilXMinBipod = pWeaponDef->GetFloat( "recoil_x_min_bipod" );
	flRecoilXMaxBipod = pWeaponDef->GetFloat( "recoil_x_max_bipod" );
	flRecoilYMinBipod = pWeaponDef->GetFloat( "recoil_y_min_bipod" );
	flRecoilYMaxBipod = pWeaponDef->GetFloat( "recoil_y_max_bipod" );

	flShake = pWeaponDef->GetFloat( "shakiness" );
	flShakeIronsights = pWeaponDef->GetFloat( "shakiness_ironsights" );
	flShakeCrouched = pWeaponDef->GetFloat( "shakiness_crouched" );
	flShakeCrouchedIronsights = pWeaponDef->GetFloat( "shakiness_crouched_ironsights" );
	flShakeProned = pWeaponDef->GetFloat( "shakiness_proned" );
	flShakePronedIronsights = pWeaponDef->GetFloat( "shakiness_proned_ironsights" );

	flShakeLowStamina = pWeaponDef->GetFloat( "shakefactor_lowstamina", 1.0f );
	flShakeMoving = pWeaponDef->GetFloat( "shakefactor_moving", 1.0f );
	flShakeWounded = pWeaponDef->GetFloat( "shakefactor_wounded", 1.0f );
	flShakeIronsightHold = pWeaponDef->GetFloat( "shakefactor_ishold", 1.0f );

	flShakeIronsightHoldTime = pWeaponDef->GetFloat( "shakefactor_isholdtime", 6.0f );

	flFreeaimDistance = pWeaponDef->GetFloat( "freeaim_distance" );
	flFreeaimScreenWeaponRelation = pWeaponDef->GetFloat( "freeaim_screenweapon_relation" );
	bFreeaimSWRFraction = pWeaponDef->GetInt( "freeaim_swr_fraction" ) ? true : false;

	flVMOffsetStandMoveX = pWeaponDef->GetFloat( "vmoffset_stand_move_x" );
	flVMOffsetStandMoveY = pWeaponDef->GetFloat( "vmoffset_stand_move_y" );
	flVMOffsetStandMoveZ = pWeaponDef->GetFloat( "vmoffset_stand_move_z" );

	flVMOffsetCrouchX = pWeaponDef->GetFloat( "vmoffset_crouch_x" );
	flVMOffsetCrouchY = pWeaponDef->GetFloat( "vmoffset_crouch_y" );
	flVMOffsetCrouchZ = pWeaponDef->GetFloat( "vmoffset_crouch_z" );

	flVMOffsetCrouchMoveX = pWeaponDef->GetFloat( "vmoffset_crouch_move_x" );
	flVMOffsetCrouchMoveY = pWeaponDef->GetFloat( "vmoffset_crouch_move_y" );
	flVMOffsetCrouchMoveZ = pWeaponDef->GetFloat( "vmoffset_crouch_move_z" );

	flVMOffsetProneX = pWeaponDef->GetFloat( "vmoffset_prone_x" );
	flVMOffsetProneY = pWeaponDef->GetFloat( "vmoffset_prone_y" );
	flVMOffsetProneZ = pWeaponDef->GetFloat( "vmoffset_prone_z" );

	flDamageScale = pWeaponDef->GetFloat( "damage_scale", 1.0f );

	pWeaponDef->deleteThis( );

	return true;
}

//=========================================================
//=========================================================
CWeaponDef *GetWeaponDef( void )
{
	return CWeaponDef::GetWeaponDef( );
}

//=========================================================
//=========================================================
#ifdef TESTING

void CC_ReloadWeaponDef( void )
{
	if( GetWeaponDef( )->Load( ) )
		Msg( "Done.\n" );
	else
		Msg( "Failed.\n" );
}

ConCommand reloadweapondef( "ins_debug_rweapondef", CC_ReloadWeaponDef );

#endif