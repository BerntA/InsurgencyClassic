//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ins_player_shared.h"
#include "weapon_grenade_base.h"
#include "in_buttons.h"
#include "gamemovement.h"
#include "keyvalues.h"
#include "grenadedef.h"

#ifdef GAME_DLL

#include "grenade_thrown_base.h"
#include "ins_utils.h"
#include "ins_shared.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
#define GRENADE_RADIUS 4.0f

#define RETHROW_DELAY 0.5f

//=========================================================
//=========================================================
#ifdef GAME_DLL

extern void *SendProxy_SendLocalWeaponDataTable( const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE( CWeaponGrenadeBase, DT_LocalGrenadeWeaponData )

#ifdef GAME_DLL

	SendPropInt( SENDINFO( m_iAttackType ) ),
	SendPropBool( SENDINFO( m_bDrawbackFinished ) ),

#else

	RecvPropInt( RECVINFO( m_iAttackType ) ),
	RecvPropBool( RECVINFO( m_bDrawbackFinished ) ),

#endif

END_NETWORK_TABLE( )

//=========================================================
//=========================================================
IMPLEMENT_NETWORKCLASS_ALIASED( WeaponGrenadeBase, DT_GrenadeBase )

BEGIN_NETWORK_TABLE( CWeaponGrenadeBase, DT_GrenadeBase )

#ifdef GAME_DLL

	SendPropDataTable( "LocalGrenadeWeaponData", 0, &REFERENCE_SEND_TABLE( DT_LocalGrenadeWeaponData ), SendProxy_SendLocalWeaponDataTable ),

#else

	RecvPropDataTable( "LocalGrenadeWeaponData", 0, 0, &REFERENCE_RECV_TABLE( DT_LocalGrenadeWeaponData ) ),

#endif

END_NETWORK_TABLE( )


//=========================================================
//=========================================================
#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA( CWeaponGrenadeBase )

	DEFINE_PRED_FIELD( m_iAttackType, FIELD_INTEGER, FTYPEDESC_INSENDTABLE ),
	DEFINE_PRED_FIELD( m_bDrawbackFinished, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),

END_PREDICTION_DATA( )

#endif

//=========================================================
//=========================================================
CWeaponGrenadeBase::CWeaponGrenadeBase( )
{
	ResetAttack( );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::Spawn( void )
{
	BaseClass::Spawn( );

	m_bLoaded = true;
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

bool CWeaponGrenadeBase::CanDrop( void )
{
	if( !BaseClass::CanDrop( ) )
		return false;

	return ( m_flNextPrimaryAttack < gpGlobals->curtime );
}

#endif

//=========================================================
//=========================================================
void CWeaponGrenadeBase::HandleDeploy( void )
{
	BaseClass::HandleDeploy( );

	ResetAttack( );
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::CanHolster( void )
{
	if( !BaseClass::CanHolster( ) )
		return false;

	if( InAttack( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	ResetAttack( );

	return BaseClass::Holster( pSwitchingTo );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::ItemPostFrame( void )
{
	if( InAttack( ) && m_bDrawbackFinished )
	{
#ifdef GAME_DLL

		if( m_flCookThreshold != 0.0f && gpGlobals->curtime >= m_flCookThreshold )
		{
			Redraw( );

			OnGrenadeCooked( );

			m_flCookThreshold = 0.0f;
		}

#endif

		CINSPlayer *pPlayer = GetINSPlayerOwner( );

		if( pPlayer )
		{
			Activity ReleasedAnim = ACT_INVALID;

			switch( m_iAttackType )
			{
				case ATTACKTYPE_THROW_HOLD:
				{
					if( ( pPlayer->m_nButtons & IN_ATTACK ) == 0 )
						ReleasedAnim = ACT_VM_THROW;

					break;
				}

				case ATTACKTYPE_THROW_COOK:
				{
					if( ( pPlayer->m_nButtons & IN_SPECIAL2 ) == 0 )
						ReleasedAnim = ACT_VM_THROW;

					break;
				}

				case ATTACKTYPE_LOB:
				{
					if( ( pPlayer->m_nButtons & IN_SPECIAL1 ) == 0 )
					{
						if( pPlayer->IsStanding( ) )
							ReleasedAnim = ACT_VM_HAULBACK;
						else
							ReleasedAnim = ACT_VM_SECONDARYATTACK;
					}

					break;
				}
			}

			if( ReleasedAnim != ACT_INVALID )
			{
				SendWeaponAnim( ReleasedAnim );
				m_bDrawbackFinished = false;
			}
		}
	}

	BaseClass::ItemPostFrame( );
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::CanAttack( void )
{
	if( !BaseClass::CanAttack( ) )
		return false;

	return ( !m_bRedraw && m_bLoaded );
}

//=========================================================
//=========================================================
int CWeaponGrenadeBase::PrimaryAttackType( void )
{
	return ATTACKTYPE_THROW_HOLD;
}

//=========================================================
//=========================================================
int CWeaponGrenadeBase::SecondaryAttackType( void )
{
	return ATTACKTYPE_LOB;
}

//=========================================================
//=========================================================
int CWeaponGrenadeBase::TertiaryAttackType( void )
{
	return ATTACKTYPE_THROW_COOK;
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::CanPrimaryAttack( void )
{
	if( !BaseClass::CanPrimaryAttack( ) )
		return false;

	return ValidAttack( PrimaryAttackType( ) );
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::CanSecondaryAttack( void )
{
	if( !BaseClass::CanSecondaryAttack( ) )
		return false;

	return ValidAttack( SecondaryAttackType( ) );
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::CanTertiaryAttack( void )
{
	if( !BaseClass::CanTertiaryAttack( ) )
		return false;

	return ValidAttack( TertiaryAttackType( ) );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::PrimaryAttack( void )
{
	StartAttack( PrimaryAttackType( ) );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::SecondaryAttack( void )
{
	StartAttack( SecondaryAttackType( ) );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::TertiaryAttack( void )
{
	StartAttack( TertiaryAttackType( ) );
}

//=========================================================
//=========================================================
#ifdef GAME_DLL

void CWeaponGrenadeBase::HandleAnimEvent( animevent_t *pEvent )
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( !pPlayer )
		return;

	switch( pEvent->event )
	{
		case EVENT_WEAPON_SEQUENCE_FINISHED:
		{
			if( m_iAttackType == ATTACKTYPE_THROW_COOK )
			{
				const CGrenadeData &GrenadeData = CGrenadeDef::GetGrenadeData( GetGrenadeID( ) );
				m_flCookThreshold = gpGlobals->curtime + GrenadeData.m_flFuse;
			}

			m_bDrawbackFinished = true;

			break;
		}

		case EVENT_WEAPON_THROW:
		case EVENT_WEAPON_THROW2:
		case EVENT_WEAPON_THROW3:
		{
			bool bThrewGrenade = true;

			switch( m_iAttackType )
			{
				case ATTACKTYPE_THROW_HOLD:
				case ATTACKTYPE_THROW_COOK:
				{
					FinishAttack( SHOT_SINGLE );
					break;
				}

				case ATTACKTYPE_LOB:
				{
					FinishAttack( SHOT_DOUBLE );
					break;
				}
				
				default:
				{
					bThrewGrenade = false;
				}
			}

			if( bThrewGrenade )
			{
				m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime + RETHROW_DELAY;
				m_flTimeWeaponIdle = FLT_MAX;
			}

			break;
		}

		default:
			BaseClass::HandleAnimEvent( pEvent );
			break;
	}
}

#endif

//=========================================================
//=========================================================
void CWeaponGrenadeBase::Redraw( void )
{
	m_bRedraw = true;

	ResetAttack( );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::ResetAttack( void )
{
	m_iAttackType = ATTACKTYPE_NONE;
	m_bDrawbackFinished = false;

#ifdef GAME_DLL

	m_flCookThreshold = 0.0f;

#endif
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::ValidAttack( int iAttackType ) const
{
	return true;
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::InAttack( void ) const
{
	return ( m_iAttackType != ATTACKTYPE_NONE );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::StartAttack( int iAttackType )
{
	// set stype
	m_iAttackType = iAttackType;

	// send the animation
	static Activity Pullbacks[ ATTACKTYPE_COUNT ] = {
		ACT_INVALID,				// ATTACKTYPE_NONE
		ACT_VM_PULLBACK_HIGH,		// ATTACKTYPE_THROW_HOLD
		ACT_VM_PULLBACK_HIGH_BAKE,	// ATTACKTYPE_THROW_COOK
		ACT_VM_PULLBACK_LOW			// ATTACKTYPE_LOB
	};

	SendWeaponAnim( Pullbacks[ iAttackType ] );

	// put both of these off indefinitely because we do not know how long
	// the player will hold the grenade
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = m_flTimeWeaponIdle = FLT_MAX;

#ifdef GAME_DLL

	// not ethe weapon has been fired
	CBasePlayer *pPlayer = GetOwner();

	if( pPlayer )
		pPlayer->NoteWeaponFired( );

#endif

	// not loaded anymore
	m_bLoaded = false;
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::FinishAttack( WeaponSound_t iSoundType )
{
	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return;

#ifdef GAME_DLL

	Vector vecSrc, vecForward;

	if( !CalculateThrowOrigin( vecSrc, &vecForward ) )
		return;

	vecForward[ 2 ] += 0.1f;

	Vector vecVelocity;
	AngularImpulse angImpulse;

	if( AttackForces( vecForward, vecVelocity, angImpulse ) )
	{
		Vector vecPlayerVelocity;
		pPlayer->GetVelocity( &vecPlayerVelocity, NULL );

		vecVelocity += vecPlayerVelocity;

		EmitGrenade( vecSrc, vecVelocity, angImpulse );
	}

#endif

	WeaponSound( iSoundType );

	Redraw( );
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

bool CWeaponGrenadeBase::UseFreeaim( void ) const
{
	return InAttack( );
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

bool CWeaponGrenadeBase::CalculateThrowOrigin( Vector &vecOrigin, Vector *pForward )
{
	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return false;

	Vector vecEye, vecForward, vecRight;
	vecEye = pPlayer->EyePosition( );
	pPlayer->EyeVectors( &vecForward, &vecRight, NULL );

	vecOrigin = vecEye + vecForward * 18.0f + vecRight * 8.0f;
	CheckThrowPosition( vecEye, vecOrigin );

	if( pForward )
		*pForward = vecForward;

	return true;
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

bool CWeaponGrenadeBase::AttackForces( const Vector &vecForward, Vector &vecVelocity, AngularImpulse &angImpulse )
{
	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return false;

	switch( m_iAttackType )
	{
		case ATTACKTYPE_THROW_HOLD:
		case ATTACKTYPE_THROW_COOK:
		{
			vecVelocity = vecForward * ( pPlayer->IsProned( ) ? 620.0f : 800.0f );
			angImpulse = AngularImpulse( 600, random->RandomInt( -1200, 1200 ), 0 );

			return true;
		}

		case ATTACKTYPE_LOB:
		{
			vecVelocity = ( vecForward * ( pPlayer->IsProned( ) ? 350.0f : 425.0f ) ) + ( pPlayer->IsProned( ) ? Vector( 0, 0, 200 ) : Vector( 0, 0, 250 ) );
			angImpulse = AngularImpulse( 200, random->RandomInt( -600, 600 ), 0 );

			return true;
		}
	}

	return false;
}

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

//=========================================================
//=========================================================
int CWeaponGrenadeBase::GetGrenadeID( void ) const
{
	return INVALID_AMMODATA;
}

//=========================================================
//=========================================================
const CGrenadeData &CWeaponGrenadeBase::GetGrenadeData( void ) const
{
	return CGrenadeDef::GetGrenadeData( GetGrenadeID( ) );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::EmitGrenade( Vector &vecOrigin, Vector &vecVelocity, AngularImpulse &angImpulse )
{
	CGrenadeThrownBase::CreateThrownGrenade( GetOwner( ), GetGrenadeID( ), vecOrigin,vecVelocity, angImpulse, m_flCookThreshold );
}

//=========================================================
//=========================================================
void CWeaponGrenadeBase::CheckThrowPosition( const Vector &vecEye, Vector &vecSrc )
{
	static Vector vecGrenadeMins = -Vector( GRENADE_RADIUS + 2 ,GRENADE_RADIUS + 2 ,GRENADE_RADIUS + 2 );
	static Vector vecGrenadeMaxs = Vector( GRENADE_RADIUS + 2, GRENADE_RADIUS + 2 ,GRENADE_RADIUS + 2 );

	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return;

	trace_t tr;
	UTIL_TraceHull( vecEye, vecSrc, vecGrenadeMins, vecGrenadeMaxs, pPlayer->PhysicsSolidMaskForEntity( ), pPlayer, pPlayer->GetCollisionGroup( ), &tr );
	
	if( tr.DidHit( ) )
		vecSrc = tr.endpos;
}

//=========================================================
//=========================================================
int CWeaponGrenadeBase::GetInflictorType( void ) const
{
	return INFLICTORTYPE_GRENADE;
}

//=========================================================
//=========================================================
int CWeaponGrenadeBase::GetInflictorID( void ) const
{
	return GetGrenadeID( );
}

#endif

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::AllowSprintStart( void )
{
	return ( BaseClass::AllowSprintStart( ) && !InAttack( ) );
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::AllowPlayerStance( int iFromStance, int iToStance )
{
	if( !BaseClass::AllowPlayerStance( iFromStance, iToStance ) )
		return false;

	if( ( iFromStance == STANCE_PRONE || iToStance == STANCE_PRONE ) && InAttack( ) )
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CWeaponGrenadeBase::AllowPlayerMovement( void ) const
{
	CINSPlayer *pPlayer = GetINSPlayerOwner( );

	if( pPlayer && pPlayer->IsProned( ) && InAttack( ) )
		return false;

	return true;
}