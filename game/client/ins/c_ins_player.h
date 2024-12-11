//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side CINSPlayer.
//
//=============================================================================//

#ifndef C_INSPLAYER_H
#define C_INSPLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "c_ins_playerlocaldata.h"
#include "ins_playeranimstate.h"
#include "ins_player_shared.h"
#include "playerstats.h"
#include "squad_data.h"
#include "beamdraw.h"
#include "weapon_defines.h"
#include "command_register.h"
#include "commander_shared.h"
#include "materialsystem/icolorcorrection.h"

//=========================================================
//=========================================================
#define MAX_PLAYER_TITLE_LENGTH ( MAX_PLAYER_NAME_LENGTH * 2 )

//=========================================================
//=========================================================
class C_INSSquad;
class C_INSObjective;
class C_SpawnProtection;
class C_MantleZone;
class C_WeaponBallisticBase;
class C_WeaponINSBase;
class CPlayerClass;
class C_PlayTeam;
class CTeamLookup;
class CPlayerModelData;

//=========================================================
//=========================================================
class C_INSPlayer : public C_BasePlayer, public IINSPlayerAnimStateHelper
{
	DECLARE_CLASS(C_INSPlayer, C_BasePlayer);

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	friend class CINSGameMovement;

public:
	C_INSPlayer();
	~C_INSPlayer();

	// Player Management
	void PreThink(void);
	void ClientThink(void);
	void PostThink(void);

	const QAngle &GetRenderAngles(void);
	void UpdateClientSideAnimation(void);

	bool ShouldDraw(void);

	void AddEntity(void);

	virtual CStudioHdr *OnNewModel( void );

	bool CreateMove(float flInputSampleTime, CUserCmd* pCmd, bool bFakeInput);

	void CalcView(Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov);
	float CalcProneRoll(const QAngle& angAngles, const Vector& vecVelocity);
	void CalcViewRoll(QAngle &aEyeAngles);
	void CalcPlayerLean( Vector &eyeOrigin, QAngle &eyeAngles );

	void ApplyPlayerView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);
	void CalcDeathCamView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);

	const QAngle &EyeAngles( void );

    Vector Weapon_ShootDirection(void);

	void DoAnimationEvent(PlayerAnimEvent_e Event);

	static C_INSPlayer *GetLocalPlayer(void);

	C_WeaponINSBase *GetActiveINSWeapon(void) const;
	C_BaseCombatWeapon *GetPrimaryWeapon(void) const;
	C_BaseCombatWeapon *GetSecondaryWeapon(void) const;
	bool HasPrimaryWeapon(void);
	bool HasSecondaryWeapon(void);
	C_BaseCombatWeapon *GetNextBestWeapon(C_BaseCombatWeapon *pCurrentWeapon);
	void Weapon_ToShouldered( bool bForce );
	void Weapon_CancelReload( void );

	void UpdateWeaponStates(void);

	bool IsRealObserver(void) const;

	static bool ParsePlayerName( int iPlayerID, bool bIncludeTitle, char *pszBuffer, int iLength );

	CPlayerModelData *GetPlayerModelData( void ) const;

	void UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity );

	const Vector GetPlayerMins( void ) const;
	const Vector GetPlayerMaxs( void ) const;

	void TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );

	bool HasZoom( void );
	bool IsZoomed( void );
	bool ActiveZoom( void );

	void AddPlayerFlag( int iFlags );
	void RemovePlayerFlag( int iFlags );
	int GetPlayerFlags( void ) const;

	float GetViewmodelFOV( void );

	bool CanDrawGlowEffects() { return !(IsObserver() || IsInVGuiInputMode()); }

	// Ragdoll Related
	C_BaseAnimating *BecomeRagdollOnClient(bool bCopyEntity);
	IRagdoll *GetRepresentativeRagdoll(void) const;
	CBaseAnimating *GetRagdollEntity(void) const;

	// Spawn Management
	void SharedSpawn(void);

	// Team Handling
	int	GetTeamID(void) const;
	C_Team *GetTeam(void) const;
	bool OnTeam(C_Team *pTeam) const;
	bool OnPlayTeam(void) const;
	C_PlayTeam *GetPlayTeam(void) const;
	CTeamLookup *GetTeamLookup(void) const;

	static C_PlayTeam *GetPlayTeam(int iPlayerID);
	static bool OnPlayTeam(int iPlayerID);

	bool CanChangeTeam(void) const { return m_INSLocal.m_bAllowTeamChange; }

	// Squad Handling
	int GetSquadID(void) const;
	int GetSlotID(void) const;
	bool IsValidSquad(void) const;
	C_INSSquad *GetSquad(void) const;
	bool OnSquad(C_INSSquad *pSquad) const;
	const char *CanSpawn(void) const;

	void GetSquadData(SquadData_t &SquadData) const;
	
	// Class Handling
	CPlayerClass *GetClass(void) const;
	int GetClassID(void) const;
	bool IsMedic(void) const;

	// Commander System
	int GetRank(void) const;
	const char *GetRankName(void) const;
	const char *GetFullRankName(void) const;
	const char *GetFullNextRankName(void) const;

	bool IsCommander(void) const;

	// Order Handling
	void HandleObjOrder( bf_read &msg );
	const CObjOrder *GetObjOrders( void ) const;
	bool HasObjOrders( void ) const;

	void HandleUnitOrder( bf_read &msg );
	const CUnitOrder *GetUnitOrders( void ) const;
	bool HasUnitOrders( void ) const;

	int GetPlayerOrder( void ) const;

	// Action Management
	static bool IsValidAction( int iType );

	// Status Management
	int GetStatusType( void ) const;
	int GetStatusID( void ) const;
	bool GetStatus( int &iType, int &iID );

	// Objective Handling
	bool IsCapturing( void ) const;
	void SetCurrentObj( C_INSObjective *pCurrentObj ) { m_pCurrentObj = pCurrentObj; }
	C_INSObjective *GetCurrentObj( void ) const { return m_pCurrentObj; }

	// Stance System
	void StanceReset( void );

	bool CanStanceTransition( void ) const;
	bool CanChangeStance( int iFromStance, int iToStance );

	void SetDesiredStance( int iStance );
	void HandleTransitionTime( float flTransitionTime );

	int GetCurrentStance( void ) const;
	int GetLastStance( void ) const;
	float GetStanceTransitionMarker( void ) const;
	bool InStanceTransition( void ) const;
	void DoProneViewEffect( bool bFrom );
	bool IsSpenceStance( void ) const;

	Vector GetCurrentViewOffset( void );

	bool IsProned(void) const;
	bool IsCrouched(void) const;
	bool IsStanding(void) const;

	bool IsMoving(void) const;
    bool IsCrawling(void) const;

	CBaseCombatWeapon *INSAnim_GetActiveWeapon( void );
	int INSAnim_GetPlayerFlags( void );
	bool INSAnim_InStanceTransition( void );
	int INSAnim_CurrentStance( void );
	int INSAnim_LastStance( void );
	int INSAnim_OldStance( void );
	int INSAnim_LeaningType( void );

	// View Mangement
	void SetViewTransition( int iTransitionTarget, float flTransitionTime );
	void SetViewTransition( const Vector &vecTransitionTarget, float flTransitionTime );

	float ViewTransitionLength( void ) const;
	float ViewTransitionEnd( void ) const;
	int ViewTransitionFrom( void ) const;
	int ViewTransitionTarget( void ) const;

	void SetViewBipodOffset( int iOffset );
	void ResetViewBipodOffset( void );
	int ViewOffsetBipod( void ) const;

	// Bandaging Management
	void Bandage(void);
	bool FinishedBandage(C_INSPlayer *pPlayer);
	bool CanBandage(C_INSPlayer *pPlayer);
	void HandleBandaging(void);
	void StopBandaging(void);
	void ResetBandaging(void);

	bool IsBandaging( void ) const;

	// Walking Management
	bool IsWalking( void ) const;
	void HandleWalking( void );

	// Sprinting and Stamina Management
	bool IsSprinting( void ) const;
	int GetStamina( void ) const;
	bool IsBrustSprint( void ) const;

	bool CanSprint(void);
	bool CanStartSprint(void);
	void HandleSprintButtons(void);
	void SimulateStamina(void);
	void ResetSprinting(void);
	void SetSprinting(bool bState);

	// Damage Decay
	void SimulateDamageDecay(void);
	int GetDamageDecay(void) const { return m_iDamageDecay; }

	 // Breathing
	void SimulateBreathing( void );
	void ResetBreathing( void );
	const QAngle &GetBreathingAdjust( void ) const;

	// Leaning
	bool IsLeaning( void ) const;
	bool CanLean( void ) const;
	void HandleLeaning( void );
	int CalculateLeanType( void ) const;

	// Bunny Hopping Control
	void ResetBunny(void);

	// Freeaim and Supported Handling
	bool IsFreeaimEnabled(void) const;
	float GetFreeaimDistance( void ) const;
	float GetFreeaimScreenWeaponRelation(void) const;
	bool UseFreeaimSWRFraction(void) const;

	const QAngle &GetFreeAimAngles(void) const;
	void SetFreeAimAngles(const QAngle &angAngles);

	const QAngle &GetSupportedFireAngles(void) const;
	void SetSupportedFireAngles(const QAngle &angAngles);

	// Stats Management
	int GetStatsMemberID(void) const;
	bool IsUsingStats(void) const;

	// Viewpoint Magement
	bool InViewpoint(void) const { return m_bSpawnedViewpoint; }

	// Spawn Protection Management
	C_SpawnProtection *GetClippingEntity(void) const;

	// Spawn Protection Management
	C_MantleZone *GetMantleEntity(void) const;

	// Developer Handling
	bool IsDeveloper(void);

	// Head Targetting
	int GetIDTarget(void) const;
	void UpdateIDTarget(void);

	void UpdateLookAt(void);

	// Custom Data Management
	static void LoadData(void);
	LoadPlayerData_t &GetData(void);

	const QAngle &HeadAngles(void) const;
	float GetLookOver(void) const;

	// Other
	bool IsMoveFrozen( void ) const;

	static void InitalSpawn( void );

	void SetIronsightsState( bool bState );

	bool IsJumping(void) const;
	bool CanJump(void) const;

	void HandleMuzzle(void);
	void GetMuzzle( Vector &vecMuzzle, QAngle &angMuzzle );

	void SetPerferredFireMode(int iPerferredFireMode) { m_iPerferredFireMode = iPerferredFireMode; }
	int GetPerferredFireMode(void) { return m_iPerferredFireMode; }
	
	const Vector& GetChaseCamOrigin(void);
	int	GetChaseCamDistance(void);

	float GetZoomFov(void) { return m_flZoomFOV; }
    void  SetZoomFov(float zFov) { m_flZoomFOV = clamp( zFov, 5.0f, 120.0f ); }

	int GetHealthType(void) const;
	float GetHealthFraction( void ) const;

	bool IsCustomized(void) const;

	bool IsReloading(void) const;

	void SetCmdValue(int iID, int iValue);
	int GetCmdValue(int iID) const;

	bool IsRunningAround( void );
	bool InSquad( void );

	void UpdateHeadPosition( void );
	bool CanLookAt( void );

	void Jumped( void );

	void SetupColorCorrection( void );
	bool IsValidColorCorrection( void ) const;
	void ResetColorCorrection( void );
	void UpdateColorCorrection( void );

	bool IsTKPunished( void ) const { return m_bTKPunished; }

	void SetAllowHeadTurn( bool bState ) { m_bAllowHeadTurn = bState; }

	// zero (#0000391 "Flashlight not visible for other players") [
	void NotifyShouldTransmit( ShouldTransmitState_t );
	// zero ]

private:

	// Player Management
	void Initialize(void);

public:

	// Bunny Hopping Control
	float m_flStartBunnyHopTime;
	float m_flBunnyHopLength;

	// Stance System
    int m_iCurrentStance;
    int m_iLastStance;
	int m_iOldStance;
    float m_flStanceTransMarker;

	// View Management
	float m_flViewTransitionLength, m_flViewTransitionEnd;
	int m_iViewTransitionFrom, m_iViewTransitionTarget;

	int m_iViewOffsetBipod;

	// Sprinting and Stamina Management
    int m_iStamina;
	float m_flStaminaUpdateThreshold;

	// Other
	CNetworkVarEmbedded(CINSPlayerLocalData, m_INSLocal);

private:

	// Player Management
	IINSPlayerAnimState *m_pPlayerAnimState;

	int m_iPlayerFlags;

	// Ragdoll Management
	EHANDLE	m_hRagdoll;

	// Commander System
	bool m_bCommander;

	// Objective Handling
	C_INSObjective *m_pCurrentObj;

	// Order Handling
	CObjOrder m_ObjOrders;
	CUnitOrder m_UnitOrders;

	// Damage Decay
	int m_iDamageDecay;
	float m_flDamageDecayThreshold;

     // Breathing
	float m_flBreathTime;
    QAngle m_angShakyHands;
	float m_flRadius;
	float m_flRadiusDesired;

	// Leaning
	int m_iLeanType;

	// Freeaim Handling
	QAngle m_angFreeAimAngles;
	QAngle m_angSupportedFireAngles;

	// Stats Management
	int m_iStatsMemberID;

	// Viewpoint Control
	bool m_bSpawnedViewpoint;

	// Spawn Protection Management
	EHANDLE m_hClippingEntity;

	// Mantle Management
	EHANDLE m_hMantleEntity;

	// Head Targetting
	Vector m_vecLookAtTarget;

	int	m_headYawPoseParam;
	int	m_headPitchPoseParam;
	float m_headYawMin;
	float m_headYawMax;
	float m_headPitchMin;
	float m_headPitchMax;

	float m_flLastBodyYaw;
	float m_flCurrentHeadYaw;
	float m_flCurrentHeadPitch;

	CountdownTimer m_blinkTimer;

	int	m_iIDEntIndex;

	// Custom Data Management
	static LoadPlayerData_t m_CustomData;

	int m_iPerferredFireMode;
	bool m_bIsCustomized;

	float m_flZoomFOV;

	CNetworkArray(int, m_iCmdRegister, CMDREGISTER_COUNT);

	ColorCorrectionHandle_t m_CCFadedHandle, m_CCDeathHandle;

	bool m_bTKPunished;

	bool m_bAllowHeadTurn;
	float m_flHeadLookThreshold;

	float m_flNextStanceThreshold;

	// zero (#0000391 "Flashlight not visible for other players") [
public:
	void HandleFlashlight();
	void ReleaseFlashlight();

	Beam_t* m_pFlashlightBeam;
	// zero ]
};

//=========================================================
//=========================================================
class C_INSRagdoll : public C_BaseAnimatingOverlay
{
	DECLARE_CLASS(C_INSRagdoll, C_BaseAnimatingOverlay);
	DECLARE_CLIENTCLASS();

public:
	C_INSRagdoll();
	virtual ~C_INSRagdoll();

	bool IsRagDoll( void ) const { return true; }

	void OnDataChanged( DataUpdateType_t type );

	int GetPlayerEntIndex(void) const;
	IRagdoll* GetIRagdoll(void) const;

	void ImpactTrace(trace_t *pTrace, int iDamageType, char *pCustomImpactName);
	void UpdateOnRemove(void);

	int BloodColor(void) { return BLOOD_COLOR_RED; }

private:
	C_INSRagdoll(const C_INSRagdoll &) { }

	void Interp_Copy(C_BaseAnimatingOverlay *pSourceEntity);
	void CreateRagdoll(void);

private:
	EHANDLE	m_hPlayer;

	CNetworkVector(m_vecRagdollVelocity);
	CNetworkVector(m_vecRagdollOrigin);
};

//=========================================================
//=========================================================
typedef CHandle<C_INSPlayer> CINSPlayerHandle;

//=========================================================
//=========================================================
inline C_INSPlayer *ToINSPlayer(CBaseEntity *pPlayer)
{
	return static_cast<C_INSPlayer*>(pPlayer);
}

//=========================================================
//=========================================================
extern int GetLocalTeamID(void);
extern C_Team *GetLocalTeam(void);
extern C_PlayTeam *GetLocalPlayTeam(void);
extern C_INSSquad *GetLocalSquad(void);

//=========================================================
//=========================================================
inline C_INSPlayer *C_INSPlayer::GetLocalPlayer(void)
{
	return ToINSPlayer( C_BasePlayer::GetLocalPlayer() );
}

#endif // C_INSPLAYER_H
