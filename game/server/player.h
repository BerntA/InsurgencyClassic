//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#ifndef PLAYER_H
#define PLAYER_H

#ifdef _WIN32
#pragma once
#endif

#include "basecombatcharacter.h"
#include "usercmd.h"
#include "playerlocaldata.h"
#include "PlayerState.h"
#include "game/server/iplayerinfo.h"
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "util_shared.h"
#include "achievement_manager.h"

// For queuing and processing usercmds
class CCommandContext
{
public:
	CUtlVector< CUserCmd > cmds;

	int				numcmds;
	int				totalcmds;
	int				dropped_packets;
	bool			paused;
};

//-----------------------------------------------------------------------------
// Forward declarations: 
//-----------------------------------------------------------------------------
class CBaseCombatWeapon;
class CBaseViewModel;
class CTeam;
class IPhysicsPlayerController;
class CUserCmd;

// for step sounds
struct surfacedata_t;

// !!!set this bit on guns and stuff that should never respawn.
#define	SF_NORESPAWN	( 1 << 30 )

//
// Player PHYSICS FLAGS bits
//
enum PlayerPhysFlag_e
{
	PFLAG_DIROVERRIDE	= ( 1<<0 ),		// override the player's directional control (trains, physics gun, etc.)
	PFLAG_DUCKING		= ( 1<<1 ),		// In the process of ducking, but totally squatted yet
	PFLAG_USING			= ( 1<<2 ),		// Using a continuous entity
	PFLAG_OBSERVER		= ( 1<<3 ),		// player is locked in stationary cam mode. Spectators can move, observers can't.
	PFLAG_VPHYSICS_MOTIONCONTROLLER = ( 1<<4 ),	// player is physically attached to a motion controller
	PFLAG_GAMEPHYSICS_ROTPUSH = (1<<5), // game physics did a rotating push that we may want to override with vphysics

	// If you add another flag here check that you aren't 
	// overwriting phys flags in the HL2 of TF2 player classes
};

//
// generic player
//

// useful cosines
#define DOT_1DEGREE   0.9998476951564
#define DOT_2DEGREE   0.9993908270191
#define DOT_3DEGREE   0.9986295347546
#define DOT_4DEGREE   0.9975640502598
#define DOT_5DEGREE   0.9961946980917
#define DOT_6DEGREE   0.9945218953683
#define DOT_7DEGREE   0.9925461516413
#define DOT_8DEGREE   0.9902680687416
#define DOT_9DEGREE   0.9876883405951
#define DOT_10DEGREE  0.9848077530122
#define DOT_15DEGREE  0.9659258262891
#define DOT_20DEGREE  0.9396926207859
#define DOT_25DEGREE  0.9063077870367
#define DOT_30DEGREE  0.866025403784
#define DOT_45DEGREE  0.707106781187

enum
{
	VPHYS_WALK = 0,
	VPHYS_CROUCH,
	VPHYS_NOCLIP,
};

extern bool gInitHUD;
extern ConVar *sv_cheats;

class CBasePlayer;
class CPlayerInfo : public IBotController, public IPlayerInfo
{
public:
	CPlayerInfo () { m_pParent = NULL; } 
	~CPlayerInfo () {}
	void SetParent( CBasePlayer *parent ) { m_pParent = parent; } 

	// IPlayerInfo interface
	virtual const char *GetName();
	virtual int			GetUserID();
	virtual const char *GetNetworkIDString();
	virtual int			GetTeamIndex();
	virtual void		ChangeTeam( int iTeamNum );
	virtual int			GetFragCount();
	virtual int			GetDeathCount();
	virtual bool		IsConnected();
	virtual int			GetArmorValue();

	virtual bool IsHLTV();
	virtual bool IsReplay();
	virtual bool IsPlayer();
	virtual bool IsFakeClient();
	virtual bool IsDead();
	virtual bool IsObserver();
	virtual const Vector GetAbsOrigin();
	virtual const QAngle GetAbsAngles();
	virtual const Vector GetPlayerMins();
	virtual const Vector GetPlayerMaxs();
	virtual const char *GetWeaponName();
	virtual const char *GetModelName();
	virtual const int GetHealth();
	virtual const int GetMaxHealth();

	// bot specific functions	
	virtual void SetAbsOrigin( Vector & vec );
	virtual void SetAbsAngles( QAngle & ang );
	virtual void RemoveAllItems();
	virtual void SetActiveWeapon( const char *WeaponName );
	virtual void SetLocalOrigin( const Vector& origin );
	virtual const Vector GetLocalOrigin( void );
	virtual void SetLocalAngles( const QAngle& angles );
	virtual const QAngle GetLocalAngles( void );
	virtual bool IsEFlagSet( int nEFlagMask );

	virtual void RunPlayerMove( CBotCmd *ucmd );
	virtual void SetLastUserCommand( const CBotCmd &cmd );

	virtual CBotCmd GetLastUserCommand();

private:
	CBasePlayer *m_pParent; 
};

class CBasePlayer : public CBaseCombatCharacter
{
public:
	DECLARE_CLASS( CBasePlayer, CBaseCombatCharacter );
protected:
	// HACK FOR BOTS
	friend class CBotManager;
	static edict_t *s_PlayerEdict; // must be set before calling constructor
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
	
	CBasePlayer();
	virtual ~CBasePlayer();

	// IPlayerInfo passthrough (because we can't do multiple inheritance)
	IPlayerInfo *GetPlayerInfo() { return &m_PlayerInfo; }
	IBotController *GetBotController() { return &m_PlayerInfo; }

	virtual void			SetModel( const char *szModelName );
	virtual void			SetBodyPitch( float flPitch );

	virtual void			UpdateOnRemove( void );

	static CBasePlayer		*CreatePlayer( const char *className, edict_t *ed );

	virtual void			CreateViewModel(void);
	CBaseViewModel			*GetViewModel(bool bObserverOK = true);
	void					HideViewModels( void );
	void					DestroyViewModels( void );

	CPlayerState			*PlayerData( void ) { return &pl; }
	
	int						RequiredEdictIndex( void ) { return ENTINDEX(edict()); } 

	void					LockPlayerInPlace( void );
	void					UnlockPlayer( void );

	virtual void			DrawDebugGeometryOverlays(void);
	
	// Networking is about to update this entity, let it override and specify it's own pvs
	virtual void			SetupVisibility( CBaseEntity *pViewEntity, unsigned char *pvs, int pvssize );
	virtual int				UpdateTransmitState();
	virtual int				ShouldTransmit( const CCheckTransmitInfo *pInfo );

	// Returns true if this player wants pPlayer to be moved back in time when this player runs usercmds.
	// Saves a lot of overhead on the server if we can cull out entities that don't need to lag compensate
	// (like team members, entities out of our PVS, etc).
	virtual bool			WantsLagCompensationOnEntity(const CBasePlayer* pPlayer, const CUserCmd* pCmd, const CBitVec<MAX_EDICTS>* pEntityTransmitBits) const;

	virtual void			Spawn( void );
	virtual void			Activate( void );
	virtual void			SharedSpawn(); // Shared between client and server.

	virtual void			ForceRespawn( void );
	virtual void			SetNewSolidFlags( bool bNonSolid );

	virtual void			InitialSpawn(void);
	virtual void			InitHUD( void ) {}
	virtual void			ShowViewPortPanel( const char * name, bool bShow = true, KeyValues *data = NULL );

	virtual void			PlayerDeathThink(void);
	virtual void			FinishDeathThink(void);

	const char				*GetTracerType( void );
	void					MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType );
	void					DoImpactEffect( trace_t &tr, int nDamageType );	

#if !defined( NO_ENTITY_PREDICTION )
	void					AddToPlayerSimulationList( CBaseEntity *other );
	void					RemoveFromPlayerSimulationList( CBaseEntity *other );
	void					SimulatePlayerSimulatedEntities( void );
	void					ClearPlayerSimulationList( void );
#endif

	// Physics simulation (player executes it's usercmd's here)
	virtual void			PhysicsSimulate( void );

	// Forces processing of usercmds (e.g., even if game is paused, etc.)
	void					ForceSimulation();

	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const;

	virtual void			PreThink( void );
	virtual void			PostThink( void );
	virtual void			TraceAttack(const CTakeDamageInfo& info, const Vector& vecDir, trace_t* ptr);
	virtual int				OnTakeDamage( const CTakeDamageInfo &info );
	virtual void			DamageEffect(float flDamage, int fDamageType) { }
	virtual void            CheckIsPlayerStuck(void);

	virtual Vector			EyePosition( );			// position of eyes
	const QAngle			&EyeAngles( );
	const QAngle& GetAnimEyeAngles(void) const { return m_angEyeAngles.Get(); }
	void					EyePositionAndVectors( Vector *pPosition, Vector *pForward, Vector *pRight, Vector *pUp );
	virtual const QAngle	&LocalEyeAngles();		// Direction of eyes
	void					EyeVectors( Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL );

	// Sets the view angles
	void					SnapEyeAngles(const QAngle& viewAngles, int iFixAngle = FIXANGLE_ABSOLUTE);
	void					ForceSnapAngles(const QAngle& Angles);

	virtual QAngle			BodyAngles();
	virtual Vector			BodyTarget( const Vector &posSrc, bool bNoisy);
	
	virtual const impactdamagetable_t &GetPhysicsImpactDamageTable();

	virtual void			Event_Killed( const CTakeDamageInfo &info );
	virtual void			Event_Dying( const CTakeDamageInfo &info );

	virtual void			PreAbsVelocityImpulse(bool bNoLimit);
	virtual void			PostAbsVelocityImpulse(bool bNoLimit);

	bool					IsHLTV( void ) const { return pl.hltv; }
	bool					IsReplay( void ) const { return pl.replay; }
	virtual	bool			IsPlayer( void ) const { return true; }			// Spectators return TRUE for this, use IsObserver to separate cases
	virtual bool			IsNetClient( void ) const { return true; }		// Bots should return FALSE for this, they can't receive NET messages
																			// Spectators should return TRUE for this

	virtual bool			IsFakeClient( void ) const;

	// Get the client index (entindex-1).
	int						GetClientIndex()	{ return ENTINDEX( edict() ) - 1; }

	// returns the player name
	const char *			GetPlayerName() { return m_szNetname; }
	void					SetPlayerName( const char *name );

	int						GetUserID() { return engine->GetPlayerUserId( edict() ); }
	const char *			GetNetworkIDString(); 
	virtual const Vector	GetPlayerMins( void ) const; // uses local player
	virtual const Vector	GetPlayerMaxs( void ) const; // uses local player

	void					VelocityPunch( const Vector &vecForce );

	void					ViewPunch( const QAngle &angleOffset );
	void					ViewPunchReset( float tolerance = 0 );
	const					QAngle& GetPunchAngle();

	void					RecoilViewPunch(const QAngle& angleOffset);
	void					RecoilViewPunchReset(float tolerance = 0);
	const					QAngle& GetRecoilPunchAngle(void);

	// View model prediction setup
	void					CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );

	// Handle view smoothing when going up stairs
	void					SmoothViewOnStairs( Vector& eyeOrigin );
	virtual void			CalcViewRoll(QAngle& eyeAngles);

	virtual void			RemoveAllItems();
	bool					IsDead() const;
	bool					HasPhysicsFlag( unsigned int flag ) { return (m_afPhysicsFlags & flag) != 0; }

	// Weapon stuff
	virtual Vector			Weapon_ShootPosition();
	virtual Vector			Weapon_ShootDirection();
	virtual	bool			Weapon_Switch(CBaseCombatWeapon *pWeapon, bool bForce = false);		// Switch to given weapon if has ammo (false if failed)
	virtual void			Weapon_SwitchToNext(void);
	virtual void			Weapon_SetLast( CBaseCombatWeapon *pWeapon );
	virtual bool			Weapon_ShouldSelectItem( CBaseCombatWeapon *pWeapon );

	// JOHN:  sends custom messages if player HUD data has changed  (eg health, ammo)
	virtual void			UpdateClientData( void );
	void					RumbleEffect( unsigned char index, unsigned char rumbleData, unsigned char rumbleFlags );
	
	virtual void			Precache( void );
	bool					IsOnLadder( void );
	virtual surfacedata_t	*GetLadderSurface( const Vector &origin );

	virtual int				FlashlightIsOn(void);
	virtual void			FlashlightTurnOn(void);
	virtual void			FlashlightTurnOff(void);
	
	void					UpdatePlayerSound ( void );
	virtual void			UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity );
	virtual void			PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual const char	   *GetOverrideStepSound( const char *pszBaseStepSoundName ) { return pszBaseStepSoundName; }

	virtual void			OnEmitFootstepSound( const CSoundParameters& params, const Vector& vecOrigin, float fVolume ) {}

	virtual Class_T			Classify ( void );

	// custom player functions
	virtual bool			ClientCommand( const CCommand &args );
	
	// Observer functions
	virtual bool			StartObserverMode(int mode); // true, if successful
	virtual void			StopObserverMode( void );	// stop spectator mode
	virtual bool			SetObserverMode(int mode); // sets new observer mode, returns true if successful
	virtual int				GetObserverMode( void ); // returns observer mode or OBS_NONE
	virtual bool			SetObserverTarget(CBaseEntity * target);
	virtual void			ObserverUse( bool bIsPressed ); // observer pressed use
	virtual CBaseEntity		*GetObserverTarget( void ); // returns players targer or NULL
	virtual CBaseEntity		*FindNextObserverTarget( bool bReverse ); // returns next/prev player to follow or NULL
	virtual int				GetNextObserverSearchStartPoint( bool bReverse ); // Where we should start looping the player list in a FindNextObserverTarget call
	virtual bool			IsValidObserverTarget(CBaseEntity * target); // true, if player is allowed to see this target
	virtual void			CheckObserverSettings(); // checks, if target still valid (didn't die etc)
	virtual void			JumptoPosition(const Vector &origin, const QAngle &angles);
	virtual void			ForceObserverMode(int mode); // sets a temporary mode, force because of invalid targets
	virtual void			ResetObserverMode(); // resets all observer related settings
	virtual void			ValidateCurrentObserverTarget( void ); // Checks the current observer target, and moves on if it's not valid anymore
	virtual void			AttemptToExitFreezeCam( void );

	virtual bool			StartReplayMode( float fDelay, float fDuration, int iEntity );
	virtual void			StopReplayMode();
	virtual int				GetDelayTicks();
	virtual int				GetReplayEntity();
	
	virtual bool			BumpWeapon(CBaseCombatWeapon* pWeapon, bool bCheckVisible);
	CBaseEntity				*HasNamedPlayerItem( const char *pszItemName );
	bool					RemovePlayerItem(CBaseCombatWeapon* pItem);
	bool 					HasWeapons( void );// do I have ANY weapons?
	virtual void			SelectLastItem(void);
	virtual void 			SelectItem(int iWeaponID);
	virtual void			ItemPreFrame( void );
	virtual void			ItemPostFrame( void );
	virtual CBaseEntity		*GiveNamedItem(const char *szName);

	void					EnableControl(bool fControl);
	virtual void			CheckTrainUpdate( void );
	void					AbortReload( void );

	void					WaterMove( void );
	float					GetWaterJumpTime() const;
	void					SetWaterJumpTime( float flWaterJumpTime );
	float					GetSwimSoundTime( void ) const;
	void					SetSwimSoundTime( float flSwimSoundTime );

	virtual void			SetPlayerUnderwater( bool state );
	void					UpdateUnderwaterState( void );
	bool					IsPlayerUnderwater( void ) { return m_bPlayerUnderwater; }

	virtual void			PlayerUse( void );

	virtual CBaseEntity		*FindUseEntity( void );
	virtual bool			IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps );
	bool					ClearUseEntity();
	CBaseEntity				*DoubleCheckUseNPC( CBaseEntity *pNPC, const Vector &vecSrc, const Vector &vecDir );

	// physics interactions
	// mass/size limit set to zero for none
	static bool				CanPickupObject( CBaseEntity *pObject, float massLimit, float sizeLimit );
	virtual void			PickupObject( CBaseEntity *pObject, bool bLimitMassAndSize = true ) {}
	virtual void			ForceDropOfCarriedPhysObjects( CBaseEntity *pOnlyIfHoldindThis = NULL ) {}
	virtual float			GetHeldObjectMass( IPhysicsObject *pHeldObject );

	virtual CBaseEntity		*GetHeldObject( void );
	
	virtual Vector			GetAutoaimVector(void);
	virtual void			GetAimVectors(Vector& forward, Vector& right, Vector& up);

	void					SetViewEntity( CBaseEntity *pEntity );
	CBaseEntity				*GetViewEntity( void ) { return m_hViewEntity; }

	virtual void			ForceClientDllUpdate( void );  // Forces all client .dll specific data to be resent to client.

	virtual void			ProcessUsercmds( CUserCmd *cmds, int numcmds, int totalcmds,
								int dropped_packets, bool paused );
	bool					IsUserCmdDataValid( CUserCmd *pCmd );

	void					AvoidPhysicsProps( CUserCmd *pCmd );

	// Run a user command. The default implementation calls ::PlayerRunCommand. 
	virtual void			PlayerRunCommand(CUserCmd *ucmd, IMoveHelper *moveHelper);
	void					RunNullCommand();
	CUserCmd *				GetCurrentCommand( void )	{ return m_pCurrentCommand; }

	audioparams_t			&GetAudioParams() { return m_Local.m_audio; }

	virtual void			DoMuzzleFlash();
	virtual void			NoteWeaponFired(void);

	virtual bool			IsProned(void) const { return false; }
	virtual bool			IsCrouched(void) const { return false; }
	virtual bool			IsStanding(void) const { return false; }

	virtual void			ChangeTeam(int iTeamID) {}
	virtual int				GetTeamID(void) const { return 0; }
	virtual int				GetTeamNumber(void) const { return GetTeamID(); }

	virtual void			CheckChatText(char* p, int bufsize);

public:
	// Player Physics Shadow
	void					SetupVPhysicsShadow( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, CPhysCollide *pStandModel, const char *pStandHullName, CPhysCollide *pCrouchModel, const char *pCrouchHullName );
	IPhysicsPlayerController* GetPhysicsController() { return m_pPhysicsController; }
	virtual void			VPhysicsCollision( int index, gamevcollisionevent_t *pEvent );
	void					VPhysicsUpdate( IPhysicsObject *pPhysics );
	virtual void			VPhysicsShadowUpdate( IPhysicsObject *pPhysics );
	virtual bool			IsFollowingPhysics( void ) { return false; }
	bool					IsRideablePhysics( IPhysicsObject *pPhysics );
	IPhysicsObject			*GetGroundVPhysics();

	virtual void			Touch( CBaseEntity *pOther );
	void					SetTouchedPhysics( bool bTouch );
	bool					TouchedPhysics( void );
	Vector					GetSmoothedVelocity( void );

	virtual	void			RefreshCollisionBounds( void );
	virtual void			InitVCollision( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity );
	virtual void			VPhysicsDestroyObject();
	void					SetVCollisionState( const Vector &vecAbsOrigin, const Vector &vecAbsVelocity, int collisionState );
	void					PostThinkVPhysics( void );
	virtual void			UpdatePhysicsShadowToCurrentPosition();
	void					UpdatePhysicsShadowToPosition( const Vector &vecAbsOrigin );
	void					UpdateVPhysicsPosition( const Vector &position, const Vector &velocity, float secondsToArrival );

	// Accessor methods
	virtual int	GetStat(int iType) const { return 0; }
	bool	IsConnected() const { return m_bConnected; }
	bool	HUDNeedsRestart() const { return m_fInitHUD; }
	float	MaxSpeed() const		{ return m_flMaxspeed; }
	bool	IsPlayerLockedInPlace() const { return m_iPlayerLocked != 0; }
	bool	IsObserver() const		{ return (m_afPhysicsFlags & PFLAG_OBSERVER) != 0; }
	float	PlayerDrownTime() const	{ return m_AirFinished; }

	int		GetObserverMode() const	{ return m_iObserverMode; }
	CBaseEntity *GetObserverTarget() const	{ return m_hObserverTarget; }

	void	SetConnected(bool bState) { m_bConnected = bState; }
	void	SetMaxSpeed( float flMaxSpeed ) { m_flMaxspeed = flMaxSpeed; }

	void	SetUseEntity( CBaseEntity *pUseEntity );
	CBaseEntity *GetUseEntity();

	virtual void RemoveSpawnProtection(void) { }

	// Used to set private physics flags PFLAG_*
	void	SetPhysicsFlag( int nFlag, bool bSet );

	// For debugging...
	void	ForceOrigin( const Vector &vecOrigin );

	// Bot accessors...
	void	SetTimeBase( float flTimeBase );
	float	GetTimeBase() const;
	void	SetLastUserCommand( const CUserCmd &cmd );
	const CUserCmd *GetLastUserCommand( void );
	
	virtual bool IsBot() const;		// IMPORTANT: This returns true for ANY type of bot. If your game uses different, incompatible types of bots check your specific bot type before casting
	virtual bool IsBotOfType( int botType ) const;	// return true if this player is a bot of the specific type (zero is invalid)
	virtual int GetBotType( void ) const;			// return a unique int representing the type of bot instance this is

	bool	IsPredictingWeapons( void ) const; 
	int		CurrentCommandNumber() const;
	const CUserCmd *GetCurrentUserCommand() const;

	int		GetLockViewanglesTickNumber() const { return m_iLockViewanglesTickNumber; }
	QAngle	GetLockViewanglesData() const { return m_qangLockViewangles; }

	int		GetFOV( void ) const;														// Get the current FOV value
	int		GetDefaultFOV( void ) const;										// Default FOV if not specified otherwise
	bool	SetFOV(int FOV, float zoomRate = 0.0f);								// Alters the base FOV of the player (must have a valid requester)
	void	SetDefaultFOV( int FOV );											// Sets the base FOV if nothing else is affecting it by zooming
	float	GetFOVDistanceAdjustFactor(); // shared between client and server

	int		GetViewmodelFOV(void) const;
	int		GetDefaultViewmodelFOV(void) const;
	void	SetViewmodelFOV(int iFOV, float flZoomRate = 0.0f);

	int		GetScopeFOV(void) const;
	void	SetScopeFOV(int iFOV);

	// Movement constraints
	void	ActivateMovementConstraint( CBaseEntity *pEntity, const Vector &vecCenter, float flRadius, float flConstraintWidth, float flSpeedFactor );
	void	DeactivateMovementConstraint( );

	// talk control
	void NotePlayerTalked() 
	{ 
		m_fLastPlayerTalkTime = gpGlobals->curtime; 
	}
	float	LastTimePlayerTalked() { return m_fLastPlayerTalkTime; }

	void	DisableButtons( int nButtons );
	void	EnableButtons( int nButtons );
	void	ForceButtons( int nButtons );
	void	UnforceButtons( int nButtons );

	//---------------------------------
	// Inputs
	//---------------------------------
	void	InputSetHealth( inputdata_t &inputdata );

	surfacedata_t *GetSurfaceData( void ) { return m_pSurfaceData; }

#if !defined(NO_STEAM)
	//----------------------------
	// Steam handling
	bool		GetSteamID( CSteamID *pID );
	uint64		GetSteamIDAsUInt64( void );
#endif

	float GetRemainingMovementTimeForUserCmdProcessing() const { return m_flMovementTimeForUserCmdProcessingRemaining; }
	float ConsumeMovementTimeForUserCmdProcessing( float flTimeNeeded )
	{
		if ( m_flMovementTimeForUserCmdProcessingRemaining <= 0.0f )
		{
			return 0.0f;
		}
		else if ( flTimeNeeded > m_flMovementTimeForUserCmdProcessingRemaining + FLT_EPSILON )
		{
			float flResult = m_flMovementTimeForUserCmdProcessingRemaining;
			m_flMovementTimeForUserCmdProcessingRemaining = 0.0f;
			return flResult;
		}
		else
		{
			m_flMovementTimeForUserCmdProcessingRemaining -= flTimeNeeded;
			if ( m_flMovementTimeForUserCmdProcessingRemaining < 0.0f )
				m_flMovementTimeForUserCmdProcessingRemaining = 0.0f;
			return flTimeNeeded;
		}
	}

private:
	// How much of a movement time buffer can we process from this user?
	float				m_flMovementTimeForUserCmdProcessingRemaining;

	// For queueing up CUserCmds and running them from PhysicsSimulate
	int					GetCommandContextCount( void ) const;
	CCommandContext		*GetCommandContext( int index );
	CCommandContext		*AllocCommandContext( void );
	void				RemoveCommandContext( int index );
	void				RemoveAllCommandContexts( void );
	CCommandContext		*RemoveAllCommandContextsExceptNewest( void );
	void				ReplaceContextCommands( CCommandContext *ctx, CUserCmd *pCommands, int nCommands );

	int					DetermineSimulationTicks( void );
	void				AdjustPlayerTimeBase( int simulation_ticks );

public:
	
	// Used by gamemovement to check if the entity is stuck.
	int m_StuckLast;
	
	// FIXME: Make these protected or private!

	// This player's data that should only be replicated to 
	//  the player and not to other players.
	CNetworkVarEmbedded( CPlayerLocalData, m_Local );

	void InitFogController( void );
	void InputSetFogController( inputdata_t &inputdata );

	// Used by env_soundscape_triggerable to manage when the player is touching multiple
	// soundscape triggers simultaneously.
	// The one at the HEAD of the list is always the current soundscape for the player.
	CUtlVector<EHANDLE> m_hTriggerSoundscapeList;

	// Player data that's sometimes needed by the engine
	CNetworkVarEmbedded( CPlayerState, pl );

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_fFlags );

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecViewOffset );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_flFriction );
	
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_hGroundEntity );

	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_lifeState );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_iHealth );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED(m_iMaxHealth);
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecBaseVelocity );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_nNextThinkTick );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_vecVelocity );
	IMPLEMENT_NETWORK_VAR_FOR_DERIVED( m_nWaterLevel );
	
	int						m_nButtons;
	int						m_afButtonPressed;
	int						m_afButtonReleased;
	int						m_afButtonLast;
	int						m_afButtonDisabled;	// A mask of input flags that are cleared automatically
	int						m_afButtonForced;	// These are forced onto the player's inputs

	int						m_nUpdateRate;		// user snapshot rate cl_updaterate
	float					m_fLerpTime;		// users cl_interp
	bool					m_bPredictWeapons; //  user has client side predicted weapons

	CNetworkQAngle(m_angEyeAngles);	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar(int, m_iSpawnInterpCounter);

	int m_iLastWeaponFireUsercmd;
	
	float		GetDeathTime( void ) { return m_flDeathTime; }

	void		SetPreviouslyPredictedOrigin( const Vector &vecAbsOrigin );
	const Vector &GetPreviouslyPredictedOrigin() const;

protected:

	void					CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void					CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void					CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles);

	virtual	void			ApplyPlayerView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov) {}
	virtual void			UpdateCollisionBounds(void) {}

	CNetworkHandle( CBaseEntity, m_hUseEntity );			// the player is currently controlling this entity because of +USE latched, NULL if no entity

	int						m_iTrain;				// Train control position

	float					m_iRespawnFrames;	// used in PlayerDeathThink() to make sure players can always respawn
 	unsigned int			m_afPhysicsFlags;	// physics flags - set when 'normal' physics should be revisited or overriden

	void					UpdateButtonState( int nUserCmdButtonMask );

	int						m_lastDamageAmount;		// Last damage taken

	Vector					m_DmgOrigin;
	float					m_DmgTake;

	CNetworkVar( float, m_flDeathTime );		// the time at which the player died  (used in PlayerDeathThink())

	CNetworkVar(int, m_iObserverMode);	// if in spectator mode != 0
	CNetworkVar(int, m_iFOV);			// field of view
	CNetworkVar(int, m_iViewmodelFOV);			// field of view
	
	int						m_iObserverLastMode; // last used observer mode
	CNetworkHandle( CBaseEntity, m_hObserverTarget );	// entity handle to m_iObserverTarget
	bool					m_bForcedObserverMode; // true, player was forced by invalid targets to switch mode

	// Player Physics Shadow
	int						m_vphysicsCollisionState;

	// Replay mode	
	float					m_fDelay;			// replay delay in seconds
	float					m_fReplayEnd;		// time to stop replay mode
	int						m_iReplayEntity;	// follow this entity in replay

private:
	void HandleFuncTrain();

// DATA
private:
	CUtlVector< CCommandContext > m_CommandContext;
	// Player Physics Shadow

protected: //used to be private, but need access for portal mod (Dave Kircher)
	IPhysicsPlayerController	*m_pPhysicsController;
	IPhysicsObject				*m_pShadowStand;
	IPhysicsObject				*m_pShadowCrouch;
	Vector						m_oldOrigin;
	Vector						m_vecSmoothedVelocity;
	bool						m_touchedPhysObject;
	bool						m_bPhysicsWasFrozen;

private:

	int						m_iPlayerSound;// the index of the sound list slot reserved for this player
	int						m_iTargetVolume;// ideal sound volume. 

	// these are time-sensitive things that we keep track of
	float					m_flSwimTime;		// how long player has been underwater

	bool					m_fInitHUD;				// True when deferred HUD restart msg needs to be sent
	bool					m_fGameHUDInitialized;

	// Multiplayer handling
	bool					m_bConnected;		// True if the player's connected

	float					m_AirFinished;
	float					m_PainFinished;

	// player locking
	int						m_iPlayerLocked;
		
protected:
	typedef CHandle<CBaseViewModel> CBaseViewModelHandle;
	CNetworkVar(CBaseViewModelHandle, m_hViewModel);

	// Last received usercmd (in case we drop a lot of packets )
	CUserCmd				m_LastCmd;
	CUserCmd				*m_pCurrentCommand;

	int						m_iLockViewanglesTickNumber;
	QAngle					m_qangLockViewangles;

	float					m_flStepSoundTime;	// time to check for next footstep sound

private:

// Replicated to all clients
	CNetworkVar( float, m_flMaxspeed );
	
// Not transmitted
	float					m_flWaterJumpTime;  // used to be called teleport_time
	Vector					m_vecWaterJumpVel;
	float					m_flSwimSoundTime;
	Vector					m_vecLadderNormal;

	int						m_nDrownDmgRate;		// Drowning damage in points per second without air.

public:
	float					m_flForwardMove;
	float					m_flSideMove;

private:

	// Used in test code to teleport the player to random locations in the map.
	Vector					m_vForcedOrigin;
	bool					m_bForceOrigin;	

	// Clients try to run on their own realtime clock, this is this client's clock
	CNetworkVar( int, m_nTickBase );

	bool					m_bGamePaused;
	float					m_fLastPlayerTalkTime;
	
	CNetworkVar( CBaseCombatWeaponHandle, m_hLastWeapon );

#if !defined( NO_ENTITY_PREDICTION )
	CUtlVector< CHandle< CBaseEntity > > m_SimulatedByThisPlayer;
#endif

	float					m_flOldPlayerZ;
	float					m_flOldPlayerViewOffsetZ;

	bool					m_bPlayerUnderwater;

	EHANDLE					m_hViewEntity;

	// Movement constraints
	CNetworkHandle( CBaseEntity, m_hConstraintEntity );
	CNetworkVector( m_vecConstraintCenter );
	CNetworkVar( float, m_flConstraintRadius );
	CNetworkVar( float, m_flConstraintWidth );
	CNetworkVar( float, m_flConstraintSpeedFactor );

	friend class CPlayerMove;
	friend class CPlayerClass;

	// Player name
	char					m_szNetname[MAX_PLAYER_NAME_LENGTH];

protected:
	friend class CGameMovement;
	friend class CINSGameMovement;
	
	// Accessors for gamemovement
	float GetStepSize( void ) const { return m_Local.m_flStepSize; }

	CNetworkVar(float, m_flLaggedMovementValue);

	// These are generated while running usercmds, then given to UpdateVPhysicsPosition after running all queued commands.
	Vector m_vNewVPhysicsPosition;
	Vector m_vNewVPhysicsVelocity;

	Vector m_vecPreviouslyPredictedOrigin; // Used to determine if non-gamemovement game code has teleported, or tweaked the player's origin
	int		m_nBodyPitchPoseParam;

	char m_szNetworkIDString[MAX_NETWORKID_LENGTH];
	CPlayerInfo m_PlayerInfo;

	// Texture names and surface data, used by CGameMovement
	int				m_surfaceProps;
	surfacedata_t*	m_pSurfaceData;
	float			m_surfaceFriction;
	char			m_chTextureType;
	char			m_chPreviousTextureType;	// Separate from m_chTextureType. This is cleared if the player's not on the ground.

	int m_iDoorTransitionIndex;
	float m_flLastTransitionUseTime;

	CNetworkHandle(CBaseCombatWeapon, m_hNextActiveWeapon);
	CNetworkVar(float, m_flNextActiveWeapon);

public:

	float  GetLaggedMovementValue( void ){ return m_flLaggedMovementValue;	}
	void   SetLaggedMovementValue( float flValue ) { m_flLaggedMovementValue = flValue;	}

	inline bool IsAutoKickDisabled(void) const;
	inline void DisableAutoKick(bool disabled);

	void	SetDoorTransition(int index) { m_iDoorTransitionIndex = index; }
	int		GetDoorTransition(void) { return m_iDoorTransitionIndex; }

	void	UpdateLastUsedTransitionTime(void) { m_flLastTransitionUseTime = gpGlobals->curtime; }
	float	GetLastUsedTransitionTime(void) const { return m_flLastTransitionUseTime; }

private:
	bool m_autoKickDisabled;

	struct StepSoundCache_t
	{
		StepSoundCache_t() : m_usSoundNameIndex( 0 ) {}
		CSoundParameters	m_SoundParameters;
		unsigned short		m_usSoundNameIndex;
	};
	// One for left and one for right side of step
	StepSoundCache_t		m_StepSoundCache[ 2 ];

	// Store the last time we successfully processed a usercommand
	float			m_flLastUserCommandTime;

public:
	virtual unsigned int PlayerSolidMask( bool brushOnly = false ) const;	// returns the solid mask for the given player, so bots can have a more-restrictive set

protected: // steam stats & achievs logic
	friend bool AchievementManager::CanLoadSteamStats(CBasePlayer* pPlayer);

	void OnReceivedSteamStats(GSStatsReceived_t* pCallback, bool bIOFailure);
	CCallResult<CBasePlayer, GSStatsReceived_t> m_SteamCallResultRequestPlayerStats;
	bool m_bHasLoadedStats;
	unsigned long long m_ullCachedSteamID;

public:
	bool HasLoadedStats(void) { return m_bHasLoadedStats; }
};

typedef CHandle<CBasePlayer> CBasePlayerHandle;

EXTERN_SEND_TABLE(DT_BasePlayer)

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline bool CBasePlayer::IsBotOfType( int botType ) const
{
	// bot type of zero is invalid
	return ( GetBotType() != 0 ) && ( GetBotType() == botType );
}

inline int CBasePlayer::GetBotType( void ) const
{
	return 0;
}

inline bool CBasePlayer::IsAutoKickDisabled( void ) const
{
	return m_autoKickDisabled;
}

inline void CBasePlayer::DisableAutoKick( bool disabled )
{
	m_autoKickDisabled = disabled;
}

inline void CBasePlayer::SetUseEntity( CBaseEntity *pUseEntity ) 
{ 
	m_hUseEntity = pUseEntity; 
}

inline CBaseEntity *CBasePlayer::GetUseEntity() 
{ 
	return m_hUseEntity;
}

// Bot accessors...
inline void CBasePlayer::SetTimeBase( float flTimeBase ) 
{ 
	m_nTickBase = TIME_TO_TICKS( flTimeBase ); 
}

inline void CBasePlayer::SetLastUserCommand( const CUserCmd &cmd ) 
{ 
	m_LastCmd = cmd; 
}

inline CUserCmd const *CBasePlayer::GetLastUserCommand( void )
{
	return &m_LastCmd;
}

inline bool CBasePlayer::IsPredictingWeapons( void ) const 
{
	return m_bPredictWeapons;
}

inline int CBasePlayer::CurrentCommandNumber() const
{
	Assert( m_pCurrentCommand );
	return m_pCurrentCommand->command_number;
}

inline const CUserCmd *CBasePlayer::GetCurrentUserCommand() const
{
	Assert( m_pCurrentCommand );
	return m_pCurrentCommand;
}

inline void CBasePlayer::SetTouchedPhysics( bool bTouch ) 
{ 
	m_touchedPhysObject = bTouch; 
}

inline bool CBasePlayer::TouchedPhysics( void )			
{ 
	return m_touchedPhysObject; 
}

//-----------------------------------------------------------------------------
// Converts an entity to a player
//-----------------------------------------------------------------------------
inline CBasePlayer *ToBasePlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;
#if _DEBUG
	return dynamic_cast<CBasePlayer *>( pEntity );
#else
	return static_cast<CBasePlayer *>( pEntity );
#endif
}

inline const CBasePlayer *ToBasePlayer( const CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;
#if _DEBUG
	return dynamic_cast<const CBasePlayer *>( pEntity );
#else
	return static_cast<const CBasePlayer *>( pEntity );
#endif
}

//--------------------------------------------------------------------------------------------------------------
/**
 * DEPRECATED: Use CollectPlayers() instead.
 * Iterate over all active players in the game, invoking functor on each.
 * If functor returns false, stop iteration and return false.
 */
template < typename Functor >
bool ForEachPlayer( Functor &func )
{
	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (FNullEnt( player->edict() ))
			continue;

		if (!player->IsPlayer())
			continue;

		if( !player->IsConnected() )
			continue;

		if (func( player ) == false)
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------------------------
/**
 * The interface for an iterative player functor
 */
class IPlayerFunctor
{
public:
	virtual void OnBeginIteration( void )						{ }		// invoked once before iteration begins
	
	virtual bool operator() ( CBasePlayer *player ) = 0;
	
	virtual void OnEndIteration( bool allElementsIterated )		{ }		// invoked once after iteration is complete whether successful or not
};

//--------------------------------------------------------------------------------------------------------------
/**
 * DEPRECATED: Use CollectPlayers() instead.
 * Specialization of ForEachPlayer template for IPlayerFunctors
 */
template <>
inline bool ForEachPlayer( IPlayerFunctor &func )
{
	func.OnBeginIteration();
	
	bool isComplete = true;
	
	for( int i=1; i<=gpGlobals->maxClients; ++i )
	{
		CBasePlayer *player = static_cast<CBasePlayer *>( UTIL_PlayerByIndex( i ) );

		if (player == NULL)
			continue;

		if (FNullEnt( player->edict() ))
			continue;

		if (!player->IsPlayer())
			continue;

		if( !player->IsConnected() )
			continue;

		if (func( player ) == false)
		{
			isComplete = false;
			break;
		}
	}
	
	func.OnEndIteration( isComplete );

	return isComplete;
}

#endif // PLAYER_H