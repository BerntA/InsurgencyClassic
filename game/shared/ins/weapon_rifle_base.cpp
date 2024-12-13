//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_rifle_base.h"
#include "in_buttons.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
float CWeaponRifleBase::GetCycleTime( void )
{
	if( IsBoltAction( ) )
		return SequenceDuration( SelectWeightedSequence( GetPrimaryAttackActivity( ) ) );
	
	return BaseClass::GetCycleTime( );
}

//=========================================================
//=========================================================
void CWeaponRifleBase::CheckReload( void )
{
	if( !IsSingleLoad( ) )
	{
		BaseClass::CheckReload( );
		return;
	}

	if( !m_bInReload )
		return;

	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	if( m_bChamberedRound && m_iClip > 0 && ( pPlayer->m_nButtons & ( IN_ATTACK | IN_SPECIAL1 | IN_SPECIAL2 ) ) )
	{
		m_bInReload = false;
		return;
	}

	if( m_flNextPrimaryAttack <= gpGlobals->curtime )
	{
		if( m_iClip < GetClipSize( ) )
		{
			// add to clip buffer
			if( !HasClips( ) )
			{
				FinishReload( );
				return;
			}

			// perform reload
			PerformReload( );

			// chamber or add to clip
			if( !m_bChamberedRound && m_iClip == 0 )
				m_bChamberedRound = true;
			else
				m_iClip++;

			// remove from clips
			int &iClipCount = m_Clips[ 0 ];
			iClipCount--;

			if( iClipCount <= 0 )
				m_Clips.Remove( 0 );
		}
		else
		{
			// clip full, stop reloading
			FinishReload( );
		}
	}
}

//=========================================================
//=========================================================
void CWeaponRifleBase::FinishReload( void )
{
	if( !IsSingleLoad( ) )
	{
		BaseClass::FinishReload( );
		return;
	}

	// we're not reloading anymore
	m_bInReload = false;

	// play last animation
	Activity ReloadEnd = ( ( !m_bChamberedRound && m_iClip > 0 ) ? ACT_VM_RELOAD_END_EMPTY : ACT_VM_RELOAD_END );
	SendWeaponAnim( ReloadEnd );

	// don't allow anything for a while
	m_flNextPrimaryAttack = gpGlobals->curtime + SequenceDuration( );
}

//=========================================================
//=========================================================
Activity CWeaponRifleBase::GetReloadActivity( void ) const
{
	if( !IsSingleLoad( ) )
	{
		if( IsEmpty( ) )
			return ACT_VM_RELOADEMPTY;

		return BaseClass::GetReloadActivity( );
	}

	if( !m_bInReload )
		return BaseClass::GetReloadActivity( );
	else if( !m_bChamberedRound && m_iClip == 0 )
		return ACT_VM_RELOAD_INSERT_PULL;
	
	return ACT_VM_RELOAD_INSERT;
}