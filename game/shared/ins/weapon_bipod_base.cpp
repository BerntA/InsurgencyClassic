#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_bipod_base.h"
#include "gamemovement.h"
#include "engine/ivdebugoverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
CWeaponBipodBase::CWeaponBipodBase( )
{
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetCrawlActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && pOwner->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
		return ACT_VM_DEPLOYED_LIFTED_IDLE;

	return BaseClass::GetCrawlActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetNormalActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && pOwner->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
	{
		if( pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
			return ACT_VM_DEPLOYED_IRON_IDLE;
		else
			return ACT_VM_DEPLOYED_IDLE;
	}

	return BaseClass::GetNormalActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetPrimaryAttackActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && ( pOwner->GetPlayerFlags( ) & FL_PLAYER_BIPOD ) )
	{
		if( pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
			return ACT_VM_DEPLOYED_IRON_FIRE;
		else
			return ACT_VM_DEPLOYED_FIRE;
	}

	return BaseClass::GetPrimaryAttackActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetDryFireActivity( void ) const
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( pOwner && ( pOwner->GetPlayerFlags( ) & FL_PLAYER_BIPOD ) )
	{
		if( pOwner->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
			return ACT_VM_DEPLOYED_IRON_DRYFIRE;
		else
			return ACT_VM_DEPLOYED_DRYFIRE;
	}

	return BaseClass::GetDryFireActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetPrimaryAttackActivityRecoil( Activity BaseAct, int iRecoilState ) const
{
	// TODO: make Jeremy rename them?
	return BaseAct;
	//return BaseClass::GetPrimaryAttackActivityRecoil( BaseAct, iRecoilState );
}

//=========================================================
//=========================================================
bool CWeaponBipodBase::AttemptBipod( void )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return false;

	// change bipod state
	if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
	{
		int iOldOffset = pPlayer->ViewOffsetBipod( );
		pPlayer->ResetViewBipodOffset( );
		pPlayer->SetViewTransition( pPlayer->GetCurrentViewOffset( ), CalcOffsetTime( iOldOffset ) );

		pPlayer->RemovePlayerFlag( FL_PLAYER_BIPOD );

		SendWeaponAnim( ACT_VM_DEPLOYED_OUT );
	}
	else
	{
		Vector vecBipodViewOffset;
		vecBipodViewOffset.Init( );

		if( !CanBipod( &vecBipodViewOffset ) )
		{
			m_flNextSecondaryAttack = gpGlobals->curtime + 0.5f;

			return false;
		}

		if( vecBipodViewOffset.z != 0 )
		{
			Vector vecEyePos = pPlayer->EyePosition( );

			int iEyeOffset, iViewOffset;
			iEyeOffset = vecBipodViewOffset.z - vecEyePos.z + 12;
			iViewOffset = pPlayer->GetViewOffset( ).z + iEyeOffset;

			pPlayer->SetViewTransition( iViewOffset, CalcOffsetTime( iEyeOffset ) );
			pPlayer->SetViewBipodOffset( iEyeOffset );
		}

		pPlayer->AddPlayerFlag( FL_PLAYER_BIPOD );

#ifdef CLIENT_DLL

		pPlayer->SetSupportedFireAngles( pPlayer->EyeAngles( ) );

#endif

		SendWeaponAnim( ACT_VM_DEPLOYED_IN );
	}

	// reset ironsights
	SetIronsights( false, true );

#ifdef CLIENT_DLL

	// XNOTE: freeaim reset is needed here.
	pPlayer->SetFreeAimAngles( vec3_angle );

#endif

	// PNOTE: I've added a little extra time here because somebody said
	// it was an expliot?

	// delay next attack until sequence ends
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = gpGlobals->curtime + SequenceDuration( ) + 0.5f;

	return true;
}

//=========================================================
//=========================================================
void CWeaponBipodBase::SecondaryAttack( void )
{
	AttemptBipod( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetReloadActivity( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer && pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
	{
		if( IsEmpty( ) )
			return ACT_VM_DEPLOYED_RELOAD_EMPTY;
		else
			return ACT_VM_DEPLOYED_RELOAD;
	}

	return BaseClass::GetReloadActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetIronsightInActivity( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer && pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
		return ACT_VM_DEPLOYED_IRON_IN;

	return BaseClass::GetIronsightInActivity( );
}

//=========================================================
//=========================================================
Activity CWeaponBipodBase::GetIronsightOutActivity( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer && pPlayer->GetPlayerFlags( ) & FL_PLAYER_BIPOD )
		return ACT_VM_DEPLOYED_IRON_OUT;

	return BaseClass::GetIronsightOutActivity( );
}

//=========================================================
//=========================================================
//#define BIPOD_DEBUG

#define BIPOD_PRONE_FLOORDIST 20.0f
#define BIPOD_PRONE_WALLTEST_BACK -8.0f
#define BIPOD_PRONE_WALLTEST_FORWARD -2.0f

#define BIPOD_UPRIGHT_WALLTEST_START -10.0f

bool CWeaponBipodBase::CanBipod( Vector *pViewOffset )
{
	if( !AllowBipod( ) )
		return false;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return false;

	// don't go bipod when in ironsights
	if( pPlayer->GetPlayerFlags( ) & FL_PLAYER_IRONSIGHTS )
		return false;

	// don't bipod when in stance transition (otherwise the view transitions get buggered)
	if( pPlayer->InStanceTransition( ) )
		return false;

	// find useful info
	Vector vecDown = Vector( 0, 0, -1 );

	QAngle vecViewAngles = pPlayer->EyeAngles( ) + pPlayer->m_Local.m_vecPunchAngle;

	Vector vecMuzzle;
	QAngle angMuzzle;
	pPlayer->GetMuzzle( vecMuzzle, angMuzzle );

	trace_t	tr;
	Vector vecShootPos = vecMuzzle;

	// handle can if bipod
	if( pPlayer->IsProned( ) )
	{
		// ... check for a close floor underneath the player
		UTIL_TraceLine( vecShootPos, vecShootPos + ( vecDown * BIPOD_PRONE_FLOORDIST ), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

		if( !tr.DidHit( ) )
			return false;

		// ... ensure there isn't a wall right front of them
		Vector vecForward, vecWTStart, vecWTEnd;

		QAngle angForward = vecViewAngles;
		angForward[ PITCH ] = 0;

		AngleVectors( angForward + pPlayer->m_Local.m_vecPunchAngle, &vecForward );

		VectorMA( vecShootPos, BIPOD_PRONE_WALLTEST_BACK, vecForward, vecWTStart );
		VectorMA( vecShootPos, BIPOD_PRONE_WALLTEST_FORWARD, vecForward, vecWTEnd );

		UTIL_TraceLine( vecWTStart, vecWTEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

	#ifdef BIPOD_DEBUG

		debugoverlay->AddLineOverlay( vecWTStart, vecWTEnd, 0, 0, 255, true, 10.0f );

	#endif

		return ( !tr.DidHit( ) );
	}

	// PNOTE: should be able to bipod on dynamic props, however, they can move so the situation
	// of a player bipoding on nothing might occur

	// find where the nearest surface is a little bit out from muzzle
	Vector vecShotDir, vecWTStart, vecWTDiff, vecWTOffset;
	AngleVectors( angMuzzle, &vecShotDir );

	// ... find a start point a little back from the muzzle
	VectorMA( vecShootPos, BIPOD_UPRIGHT_WALLTEST_START, vecShotDir, vecWTStart );

	// ... move halfway down when crouched
	if( pPlayer->IsCrouched( ) )
		vecWTStart -= -vecDown * ( ( VEC_DUCK_VIEW.z - VEC_VIEW.z ) * 0.5f );

	// ... find the length of the test and the final position
	vecWTDiff = ( vecDown * ( ( VEC_VIEW.z - VEC_DUCK_VIEW.z ) * 0.65f ) );
	vecWTOffset = vecWTStart + vecWTDiff;

	// ... do the test
	UTIL_TraceLine( vecWTStart, vecWTOffset, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr );

#ifdef BIPOD_DEBUG

	debugoverlay->AddLineOverlay( vecWTStart, vecWTOffset, 0, 255, 0, true, 10.0f );

#endif

	// don't let them bipod when there is just thin air
	if( !tr.DidHit( ) )
		return false;

	// don't let them bipod when it's *all* in a solid
	if( tr.allsolid )
		return false;

	// final test and save
	if( tr.startsolid )
	{
		// don't let them start in a solid when standing because they cannot grow taller to match the wall
		if( !pPlayer->IsStanding( ) )
			return false;

		if( pViewOffset )
			*pViewOffset = vecWTStart + ( vecWTDiff * tr.fractionleftsolid );
	}
	else if( pViewOffset )
	{
		*pViewOffset = tr.endpos;
	}

	return true;
}

//=========================================================
//=========================================================
bool CWeaponBipodBase::Use( void )
{
	return AttemptBipod( );
}

//=========================================================
//=========================================================
float CWeaponBipodBase::CalcOffsetTime( int iEyeOffset ) const
{
	return abs( iEyeOffset * 0.05f );
}

//=========================================================
//=========================================================
bool CWeaponBipodBase::CanHolster( void )
{
	CINSPlayer *pOwner = GetINSPlayerOwner( );

	if( !pOwner )
		return true;

	return ( ( pOwner->GetPlayerFlags( ) & FL_PLAYER_BIPOD ) == 0 );
}
