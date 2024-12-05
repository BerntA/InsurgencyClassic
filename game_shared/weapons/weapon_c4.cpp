//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "weapon_c4.h"
#include "in_buttons.h"
#include "weapon_defines.h"

#ifdef GAME_DLL

#include "grenade_c4.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponC4, DT_WeaponC4 )

BEGIN_NETWORK_TABLE( CWeaponC4, DT_WeaponC4 )

#ifdef GAME_DLL

	SendPropTime( SENDINFO( m_flDeployTime ) )

#else

	RecvPropTime( RECVINFO( m_flDeployTime ) )

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponC4 )

	DEFINE_PRED_FIELD( m_flDeployTime, FIELD_FLOAT, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

LINK_ENTITY_TO_CLASS( weapon_c4, CWeaponC4 );
PRECACHE_WEAPON_REGISTER( weapon_c4 );
REGISTER_WEAPON_DATA( WEAPON_C4, WEAPONTYPE_EQUIPMENT, weapon_c4, "C4" );
REGISTER_WEAPON_DATAHELPER( WEAPON_C4, INSWeaponInfo_t );

//=========================================================
//=========================================================
CWeaponC4::CWeaponC4( )
{
}

//=========================================================
//=========================================================
int CWeaponC4::GetWeaponID( void ) const
{
	return WEAPON_C4;
}

//=========================================================
//=========================================================
void CWeaponC4::Spawn( void )
{
	BaseClass::Spawn( );

	Precache( );

	m_flDeployTime = 0.0f;
	m_bFirstDeploy = false;
}

//=========================================================
//=========================================================
void CWeaponC4::Precache( void )
{
	BaseClass::Precache( );

#ifndef CLIENT_DLL

	UTIL_PrecacheOther( "c4_grenade" );

#endif
}

//=========================================================
//=========================================================
void CWeaponC4::ItemPostFrame( void )
{
	CBasePlayer *pOwner = GetOwner( );

	if( !pOwner )
		return;

	if( m_flDeployTime != 0.0f )
	{
		bool bFinished = false;

		if( ( !( pOwner->m_nButtons & IN_ATTACK ) ) && m_bFirstDeploy )
		{
			m_flDeployTime = 0.0f;
			m_flNextPrimaryAttack = gpGlobals->curtime + 0.5f;

			bFinished = true;
		}
		else if( gpGlobals->curtime >= m_flDeployTime )
		{
			if( m_bFirstDeploy )
			{
				SendWeaponAnim( ACT_VM_SECONDARYATTACK );

				m_flNextPrimaryAttack = m_flDeployTime = gpGlobals->curtime + SequenceDuration( );

				m_bFirstDeploy = false;
			}
			else
			{
				// attach it!
				AttachC4( );

				m_bFirstDeploy = true;
				m_flDeployTime = 0.0f;

				bFinished = true;
			}
		}

		// reset movetype
		if( bFinished )
		{
			m_flTimeWeaponIdle = gpGlobals->curtime;
			pOwner->SetMoveType(MOVETYPE_WALK);
		}
	}

	BaseClass::ItemPostFrame( );
}

//=========================================================
//=========================================================
void CWeaponC4::PrimaryAttack(void)
{
	if(m_flDeployTime != 0.0f || m_bRedraw)
		return;

	//  ensure that we're looking at a surface
	if(!CanAttachC4())
		return;

	// start the deploy animation
	SendWeaponAnim(ACT_VM_PRIMARYATTACK);
	m_bFirstDeploy = true;
	m_flTimeWeaponIdle = FLT_MAX;

	// stop the player from moving
	CBasePlayer *pOwner = GetOwner();

	if(pOwner->GetMoveType() != MOVETYPE_WALK)
		return;

	if(pOwner)
		pOwner->SetMoveType(MOVETYPE_NONE);

	// set deploy time
	m_flNextPrimaryAttack = m_flDeployTime = gpGlobals->curtime + SequenceDuration();
}

//=========================================================
//=========================================================
bool CWeaponC4::CanAttachC4(void)
{
	CBasePlayer *pOwner = GetOwner();

	if(!pOwner)
		return false;

	Vector vecSrc, vecEnd, vecViewOffset;

	vecSrc = pOwner->EyePosition();
	vecViewOffset = pOwner->GetViewOffset();
	vecEnd = vecSrc + (Vector(0, 0, -1) * (vecViewOffset.z + 8));

	trace_t tr;
	UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if(tr.fraction < 1.0)
	{
		// don't attach to people
		if(ToBasePlayer(tr.m_pEnt))
			return false;

		return true;
	}
	else
	{
		return false;
	}
}

//=========================================================
//=========================================================
void CWeaponC4::AttachC4(void)
{
	CBasePlayer *pOwner = GetOwner();

	if(!pOwner)
		return;

	Vector vecSrc, vecEnd, vecViewOffset;

	vecSrc = pOwner->EyePosition();
	vecViewOffset = pOwner->GetViewOffset();
	vecEnd = vecSrc + (Vector(0, 0, -1) * (vecViewOffset.z + 8));

	trace_t tr;
	UTIL_TraceLine(vecSrc, vecEnd, MASK_SOLID, pOwner, COLLISION_GROUP_NONE, &tr);

	if(tr.fraction < 1.0)
	{
	#ifndef CLIENT_DLL
		QAngle vecAngles;
		VectorAngles(tr.plane.normal, vecAngles);

		vecAngles.x += 90;

		//CC4Grenade::CreateC4(tr.endpos + tr.plane.normal * 3, vecAngles, pOwner, AMMO_C4);

		//CTripmineGrenade *pMine = (CTripmineGrenade *)pEnt;
		//pMine->m_hOwner = GetOwner();
	#endif

		//Redraw();
	}
}