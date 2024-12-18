//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Base combat character with no AI
//
// $NoKeywords: $
//=============================================================================//

#ifndef BASECOMBATCHARACTER_H
#define BASECOMBATCHARACTER_H

#include <limits.h>

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "baseentity.h"
#include "baseflex.h"
#include "damagemodifier.h"
#include "utllinkedlist.h"
#include "physics_impact_damage.h"
#include "gibs_shared.h"
#include "soundent.h"
#include "utlvector.h"
#include "tier1/functors.h"

typedef CHandle<CBaseCombatWeapon> CBaseCombatWeaponHandle;

Vector VecCheckToss(CBaseEntity* pEdict, Vector vecSpot1, Vector vecSpot2, float flHeightMaxRatio, float flGravityAdj, bool bRandomize, Vector* vecMins = NULL, Vector* vecMaxs = NULL);
Vector VecCheckToss(CBaseEntity* pEntity, ITraceFilter* pFilter, Vector vecSpot1, Vector vecSpot2, float flHeightMaxRatio, float flGravityAdj, bool bRandomize, Vector* vecMins = NULL, Vector* vecMaxs = NULL);
Vector VecCheckThrow(CBaseEntity* pEdict, const Vector& vecSpot1, Vector vecSpot2, float flSpeed, float flGravityAdj = 1.0f, Vector* vecMins = NULL, Vector* vecMaxs = NULL);

extern Vector g_vecAttackDir;

// -------------------------------------
//  Capability Bits
// -------------------------------------

enum Capability_t 
{
	bits_CAP_MOVE_GROUND			= 0x00000001, // walk/run
	bits_CAP_MOVE_JUMP				= 0x00000002, // jump/leap
	bits_CAP_MOVE_FLY				= 0x00000004, // can fly, move all around
	bits_CAP_MOVE_CLIMB				= 0x00000008, // climb ladders
	bits_CAP_MOVE_SWIM				= 0x00000010, // navigate in water			// UNDONE - not yet implemented
	bits_CAP_MOVE_CRAWL				= 0x00000020, // crawl						// UNDONE - not yet implemented
	bits_CAP_MOVE_SHOOT				= 0x00000040, // tries to shoot weapon while moving
	bits_CAP_SKIP_NAV_GROUND_CHECK	= 0x00000080, // optimization - skips ground tests while computing navigation
	bits_CAP_USE					= 0x00000100, // open doors/push buttons/pull levers
	//bits_CAP_HEAR					= 0x00000200, // can hear forced sounds
	bits_CAP_AUTO_DOORS				= 0x00000400, // can trigger auto doors
	bits_CAP_OPEN_DOORS				= 0x00000800, // can open manual doors
	bits_CAP_TURN_HEAD				= 0x00001000, // can turn head, always bone controller 0
	bits_CAP_WEAPON_RANGE_ATTACK1	= 0x00002000, // can do a weapon range attack 1
	bits_CAP_WEAPON_RANGE_ATTACK2	= 0x00004000, // can do a weapon range attack 2
	bits_CAP_WEAPON_MELEE_ATTACK1	= 0x00008000, // can do a weapon melee attack 1
	bits_CAP_WEAPON_MELEE_ATTACK2	= 0x00010000, // can do a weapon melee attack 2
	bits_CAP_INNATE_RANGE_ATTACK1	= 0x00020000, // can do a innate range attack 1
	bits_CAP_INNATE_RANGE_ATTACK2	= 0x00040000, // can do a innate range attack 1
	bits_CAP_INNATE_MELEE_ATTACK1	= 0x00080000, // can do a innate melee attack 1
	bits_CAP_INNATE_MELEE_ATTACK2	= 0x00100000, // can do a innate melee attack 1
	bits_CAP_USE_WEAPONS			= 0x00200000, // can use weapons (non-innate attacks)
	//bits_CAP_STRAFE					= 0x00400000, // strafe ( walk/run sideways)
	bits_CAP_ANIMATEDFACE			= 0x00800000, // has animated eyes/face
	bits_CAP_USE_SHOT_REGULATOR		= 0x01000000, // Uses the shot regulator for range attack1
	bits_CAP_FRIENDLY_DMG_IMMUNE	= 0x02000000, // don't take damage from npc's that are D_LI
	bits_CAP_SQUAD					= 0x04000000, // can form squads
	bits_CAP_DUCK					= 0x08000000, // cover and reload ducking
	bits_CAP_NO_HIT_PLAYER			= 0x10000000, // don't hit players
	bits_CAP_AIM_GUN				= 0x20000000, // Use arms to aim gun, not just body
	bits_CAP_NO_HIT_SQUADMATES		= 0x40000000, // none
	bits_CAP_SIMPLE_RADIUS_DAMAGE	= 0x80000000, // Do not use robust radius damage model on this character.
};

#define bits_CAP_DOORS_GROUP    (bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)
#define bits_CAP_RANGE_ATTACK_GROUP	(bits_CAP_WEAPON_RANGE_ATTACK1 | bits_CAP_WEAPON_RANGE_ATTACK2)
#define bits_CAP_MELEE_ATTACK_GROUP	(bits_CAP_WEAPON_MELEE_ATTACK1 | bits_CAP_WEAPON_MELEE_ATTACK2)

class CBaseCombatWeapon;

#define BCC_DEFAULT_LOOK_TOWARDS_TOLERANCE 0.9f

enum Disposition_t 
{
	D_ER,		// Undefined - error
	D_HT,		// Hate
	D_FR,		// Fear
	D_LI,		// Like
	D_NU		// Neutral
};

struct Relationship_t
{
	EHANDLE			entity;			// Relationship to a particular entity
	Class_T			classType;		// Relationship to a class  CLASS_NONE = not class based (Def. in baseentity.h)
	Disposition_t	disposition;	// D_HT (Hate), D_FR (Fear), D_LI (Like), D_NT (Neutral)
};

//-----------------------------------------------------------------------------
// Purpose: This should contain all of the combat entry points / functionality 
// that are common between NPCs and players
//-----------------------------------------------------------------------------
class CBaseCombatCharacter : public CBaseFlex
{
	DECLARE_CLASS( CBaseCombatCharacter, CBaseFlex );

public:
	CBaseCombatCharacter(void);
	virtual ~CBaseCombatCharacter(void);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
	DECLARE_PREDICTABLE();

public:

	virtual void		Spawn( void );
	virtual void		Precache();

	virtual const impactdamagetable_t	&GetPhysicsImpactDamageTable( void );

	virtual int			TakeHealth( float flHealth, int bitsDamageType );

	virtual	bool		FVisible ( CBaseEntity *pEntity, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL ); // true iff the parameter can be seen by me.
	virtual bool		FVisible( const Vector &vecTarget, int traceMask = MASK_BLOCKLOS, CBaseEntity **ppBlocker = NULL )	{ return BaseClass::FVisible( vecTarget, traceMask, ppBlocker ); }
	static void			ResetVisibilityCache( CBaseCombatCharacter *pBCC = NULL );

	virtual bool		FInViewCone( CBaseEntity *pEntity );
	virtual bool		FInViewCone( const Vector &vecSpot );

	virtual bool		FInAimCone( CBaseEntity *pEntity );
	virtual bool		FInAimCone( const Vector &vecSpot );
	
	virtual bool		ShouldShootMissTarget( CBaseCombatCharacter *pAttacker );
	virtual CBaseEntity *FindMissTarget( void );

	virtual QAngle		BodyAngles();
	virtual Vector		BodyDirection2D( void );
	virtual Vector		BodyDirection3D( void );
	virtual Vector		HeadDirection2D( void )	{ return BodyDirection2D( ); }; // No head motion so just return body dir
	virtual Vector		HeadDirection3D( void )	{ return BodyDirection2D( ); }; // No head motion so just return body dir
	virtual Vector		EyeDirection2D( void ) 	{ return HeadDirection2D( );  }; // No eye motion so just return head dir
	virtual Vector		EyeDirection3D( void ) 	{ return HeadDirection3D( );  }; // No eye motion so just return head dir

	virtual void SetTransmit( CCheckTransmitInfo *pInfo, bool bAlways );

	virtual Activity	NPC_TranslateActivity( Activity baseAct );

	// -----------------------
	// Weapons
	// -----------------------
	virtual CBaseCombatWeapon*	Weapon_Create( const char *pWeaponName );
	virtual Activity	Weapon_TranslateActivity(Activity baseAct);
	virtual void		Weapon_SetActivity( Activity newActivity, float duration );
	virtual void		Weapon_FrameUpdate( void );
	virtual CBaseCombatWeapon*	Weapon_OwnsThisType(int iWeaponID) const;  // True if already owns a weapon of this class
	virtual CBaseCombatWeapon*	Weapon_GetBySlot(int slot) const;
	virtual CBaseCombatWeapon* GetNextBestWeapon(CBaseCombatWeapon* pCurrentWeapon);
	virtual void		Weapon_Equip( CBaseCombatWeapon *pWeapon );			// Adds weapon to player
	virtual bool		Weapon_Detach( CBaseCombatWeapon *pWeapon );		// Clear any pointers to the weapon.
	virtual bool		Weapon_CanUse(CBaseCombatWeapon* pWeapon) { return true; }
	virtual bool		Weapon_CanDrop(CBaseCombatWeapon* pWeapon) const;
	virtual bool		Weapon_Drop(CBaseCombatWeapon* pWeapon, bool bForce, bool bNoSwitch, const Vector* pVelocity);
	virtual	bool		Weapon_Switch(CBaseCombatWeapon *pWeapon, bool bForce = false);		// Switch to given weapon if has ammo (false if failed)
	virtual	Vector		Weapon_ShootPosition( );		// gun position at current position/orientation
	virtual	bool		Weapon_CanSwitchTo(CBaseCombatWeapon *pWeapon);

	virtual bool			CanBecomeServerRagdoll( void ) { return true; }

	// -----------------------
	// Damage
	// -----------------------
	// Don't override this for characters, override the per-life-state versions below
	virtual int				OnTakeDamage( const CTakeDamageInfo &info );

	// Override these to control how your character takes damage in different states
	virtual int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual int				OnTakeDamage_Dying( const CTakeDamageInfo &info );
	virtual int				OnTakeDamage_Dead( const CTakeDamageInfo &info );

		// utility function to calc damage force
	virtual Vector			CalcDamageForceVector( const CTakeDamageInfo &info );

	virtual int				BloodColor();

	// Character killed (only fired once)
	virtual void			Event_Killed( const CTakeDamageInfo &info );

	// Killed a character
	void InputKilledNPC( inputdata_t &inputdata );
	virtual void OnKilledNPC( CBaseCombatCharacter *pKilled ) {}; 

	// Character entered the dying state without being gibbed (only fired once)
	virtual void			Event_Dying( const CTakeDamageInfo &info );

	// character died and should become a ragdoll now
	// return true if converted to a ragdoll, false to use AI death
	virtual bool			BecomeRagdoll( const CTakeDamageInfo &info, const Vector &forceVector );
	virtual void			FixupBurningServerRagdoll( CBaseEntity *pRagdoll );

	virtual bool			BecomeRagdollBoogie( CBaseEntity *pKiller, const Vector &forceVector, float duration, int flags );

	virtual CBaseEntity		*CheckTraceHullAttack(float flDist, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float forceScale = 1.0f, bool bDamageAnyNPC = false, bool bDirect = false);
	virtual CBaseEntity		*CheckTraceHullAttack( const Vector &vStart, const Vector &vEnd, const Vector &mins, const Vector &maxs, int iDamage, int iDmgType, float flForceScale = 1.0f, bool bDamageAnyNPC = false );

	virtual CBaseCombatCharacter *MyCombatCharacterPointer( void ) { return this; }

	// VPHYSICS
	virtual void			VPhysicsShadowCollision( int index, gamevcollisionevent_t *pEvent );
	virtual void			VPhysicsUpdate( IPhysicsObject *pPhysics );
	virtual float			CalculatePhysicsStressDamage( vphysics_objectstress_t *pStressOut, IPhysicsObject *pPhysics );
	virtual void			ApplyStressDamage( IPhysicsObject *pPhysics, bool bRequireLargeObject );

	virtual void			PushawayTouch( CBaseEntity *pOther ) {}

	void SetImpactEnergyScale( float fScale ) { m_impactEnergyScale = fScale; }

	virtual void			UpdateOnRemove( void );

	virtual Disposition_t	IRelationType(CBaseEntity *pTarget, int relation = CLASS_NONE);

	virtual void			SetLightingOriginRelative( CBaseEntity *pLightingOrigin );

	virtual bool IsMaterialOverlayFlagActive(int nFlag) { return (m_nMaterialOverlayFlags & nFlag) != 0; }

	virtual void AddMaterialOverlayFlag(int nFlag)
	{
		if (IsMaterialOverlayFlagActive(nFlag))
			return;

		m_nMaterialOverlayFlags |= nFlag;
	}

	virtual void RemoveMaterialOverlayFlag(int nFlag)
	{
		if (!IsMaterialOverlayFlagActive(nFlag))
			return;

		m_nMaterialOverlayFlags &= ~nFlag;
	}

	virtual float GetIdealSpeed() const;
	virtual float GetIdealAccel() const;

protected:
	Relationship_t *FindEntityRelationship(CBaseEntity *pTarget = NULL, int relation = CLASS_NONE);

	// Material Overlay Logic:
	CNetworkVar(int, m_nMaterialOverlayFlags);

	// BB2 Gib System:
	CNetworkVar(int, m_nGibFlags);
	float m_flGibHealth[4];

	int	 m_bitsDamageType;	// what types of damage has player taken

	virtual int AllowEntityToBeGibbed(void) { return GIB_NO_GIBS; } // Override this to enable gibs.
	virtual bool CanGibEntity(const CTakeDamageInfo &info);
	virtual void OnGibbedGroup(int hitgroup, bool bExploded) { }
	virtual void OnSetGibHealth(void);

	int GetGibFlags(void) { return m_nGibFlags; }
	void SetGibFlag(int nFlag) { m_nGibFlags = nFlag; }
	void AddGibFlag(int nFlag) { m_nGibFlags |= nFlag; }
	void RemoveGibFlag(int nFlag) { m_nGibFlags &= ~nFlag; }
	void ClearGibFlags(void) { m_nGibFlags = 0; }
	bool IsGibFlagActive(int nFlag) { return (m_nGibFlags & nFlag) != 0; }

	// Any damage above this to the 'generic' region or chest region will explode the entity.
	virtual float GetExplodeFactor(void) { return 50.0f; }

public:

	// Blood color (see BLOOD_COLOR_* macros in baseentity.h)
	virtual void SetBloodColor( int nBloodColor );

	// Weapons..
	virtual CBaseCombatWeapon*	GetActiveWeapon() const;
	virtual int					WeaponCount() const;
	virtual CBaseCombatWeapon*	GetWeapon( int i ) const;
	virtual bool				RemoveWeapon(CBaseCombatWeapon* pWeapon);
	virtual void				RemovedWeapon(CBaseCombatWeapon* pWeapon) {}
	virtual void				RemoveAllWeapons();
	virtual	Vector				GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget = NULL );
	virtual	float				GetSpreadBias(  CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget );
	virtual void				DoMuzzleFlash();

	// Relationships
	static void			AllocateDefaultRelationships( );
	static void			SetDefaultRelationship( Class_T nClass, Class_T nClassTarget,  Disposition_t nDisposition );
	static void			InitDefaultAIRelationships(void);
	Disposition_t		GetDefaultRelationshipDisposition( Class_T nClassTarget );
	virtual void		AddEntityRelationship( CBaseEntity *pEntity, Disposition_t nDisposition );
	virtual bool		RemoveEntityRelationship( CBaseEntity *pEntity );
	virtual void		AddClassRelationship( Class_T nClass, Disposition_t nDisposition );

	virtual void		ChangeTeam( int iTeamNum );

	// This is a hack to blat out the current active weapon...
	// Used by weapon_slam + game_ui
	virtual void SetActiveWeapon( CBaseCombatWeapon *pNewWeapon );
	virtual void ClearActiveWeapon() { SetActiveWeapon( NULL ); }
	virtual void OnChangeActiveWeapon( CBaseCombatWeapon *pOldWeapon, CBaseCombatWeapon *pNewWeapon ) {}

	// I can't use my current weapon anymore. Switch me to the next best weapon.
	virtual bool SwitchToNextBestWeapon(CBaseCombatWeapon *pCurrent);

	// This is a hack to copy the relationship strings used by monstermaker
	void SetRelationshipString( string_t theString ) { m_RelationshipString = theString; }

	float				GetNextAttack() const { return m_flNextAttack; }
	void				SetNextAttack( float flWait ) { m_flNextAttack = flWait; }

	bool				m_bForceServerRagdoll;

	// Pickup prevention
	bool				IsAllowedToPickupWeapons( void ) { return !m_bPreventWeaponPickup; }
	void				SetPreventWeaponPickup( bool bPrevent ) { m_bPreventWeaponPickup = bPrevent; }
	bool				m_bPreventWeaponPickup;

public:
	// returns the last body region that took damage
	int	LastHitGroup() const				{ return m_LastHitGroup; }
	void SetLastHitGroup( int nHitGroup )	{ m_LastHitGroup = nHitGroup; }

public:
	CNetworkVar( float, m_flNextAttack );			// cannot attack again until this time

protected:
	int			m_bloodColor;			// color of blood particless

	// -------------------
	// combat ability data
	// -------------------
	float		m_flFieldOfView;		// cosine of field of view for this character
	Vector		m_HackedGunPos;			// HACK until we can query end of gun
	string_t	m_RelationshipString;	// Used to load up relationship keyvalues
	float		m_impactEnergyScale;// scale the amount of energy used to calculate damage this ent takes due to physics

	// attack/damage
	int					m_LastHitGroup;			// the last body region that took damage

private:
	
	static Relationship_t**		m_DefaultRelationship;

	// ---------------
	//  Relationships
	// ---------------
	CUtlVector<Relationship_t>		m_Relationship;						// Array of relationships

public:

	// Usable character items 
	CNetworkArray( CBaseCombatWeaponHandle, m_hMyWeapons, MAX_PWEAPONS);
	CNetworkHandle( CBaseCombatWeapon, m_hActiveWeapon );

protected:

	friend class CCleanupDefaultRelationShips;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
inline int	CBaseCombatCharacter::WeaponCount() const
{
	return MAX_PWEAPONS;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : i - 
//-----------------------------------------------------------------------------
inline CBaseCombatWeapon *CBaseCombatCharacter::GetWeapon( int i ) const
{
	Assert( (i >= 0) && (i < MAX_PWEAPONS) );
	return m_hMyWeapons[i].Get();
}

EXTERN_SEND_TABLE(DT_BaseCombatCharacter);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTraceFilterMelee : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterMelee );
	
	CTraceFilterMelee( const IHandleEntity *passentity, int collisionGroup, CTakeDamageInfo *dmgInfo, float flForceScale, bool bDamageAnyNPC )
		: m_pPassEnt(passentity), m_collisionGroup(collisionGroup), m_dmgInfo(dmgInfo), m_pHit(NULL), m_flForceScale(flForceScale), m_bDamageAnyNPC(bDamageAnyNPC)
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask );

public:
	const IHandleEntity *m_pPassEnt;
	int					m_collisionGroup;
	CTakeDamageInfo		*m_dmgInfo;
	CBaseEntity			*m_pHit;
	float				m_flForceScale;
	bool				m_bDamageAnyNPC;
};

#endif // BASECOMBATCHARACTER_H