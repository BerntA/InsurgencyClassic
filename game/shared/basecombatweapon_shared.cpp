//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Base Weapon Handling - Handles FX, Bash, Special stuff...
//
//========================================================================================//

#include "cbase.h"
#include "in_buttons.h"
#include "engine/IEngineSound.h"
#include "ammodef.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "datacache/imdlcache.h"
#include "activitylist.h"
#include "weapon_defines.h"
#include "weapon_parse.h"

#ifdef CLIENT_DLL
#include "action_helper_types.h"
#else
#include "soundent.h"
#include "eventqueue.h"
#include "fmtstr.h"
#include "particle_parse.h"
#include "te_effect_dispatch.h"
#include "ilagcompensationmanager.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef CLIENT_DLL

BEGIN_PREDICTION_DATA(CBaseCombatWeapon)

DEFINE_PRED_FIELD(m_nNextThinkTick, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),

// networked
DEFINE_PRED_FIELD(m_hOwner, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iState, FIELD_INTEGER, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_flHolsterTime, FIELD_EHANDLE, FTYPEDESC_INSENDTABLE),
DEFINE_PRED_FIELD(m_iViewModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
DEFINE_PRED_FIELD(m_iWorldModelIndex, FIELD_INTEGER, FTYPEDESC_INSENDTABLE | FTYPEDESC_MODELINDEX),
DEFINE_PRED_FIELD_TOL(m_flNextPrimaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
DEFINE_PRED_FIELD_TOL(m_flNextSecondaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
DEFINE_PRED_FIELD_TOL(m_flNextTertiaryAttack, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),
DEFINE_PRED_FIELD_TOL(m_flTimeWeaponIdle, FIELD_FLOAT, FTYPEDESC_INSENDTABLE, TD_MSECTOLERANCE),

// not networked
DEFINE_FIELD(m_bInReload, FIELD_BOOLEAN),
DEFINE_FIELD(m_flNextEmptyAttack, FIELD_FLOAT),
DEFINE_FIELD(m_Activity, FIELD_INTEGER),
DEFINE_FIELD(m_fFireDuration, FIELD_FLOAT),
DEFINE_FIELD(m_iszName, FIELD_INTEGER),

END_PREDICTION_DATA()

#endif

// special hack since we're aliasing the name C_BaseCombatWeapon with a macro on the client
IMPLEMENT_NETWORKCLASS_ALIASED(BaseCombatWeapon, DT_BaseCombatWeapon)

//=========================================================
//=========================================================
#ifdef GAME_DLL

BEGIN_DATADESC(CBaseCombatWeapon)

DEFINE_FUNCTION(FallThink),
DEFINE_FUNCTION(Materialize),

END_DATADESC()

#endif

//=========================================================
//=========================================================
#ifdef GAME_DLL

void* SendProxy_SendLocalWeaponDataTable(const SendProp* pProp, const void* pStruct, const void* pVarData, CSendProxyRecipients* pRecipients, int objectID)
{
	// Get the weapon entity
	CBaseCombatWeapon* pWeapon = (CBaseCombatWeapon*)pVarData;

	if (pWeapon)
	{
		// only send this chunk of data to the player carrying this weapon
		CBasePlayer* pPlayer = pWeapon->GetOwner();

		if (pPlayer)
		{
			pRecipients->SetOnly(pPlayer->GetClientIndex());
			return (void*)pVarData;
		}
	}

	return NULL;
}

REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_SendLocalWeaponDataTable);

#endif

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE_NOBASE(CBaseCombatWeapon, DT_LocalWeaponData)

#ifdef GAME_DLL

SendPropTime(SENDINFO(m_flHolsterTime)),
SendPropInt(SENDINFO(m_nNextThinkTick)),
SendPropTime(SENDINFO(m_flNextPrimaryAttack)),
SendPropTime(SENDINFO(m_flNextSecondaryAttack)),
SendPropTime(SENDINFO(m_flNextTertiaryAttack)),
SendPropTime(SENDINFO(m_flTimeWeaponIdle)),

#else

RecvPropTime(RECVINFO(m_flHolsterTime)),
RecvPropInt(RECVINFO(m_nNextThinkTick)),
RecvPropTime(RECVINFO(m_flNextPrimaryAttack)),
RecvPropTime(RECVINFO(m_flNextSecondaryAttack)),
RecvPropTime(RECVINFO(m_flNextTertiaryAttack)),
RecvPropTime(RECVINFO(m_flTimeWeaponIdle)),

#endif

END_NETWORK_TABLE()

//=========================================================
//=========================================================
BEGIN_NETWORK_TABLE(CBaseCombatWeapon, DT_BaseCombatWeapon)

#ifdef GAME_DLL

SendPropDataTable("LocalWeaponData", 0, &REFERENCE_SEND_TABLE(DT_LocalWeaponData), SendProxy_SendLocalWeaponDataTable),
SendPropModelIndex(SENDINFO(m_iViewModelIndex)),
SendPropModelIndex(SENDINFO(m_iWorldModelIndex)),
SendPropEHandle(SENDINFO(m_hOwner)),
SendPropInt(SENDINFO(m_iState), 2, SPROP_UNSIGNED),

#else

RecvPropDataTable("LocalWeaponData", 0, 0, &REFERENCE_RECV_TABLE(DT_LocalWeaponData)),
RecvPropInt(RECVINFO(m_iViewModelIndex)),
RecvPropInt(RECVINFO(m_iWorldModelIndex)),
RecvPropEHandle(RECVINFO(m_hOwner)),
RecvPropInt(RECVINFO(m_iState)),

#endif

END_NETWORK_TABLE()

//=========================================================
//=========================================================
CBaseCombatWeapon::CBaseCombatWeapon()
{
	SetPredictionEligible(true);
	AddSolidFlags(FSOLID_TRIGGER);

	m_bInReload = false;

#ifdef CLIENT_DLL

	m_iState = m_iOldState = WEAPON_NOT_CARRIED;
	m_flLastPrimaryAttack = m_flLastSecondaryAttack = m_flLastTertiaryAttack = 0.0f;

#endif

	m_hWeaponFileInfo = GetInvalidWeaponInfoHandle();
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::Spawn(void)
{
	Precache();

	SetSolid(SOLID_BBOX);

	// weapons won't show up in trace calls if they are being carried...
	RemoveEFlags(EFL_USE_PARTITION_WHEN_NOT_SOLID);

	m_iState = WEAPON_NOT_CARRIED;

	SetModel(GetWorldModel());

#ifdef GAME_DLL

	FallInit();
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	m_takedamage = DAMAGE_EVENTS_ONLY;

	SetBlocksLOS(false);

#endif

	m_flNextEmptyAttack = 0.0f;

	// bloat the box for player pickup
	CollisionProp()->UseTriggerBounds(true, 36);

	// use more efficient bbox culling on the client. Otherwise, it'll setup bones for most
	// characters even when they're not in the frustum.
	AddEffects(EF_BONEMERGE_FASTCULL);
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::Precache(void)
{
#ifdef CLIENT_DLL

	Assert(Q_strlen(GetClassname()) > 0);

#endif

	// add this weapon to the weapon registry, and get our index into it
	// Get weapon data from script file
	if (ReadWeaponDataFromFileForSlot(filesystem, GetClassname(), &m_hWeaponFileInfo, GetEncryptionKey()))
	{
		// precache models
		m_iViewModelIndex = CBaseEntity::PrecacheModel(GetViewModel());
		m_iWorldModelIndex = CBaseEntity::PrecacheModel(GetWorldModel());

		// precache sounds, too
		for (int i = 0; i < NUM_SHOOT_SOUND_TYPES; ++i)
		{
			const char* pszShootSound = GetShootSound(i);

			if (pszShootSound && pszShootSound[0])
				CBaseEntity::PrecacheScriptSound(pszShootSound);
		}
	}
	else
	{
		AssertMsg(false, "CBaseCombatWeapon::Precache, Missing Weapon Script");

		Msg("ERROR: Weapon Data File for %s is Missing\n", GetClassname());
	}
}

//=========================================================
//=========================================================
int	CBaseCombatWeapon::ObjectCaps(void)
{
	int iCaps = BaseClass::ObjectCaps();

	if (!IsFollowingEntity())
		iCaps |= FCAP_IMPULSE_USE;

	return iCaps;
}

//=========================================================
//=========================================================
int CBaseCombatWeapon::GetWeaponType(void) const
{
	return WeaponIDToType(GetWeaponID());
}

//=========================================================
//=========================================================
const FileWeaponInfo_t* CBaseCombatWeapon::GetWpnData(void) const
{
	return GetFileWeaponInfoFromHandle(m_hWeaponFileInfo);
}

//=========================================================
//=========================================================
const char* CBaseCombatWeapon::GetViewModel(void) const
{
	return GET_WEAPON_DATA(szViewModel);
}

//=========================================================
//=========================================================
const char* CBaseCombatWeapon::GetWorldModel(void) const
{
	return GET_WEAPON_DATA(szWorldModel);
}

//=========================================================
//=========================================================
const char* CBaseCombatWeapon::GetAnimSuffix(void) const
{
	return GET_WEAPON_DATA(szAnimationSuffix);
}

//=========================================================
//=========================================================
const char* CBaseCombatWeapon::GetName(void) const
{
	return WeaponIDToName(GetWeaponID());
}

//=========================================================
//=========================================================
float CBaseCombatWeapon::GetWeight(void) const
{
	return GET_WEAPON_DATA(flWeight);
}

#ifdef CLIENT_DLL

//=========================================================
//=========================================================
model_t* CBaseCombatWeapon::GetShellModel(void) const
{
	return NULL;
}

#endif

//=========================================================
//=========================================================
const char* CBaseCombatWeapon::GetShootSound(int iIndex) const
{
	return GET_WEAPON_DATA(aShootSounds[iIndex]);
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::IsActiveWeapon(void)
{
	CBasePlayer* pPlayer = GetOwner();
	return (pPlayer && pPlayer->GetActiveWeapon() == this);
}

//=========================================================
//=========================================================
CBasePlayer* CBaseCombatWeapon::GetOwner(void) const
{
	return m_hOwner.Get();
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::SetOwner(CBasePlayer* pOwner)
{
	m_hOwner = pOwner;

#ifndef CLIENT_DLL

	DispatchUpdateTransmitState();

#else

	UpdateVisibility();

#endif
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HasAmmo(void) const
{
	return false;
}

//=========================================================
//=========================================================
int CBaseCombatWeapon::GetAmmoCount(void) const
{
	return 0;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::SetWeaponIdleTime(float flTime)
{
	m_flTimeWeaponIdle = flTime;
}

//=========================================================
//=========================================================
float CBaseCombatWeapon::GetWeaponIdleTime(void)
{
	return m_flTimeWeaponIdle;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::AutoRemove(void)
{
#ifdef GAME_DLL
	SetThink(&CBaseEntity::SUB_Remove);
	SetNextThink(gpGlobals->curtime + 30.0f);
#endif
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::Equip(CBasePlayer* pOwner)
{
	// attach the weapon to an owner
	SetAbsVelocity(vec3_origin);
	RemoveSolidFlags(FSOLID_TRIGGER);
	FollowEntity(pOwner);
	SetOwner(pOwner);
	SetOwnerEntity(pOwner);

	// reset
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime;

	SetTouch(NULL);
	SetThink(NULL);

#ifdef GAME_DLL

	VPhysicsDestroyObject();

#endif

	// set viewmodel
	SetModel(GetViewModel());
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::SetActivity(Activity act, float flDuration)
{
	// Adrian: Oh man...
#if !defined( CLIENT_DLL )

	SetModel(GetWorldModel());

#endif

	int iSequence = SelectWeightedSequence(act);

	// FORCE IDLE on sequences we don't have (which should be many)
	if (iSequence == ACTIVITY_NOT_AVAILABLE)
	{
		iSequence = SelectWeightedSequence(GetWeaponIdleActivity());
	}

	//Adrian: Oh man again...
#if !defined( CLIENT_DLL )

	SetModel(GetViewModel());

#endif

	if (iSequence != ACTIVITY_NOT_AVAILABLE)
	{
		SetSequence(iSequence);
		SetActivity(act);
		SetCycle(0);
		ResetSequenceInfo();

		if (flDuration > 0)
		{
			// FIXME: does this even make sense in non-shoot animations?
			m_flPlaybackRate = SequenceDuration(iSequence) / flDuration;
			m_flPlaybackRate = min(m_flPlaybackRate, 12.0);  // FIXME; magic number!, network encoding range
		}
		else
		{
			m_flPlaybackRate = 1.0;
		}
	}
}

//=========================================================
//=========================================================
int CBaseCombatWeapon::UpdateClientData(CBasePlayer* pPlayer)
{
	// NOTE: no shadows for when the weapon is on the ground
	m_iState = (pPlayer->GetActiveWeapon() == this) ? WEAPON_IS_ACTIVE : WEAPON_IS_CARRIED_BY_PLAYER;

	return 1;
}

//=========================================================
//=========================================================
CBaseViewModel* CBaseCombatWeapon::GetOwnerViewModel(void)
{
	CBasePlayer* pOwner = GetOwner();

	if (!pOwner)
		return NULL;

	return pOwner->GetViewModel();
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::SendViewModelAnim(int nSequence)
{
#ifdef CLIENT_DLL

	if (!IsPredicted())
		return;

#endif

	if (nSequence < 0)
		return;

	CBaseViewModel* pVM = GetOwnerViewModel();

	if (!pVM)
		return;

	SetViewModel();
	pVM->SendViewModelMatchingSequence(nSequence);
}

//=========================================================
//=========================================================
float CBaseCombatWeapon::GetViewModelSequenceDuration(void)
{
	CBaseViewModel* pVM = GetOwnerViewModel();

	if (!pVM)
		return 0.0f;

	SetViewModel();
	return pVM->SequenceDuration();
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::IsViewModelSequenceFinished(void)
{
	// these are not valid activities and always complete immediately
	if (GetActivity() == ACT_RESET || GetActivity() == ACT_INVALID)
		return true;

	CBaseViewModel* pVM = GetOwnerViewModel();

	if (!pVM)
		return true;

	return pVM->IsSequenceFinished();
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::SetViewModel(void)
{
	CBaseViewModel* pVM = GetOwnerViewModel();

	if (!pVM)
		return;

	pVM->SetWeaponModel(GetViewModel(), this);
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::SendWeaponAnim(int iActivity)
{
	// for now, just set the ideal activity and be done with it
	return SetIdealActivity((Activity)iActivity);
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::SetWeaponVisible(bool bVisible)
{
	CBaseViewModel* pVM = GetOwnerViewModel();

	if (bVisible)
	{
		RemoveEffects(EF_NODRAW);

		if (pVM)
			pVM->RemoveEffects(EF_NODRAW);
	}
	else
	{
		AddEffects(EF_NODRAW);

		if (pVM)
			pVM->AddEffects(EF_NODRAW);
	}
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::IsWeaponVisible(void)
{
	CBaseViewModel* pVM = GetOwnerViewModel();

	if (!pVM)
		return false;

	return (!pVM->IsEffectActive(EF_NODRAW));
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::DefaultDeploy(char* szViewModel, char* szWeaponModel, int iActivity, char* szAnimExt)
{
	CBasePlayer* pOwner = GetOwner();
	if (!pOwner || !pOwner->IsAlive())
		return;

	SetViewModel();
	SendWeaponAnim(iActivity);

	pOwner->SetNextAttack(gpGlobals->curtime + SequenceDuration());

	m_flHolsterTime = 0.0f;
	m_flTimeWeaponIdle = m_flNextTertiaryAttack = m_flNextSecondaryAttack = m_flNextPrimaryAttack = gpGlobals->curtime;

	SetWeaponVisible(true);
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanDeploy(void)
{
	CBasePlayer* pOwner = GetOwner();
	return (pOwner && pOwner->IsAlive() && !TestWaterAttack());
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::Deploy(void)
{
	MDLCACHE_CRITICAL_SECTION();
	DefaultDeploy((char*)GetViewModel(), (char*)GetWorldModel(), GetDrawActivity(), (char*)GetAnimSuffix());

	HandleDeploy();
}

//=========================================================
//=========================================================
Activity CBaseCombatWeapon::GetDrawActivity(void)
{
	if (UseEmptyAnimations())
		return ACT_VM_DRAW_EMPTY;

	return ACT_VM_DRAW;
}

//=========================================================
//=========================================================
Activity CBaseCombatWeapon::GetHolsterActivity(void)
{
	if (UseEmptyAnimations())
		return ACT_VM_HOLSTER_EMPTY;

	return ACT_VM_HOLSTER;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanHolster(void)
{
	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::Holster(CBaseCombatWeapon* pSwitchingTo)
{
	CBasePlayer* pPlayer = GetOwner();

	// not reloading
	m_bInReload = false;

	// kill any think functions
	SetThink(NULL);

	// send holster animation
	Activity Holster = GetHolsterActivity();
	SendWeaponAnim(Holster);

	// cancel any reload
	AbortReload();

	// some weapon's don't have holster anims yet, so detect that
	float flHolsterEnd = gpGlobals->curtime;
	bool bValidHolster = (GetActivity() == Holster);

	if (bValidHolster)
	{
		MDLCACHE_CRITICAL_SECTION();
		flHolsterEnd += SequenceDuration();
	}
	else
	{
		SetWeaponVisible(false);
	}

	if (pPlayer)
		pPlayer->SetNextAttack(flHolsterEnd);

	// set the holster time
	if (flHolsterEnd != 0.0f)
		m_flHolsterTime = flHolsterEnd;

	return true;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::ItemPreFrame(void)
{
	MaintainIdealActivity();
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::ItemPostFrame(void)
{
	CBasePlayer* pOwner = GetOwner();

	if (!pOwner)
		return;

	// when holster time is over, set the weapon to invisible
	if (m_flHolsterTime != 0.0f && gpGlobals->curtime >= m_flHolsterTime)
	{
		SetWeaponVisible(false);
		m_flHolsterTime = 0.0f;

		pOwner->SetNextAttack(FLT_MAX);

		return;
	}

	// check reload
	CheckReload();

	// see how long we've been firing for
	m_fFireDuration = (pOwner->m_nButtons & IN_ATTACK) ? (m_fFireDuration + gpGlobals->frametime) : 0.0f;

	// check weapon action buttons
	bool bFired = false;

	if ((pOwner->m_nButtons & IN_ATTACK) && (gpGlobals->curtime >= m_flNextPrimaryAttack))
	{
		// NOTE: There is a bug with this code with regards to the way machine guns catch the leading edge trigger
		// on the player hitting the attack key.  It relies on the gun catching that case in the same frame.
		// However, because the player can also be doing a secondary attack, the edge trigger may be missed.
		// We really need to hold onto the edge trigger and only clear the condition when the gun has fired its
		// first shot.  Right now that's too much of an architecture change -- jdw

		if ((pOwner->m_afButtonPressed & IN_ATTACK) || (pOwner->m_afButtonReleased & IN_SPECIAL1) || (pOwner->m_afButtonReleased & IN_SPECIAL2))
			m_flNextPrimaryAttack = gpGlobals->curtime;

		if (HasPrimaryAttack())
			bFired = HandlePrimaryAttack();
		else
			bFired = CanPrimaryAttack();

		if (bFired)
			PrimaryAttack();

#ifdef CLIENT_DLL

		if (StoreLastPrimaryAttack())
			m_flLastPrimaryAttack = m_flNextPrimaryAttack;

#endif
	}

	if (!bFired && (pOwner->m_nButtons & IN_SPECIAL1) && (m_flNextSecondaryAttack <= gpGlobals->curtime))
	{
		bFired = true;

		if (HasSecondaryAttack())
			bFired = HandleSecondaryAttack();
		else
			bFired = CanSecondaryAttack();

		if (bFired)
			SecondaryAttack();

#ifdef CLIENT_DLL

		if (StoreLastSecondaryAttack())
			m_flLastSecondaryAttack = m_flNextSecondaryAttack;

#endif
	}

	if (!bFired && (pOwner->m_nButtons & IN_SPECIAL2) && (m_flNextTertiaryAttack <= gpGlobals->curtime))
	{
		bFired = true;

		if (HasTertiaryAttack())
			bFired = HandleTertiaryAttack();
		else
			bFired = CanTertiaryAttack();

		if (bFired)
			TertiaryAttack();

#ifdef CLIENT_DLL

		if (StoreLastTertiaryAttack())
			m_flLastTertiaryAttack = m_flNextTertiaryAttack;

#endif
	}

	// pressed the reload key
	if (pOwner->m_nButtons & IN_RELOAD && !m_bInReload && CanReload())
	{
		Reload();
		m_fFireDuration = 0.0f;
	}

	// idle
	WeaponIdle();
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::UseEmptyAnimations(void) const
{
	return (EnableEmptyAnimations() && ShouldEmptyAnimate());
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::EnableEmptyAnimations(void) const
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::ShouldEmptyAnimate(void) const
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::TestWaterAttack(void) const
{
	if (AllowWaterAttack())
		return false;

	CBasePlayer* pPlayer = GetOwner();
	return (pPlayer && pPlayer->GetWaterLevel() == WL_Eyes);
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::AllowWaterAttack(void) const
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HandleAttack(void)
{
	if (IsEmptyAttack())
	{
		if (CanEmptyAttack())
		{
			EmptyAttack();
			m_flNextPrimaryAttack = m_flNextEmptyAttack;
		}

		return false;
	}

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanAttack(void)
{
	CBasePlayer* pOwner = GetOwner();

	if (!pOwner)
		return false;

	if (TestWaterAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::IsEmptyAttack(void)
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanEmptyAttack(void)
{
	return (m_flNextEmptyAttack < gpGlobals->curtime);
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::EmptyAttack(void)
{
	m_flNextEmptyAttack = gpGlobals->curtime + 0.5f;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HasPrimaryAttack(void)
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HasSecondaryAttack(void)
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HasTertiaryAttack(void)
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HandlePrimaryAttack(void)
{
#ifdef CLIENT_DLL

	if (m_flLastPrimaryAttack > gpGlobals->curtime)
		return false;

#endif

	if (!CanPrimaryAttack())
		return false;

	if (!HandleAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HandleSecondaryAttack(void)
{
#ifdef CLIENT_DLL

	if (m_flLastSecondaryAttack > gpGlobals->curtime)
		return false;

#endif

	if (!CanSecondaryAttack())
		return false;

	if (!HandleAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::HandleTertiaryAttack(void)
{
#ifdef CLIENT_DLL

	if (m_flLastTertiaryAttack > gpGlobals->curtime)
		return false;

#endif

	if (!CanTertiaryAttack())
		return false;

	if (!HandleAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanPrimaryAttack(void)
{
	if (!CanAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanSecondaryAttack(void)
{
	if (!CanAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanTertiaryAttack(void)
{
	if (!CanAttack())
		return false;

	return true;
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

//=========================================================
//=========================================================
bool CBaseCombatWeapon::StoreLastPrimaryAttack(void)
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::StoreLastSecondaryAttack(void)
{
	return false;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::StoreLastTertiaryAttack(void)
{
	return false;
}

#endif

//=========================================================
//=========================================================
void CBaseCombatWeapon::ItemBusyFrame(void)
{
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::WeaponSound(WeaponSound_t sound_type, float flSoundTime /* = 0.0f */)
{
#ifdef CLIENT_DLL

	const char* pszShootSound = GET_WEAPON_DATA(aShootSounds[sound_type]);

#else

	const char* pszShootSound = GetShootSound(sound_type);

#endif

	if (!pszShootSound || *pszShootSound == '\0')
		return;

#ifdef CLIENT_DLL

	CBroadcastRecipientFilter filter;

	if (!te->CanPredict())
		return;

	CBaseEntity::EmitSound(filter, GetOwner()->entindex(), pszShootSound, &GetOwner()->GetAbsOrigin());

#else

	CSoundParameters params;

	if (!GetParametersForSound(pszShootSound, params, NULL))
		return;

	CBasePlayer* pOwner = GetOwner();

	if (params.play_to_owner_only)
	{
		// am I only to play to my owner?
		if (pOwner)
		{
			CSingleUserRecipientFilter filter(GetOwner());

			if (IsPredicted())
				filter.UsePredictionRules();

			EmitSound(filter, pOwner->entindex(), pszShootSound, NULL, flSoundTime);
		}
	}
	else
	{
		// play weapon sound from the owner
		if (pOwner)
		{
			CPASAttenuationFilter filter(pOwner, params.soundlevel);

			if (IsPredicted())
				filter.UsePredictionRules();

			EmitSound(filter, pOwner->entindex(), pszShootSound, NULL, flSoundTime);

#ifdef GAME_DLL

			if (sound_type == EMPTY)
				CSoundEnt::InsertSound(SOUND_COMBAT, pOwner->GetAbsOrigin(), SOUNDENT_VOLUME_EMPTY, 0.2, pOwner);

#endif
		}
		else
		{
			// if no owner play from the weapon (this is used for thrown items)
			CPASAttenuationFilter filter(this, params.soundlevel);

			if (IsPredicted())
				filter.UsePredictionRules();

			EmitSound(filter, entindex(), pszShootSound, NULL, flSoundTime);
		}
	}
#endif
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::StopWeaponSound(WeaponSound_t sound_type)
{
	// If we have some sounds from the weapon classname.txt file, play a random one of them
	const char* pszShootSound = GetShootSound(sound_type);

	if (!pszShootSound || *pszShootSound == '\0')
		return;

	CSoundParameters params;

	if (!GetParametersForSound(pszShootSound, params, NULL))
		return;

	CBasePlayer* pOwner = GetOwner();

	// am I only to play to my owner?
	if (params.play_to_owner_only)
	{
		if (pOwner)
			StopSound(pOwner->entindex(), pszShootSound);
	}
	else
	{
		// play weapon sound from the owner, if no owner play from 
		// the weapon (this is used for thrown items)
		if (pOwner)
			StopSound(pOwner->entindex(), pszShootSound);
		else
			StopSound(entindex(), pszShootSound);
	}
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::WeaponIdle(void)
{
	CBasePlayer* pOwner = GetOwner();

	if (pOwner && gpGlobals->curtime >= m_flTimeWeaponIdle)
		SendWeaponAnim(GetWeaponIdleActivity());
}

//=========================================================
//=========================================================
Activity CBaseCombatWeapon::GetWeaponIdleActivity(void) const
{
	return ACT_VM_IDLE;
}

//=========================================================
//=========================================================
Activity CBaseCombatWeapon::GetPrimaryAttackActivity(void) const
{
	return ACT_VM_PRIMARYATTACK;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::CanReload(void)
{
	return false;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::CheckReload(void)
{
	if (m_bInReload && gpGlobals->curtime >= m_flNextPrimaryAttack)
	{
		FinishReload();
		m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = gpGlobals->curtime;
	}
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::FinishReload(void)
{
	// we're not reloading anymore
	m_bInReload = false;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::AbortReload(void)
{
	if (!m_bInReload)
		return;

#ifdef CLIENT_DLL

	StopWeaponSound(RELOAD);

#endif

	m_bInReload = false;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::Reload(void)
{
	// ensure the owner is valid and a player
	CBasePlayer* pPlayer = GetOwner();

	if (!pPlayer)
		return;

	// setup the reload
	if (IsEmptyReload())
	{
		HandleEmptyReload();
		return;
	}

	// perform reload
	PerformReload();

	// started reload
	StartReload();
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::StartReload(void)
{
	m_bInReload = true;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::PerformReload(void)
{
	// ensure the owner is valid and a player
	CBasePlayer* pPlayer = GetOwner();

	if (!pPlayer)
		return;

#ifdef CLIENT_DLL

	// play the sound
	WeaponSound(RELOAD);

#endif

	// play the animation
	SendWeaponAnim(GetReloadActivity());

	// don't let anything else happen in the mean time
	float flReloadTime = gpGlobals->curtime + SequenceDuration();

	// HACKHACK: this is because INS_VM_IIDLE uses a transition
	// and SequenceDuration will just return the length of
	// the blending animation
	if (m_nIdealSequence != GetSequence())
		flReloadTime += SequenceDuration(m_nIdealSequence);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = m_flNextTertiaryAttack = m_flTimeWeaponIdle = flReloadTime;
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::IsEmptyReload(void)
{
	return false;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::HandleEmptyReload(void)
{
}

//=========================================================
//=========================================================
Activity CBaseCombatWeapon::GetReloadActivity(void) const
{
	if (UseEmptyAnimations())
		return GetEmptyReloadActivity();

	return ACT_VM_RELOAD;
}

//=========================================================
//=========================================================
Activity CBaseCombatWeapon::GetEmptyReloadActivity(void) const
{
	return ACT_VM_RELOAD_EMPTY;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::MaintainIdealActivity(void)
{
	// must be transitioning
	if (GetActivity() != ACT_TRANSITION)
		return;

	// must not be at our ideal already 
	if ((GetActivity() == m_IdealActivity) && (GetSequence() == m_nIdealSequence))
		return;

	// mMust be finished with the current animation
	if (!IsViewModelSequenceFinished())
		return;

	// move to the next animation towards our ideal
	SendWeaponAnim(m_IdealActivity);
}

//=========================================================
//=========================================================
bool CBaseCombatWeapon::SetIdealActivity(Activity ideal)
{
	MDLCACHE_CRITICAL_SECTION();
	int	idealSequence = SelectWeightedSequence(ideal);

	if (idealSequence == -1)
		return false;

	// take the new activity
	m_IdealActivity = ideal;
	m_nIdealSequence = idealSequence;

	// find the next sequence in the potential chain of sequences leading to our ideal one
	int nextSequence = FindTransitionSequence(GetSequence(), m_nIdealSequence, NULL);

	// don't use transitions when we're deploying
	if (nextSequence != m_nIdealSequence && IsWeaponVisible() && ideal != GetHolsterActivity() && ideal != GetDrawActivity())
	{
		// set our activity to the next transitional animation
		SetActivity(ACT_TRANSITION);
		SetSequence(nextSequence);
		SendViewModelAnim(nextSequence);
	}
	else
	{
		// set our activity to the ideal
		SetActivity(m_IdealActivity);
		SetSequence(m_nIdealSequence);
		SendViewModelAnim(m_nIdealSequence);
	}

	// set the next time the weapon will idle
	float flNextIdle = gpGlobals->curtime + SequenceDuration();

	if (flNextIdle > m_flTimeWeaponIdle)
		m_flTimeWeaponIdle = flNextIdle;

	return true;
}

//=========================================================
//=========================================================
void CBaseCombatWeapon::AddViewmodelBob(CBaseViewModel* viewmodel, Vector& origin, QAngle& angles)
{
	Vector	forward, right;
	AngleVectors(angles, &forward, &right, NULL);

	CalcViewmodelBob();

	// Apply bob, but scaled down to 40%
	VectorMA(origin, m_flVerticalBob * 0.1f, forward, origin);

	// Z bob a bit more
	origin[2] += m_flVerticalBob * 0.1f;

	// bob the angles
	angles[ROLL] += m_flVerticalBob * 0.5f;
	angles[PITCH] -= m_flVerticalBob * 0.4f;

	angles[YAW] -= m_flLateralBob * 0.3f;

	VectorMA(origin, m_flLateralBob * 0.8f, right, origin);
}

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

int CBaseCombatWeapon::ActionType(void) const
{
	return ACTION_WEAPON;
}

#endif