//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef COMBATWEAPON_SHARED_H
#define COMBATWEAPON_SHARED_H
#ifdef _WIN32
#pragma once
#endif

#include "vphysics_interface.h"
#include "predictable_entity.h"
#include "soundflags.h"
#include "baseviewmodel_shared.h"
#include "weapon_parse.h"

#if defined( CLIENT_DLL )
#include "interpolatedvar.h"
#define CBaseCombatWeapon C_BaseCombatWeapon
#endif

class CBasePlayer;
class IPhysicsConstraint;
class CHudTexture;
class Color;
struct SelectedWeapon_t;

// start with a constraint in place (don't drop to floor)
#define	SF_WEAPON_START_CONSTRAINED	(1<<0)	

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

#define WEAPON_INVALID -1

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
class CBaseCombatWeapon : public CBaseAnimating
{
public:
	DECLARE_CLASS(CBaseCombatWeapon, CBaseAnimating);
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

public:
	CBaseCombatWeapon();

	// Weapon Identification
	virtual int GetWeaponID(void) const { return WEAPON_INVALID; }
	int GetWeaponType(void) const;
	virtual int GetWeaponClass(void) const { return WEAPON_INVALID; }

	virtual bool IsExhaustible(void) { return false; }

	// General
	virtual bool IsPredicted(void) const { return true; }

	virtual void Spawn(void);
	virtual void Precache(void);
	void AutoRemove(void);

	int ObjectCaps(void);

#ifdef CLIENT_DLL
	void CreateMove( float flInputSampleTime, CUserCmd *pCmd, const QAngle &vecOldViewAngles ) { }
#endif

	bool IsActiveWeapon( void );

	// Handling of Weapon
	void Equip(CBasePlayer *pOwner);

#ifdef GAME_DLL

	virtual bool CanDrop( void );

	bool Drop( bool bNoSwitch, const Vector *pVelocity );

#endif

	int	UpdateClientData(CBasePlayer *pPlayer);

	void SetWeaponVisible(bool bVisible);
	bool IsWeaponVisible(void);

	// Ammo Management
	virtual bool HasAmmo( void ) const;
	virtual int GetAmmoCount( void ) const;

	virtual void GiveClip( int iCount ) { }
	virtual void GiveAmmo( int iCount ) { }

	// Viewmodel Management
	void SetViewModel(void);
	CBaseViewModel *GetOwnerViewModel(void);
	virtual bool ShouldDrawViewModel(void) const { return true; }
	bool SendWeaponAnim(int iActivity);
	void SendViewModelAnim(int nSequence);
	float GetViewModelSequenceDuration(void);
	bool IsViewModelSequenceFinished(void);

	// Deployment
	void DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt);
	virtual bool CanDeploy(void);
	virtual void Deploy(void);
	virtual void HandleDeploy(void) { }

	// Holstering
	virtual bool CanHolster( void );
	virtual bool Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	float GetHolsterTime( void ) const { return m_flHolsterTime; }

	// Thinking
	virtual void ItemPreFrame(void);					// called each frame by the player PreThink
	virtual void ItemPostFrame(void);					// called each frame by the player PostThink
	virtual void ItemBusyFrame(void);					// called each frame by the player PostThink, if the player's not ready to attack yet
	virtual void WeaponIdle(void);						// called when no buttons pressed

	// Idling
	void SetWeaponIdleTime(float flTime);
	float GetWeaponIdleTime(void);

	// Attacking
	bool TestWaterAttack( void ) const;
	virtual bool AllowWaterAttack( void ) const;

	bool HandleAttack( void );
	virtual bool CanAttack( void );

	virtual bool IsEmptyAttack( void );
	virtual bool CanEmptyAttack( void );
	virtual void EmptyAttack( void );

	virtual bool HasPrimaryAttack( void );
	virtual bool HasSecondaryAttack( void );
	virtual bool HasTertiaryAttack( void );

	bool HandlePrimaryAttack( void );
	bool HandleSecondaryAttack( void );
	bool HandleTertiaryAttack( void );

	virtual bool CanPrimaryAttack( void );
	virtual bool CanSecondaryAttack( void );
	virtual bool CanTertiaryAttack( void );

#ifdef CLIENT_DLL

	virtual bool StoreLastPrimaryAttack( void );
	virtual bool StoreLastSecondaryAttack( void );
	virtual bool StoreLastTertiaryAttack( void );

#endif

	virtual void PrimaryAttack( void ) { }
	virtual void SecondaryAttack( void ) { }
	virtual void TertiaryAttack( void ) { }

	// Empty Handling
	bool UseEmptyAnimations( void ) const;
	virtual bool EnableEmptyAnimations( void ) const;
	virtual bool ShouldEmptyAnimate( void ) const;

	// Animations
	virtual Activity GetPrimaryAttackActivity(void) const;

	virtual Activity GetDrawActivity(void);
	virtual Activity GetHolsterActivity(void);
	virtual Activity GetWeaponIdleActivity(void) const;

	virtual void DoAnimationEvent(int iEvent) { }

	void SetActivity(Activity act, float flDuration);
	inline void	SetActivity(Activity eActivity) { m_Activity = eActivity; }
	inline Activity GetActivity(void) { return m_Activity; }

	acttable_t *ActivityList(void) { return NULL; }
	int	ActivityListCount(void) { return 0; }

	Activity GetIdealActivity(void) const { return m_IdealActivity; }
	int GetIdealSequence(void) const { return m_nIdealSequence; }

	bool SetIdealActivity(Activity ideal);
	void MaintainIdealActivity(void);

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
	bool IsReloading(void) const { return m_bInReload; }

	// Sound
	void WeaponSound(WeaponSound_t sound_type, float flSoundTime = 0.0f);
	void StopWeaponSound(WeaponSound_t sound_type);

	// Misc
	CBasePlayer	*GetOwner(void) const;
	
	void SetOwner(CBasePlayer *pOwner);

	void AddViewmodelBob(CBaseViewModel *viewmodel, Vector &vecOrigin, QAngle &angAngles);
	virtual void CalcViewmodelBob( void ) { }

	int	WeaponState(void) const { return m_iState; }

	virtual bool Use( void ) { return false; }

	// Weapon Info Accessors
	const FileWeaponInfo_t *GetWpnData(void) const;
	const char *GetViewModel(void) const;
	const char *GetWorldModel(void) const;
	const char *GetAnimSuffix(void) const;
	char const *GetName(void) const;
	char const *GetShootSound(int iIndex) const;
	float GetWeight(void) const;
	bool IsPassiveWeapon(bool) const;

#ifdef GAME_DLL

	DECLARE_DATADESC();

	// General
	void FallInit(void);
	void FallThink(void);

	// Spawning
	void Materialize(void);

	// Dropping and Destruction
	virtual void Delete( void );

	// Animation
	void Operator_FrameUpdate(CBasePlayer *pOperator);
	void Operator_HandleAnimEvent(animevent_t *pEvent, CBasePlayer *pOperator);
	void HandleAnimEvent(animevent_t *pEvent);

	// Misc
	int	UpdateTransmitState(void);

	void Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float flValue );

#else

	virtual model_t	*GetShellModel(void) const;

	bool ShouldPredict(void);
	void OnDataChanged(DataUpdateType_t type);

	void BoneMergeFastCullBloat(Vector &localMins, Vector &localMaxs, const Vector &thisEntityMins, const Vector &thisEntityMaxs) const;
	virtual bool OnFireEvent(C_BaseViewModel *pViewModel, const Vector& origin, const QAngle& angles, int event, const char *options ) { return false; }

	// Should this object cast shadows?
	virtual ShadowType_t ShadowCastType(void);
	virtual void SetDormant(bool bDormant);
	virtual void OnRestore(void);

	virtual void ViewModelDrawn(CBaseViewModel *pViewModel);

	// Get the position that bullets are seen coming out. Note: the returned values are different
	// for first person and third person.
	bool GetShootPosition(Vector &vOrigin, QAngle &vAngles);
	
	// Weapon state checking
	virtual bool IsCarriedByLocalPlayer(void);
	virtual bool IsActiveByLocalPlayer(void);

	bool IsBeingCarried(void) const;

	// Is the carrier alive?
	bool IsCarrierAlive(void) const;

	// Returns the aiment render origin + angles
	virtual int	 DrawModel( int iFlags);
	virtual bool ShouldDraw(void);
	virtual bool ShouldDrawPickup(void);
	virtual void HandleInput(void) { }
	virtual void OverrideMouseInput(float *x, float *y) { }
	virtual int	 KeyInput(int down, int keynum, const char *pszCurrentBinding) { return 1; }
	virtual bool AddLookShift(void) { return true; }

	virtual void GetViewmodelBoneControllers(C_BaseViewModel *pViewModel, float controllers[MAXSTUDIOBONECTRLS]) { return; }

	virtual void NotifyShouldTransmit(ShouldTransmitState_t state);
	WEAPON_FILE_INFO_HANDLE	GetWeaponFileInfoHandle() { return m_hWeaponFileInfo; }

	virtual int GetWorldModelIndex(void);

	virtual void GetToolRecordingState( KeyValues *msg );

	// Action Type
	int ActionType( void ) const;

#endif

protected:

#ifdef GAME_DLL

	virtual bool DoThrow( void );
	void Throw( const Vector *pVelocity );

	virtual void OnDrop( void );

#endif

protected:
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_nNextThinkTick );

	string_t m_iszName;

	CNetworkVar( int, m_iViewModelIndex );
	CNetworkVar( int, m_iWorldModelIndex );

	typedef CHandle< CBasePlayer > CBasePlayerHandle;
	CNetworkVar( CBasePlayerHandle, m_hOwner );

	CNetworkVar( int, m_iState );

	CNetworkVar( float, m_flHolsterTime );

	bool m_bInReload;

	CNetworkVar( float, m_flNextPrimaryAttack );
	CNetworkVar( float, m_flNextSecondaryAttack );
	CNetworkVar( float, m_flNextTertiaryAttack );
	CNetworkVar( float, m_flTimeWeaponIdle );

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

//-----------------------------------------------------------------------------
// Purpose: casting from entity
//-----------------------------------------------------------------------------
inline CBaseCombatWeapon *ToBaseCombatWeapon( CBaseEntity *pEntity )
{
	return dynamic_cast< CBaseCombatWeapon* >( pEntity );
}

#endif // COMBATWEAPON_SHARED_H
