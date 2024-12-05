//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "animation.h"
#include "basecombatweapon.h"
#include "gamerules.h"		// For g_pGameRules
#include <KeyValues.h>
#include "ammodef.h"
#include "baseviewmodel.h"
#include "in_buttons.h"
#include "soundent.h"
#include "weapon_parse.h"
#include "game.h"
#include "engine/IEngineSound.h"
#include "sendproxy.h"
#include "tier1/strtools.h"
#include "npcevent.h"
#include "igamesystem.h"
#include "collisionutils.h"
#include "func_break.h"
#include "GameBase_Server.h"
#include "GameBase_Shared.h"
#include "eventlist.h"

#ifdef HL2MP
	#include "hl2mp_gamerules.h"
#endif

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

//-----------------------------------------------------------------------------
// Purpose: Precache global weapon sounds
//-----------------------------------------------------------------------------
void W_Precache(void)
{
	PrecacheFileWeaponInfoDatabase(filesystem, GameBaseShared()->GetEncryptionKey());

	g_sModelIndexFireball = CBaseEntity::PrecacheModel ("sprites/zerogxplode.vmt"); // fireball

	g_sModelIndexSmoke = CBaseEntity::PrecacheModel ("sprites/steam1.vmt"); // smoke
	g_sModelIndexBubbles = CBaseEntity::PrecacheModel ("sprites/bubble.vmt"); //bubbles
	g_sModelIndexLaser = CBaseEntity::PrecacheModel( (char *)g_pModelNameLaser );

	GameBaseShared()->GetSharedGameDetails()->Precache();

	CBaseEntity::PrecacheModel("effects/bubble.vmt"); // bubble trails
	CBaseEntity::PrecacheModel("models/weapons/w_bullet.mdl");

	CBaseEntity::PrecacheScriptSound( "BaseCombatWeapon.WeaponDrop" );
	CBaseEntity::PrecacheScriptSound( "BaseCombatWeapon.WeaponMaterialize" );
}

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


void CBaseCombatWeapon::Operator_FrameUpdate( CBaseCombatCharacter *pOperator )
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

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	if ( (pEvent->type & AE_TYPE_NEWEVENTSYSTEM) && (pEvent->type & AE_TYPE_SERVER) )
	{
		if ( pEvent->event == AE_NPC_WEAPON_FIRE )
		{			
			return; // unused
		}
		else if ( pEvent->event == AE_WPN_PLAYWPNSOUND )
		{
			int iSnd = GetWeaponSoundFromString(pEvent->options);
			if ( iSnd != -1 )
			{
				WeaponSound( (WeaponSound_t)iSnd );
			}
		}
		else if (pEvent->event == AE_WPN_MELEE_START)
		{
			int type = atoi(pEvent->options);
			if (type)
				MeleeAttackStart(type);

			return;
		}
		else if (pEvent->event == AE_WPN_MELEE_END)
		{
			MeleeAttackEnd();
			return;
		}
	}

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

//-----------------------------------------------------------------------------
// Purpose: Make the weapon visible and tangible
//-----------------------------------------------------------------------------
CBaseEntity* CBaseCombatWeapon::Respawn( void )
{
	Vector vecOrigin = g_pGameRules->VecWeaponRespawnSpot(this);

	// make a copy of this weapon that is invisible and inaccessible to players (no touch function). The weapon spawn/respawn code
	// will decide when to make the weapon visible and touchable.
	CBaseEntity *pNewWeapon = CBaseEntity::Create(GetClassname(), vecOrigin, GetLocalAngles(), GetOwnerEntity());
	if ( pNewWeapon )
	{
		CBaseCombatWeapon *pNewCombatWeapon = (CBaseCombatWeapon*)pNewWeapon;
		if (pNewCombatWeapon)
			pNewCombatWeapon->SetWeaponStartAmmo(this->GetWeaponStartAmmo());

		pNewWeapon->AddEffects( EF_NODRAW );// invisible for now
		pNewWeapon->SetTouch( NULL );// no touch
		pNewWeapon->SetThink( &CBaseCombatWeapon::AttemptToMaterialize );

		UTIL_DropToFloor( this, MASK_SOLID );

		// not a typo! We want to know when the weapon the player just picked up should respawn! This new entity we created is the replacement,
		// but when it should respawn is based on conditions belonging to the weapon that was taken.
		pNewWeapon->SetNextThink(gpGlobals->curtime);
	}
	else
	{
		Warning("Respawn failed to create %s!\n", GetClassname() );
	}

	return pNewWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: Weapons ignore other weapons when LOS tracing
//-----------------------------------------------------------------------------
class CWeaponLOSFilter : public CTraceFilterSkipTwoEntities
{
	DECLARE_CLASS( CWeaponLOSFilter, CTraceFilterSkipTwoEntities );
public:

	CWeaponLOSFilter( IHandleEntity *pHandleEntity, IHandleEntity *pHandleEntity2, int collisionGroup ) : CTraceFilterSkipTwoEntities( pHandleEntity, pHandleEntity2, collisionGroup )
	{
	}

	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		CBaseEntity *pEntity = (CBaseEntity *)pServerEntity;

		if ((pEntity->GetCollisionGroup() == COLLISION_GROUP_WEAPON) || pEntity->IsBaseCombatWeapon())
			return false;

		if (pEntity->GetHealth() > 0)
		{
			CBreakable *pBreakable = dynamic_cast<CBreakable *>(pEntity);
			if (pBreakable  && pBreakable->IsBreakable() && pBreakable->GetMaterialType() == matGlass)
				return false;
		}

		return BaseClass::ShouldHitEntity(pServerEntity, contentsMask);
	}
};

//-----------------------------------------------------------------------------
// Purpose: Check the weapon LOS for an owner at an arbitrary position
//			If bSetConditions is true, LOS related conditions will also be set
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions )
{
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Base class always returns not bits
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::WeaponRangeAttack1Condition( float flDot, float flDist )
{
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Base class always returns not bits
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::WeaponRangeAttack2Condition( float flDot, float flDist )
{
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Base class always returns not bits
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Base class always returns not bits
//-----------------------------------------------------------------------------
int CBaseCombatWeapon::WeaponMeleeAttack2Condition( float flDot, float flDist )
{
	return -1;
}

//====================================================================================
// WEAPON DROPPING / DESTRUCTION
//====================================================================================
void CBaseCombatWeapon::Delete( void )
{
	SetTouch( NULL );
	// FIXME: why doesn't this just remove itself now?
	SetThink(&CBaseCombatWeapon::SUB_Remove);
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CBaseCombatWeapon::DestroyItem( void )
{
	CBaseCombatCharacter *pOwner = m_hOwner.Get();

	if ( pOwner )
	{
		// if attached to a player, remove. 
		pOwner->RemovePlayerItem( this );
	}

	Kill( );
}

void CBaseCombatWeapon::Kill( void )
{
	SetTouch( NULL );
	// FIXME: why doesn't this just remove itself now?
	// FIXME: how is this different than Delete(), and why do they have the same code in them?
	SetThink(&CBaseCombatWeapon::SUB_Remove);
	SetNextThink( gpGlobals->curtime + 0.1f );
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
	else
	{
	}	

	SetPickupTouch();
	
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
#ifdef HL2MP
	if ( HasSpawnFlags( SF_NORESPAWN ) == false )
	{
		VPhysicsInitNormal( SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false );
		SetMoveType( MOVETYPE_VPHYSICS );
	}
#else
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER );
#endif

	SetPickupTouch();

	SetThink (NULL);
}

//-----------------------------------------------------------------------------
// Purpose: See if the game rules will let this weapon respawn
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::AttemptToMaterialize( void )
{
	Materialize();
}

//-----------------------------------------------------------------------------
// Purpose: Weapon has been picked up, should it respawn?
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::CheckRespawn( void )
{
	if (!CanRespawnWeapon() || m_bSuppressRespawn)
		return;

	switch ( g_pGameRules->WeaponShouldRespawn( this ) )
	{
	case GR_WEAPON_RESPAWN_YES:
		Respawn();
		break;
	case GR_WEAPON_RESPAWN_NO:
		return;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CBaseCombatWeapon::ObjectCaps( void )
{ 
	int caps = BaseClass::ObjectCaps();
	if ( !IsFollowingEntity() && !HasSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP) )
	{
		caps |= FCAP_IMPULSE_USE;
	}

	return caps;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (IsDissolving())
		return;

	if (HasSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP))
		return;

	CHL2MP_Player *pClient = ToHL2MPPlayer(pActivator);
	if (pClient)
	{
		m_OnPlayerUse.FireOutput(pActivator, pCaller);

		// Bump the weapon to try equipping it before picking it up physically. This is
		// important in a few spots in the game where the player could potentially +use pickup
		// and then THROW AWAY a vital weapon, rendering them unable to continue the game.
		if (pClient->BumpWeapon(this))
			OnPickedUp(pClient);

		//pClient->PickupObject(this, false);
	}
}
