//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for Insurgency
//              Sure, there will only ever be one type of the player in the game (well, two
//              there are bots as well) but to ensure the player code isn't in one 
//				huge 1000 line file, I've seperated it out.
//
// $NoKeywords: $
//=============================================================================//

#ifndef INS_PLAYER_H
#define INS_PLAYER_H

#pragma once

#include "ins_playerlocaldata.h"
#include "ins_playeranimstate.h"
#include "ins_player_shared.h"
#include "playerstats.h"
#include "squad_data.h"
#include "team_lookup.h"
#include "command_register.h"
#include "ins_inventory_shared.h"

//=========================================================
//=========================================================
class CINSPlayer;
class CBaseAnimatingOverlay;
class CPlayTeam;
class CINSSquad;
class CWeaponBallisticBase;
class CSpawnProtection;
class CMantleZone;
class CTeamLookup;
class CWeaponINSBase;
class CINSTouch;
class CObjOrder;
class CSoundPatch;

//=========================================================
//=========================================================
enum PlayerSpawnType_t
{
	PSPAWN_NONE = 0,	// spawn normally
	PSPAWN_VIEWPOINT,	// spawn in a viewpoint
	PSPAWN_OBSERVER		// spawn as an observer in a viewpoint
};

enum PlayerVoiceType_t
{
	PVOICETYPE_ALL = 0,
	PVOICETYPE_TEAM,
	PVOICETYPE_SQUAD
};

#define MEMBERID_INVALID -1

#define FADEOUT_TEAMCHANGE_TIME 1.5f
#define INITIAL_FADEIN 3.2f

//=========================================================
//=========================================================
#define MAX_DAMAGEINFO 8

enum DamageInfoPlayerTypes_t
{
	DAMAGEINFO_PLAYER_ACTIVE = 0,
	DAMAGEINFO_PLAYER_INACTIVE,
	DAMAGEINFO_PLAYER_COUNT
};

enum DamageInfoDamageTypes_t
{
	DAMAGEINFO_LIGHT = 0,
	DAMAGEINFO_MODERATE,
	DAMAGEINFO_SERIOUS,
	DAMAGEINFO_COUNT
};

//=========================================================
//=========================================================
struct DamageInfo_t
{
	int m_iDamageType;
	int m_iTeamID;
};

struct DamageTaken_t : public DamageInfo_t
{
	EHANDLE m_hPlayer;
};

struct DamageGiven_t : public DamageInfo_t
{
	EHANDLE m_hPlayer;
};

/*struct DamageGiven_t : public DamageGiven_t
{
	char m_szName[ MAX_PLAYER_NAME_LENGTH ];
};*/

//=========================================================
//=========================================================
class CINSPlayer : public CBasePlayer, public IINSPlayerAnimStateHelper, public IInventoryManager
{
	DECLARE_CLASS( CINSPlayer, CBasePlayer );

	DECLARE_DATADESC( );
	DECLARE_SERVERCLASS( );
	DECLARE_PREDICTABLE( );

	friend class CINSGameMovement;

public:
	CINSPlayer( );
	~CINSPlayer( );

	// Player Management
	void UpdateOnRemove(void);

	static CINSPlayer *CreatePlayer(const char *className, edict_t *ed);

	void PreThink(void);
	void PostThink(void);
	void TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr);
	int OnTakeDamage(const CTakeDamageInfo &info);
	int	OnTakeDamage_Alive(const CTakeDamageInfo &info);
	void Event_Killed(const CTakeDamageInfo &info);
	void Event_Dying(void);

	void PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper);

	float CalcProneRoll(const QAngle& angAngles, const Vector& vecVelocity);
	void CalcViewRoll(QAngle &aEyeAngles);
	void CalcView(Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov);
	void CalcPlayerLean( Vector &eyeOrigin, QAngle &eyeAngles );
	void CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );

	void ApplyPlayerView( Vector &eyeOrigin, QAngle &eyeAngles, float &fov );

	void UpdateCollisionBounds( void );

    Vector Weapon_ShootDirection(void);

	void UpdateClientData(void);

	void FinishDeathThink(void);

	bool StartObserverMode(int iMode);
	bool SetObserverMode(int iMode);
	bool IsValidObserverTarget(CBaseEntity *pTarget);
	void CheckObserverSettings(void);
	bool IsRealObserver(void) const;

	bool ClientCommand(const CCommand& args);

	CWeaponINSBase *GetActiveINSWeapon(void) const;
	CBaseCombatWeapon *GetPrimaryWeapon(void) const;
	CBaseCombatWeapon *GetSecondaryWeapon(void) const;
	bool HasPrimaryWeapon(void);
	bool HasSecondaryWeapon(void);
	CBaseCombatWeapon *GetNextBestWeapon(CBaseCombatWeapon *pCurrentWeapon);
	bool Weapon_CanUse(CBaseCombatWeapon *pWeapon);
	bool BumpWeapon(CBaseCombatWeapon *pWeapon, bool bCheckVisible);
	bool Weapon_CanDrop(CBaseCombatWeapon *pWeapon) const;
	void Weapon_Equip(CBaseCombatWeapon *pWeapon);
	bool Weapon_EquipAmmoOnly(CBaseCombatWeapon *pWeapon);
	int Weapon_GetEmptySlot(int iWeaponType) const;
	void Weapon_ToShouldered( bool bForce );
	void Weapon_CancelReload( void );

	void RemovedWeapon(CBaseCombatWeapon *pWeapon);
	void RemoveAllItems(void);
	void RemoveAllWeapons(void);

	void DoAnimationEvent(PlayerAnimEvent_e Event);

	int GetHealthType(void) const;
	float GetHealthFraction( void ) const;

	void UpdateWeaponStates(void);

	CPlayerModelData *GetPlayerModelData( void ) const;

	void UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity );

	const Vector GetPlayerMins( void ) const;
	const Vector GetPlayerMaxs( void ) const;

	void CommitSuicide( void );
	void CommitSuicide( bool bForce );
	void CommitSuicide( int iType, bool bForce );

	void DamageEffect( float flDamage, int iDamageType );

	Vector CalcDamageForceVector( const CTakeDamageInfo &info );

	void NoteWeaponFired( void );

	void AddPlayerFlag( int iFlags );
	void RemovePlayerFlag( int iFlags );
	int GetPlayerFlags( void ) const;

	// Ragdoll Related
	bool BecomeRagdollOnClient(const Vector &force) { return true; }
	bool CanBecomeRagdoll(void) { return true; }

	// Spawn Management
	void Precache(void);
	void Spawn(void);
	void SharedSpawn(void);
	void InitialSpawn(void);

	void SpawnDead(void);

	void SpawnReinforcement( void );

	int LastSpawnType( void ) const { return m_iLastSpawnType; }

	bool AllowReinforcement( void ) const { return m_bAllowReinforcement; }

	// Team Handling
	void ChangeTeam(int iTeamID);
	void RemoveFromTeam(void);

	CTeam *GetTeam(void) const;
	int GetTeamID(void) const { return m_iTeamID; }
	bool OnTeam(CTeam *pTeam) const;
	bool OnPlayTeam(void) const;
	CPlayTeam *GetPlayTeam(void) const;
	CTeamLookup *GetTeamLookup(void) const;

	bool CanChangeTeam(void) const { return m_INSLocal.m_bAllowTeamChange; }
	void EnableTeamChange(void) { m_INSLocal.m_bAllowTeamChange = true; }

	void ChangedTeam( void ) { m_bChangedTeam = true; }
	bool HasChangedTeam( void ) { return m_bChangedTeam; }

	void ChangedSquad( void ) { m_bChangedSquad = true; }
	bool HasChangedSquad( void ) { return m_bChangedSquad; }

	// Squad Handling
	CINSSquad *GetSquad(void) const;
	bool OnSquad(CINSSquad *pSquad) const;
	bool ChangeSquad( const SquadData_t &SquadData, bool bWhenDie );
	bool ChangeSquad( const SquadData_t &SquadData );
	bool RemoveSquad( void );
	void ResetSquad(void);

	void AttemptSquadUpdate(void);

	bool IsValidSquad(void) const;
	int	GetSquadID(void) const { return m_SquadData.GetSquadID(); }
	int	GetSlotID(void) const { return m_SquadData.GetSlotID(); }
	const SquadData_t &GetSquadData(void) const { return m_SquadData; }

	EncodedSquadData_t GetEncodedSquadData(void);

	// Class Handling
	bool IsFirstSquadChange(void) { return m_bFirstSquadChange; }

	CPlayerClass *GetClass(void) const;
	int GetClassID(void) const;
	bool IsMedic(void) const;

	// Commander System
	void SetCommander(bool bState) { m_bCommander = bState; }
	bool IsCommander(void) const;

	void UpdateRank( void );
	inline int GetRank( void ) const { return m_iRank; }
	inline void SetRank( int iRank ) { m_iRank=iRank; }
	const char *GetRankName( void ) const;
	const char *GetFullRankName( void ) const;

	// Order Handling
	void ResetOrders( void );

	bool HasObjOrders( void ) const;
	const CObjOrder *GetObjOrders( void ) const;
	void AssignedObjOrders( int iID );

	bool HasUnitOrders( void ) const;
	const CUnitOrder *GetUnitOrders( void ) const;
	void AssignedUnitOrders( int iID );

	void ResetPlayerOrders( void );
	void AssignPlayerOrders( int iID );
	void PlayerOrderResponse( int iType );
	int GetPlayerOrder( void ) const { return m_iPlayerOrderID; }

	// Help Management
	void ResetHelp( void );
	void SetNeedsHelp( void );
	bool NeedsHelp( void ) const { return m_bNeedsHelp; }

	// Action Management
	static bool IsValidAction( int iID );

	void ResetActions( void );
	void SendAction( int iType );

	// Status Management
	void ResetStatus( void );
	void ResetStatus( int iType );

	void UpdateStatus( int iType, int iID );

	int GetStatusType( void ) const { return m_iStatusType; }
	int GetStatusID( void ) const { return m_iStatusID; }
	bool GetStatus( int &iType, int &iID );

	// Objective Handling
	void ResetCurrentObj( void );
	void ResetObjExitTime( void );

	bool InObjective( void ) const;

	void SetCurrentObjective( CINSObjective *pObjective );
	CINSObjective *GetCurrentObjective( void ) const { return m_pCurrentObjective; }

	void LeftCurrentObj(void);

	bool IsOutsideCapturing(void) const { return (m_flObjExitTime != 0.0f); }
	float GetObjExitTime(void) const { return m_flObjExitTime; }

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

	bool IsProned( void ) const;
	bool IsCrouched( void ) const;
	bool IsStanding( void ) const;

	bool IsMoving( void ) const;
    bool IsCrawling( void ) const;

	CBaseCombatWeapon *INSAnim_GetActiveWeapon( void );
	int INSAnim_GetPlayerFlags( void );
	bool INSAnim_InStanceTransition( void );
    int INSAnim_CurrentStance( void );
	int INSAnim_LastStance( void );
	int INSAnim_OldStance( void );
	int INSAnim_LeaningType( void );

	// Class Preferences
	void ClearClassPreference( void );
	int GetClassPreference( int iID ) const;

	// Layout Customisation
	void CustomiseLayout(int iLayoutType, int iID);
	void ClearLayoutCustomisation(void);
	int GetLayoutCustomisation(int iLayoutType) const;

	// View Mangement
	void SetViewTransition( int iTransitionTarget, float flTransitionTime );
	void SetViewTransition( const Vector &vecTransitionTarget, float flTransitionTime );
	void ResetViewTransition( void );

	float ViewTransitionLength( void ) const;
	float ViewTransitionEnd( void ) const;
	int ViewTransitionFrom( void ) const;
	int ViewTransitionTarget( void ) const;

	void SetViewBipodOffset( int iOffset );
	void ResetViewBipodOffset( void );
	int ViewOffsetBipod( void ) const;

	// Gamerules Related	
	void SetupEndgameCamera(CBaseEntity *pWinningObject);

	// Bandaging Management
	bool IsBeingBandaged( void );

	// Sprinting and Stamina Management
	bool IsSprinting( void ) const;
	int GetStamina( void ) const;
	bool IsBrustSprint( void ) const;

	// Weight
	void UpdateWeightFactor(void);

	// Damage Decay
	int GetDamageDecay(void) const { return m_iDamageDecay; }

	// Breathing
	void ResetBreathing( void );
	const QAngle &GetBreathingAdjust( void ) const;

	// Leaning
	bool IsLeaning( void ) const;
	
	// Bunny Hopping Control
	void ResetBunny(void);

	// Inventory
	CBaseCombatWeapon *CreateWeapon( int iWeaponID );

	bool AddWeapon( int iWeaponID, int iClipCount, int iAmmoCount );
	bool AddEquipment( int iItemID, int iCount );

	CBaseCombatWeapon *FindEquipment( int iWeaponID );

	// Morale Management
	int GetMorale( void ) const { return m_iMorale; }
	void UpdateMorale( void );

	// Stats Management
	int GetStatsMemberID( void ) const;
	bool IsUsingStats( void ) const;

	void StatsLogin( int iMemberID );
	void StatsLogout( void );

	void ResetStats( void );
	void UnmaskStats( void );

	void SendStatNotice( int iAmount, const char* pszType );

	void IncrementStat( int iType, int iIncrement );
	void IncrementWeaponStat( int iWeaponID, int iType, int iIncrement );
	inline void BumpStat( int iType );
	inline void BumpWeaponStat( int iWeaponID, int iType );

	const CGamePlayerStats &GetUpdateStats( void ) const;
	const CGamePlayerStats &GetStats( void ) const { return m_Stats; }
	const CGamePlayerStats &GetMaskedStats( void ) const { return m_MaskedStats; }
	const CPlayerStats &GetAggregateStats( void ) const { return m_AggregateStats; }

	// Firendly Fire Message Management
	bool CanSendFFMessage(void) const;
	void ExtendFFMessage(void);

	// Viewpoint Magement
	void ViewpointAdd( void );
	void ViewpointRemove( void );

	bool InViewpoint( void ) const { return m_bSpawnedViewpoint; }

	// Damage Information Management
	static int GetDamageInfoType( int iDamage );

	void AddDamageTaken( CINSPlayer *pPlayer, int iDamageType );
	int GetDamageTakenCount( void );
	const DamageTaken_t &GetDamageTaken( int iID );

	void SendDeathInfo( int iType, CBaseEntity *pScorer, int iDistance, int iInflicatorType, int iInflicatorID, int iDamageType );

	// Spawn Protection Management
	void SetClippingEntity(CSpawnProtection *pEntity);
	void RemoveClippingEntity(void);
	CSpawnProtection *GetClippingEntity(void) const;

	// Mantaling Management
	void SetMantalingEntity(CMantleZone *pEntity);
	void RemoveMantalingEntity(void);
	CMantleZone *GetMantleEntity(void) const;

	// Talk Control
	bool CanSpeak(void);
	void NotePlayerTalked(void);

	// Camera Management
	bool InCameraMode(void) const { return m_bInCameraMode; }

	// Area Management
	void EnterCurrentArea( CINSTouch *pArea );
	void LeaveCurrentArea( void );
	const char *GetCurrentArea( void ) { return m_szLastArea; }

	// Developer Handling
	bool IsDeveloper(void) { return false; }

	// Custom Data Management
	static void LoadData(void);
	LoadPlayerData_t &GetData(void);

	const QAngle &HeadAngles( void ) const;

	// Other
	bool IsMoveFrozen( void ) const;

	void SetIronsightsState( bool bState );

	void ExecuteRemoveCommon( void );

	bool IsJumping(void) const;
	bool CanJump(void) const;

	void GetMuzzle(Vector &vecMuzzle, QAngle &angMuzzle);

	void SetPlayerModel(void);

	int GetVoiceType(void) const { return m_iVoiceType; }

	virtual CBasePlayer *GetScorer(void) { return this; }

	float GetTimeJoined(void) const { return m_flTimeJoined; }

	void SetEntityPosition(CBaseEntity *pPoint);

	void FadeToBlack(float flLength);
	void FadeOutBlack(float flLength);

	void SetCanShowDeathMenu( void );
	virtual int GetDeathMenuType( void ) const;

	bool IsCustomized(void) const;

	void ClearPain(void);
	void SendPain(int iType);
	void ConcussionEffect( bool bMajor );
	void ResetConcussionEffect( void );
	
	bool IsReloading(void) const;

	void SetCmdValue(int iID, int iValue);
	int GetCmdValue(int iID) const;

	bool IsRunningAround( void );
	bool InSquad( void );

	void GivePowerball( void );
	void StripPowerball( void );
	bool HasPowerball( void ) const { return m_bHasPowerball; }

	void Jumped( void );

	void UpdateHealth( int iNewHealth );

	bool IsTKPunished( void ) const { return m_bTKPunished; }
	void PunishTK( void ) { m_bTKPunished = true; }
	void PunishTKRemove( void ) { m_bTKPunished = false; }

	void MassVGUIHide( void );

	void DeathIgnored( void ) { m_bDeathIgnored = true; }

	//deathz0rz [
	const IINSPlayerAnimState* GetAnimState() { return m_pPlayerAnimState; };
	//deathz0rz ]

	float GetHeadTurnThreshold( void ) const { return m_flHeadTurnThreshold; }

	bool SendHints( void ) const;

	void DisableAutoSwitch( bool bState ) { m_bDisableAutoSwitch = bState; }

	void SetGimped( bool bState );
	bool IsGimped( void ) const;

	bool HandleChatMessage( char *pszMessage );

	void ClientSettingsChanged( void );

private:

	// Status Management
	void SetStatus( int iType, int iID );

	// Ragdoll Management
	void CreateRagdollEntity( const Vector &vecForce );

	// Clip Management
	bool AddFinalClip(int iWeaponType, int iClip);

	// Bandaging Handling
	void Bandage( void );
	bool FinishedBandage( CINSPlayer *pPlayer );
	bool CanBandage( CINSPlayer *pPlayer );
	void HandleBandaging( void );
	void StopBandaging( void );
	void ResetBandaging( void );

	bool IsBandaging( void ) const;

	// Walking Management
	bool IsWalking( void ) const;
	void HandleWalking( void );

	// Sprinting and Stamina Management
	bool CanSprint(void);
	bool CanStartSprint(void);
	void HandleSprintButtons(void);
	void SimulateStamina(void);
	void ResetSprinting(void);
	void SetSprinting(bool bState);
	
	// Damage Decay Management
	void SimulateDamageDecay(void);
	void ResetDamageDecay(void);

	// Breathing
	void SimulateBreathing(void);

	// Leaning
	bool CanLean( void ) const;
	void HandleLeaning( void );
	int CalculateLeanType( void ) const;

	// Status Management
	int ValidStatus( int iType ) const;

	// Objective Handling
	void ExitObjective(void);

	// Camera Management
	void SetCameraMode(int iMode);
	bool CanCameraMode(void);

	// Misc
	void ImpulseCommands( int iImpulse );

	bool AllowNameChange( void );
	void AttemptNameUpdate( void );

public:

	// Bunny Hopping Control
	CNetworkVar( float, m_flStartBunnyHopTime );
	CNetworkVar( float, m_flBunnyHopLength );

	// Stance System
    CNetworkVar( int, m_iCurrentStance );
    CNetworkVar( int, m_iLastStance );
	CNetworkVar( int, m_iOldStance );
    CNetworkVar( float, m_flStanceTransMarker );

	CNetworkVar( float, m_flNextStanceThreshold );

	// View Management
	CNetworkVar( float, m_flViewTransitionLength );
	CNetworkVar( float, m_flViewTransitionEnd );
	CNetworkVar( int, m_iViewTransitionFrom );
	CNetworkVar( int, m_iViewTransitionTarget );

	CNetworkVar( int, m_iViewOffsetBipod );

	// Other
	CNetworkVarEmbedded( CINSPlayerLocalData, m_INSLocal );

	// Commander System
	static RankBoundaries_t m_RankBoundaries;

private:

	// Player Management
	IINSPlayerAnimState *m_pPlayerAnimState;

	CPlayerModelData *m_pPlayerModelData;

	float m_fNextSuicideTime;

	CNetworkVar( int, m_iPlayerFlags );

	// Ragdoll Management
	CNetworkHandle(CBaseEntity, m_hRagdoll);

	// Spawn Management
	int m_iLastSpawnType;
	bool m_bAllowReinforcement;

	// Team Handling
	int m_iTeamID;

	bool m_bChangedTeam;

	// Squad Management
	SquadData_t m_SquadData, m_NextSquadData;
	bool m_bFirstSquadChange;

	bool m_bChangedSquad;

	CNetworkVar( bool, m_bCommander );
	int m_iRank;

	// Order System
	int m_iPlayerOrderID;

	// Help Management
	bool m_bNeedsHelp;
	float m_flHelpTimeout;

	// Action Management
	float m_flActionThreshold[ PACTION_COUNT ];

	// Status Management
	int m_iStatusType, m_iStatusID;

	// Objective Handling
	CINSObjective *m_pCurrentObjective;
	bool m_bInsideObj;
	float m_flObjExitTime;

	// Stamina
	CNetworkVar( int, m_iStamina );
	CNetworkVar( float, m_flStaminaUpdateThreshold );

	CSoundPatch *m_pStaminaSound;

	// Class Preferences
	ClassPreferences_t m_ClassPreferences;

	// Layout Customization
	LayoutCustomisation_t m_LayoutCustomisation;

	// Weight
	float m_flWeight;

	// Bandages
	int m_iHitCounts[ HIRGROUP_COUNT ];

	CNetworkVar( int, m_iBandadgeCount );

	// Damage Decay
	CNetworkVar( int, m_iDamageDecay );
	CNetworkVar( float, m_flDamageDecayThreshold );

     // Breathing
     float m_flBreathTime;
     CNetworkQAngle( m_angShakyHands );
	 CNetworkVar( float, m_flRadius );
	 CNetworkVar( float, m_flRadiusDesired );

	// Leaning
	CNetworkVar( int, m_iLeanType );

	// Morale Management
	int m_iMorale;

	// Stats Management
	CGamePlayerStats m_Stats,		// stats, important to the game, since the last score reset
				m_MaskedStats;		// m_Stats but without updated kills and deaths

	CPlayerStats m_AggregateStats;	// full stats since the player joined the server

	CNetworkVar( int, m_iStatsMemberID );

	// Viewpoint Control
	CNetworkVar( bool, m_bSpawnedViewpoint );

	// Damage Information Management
	EHANDLE m_LastKiller;
	
	CUtlVector< DamageTaken_t > m_DamageTaken;

	// Spawn Protection Management (:: Create a CBaseHandle typedef)
	CNetworkHandle(CBaseEntity, m_hClippingEntity);

	// Mantaling Management (:: Create a CBaseHandle typedef)
	CNetworkHandle(CBaseEntity, m_hMantleEntity);

	// Camera Management
	bool m_bInCameraMode;

	// Custom Data Management
	static LoadPlayerData_t m_CustomData;

	// Area Management
	char m_szLastArea[ MAX_LASTAREA_LENGTH ];

	// Other
	CNetworkVar( bool, m_bWalking );

	Vector m_vecMuzzle;
	QAngle m_angMuzzle;

	CNetworkVar(bool, m_bIsCustomized);
	float m_flLastFFAttack;
	int m_iVoiceType;
	bool m_bCanShowDeathMenu;
	float m_flTimeJoined;
	bool m_bAllowNameUpdate;
	float m_flNextTalkTime;
	Vector m_vecTotalForce, m_vecTotalBulletForce;
	bool m_bDeadFade;
	bool m_bDeathIgnored;
	float m_flNextPickupSound;
	CNetworkArray(int, m_iCmdRegister, CMDREGISTER_COUNT);
	bool m_bHasPowerball;
	CNetworkVar( bool, m_bTKPunished );
	bool m_bDisableAutoSwitch;
	float m_flInvincibilityTheshold;

    float m_flHeadTurnThreshold;

	bool m_bGimped;
};

//=========================================================
//=========================================================
class CINSRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS(CINSRagdoll, CBaseAnimatingOverlay);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CINSRagdoll();

	void Spawn( void );

	void Vanish(void);
	void SUB_Vanish(void);

	void SetDroppedWeapon(CBaseCombatWeapon *pWeapon) { m_DroppedWeapon = pWeapon; }

	// transmit ragdolls to everyone.
	virtual int UpdateTransmitState() { return SetTransmitState( FL_EDICT_ALWAYS ); }

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );

	float m_flStartTime;

	int m_iVanishTicks;

#ifdef GAME_DLL

	EHANDLE m_DroppedWeapon;

#endif
};

//=========================================================
//=========================================================
void CINSPlayer::BumpStat( int iType )
{
	IncrementStat( iType, 1 );
}

//=========================================================
//=========================================================
void CINSPlayer::BumpWeaponStat( int iWeaponID, int iType )
{
	IncrementWeaponStat( iWeaponID, iType, 1 );
}

//=========================================================
//=========================================================
typedef CHandle< CINSPlayer > CINSPlayerHandle;

//=========================================================
//=========================================================
inline CINSPlayer *ToINSPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer( ) )
		return NULL;

	return static_cast< CINSPlayer* >( pEntity );
}

#endif	// SDK_PLAYER_H
