//=========       Copyright © Reperio-Studios 2015 @ Bernt A Eide!       ============//
//
// Purpose: Base Weapon Handling - Handles FX, Bash, Special stuff...
//
//==================================================================================//

#ifndef COMBATWEAPON_SHARED_H
#define COMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "sharedInterface.h"
#include "vphysics_interface.h"
#include "predictable_entity.h"
#include "soundflags.h"
#include "weapon_parse.h"
#include "baseviewmodel_shared.h"
#include "utlmap.h"

#if defined( CLIENT_DLL )
#include "interpolatedvar.h"
#define CBaseCombatWeapon C_BaseCombatWeapon
#endif

#if !defined( CLIENT_DLL )
void* SendProxy_SendLocalWeaponDataTable(const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID);
#endif

class CBasePlayer;
class CBaseCombatCharacter;
class IPhysicsConstraint;
class CUserCmd;

#define SF_WEAPON_NO_PLAYER_PICKUP	(1<<1)
#define SF_WEAPON_NO_PHYSCANNON_PUNT (1<<2)
#define SF_WEAPON_NO_MOTION (1<<3)

// Put this in your derived class definition to declare it's activity table
// UNDONE: Cascade these?
#define DECLARE_ACTTABLE()		static acttable_t m_acttable[];\
	acttable_t *ActivityList( void );\
	int ActivityListCount( void );

// You also need to include the activity table itself in your class' implementation:
// e.g.
//	acttable_t	CWeaponStunstick::m_acttable[] = 
//	{
//		{ ACT_MELEE_ATTACK1, ACT_MELEE_ATTACK_SWING, TRUE },
//	};
//
// The stunstick overrides the ACT_MELEE_ATTACK1 activity, replacing it with ACT_MELEE_ATTACK_SWING.
// This animation is required for this weapon's operation.
//

// Put this after your derived class' definition to implement the accessors for the
// activity table.
// UNDONE: Cascade these?
#define IMPLEMENT_ACTTABLE(className) \
	acttable_t *className::ActivityList( void ) { return m_acttable; } \
	int className::ActivityListCount( void ) { return ARRAYSIZE(m_acttable); } \

typedef struct
{
	int			baseAct;
	int			weaponAct;
	bool		required;
} acttable_t;

class CHudTexture;
class Color;

#define WEAPON_INVALID -1

namespace vgui2
{
	typedef unsigned long HFont;
}

// -----------------------------------------
//	Vector cones
// -----------------------------------------
// VECTOR_CONE_PRECALCULATED - this resolves to vec3_origin, but adds some
// context indicating that the person writing the code is not allowing
// FireBullets() to modify the direction of the shot because the shot direction
// being passed into the function has already been modified by another piece of
// code and should be fired as specified. See GetActualShotTrajectory(). 

// NOTE: The way these are calculated is that each component == sin (degrees/2)
#define VECTOR_CONE_PRECALCULATED	vec3_origin
#define VECTOR_CONE_1DEGREES		Vector( 0.00873, 0.00873, 0.00873 )
#define VECTOR_CONE_2DEGREES		Vector( 0.01745, 0.01745, 0.01745 )
#define VECTOR_CONE_3DEGREES		Vector( 0.02618, 0.02618, 0.02618 )
#define VECTOR_CONE_4DEGREES		Vector( 0.03490, 0.03490, 0.03490 )
#define VECTOR_CONE_5DEGREES		Vector( 0.04362, 0.04362, 0.04362 )
#define VECTOR_CONE_6DEGREES		Vector( 0.05234, 0.05234, 0.05234 )
#define VECTOR_CONE_7DEGREES		Vector( 0.06105, 0.06105, 0.06105 )
#define VECTOR_CONE_8DEGREES		Vector( 0.06976, 0.06976, 0.06976 )
#define VECTOR_CONE_9DEGREES		Vector( 0.07846, 0.07846, 0.07846 )
#define VECTOR_CONE_10DEGREES		Vector( 0.08716, 0.08716, 0.08716 )
#define VECTOR_CONE_15DEGREES		Vector( 0.13053, 0.13053, 0.13053 )
#define VECTOR_CONE_20DEGREES		Vector( 0.17365, 0.17365, 0.17365 )

//-----------------------------------------------------------------------------
// Purpose: Base weapon class, shared on client and server
//-----------------------------------------------------------------------------

#define BASECOMBATWEAPON_DERIVED_FROM		CBaseAnimating

class CBaseCombatWeapon : public BASECOMBATWEAPON_DERIVED_FROM
{
public:
	DECLARE_CLASS(CBaseCombatWeapon, BASECOMBATWEAPON_DERIVED_FROM);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CBaseCombatWeapon();
	virtual 				~CBaseCombatWeapon();

	virtual bool			IsBaseCombatWeapon(void) const { return true; }
	virtual CBaseCombatWeapon* MyCombatWeaponPointer(void) { return this; }

	virtual int GetWeaponID(void) const { return WEAPON_INVALID; }
	virtual int GetWeaponType(void) const;
	virtual int GetWeaponClass(void) const { return WEAPON_INVALID; }

	virtual bool IsExhaustible(void) { return false; }
	virtual bool IsPredicted(void) const { return true; }

	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void AutoRemove(void);

	virtual int ObjectCaps(void);

#ifdef CLIENT_DLL
	virtual void CreateMove(float flInputSampleTime, CUserCmd* pCmd, const QAngle& vecOldViewAngles) {}
#endif

	virtual bool IsActiveWeapon(void);
	virtual void Equip(CBasePlayer* pOwner);

#ifdef GAME_DLL
	virtual bool CanDrop(void);
	virtual bool Drop(bool bNoSwitch, const Vector* pVelocity);
#endif

	virtual int	UpdateClientData(CBasePlayer* pPlayer);

	virtual void SetWeaponVisible(bool bVisible);
	virtual bool IsWeaponVisible(void);

	// Ammo Management
	virtual bool HasAmmo(void) const;
	virtual int GetAmmoCount(void) const;

	virtual void GiveClip(int iCount) {}
	virtual void GiveAmmo(int iCount) {}

	// Viewmodel Management
	virtual void SetViewModel(void);
	virtual CBaseViewModel* GetOwnerViewModel(void);
	virtual bool ShouldDrawViewModel(void) const { return true; }
	virtual bool SendWeaponAnim(int iActivity);
	virtual void SendViewModelAnim(int nSequence);
	virtual float GetViewModelSequenceDuration(void);
	virtual bool IsViewModelSequenceFinished(void);

	// Deployment
	virtual void DefaultDeploy(char* szViewModel, char* szWeaponModel, int iActivity, char* szAnimExt);
	virtual bool CanDeploy(void);
	virtual void Deploy(void);
	virtual void HandleDeploy(void) {}

	// Holstering
	virtual bool CanHolster(void);
	virtual bool Holster(CBaseCombatWeapon* pSwitchingTo = NULL);
	virtual float GetHolsterTime(void) const { return m_flHolsterTime; }

	// Thinking
	virtual void ItemPreFrame(void);					// called each frame by the player PreThink
	virtual void ItemPostFrame(void);					// called each frame by the player PostThink
	virtual void ItemBusyFrame(void);					// called each frame by the player PostThink, if the player's not ready to attack yet
	virtual void WeaponIdle(void);						// called when no buttons pressed

	// Idling
	virtual void SetWeaponIdleTime(float flTime);
	virtual float GetWeaponIdleTime(void);

	// Attacking
	virtual bool TestWaterAttack(void) const;
	virtual bool AllowWaterAttack(void) const;

	virtual bool HandleAttack(void);
	virtual bool CanAttack(void);

	virtual bool IsEmptyAttack(void);
	virtual bool CanEmptyAttack(void);
	virtual void EmptyAttack(void);

	virtual bool HasPrimaryAttack(void);
	virtual bool HasSecondaryAttack(void);
	virtual bool HasTertiaryAttack(void);

	virtual bool HandlePrimaryAttack(void);
	virtual bool HandleSecondaryAttack(void);
	virtual bool HandleTertiaryAttack(void);

	virtual bool CanPrimaryAttack(void);
	virtual bool CanSecondaryAttack(void);
	virtual bool CanTertiaryAttack(void);

#ifdef CLIENT_DLL

	virtual bool StoreLastPrimaryAttack(void);
	virtual bool StoreLastSecondaryAttack(void);
	virtual bool StoreLastTertiaryAttack(void);

#endif

	virtual void PrimaryAttack(void) {}
	virtual void SecondaryAttack(void) {}
	virtual void TertiaryAttack(void) {}

	// Empty Handling
	bool UseEmptyAnimations(void) const;
	virtual bool EnableEmptyAnimations(void) const;
	virtual bool ShouldEmptyAnimate(void) const;

	// Animations
	virtual Activity GetPrimaryAttackActivity(void) const;

	virtual Activity GetDrawActivity(void);
	virtual Activity GetHolsterActivity(void);
	virtual Activity GetWeaponIdleActivity(void) const;

	virtual void DoAnimationEvent(int iEvent) {}

	void SetActivity(Activity act, float flDuration);
	inline void	SetActivity(Activity eActivity) { m_Activity = eActivity; }
	inline Activity GetActivity(void) { return m_Activity; }

	virtual acttable_t* ActivityList(void) { return NULL; }
	virtual int	ActivityListCount(void) { return 0; }

	virtual Activity GetIdealActivity(void) const { return m_IdealActivity; }
	virtual int GetIdealSequence(void) const { return m_nIdealSequence; }

	virtual bool SetIdealActivity(Activity ideal);
	virtual void MaintainIdealActivity(void);

	// Reloading
	virtual bool CanReload(void);
	virtual void CheckReload(void);
	virtual void StartReload(void);
	virtual void PerformReload(void);
	virtual void FinishReload(void);
	virtual void AbortReload(void);
	virtual void Reload(void);
	virtual bool IsEmptyReload(void);
	virtual void HandleEmptyReload(void);
	virtual Activity GetReloadActivity(void) const;
	virtual Activity GetEmptyReloadActivity(void) const;
	virtual bool IsReloading(void) const { return m_bInReload; }

	// Sound
	virtual void WeaponSound(WeaponSound_t sound_type, float flSoundTime = 0.0f);
	virtual void StopWeaponSound(WeaponSound_t sound_type);

	// Misc
	virtual CBasePlayer* GetOwner(void) const;

	virtual void SetOwner(CBasePlayer* pOwner);

	virtual void AddViewmodelBob(CBaseViewModel* viewmodel, Vector& vecOrigin, QAngle& angAngles);
	virtual void CalcViewmodelBob(void) {}

	virtual int	WeaponState(void) const { return m_iState; }

	virtual bool Use(void) { return false; }

	// Weapon Info Accessors
	virtual const FileWeaponInfo_t* GetWpnData(void) const;
	virtual const char* GetViewModel(void) const;
	virtual const char* GetWorldModel(void) const;
	virtual const char* GetAnimSuffix(void) const;
	virtual char const* GetName(void) const;
	virtual char const* GetShootSound(int iIndex) const;
	virtual float GetWeight(void) const;
	virtual bool IsPassiveWeapon(bool) const;

#ifdef GAME_DLL

	DECLARE_DATADESC();

	// General
	virtual void FallInit(void);
	virtual void FallThink(void);

	// Spawning
	virtual void Materialize(void);

	// Dropping and Destruction
	virtual void Delete(void);

	// Animation
	virtual void Operator_FrameUpdate(CBaseCombatCharacter* pOperator);
	virtual void Operator_HandleAnimEvent(animevent_t* pEvent, CBaseCombatCharacter* pOperator);
	virtual void HandleAnimEvent(animevent_t* pEvent);

	// Misc
	virtual int	UpdateTransmitState(void);

	virtual void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue);

#else

	virtual model_t* GetShellModel(void) const;

	virtual bool ShouldPredict(void);
	virtual void OnDataChanged(DataUpdateType_t type);

	virtual void BoneMergeFastCullBloat(Vector& localMins, Vector& localMaxs, const Vector& thisEntityMins, const Vector& thisEntityMaxs) const;
	virtual bool OnFireEvent(C_BaseViewModel* pViewModel, const Vector& origin, const QAngle& angles, int event, const char* options) { return false; }

	// Should this object cast shadows?
	virtual ShadowType_t ShadowCastType(void);
	virtual void SetDormant(bool bDormant);
	virtual void OnRestore(void);

	virtual void ViewModelDrawn(CBaseViewModel* pViewModel);

	// Get the position that bullets are seen coming out. Note: the returned values are different
	// for first person and third person.
	virtual bool GetShootPosition(Vector& vOrigin, QAngle& vAngles);

	// Weapon state checking
	virtual bool IsCarriedByLocalPlayer(void);
	virtual bool IsActiveByLocalPlayer(void);

	virtual bool IsBeingCarried(void) const;

	// Is the carrier alive?
	virtual bool IsCarrierAlive(void) const;

	// Returns the aiment render origin + angles
	virtual int	 DrawModel(int iFlags);
	virtual bool ShouldDraw(void);
	virtual bool ShouldDrawPickup(void);
	virtual void HandleInput(void) {}
	virtual void OverrideMouseInput(float* x, float* y) {}
	virtual int	 KeyInput(int down, int keynum, const char* pszCurrentBinding) { return 1; }
	virtual bool AddLookShift(void) { return true; }

	virtual void GetViewmodelBoneControllers(C_BaseViewModel* pViewModel, float controllers[MAXSTUDIOBONECTRLS]) { return; }
	virtual void NotifyShouldTransmit(ShouldTransmitState_t state);

	WEAPON_FILE_INFO_HANDLE	GetWeaponFileInfoHandle() { return m_hWeaponFileInfo; }

	virtual int GetWorldModelIndex(void);
	virtual void GetToolRecordingState(KeyValues* msg);

	// Action Type
	virtual int ActionType(void) const;

#endif

protected:

#ifdef GAME_DLL
	virtual bool DoThrow(void);
	virtual void Throw(const Vector* pVelocity);
	virtual void OnDrop(void);
#endif

protected:
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_nNextThinkTick);

	string_t m_iszName;

	CNetworkVar(int, m_iViewModelIndex);
	CNetworkVar(int, m_iWorldModelIndex);

	typedef CHandle< CBasePlayer > CBasePlayerHandle;
	CNetworkVar(CBasePlayerHandle, m_hOwner);

	CNetworkVar(int, m_iState);

	CNetworkVar(float, m_flHolsterTime);

	bool m_bInReload;

	CNetworkVar(float, m_flNextPrimaryAttack);
	CNetworkVar(float, m_flNextSecondaryAttack);
	CNetworkVar(float, m_flNextTertiaryAttack);
	CNetworkVar(float, m_flTimeWeaponIdle);

#ifdef CLIENT_DLL
	float m_flLastPrimaryAttack;
	float m_flLastSecondaryAttack;
	float m_flLastTertiaryAttack;
#endif

	float m_flNextEmptyAttack;
	float m_fFireDuration;

	float m_flBobTime;
	float m_flLastBobTime;

	float m_flLateralBob;
	float m_flVerticalBob;

private:
	Activity m_Activity;
	int	m_nIdealSequence;
	Activity m_IdealActivity;

	WEAPON_FILE_INFO_HANDLE	m_hWeaponFileInfo;

#ifdef CLIENT_DLL
	bool m_bJustRestored;
	int m_iOldState;
#endif
};

inline CBaseCombatWeapon* ToBaseCombatWeapon(CBaseEntity* pEntity)
{
	return dynamic_cast<CBaseCombatWeapon*>(pEntity);
}

#endif // COMBATWEAPON_SHARED_H