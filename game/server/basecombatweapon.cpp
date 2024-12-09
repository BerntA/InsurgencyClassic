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
#include "clipdef.h"
#include "GameBase_Server.h"
#include "GameBase_Shared.h"
#include "eventlist.h"

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
	// PrecacheFileWeaponInfoDatabase(filesystem, GameBaseShared()->GetEncryptionKey());

	g_sModelIndexFireball = CBaseEntity::PrecacheModel ("sprites/zerogxplode.vmt"); // fireball
	g_sModelIndexWExplosion = CBaseEntity::PrecacheModel("sprites/WXplo1.vmt");
	g_sModelIndexSmoke = CBaseEntity::PrecacheModel ("sprites/steam1.vmt"); // smoke
	g_sModelIndexBubbles = CBaseEntity::PrecacheModel ("sprites/bubble.vmt"); //bubbles
	g_sModelIndexBloodSpray = CBaseEntity::PrecacheModel("sprites/bloodspray.vmt");
	g_sModelIndexBloodDrop = CBaseEntity::PrecacheModel("sprites/blood.vmt");
	g_sModelIndexLaserDot = CBaseEntity::PrecacheModel("sprites/laserdot.vmt");
	g_sModelIndexLaser = CBaseEntity::PrecacheModel( (char *)g_pModelNameLaser );

	GameBaseShared()->GetSharedGameDetails()->Precache();

	CBaseEntity::PrecacheModel("sprites/fire1.vmt"); // precache C_EntityFlame
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

void CBaseCombatWeapon::Operator_FrameUpdate(CBaseCombatCharacter* pOperator)
{
	StudioFrameAdvance(); // animate

	if (IsSequenceFinished())
	{
		if (SequenceLoops())
		{
			// animation does loop, which means we're playing subtle idle. Might need to fidget.
			int iSequence = SelectWeightedSequence(GetActivity());
			if (iSequence != ACTIVITY_NOT_AVAILABLE)
			{
				ResetSequence(iSequence);	// Set to new anim (if it's there)
			}
		}
		}

	// Animation events are passed back to the weapon's owner/operator
	DispatchAnimEvents(pOperator);

	// Update and dispatch the viewmodel events
	CBasePlayer* pOwner = ToBasePlayer(GetOwner());
	if (pOwner == NULL)
		return;

	CBaseViewModel* vm = pOwner->GetViewModel();
	if (vm != NULL)
	{
		vm->StudioFrameAdvance();
		vm->DispatchAnimEvents(this);
	}
	}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEvent - 
//			*pOperator - 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
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

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPicker - 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::CanDrop(void)
{
	return (!m_bInReload && GetIdealActivity() != GetDrawActivity() && GetIdealActivity() != GetHolsterActivity());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::DoThrow(void)
{
	CBasePlayer* pPlayer = GetOwner();

	Throw(NULL);

	if (pPlayer)
	{
		if (pPlayer && !pPlayer->SwitchToNextBestWeapon(NULL))
		{
			CBaseViewModel* pVM = pPlayer->GetViewModel();

			if (pVM)
				pVM->AddEffects(EF_NODRAW);
		}

		pPlayer->Weapon_Detach(this);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::OnDrop(void)
{
	SendWeaponAnim(ACT_VM_IDLE);
}

//-----------------------------------------------------------------------------
// Purpose: Drop/throw the weapon with the given velocity.
//-----------------------------------------------------------------------------
bool CBaseCombatWeapon::Drop(bool bNoSwitch, const Vector* pVelocity)
{
	// we've dropped!
	OnDrop();

	// switch to next best
	CBasePlayer* pPlayer = GetOwner();
	if (!pPlayer)
		return false;

	// set position
	Vector vThrowPos = pPlayer->Weapon_ShootPosition() - Vector(0, 0, 12);
	SetAbsOrigin(vThrowPos);

	QAngle angGunAngles;
	VectorAngles(pPlayer->BodyDirection2D(), angGunAngles);
	SetAbsAngles(angGunAngles);

	// force basic throw if no switching
	if (bNoSwitch && IsActiveWeapon())
	{
		// do throw
		Throw(pVelocity);

		// set as hidden
		CBaseViewModel* pVM = pPlayer->GetViewModel();

		if (pVM)
			pVM->AddEffects(EF_NODRAW);

		// detach the weapon
		pPlayer->Weapon_Detach(this);

		return true;
	}

	return DoThrow();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Throw(const Vector* pVelocity)
{
	// work out velocity
	Vector vecVelocity;
	vecVelocity.Init();

	if (pVelocity)
	{
		vecVelocity = *pVelocity;
	}
	else
	{
		CBasePlayer* pPlayer = GetOwner();
		if (pPlayer)
			vecVelocity = pPlayer->BodyDirection3D() * random->RandomInt(200.0f, 300.f) + Vector(0, 0, 100);
	}

	// clear follow stuff, setup for collision
	StopAnimation();
	StopFollowingEntity();
	SetMoveType(MOVETYPE_VPHYSICS);
	m_iState = WEAPON_NOT_CARRIED;
	RemoveEffects(EF_NODRAW);
	FallInit();
	SetGroundEntity(NULL);
	SetTouch(NULL);

	IPhysicsObject* pObj = VPhysicsGetObject();

	if (pObj != NULL)
	{
		AngularImpulse angImp(200, 200, 200);
		pObj->AddVelocity(&vecVelocity, &angImp);
	}
	else
	{
		SetAbsVelocity(vecVelocity);
	}

	SetNextThink(gpGlobals->curtime + 1.0f);
	SetOwnerEntity(NULL);
	SetOwner(NULL);
}

//====================================================================================
// WEAPON DROPPING / DESTRUCTION
//====================================================================================
void CBaseCombatWeapon::Delete( void )
{
	UTIL_Remove(this);
}

//====================================================================================
// FALL TO GROUND
//====================================================================================
//-----------------------------------------------------------------------------
// Purpose: Setup for the fall
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FallInit( void )
{
	SetModel(GetWorldModel());
	VPhysicsDestroyObject();

	if (!VPhysicsInitNormal(SOLID_BBOX, GetSolidFlags() | FSOLID_TRIGGER, false))
	{
		SetMoveType(MOVETYPE_FLYGRAVITY);
		SetSolid(SOLID_BBOX);
		AddSolidFlags(FSOLID_TRIGGER);
	}

	SetThink(&CBaseCombatWeapon::FallThink);

	SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Items that have just spawned run this think to catch them when 
//			they hit the ground. Once we're sure that the object is grounded, 
//			we change its solid type to trigger and set it in a large box that 
//			helps the player get it.
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::FallThink ( void )
{
	SetNextThink(gpGlobals->curtime + 0.1f);

	bool shouldMaterialize = false;
	IPhysicsObject* pPhysics = VPhysicsGetObject();
	if (pPhysics)
	{
		shouldMaterialize = pPhysics->IsAsleep();
	}
	else
	{
		shouldMaterialize = (GetFlags() & FL_ONGROUND) ? true : false;
	}

	if (shouldMaterialize)
	{
		// clatter if we have an owner (i.e., dropped by someone)
		// don't clatter if the gun is waiting to respawn (if it's waiting, it is invisible!)
		if (GetOwnerEntity())
		{
			EmitSound("BaseCombatWeapon.WeaponDrop");
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
	if (IsEffectActive(EF_NODRAW))
	{
		RemoveEffects(EF_NODRAW);
		DoMuzzleFlash();
	}

	SetThink(NULL);
	AutoRemove(); // TODO add flag to disable this, for custom maps??
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseCombatWeapon::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (IsDissolving() || HasSpawnFlags(SF_WEAPON_NO_PLAYER_PICKUP))
		return;

	CBasePlayer* pPlayer = ToBasePlayer(pActivator);
	if (pPlayer == NULL)
		return;

	pPlayer->BumpWeapon(this, true);
}