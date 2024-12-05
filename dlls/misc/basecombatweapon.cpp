//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
//#include "ai_basenpc.h"
#include "animation.h"
#include "basecombatweapon.h"
#include "gamerules.h"		// For g_pGameRules
#include <keyvalues.h>
#include "ammodef.h"
#include "weapondef.h"
#include "baseviewmodel.h"
#include "in_buttons.h"
#include "soundent.h"
#include "weapon_parse.h"
#include "game.h"
#include "engine/ienginesound.h"
#include "sendproxy.h"
#include "vstdlib/strtools.h"
#include "vphysics/constraints.h"
#include "npcevent.h"
#include "igamesystem.h"
#include "collisionutils.h"
#include "iservervehicle.h"
#include "clipdef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// -----------------------------------------
//	Sprite Index info
// -----------------------------------------
short		g_sModelIndexLaser;			// holds the index for the laser beam
const char	*g_pModelNameLaser = "sprites/laserbeam.vmt";
short		g_sModelIndexLaserDot;		// holds the index for the laser beam dot
short		g_sModelIndexFireball;		// holds the index for the fireball
short		g_sModelIndexSmoke;			// holds the index for the smoke cloud
short		g_sModelIndexWExplosion;	// holds the index for the underwater explosion
short		g_sModelIndexBubbles;		// holds the index for the bubbles model
short		g_sModelIndexBloodDrop;		// holds the sprite index for the initial blood
short		g_sModelIndexBloodSpray;	// holds the sprite index for splattered blood

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: Precache global weapon sounds
//-----------------------------------------------------------------------------
void W_Precache( void )
{
	g_sModelIndexFireball = CBaseEntity::PrecacheModel( "sprites/zerogxplode.vmt" );
	g_sModelIndexWExplosion = CBaseEntity::PrecacheModel( "sprites/WXplo1.vmt" );
	g_sModelIndexSmoke = CBaseEntity::PrecacheModel( "sprites/steam1.vmt" );
	g_sModelIndexBubbles = CBaseEntity::PrecacheModel( "sprites/bubble.vmt" );
	g_sModelIndexBloodSpray = CBaseEntity::PrecacheModel( "sprites/bloodspray.vmt" );
	g_sModelIndexBloodDrop = CBaseEntity::PrecacheModel( "sprites/blood.vmt" );
	g_sModelIndexLaser = CBaseEntity::PrecacheModel( (char* )g_pModelNameLaser );
	g_sModelIndexLaserDot = CBaseEntity::PrecacheModel( "sprites/laserdot.vmt" );
	
	CBaseEntity::PrecacheModel( "sprites/fire1.vmt" ); // precache C_EntityFlame

	CBaseEntity::PrecacheScriptSound( "BaseCombatWeapon.WeaponDrop" );
}

// Pongles ]

//-----------------------------------------------------------------------------
// Purpose: Transmit weapon data
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::UpdateTransmitState( void)
{
	// If the weapon is being carried by a CBaseCombatCharacter, let the combat character do the logic
	// about whether or not to transmit it.
	if ( GetOwner() )
	{	
		return SetTransmitState( FL_EDICT_PVSCHECK );
	}
	else
	{
		// If it's just lying around, then use CBaseEntity's visibility test to see if it should be sent.
		return BaseClass::UpdateTransmitState();
	}
}


void CBaseCombatWeapon::Operator_FrameUpdate( CBasePlayer *pOperator )
{
	StudioFrameAdvance( ); // animate

	if ( IsSequenceFinished() )
	{
		if ( SequenceLoops() )
		{
			// animation does loop, which means we're playing subtle idle. Might need to fidget.
			int iSequence = SelectWeightedSequence( GetActivity() );
			if ( iSequence != ACTIVITY_NOT_AVAILABLE )
			{
				ResetSequence( iSequence );	// Set to new anim (if it's there)
			}
		}
#if 0
		else
		{
			// animation that just ended doesn't loop! That means we just finished a fidget
			// and should return to our heaviest weighted idle (the subtle one)
			SelectHeaviestSequence( GetActivity() );
		}
#endif
	}

	// Animation events are passed back to the weapon's owner/operator
	DispatchAnimEvents( pOperator );

	// Update and dispatch the viewmodel events
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;

	CBaseViewModel *vm = pOwner->GetViewModel();
	
	if ( vm != NULL )
	{
		vm->StudioFrameAdvance();
		vm->DispatchAnimEvents( this );
	}
}

	// NOTE: This should never be called when a character is operating the weapon.  Animation events should be
	// routed through the character, and then back into CharacterAnimEvent() 

void CBaseCombatWeapon::Operator_HandleAnimEvent( animevent_t *pEvent, CBasePlayer *pOperator )
{
	DevWarning( 2, "Unhandled animation event %d from %s --> %s\n", pEvent->event, pOperator->GetClassname(), GetClassname() );
}

// NOTE: This should never be called when a character is operating the weapon.  Animation events should be
// routed through the character, and then back into CharacterAnimEvent() 
void CBaseCombatWeapon::HandleAnimEvent( animevent_t *pEvent )
{
	//If the player is receiving this message, pass it through
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner != NULL )
	{
		Operator_HandleAnimEvent( pEvent, pOwner );
	}
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::CanDrop( void )
{
	return ( !m_bInReload && GetIdealActivity( ) != GetDrawActivity( ) && GetIdealActivity( ) != GetHolsterActivity( ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DoThrow( void )
{
	CBasePlayer *pPlayer = GetOwner( );

	Throw( NULL );

	if( pPlayer )
	{
		if( pPlayer && !pPlayer->SwitchToNextBestWeapon( NULL ) )
		{
			CBaseViewModel *pVM = pPlayer->GetViewModel( );

			if( pVM )
				pVM->AddEffects( EF_NODRAW );
		}

		pPlayer->Weapon_Detach( this );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::OnDrop( void )
{
	SendWeaponAnim( ACT_VM_IDLE );
}

//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Drop( bool bNoSwitch, const Vector *pVelocity )
{
	// we've dropped!
	OnDrop( );

	// switch to next best
	CBasePlayer *pPlayer = GetOwner( );

	if( !pPlayer )
		return false;

	// set position
	Vector vThrowPos = pPlayer->Weapon_ShootPosition( ) - Vector( 0, 0, 12 );
	SetAbsOrigin( vThrowPos );

	QAngle angGunAngles;
	VectorAngles( pPlayer->BodyDirection2D( ), angGunAngles );
	SetAbsAngles( angGunAngles );

	// force basic throw if no switching
	if( bNoSwitch && IsActiveWeapon( ) )
	{
		// do throw
		Throw( pVelocity );

		// set as hidden
		CBaseViewModel *pVM = pPlayer->GetViewModel( );

		if( pVM )
			pVM->AddEffects( EF_NODRAW );

		// detach the weapon
		pPlayer->Weapon_Detach( this );

		return true;
	}
	
    return DoThrow( );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Throw( const Vector *pVelocity )
{
	// work out velocity
	Vector vecVelocity;
	vecVelocity.Init( );

	if( pVelocity )
	{
		vecVelocity = *pVelocity;
	}
	else
	{
		CBasePlayer *pPlayer = GetOwner( );

		if( pPlayer )
			vecVelocity = pPlayer->BodyDirection3D( ) * random->RandomInt( 200.0f, 300.f ) + Vector( 0, 0, 100 );	
	}

	// clear follow stuff, setup for collision
	StopAnimation( );
	StopFollowingEntity( );
	SetMoveType( MOVETYPE_VPHYSICS );
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects( EF_NODRAW );
	FallInit( );
	SetGroundEntity( NULL );
	SetTouch( NULL );

	IPhysicsObject *pObj = VPhysicsGetObject( );

	if( pObj != NULL )
	{
		AngularImpulse angImp( 200, 200, 200 );
		pObj->AddVelocity( &vecVelocity, &angImp );
	}
	else
	{
		SetAbsVelocity( vecVelocity );
	}

	SetNextThink( gpGlobals->curtime + 1.0f );
	SetOwnerEntity( NULL );
	SetOwner( NULL );
}

// Pongles ]

//====================================================================================
// WEAPON DROPPING / DESTRUCTION
//====================================================================================
void CBaseCombatWeapon::Delete( void )
{
	UTIL_Remove( this );
}


//====================================================================================
// FALL TO GROUND
//====================================================================================
//-----------------------------------------------------------------------------
// Purpose: Setup for the fall
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FallInit( void )
{
	SetModel( GetWorldModel() );
	VPhysicsDestroyObject();

	if ( !VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false ) )
	{
		SetMoveType( MOVETYPE_FLYGRAVITY );
		SetSolid( SOLID_BBOX );
		AddSolidFlags( FSOLID_TRIGGER );
	}

	SetThink( &CBaseCombatWeapon::FallThink );

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: Items that have just spawned run this think to catch them when 
//			they hit the ground. Once we're sure that the object is grounded, 
//			we change its solid type to trigger and set it in a large box that 
//			helps the player get it.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FallThink ( void )
{
	SetNextThink( gpGlobals->curtime + 0.1f );

	bool shouldMaterialize = false;
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if ( pPhysics )
	{
		shouldMaterialize = pPhysics->IsAsleep();
	}
	else
	{
		shouldMaterialize = (GetFlags() & FL_ONGROUND) ? true : false;
	}

	if ( shouldMaterialize )
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if ( GetOwnerEntity() )
		{
			EmitSound( "BaseCombatWeapon.WeaponDrop" );
		}
		Materialize(); 
	}
}

//====================================================================================
// WEAPON SPAWNING
//====================================================================================
//-----------------------------------------------------------------------------
// Purpose: Make a weapon visible and tangible
//-----------------------------------------------------------------------------// 
void CBaseCombatWeapon::Materialize( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
	{
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
	}

	SetThink(NULL);

	// Pongles [
	AutoRemove();
	// Pongles ]
}


// Pongles [
/*class CWeaponList : public CAutoGameSystem
{
public:
	virtual void LevelShutdownPostEntity()  
	{ 
		m_list.Purge();
	}

	void AddWeapon( CBaseCombatWeapon *pWeapon )
	{
		m_list.AddToTail( pWeapon );
	}

	void RemoveWeapon( CBaseCombatWeapon *pWeapon )
	{
		m_list.FindAndRemove( pWeapon );
	}
	CUtlLinkedList< CBaseCombatWeapon * > m_list;
};

CWeaponList g_WeaponList;

void OnBaseCombatWeaponCreated( CBaseCombatWeapon *pWeapon )
{
	g_WeaponList.AddWeapon( pWeapon );
}

void OnBaseCombatWeaponDestroyed( CBaseCombatWeapon *pWeapon )
{
	g_WeaponList.RemoveWeapon( pWeapon );
}

int CBaseCombatWeapon::GetAvailableWeaponsInBox( CBaseCombatWeapon **pList, int listMax, const Vector &mins, const Vector &maxs )
{
	// linear search all weapons
	int count = 0;
	int index = g_WeaponList.m_list.Head();
	while ( index != g_WeaponList.m_list.InvalidIndex() )
	{
		CBaseCombatWeapon *pWeapon = g_WeaponList.m_list[index];
		// skip any held weapon
		if ( !pWeapon->GetOwner() )
		{
			// restrict to mins/maxs
			if ( IsPointInBox( pWeapon->GetAbsOrigin(), mins, maxs ) )
			{
				if ( count < listMax )
				{
					pList[count] = pWeapon;
					count++;
				}
			}
		}
		index = g_WeaponList.m_list.Next( index );
	}

	return count;
}*/
// Pongles ]

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );
	
	if ( pPlayer )
	{
		//
		// Bump the weapon to try equipping it before picking it up physically. This is
		// important in a few spots in the game where the player could potentially +use pickup
		// and then THROW AWAY a vital weapon, rendering them unable to continue the game.
		//
		pPlayer->BumpWeapon( this, true );
	}
}

// Pongles [
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CBaseCombatWeapon::InputHideWeapon( inputdata_t &inputdata )
{
	// Only hide if we're still the active weapon. If we're not the active weapon
	if ( GetOwner() && GetOwner()->GetActiveWeapon() == this )
	{
		SetWeaponVisible( false );
	}
}*/
// Pongles ]