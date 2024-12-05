//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client-side CBasePlayer.
//
//			- Manages the player's flashlight effect.
//
//=============================================================================//

#ifndef C_BASEPLAYER_H
#define C_BASEPLAYER_H
#ifdef _WIN32
#pragma once
#endif

#include "c_playerlocaldata.h"
#include "c_baseflex.h"
#include "playerstate.h"
#include "usercmd.h"
#include "shareddefs.h"
#include "timedevent.h"
#include "smartptr.h"
#include "fx_water.h"
#include "color.h"

class C_BaseCombatWeapon;
class C_BaseViewModel;
class C_FuncLadder;
class CFlashlightEffect;

extern int g_nKillCamMode;
extern int g_nKillCamTarget1;
extern int g_nKillCamTarget2;
extern int g_nUsedPrediction; 

class C_CommandContext
{
public:
	bool			needsprocessing;

	CUserCmd		cmd;
	int				command_number;
};

class C_PredictionError
{
public:
	float	time;
	Vector	error;
};

#define CHASE_CAM_DISTANCE		96.0f
#define WALL_OFFSET				6.0f

//-----------------------------------------------------------------------------
// Purpose: Base Player class
//-----------------------------------------------------------------------------
class C_BasePlayer : public C_BaseFlex
{
public:
	DECLARE_CLASS( C_BasePlayer, C_BaseFlex );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_BasePlayer();
	virtual			~C_BasePlayer();

	virtual void	Spawn( void );
	virtual void	SharedSpawn(); // Shared between client and server.

	// IClientEntity overrides.
	// Pongles [
	//virtual void	OnPreDataChanged( DataUpdateType_t updateType );
	virtual void	OnDataChanged( DataUpdateType_t updateType );
	// Pongles ]

	virtual void	PostDataUpdate( DataUpdateType_t updateType );
	
	virtual void	ReceiveMessage( int classID, bf_read &msg );

	virtual void	OnRestore();

	virtual void	AddEntity( void );

	virtual void	GetToolRecordingState( KeyValues *msg );

	void	SetAnimationExtension( const char *pExtension );

	C_BaseViewModel		*GetViewModel( void );
	C_BaseCombatWeapon	*GetActiveWeapon( void ) const;
	int					WeaponCount() const;
	C_BaseCombatWeapon	*GetWeapon( int i ) const;

	float				GetNextAttack() const { return m_flNextAttack; }
	void				SetNextAttack( float flWait ) { m_flNextAttack = flWait; }

	// View model prediction setup
	virtual void		CalcView( Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov );
	virtual void		CalcViewModelView( const Vector& eyeOrigin, const QAngle& eyeAngles );

	// Handle view smoothing when going up stairs
	void				SmoothViewOnStairs( Vector& eyeOrigin );
    virtual void		CalcViewRoll( QAngle& eyeAngles );
	void				CreateWaterEffects( void );

	virtual Vector			Weapon_ShootPosition();
	virtual Vector			Weapon_ShootDirection();

	// Input handling
	virtual void	CreateMove( float flInputSampleTime, CUserCmd *pCmd );
	virtual void	AvoidPhysicsProps( CUserCmd *pCmd );
	
	virtual void	PlayerUse( void );
	CBaseEntity		*FindUseEntity( void );
	virtual bool	IsUseableEntity( CBaseEntity *pEntity, unsigned int requiredCaps );

	// Data handlers
	virtual bool	IsPlayer( void ) const { return true; };
	bool IsBot( void ) const { return false; }

	virtual int		GetHealth() const;

	// observer mode
	virtual int			GetObserverMode() const;
	virtual CBaseEntity	*GetObserverTarget() const;
	void			SetObserverTarget( EHANDLE hObserverTarget );

	bool IsObserver() const;
	bool IsHLTV() const;
	void ResetObserverMode();

	// Eye position..
	virtual Vector		 EyePosition();
	virtual const QAngle &EyeAngles();		// Direction of eyes
	void				 EyePositionAndVectors( Vector *pPosition, Vector *pForward, Vector *pRight, Vector *pUp );
	virtual const QAngle &LocalEyeAngles();		// Direction of eyes

	// This can be overridden to return something other than m_pRagdoll if the mod uses separate 
	// entities for ragdolls.
	virtual IRagdoll* GetRepresentativeRagdoll() const;
	virtual CBaseAnimating *GetRagdollEntity() const;

	// Returns eye vectors
	void			EyeVectors( Vector *pForward, Vector *pRight = NULL, Vector *pUp = NULL );

	// Flashlight
	void	Flashlight( void );
	void	UpdateFlashlight( void );

	// Weapon selection code
	virtual bool				IsAllowedToSwitchWeapons( void ) { return !IsObserver(); }
	virtual C_BaseCombatWeapon	*GetActiveWeaponForSelection( void );

	// Returns the view model if this is the local player. If you're in third person or 
	// this is a remote player, it returns the active weapon
	virtual C_BaseAnimating*	GetRenderedWeaponModel();

	virtual bool				IsOverridingViewmodel( void ) { return false; };
	virtual int					DrawOverriddenViewmodel( C_BaseViewModel *pViewmodel, int flags ) { return 0; };

	virtual float				GetDefaultAnimSpeed( void ) { return 1.0; }

	void						SetMaxSpeed( float flMaxSpeed ) { m_flMaxspeed = flMaxSpeed; }
	float						MaxSpeed() const		{ return m_flMaxspeed; }

	// Should this object cast shadows?
	virtual ShadowType_t		ShadowCastType();

	virtual void CreateLightEffects( void ) {}
	virtual bool ShouldReceiveProjectedTextures( int flags );

	bool						IsLocalPlayer( void ) const;

	// Global/static methods
	static bool					ShouldDrawLocalPlayer();
	static C_BasePlayer			*GetLocalPlayer( void );
	int							GetUserID( void );

	// Called by the view model if its rendering is being overridden.
	virtual bool		ViewModel_IsTransparent( void );

#if !defined( NO_ENTITY_PREDICTION )
	void						AddToPlayerSimulationList( C_BaseEntity *other );
	void						SimulatePlayerSimulatedEntities( void );
	void						RemoveFromPlayerSimulationList( C_BaseEntity *ent );
	void						ClearPlayerSimulationList( void );
#endif

	virtual void				PhysicsSimulate( void );
	virtual unsigned int	PhysicsSolidMaskForEntity( void ) const { return MASK_PLAYERSOLID; }

	// Prediction stuff
	virtual bool				ShouldPredict( void );

	virtual void				PreThink( void );
	virtual void				PostThink( void );

	virtual void				ItemPreFrame( void );
	virtual void				ItemPostFrame( void );
	virtual void				AbortReload( void );

	virtual void				SelectLastItem(void);
	virtual void				Weapon_SetLast( C_BaseCombatWeapon *pWeapon );
	virtual bool				Weapon_ShouldSetLast( C_BaseCombatWeapon *pOldWeapon, C_BaseCombatWeapon *pNewWeapon ) { return true; }
	virtual bool				Weapon_ShouldSelectItem( C_BaseCombatWeapon *pWeapon );
	virtual C_BaseCombatWeapon *GetLastWeapon( void ) { return m_hLastWeapon.Get(); }
	C_BaseCombatWeapon*			Weapon_Owns( int iWeaponID ) const;  // True if already owns a weapon of this class
	virtual	bool				Weapon_Switch( C_BaseCombatWeapon *pWeapon, bool bForce = false );
	virtual	void				Weapon_SwitchToNext( void );
	virtual bool				Weapon_CanSwitchTo(C_BaseCombatWeapon *pWeapon);
	
	// I can't use my current weapon anymore. Switch me to the next best weapon.
	bool SwitchToNextBestWeapon(C_BaseCombatWeapon *pCurrent);

	// I can't use this weapon anymore, get me the next best one.
	C_BaseCombatWeapon *GetNextBestWeapon( C_BaseCombatWeapon *pCurrentWeapon );

	virtual void 				SelectItem( int iWeaponID );

	virtual int			BloodColor();

	virtual void				UpdateClientData( void );

	float						GetFOV( void );	
	int							GetDefaultFOV( void ) const;
  
    // xENO [

    virtual float				GetViewmodelFOV( void );
	int							GetDefaultViewmodelFOV( void ) const;

	int							GetScopeFOV( void ) const;

    // ] xENO

	// Pongles [

	virtual bool				HasZoom( void )		{ return false; }
	virtual bool				IsZoomed( void )	{ return false; }
	virtual bool				ActiveZoom( void )	{ return false; }

	// Pongles ]

	float						GetFOVDistanceAdjustFactor();

	virtual void				ViewPunch( const QAngle &angleOffset );
	void						ViewPunchReset( float tolerance = 0 );
	virtual void				RecoilViewPunch( const QAngle &angleOffset );
	void						RecoilViewPunchReset( float tolerance = 0 );

	void						UpdateButtonState( int nUserCmdButtonMask );
	int							GetImpulse( void ) const;

	virtual void				Simulate();

	virtual bool				ShouldInterpolate();

	virtual bool				ShouldDraw();
	virtual int					DrawModel( int flags );

	// Called when not in tactical mode. Allows view to be overriden for things like driving a tank.
	virtual void				OverrideView( CViewSetup *pSetup );

	// returns the player name
	const char *				GetPlayerName();
	virtual const Vector		GetPlayerMins( void ) const;
	virtual const Vector		GetPlayerMaxs( void ) const;

	// Is the player dead?
	bool				IsPlayerDead();

	// Vehicles...
	IClientVehicle			*GetVehicle();

	bool			IsInAVehicle() const	{ return ( NULL != m_hVehicle.Get() ) ? true : false; }
	virtual void	SetVehicleRole( int nRole );
	void					LeaveVehicle( void );

	bool					UsingStandardWeaponsInVehicle( void );

	virtual void			SetAnimation( PLAYER_ANIM playerAnim );

	float					GetTimeBase( void ) const;
	float					GetFinalPredictedTime() const;

	bool					IsInVGuiInputMode() const;

	C_CommandContext		*GetCommandContext();

	// Get the command number associated with the current usercmd we're running (if in predicted code).
	int CurrentCommandNumber() const;
	const CUserCmd *GetCurrentUserCommand() const;

	const QAngle& GetPunchAngle();
	void SetPunchAngle( const QAngle &angle );

	const QAngle& GetRecoilPunchAngle();
	void SetRecoilPunchAngle( const QAngle &angle );

	float					GetWaterJumpTime() const;
	void					SetWaterJumpTime( float flWaterJumpTime );
	float					GetSwimSoundTime( void ) const;
	void					SetSwimSoundTime( float flSwimSoundTime );

	virtual void DoMuzzleFlash();

	virtual void UpdateStepSound( surfacedata_t *psurface, const Vector &vecOrigin, const Vector &vecVelocity );
	virtual void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );
	virtual surfacedata_t * GetFootstepSurface( const Vector &origin, const char *surfaceName );

	// Called by prediction when it detects a prediction correction.
	// vDelta is the line from where the client had predicted the player to at the usercmd in question,
	// to where the server says the client should be at said usercmd.
	void NotePredictionError( const Vector &vDelta );

	// Called by the renderer to apply the prediction error smoothing.
	void GetPredictionErrorSmoothingVector( Vector &vOffset ); 

	virtual void ExitLadder() {}

	surfacedata_t *GetSurfaceData( void ) { return m_pSurfaceData; }

	void SetLadderNormal( Vector vecLadderNormal ) { m_vecLadderNormal = vecLadderNormal; }

	// eye handling
	QAngle GetAnimEyeAngles( void ) { return m_angEyeAngles; }

	void SnapEyeAngles(QAngle &Angle, int iFixAngle = FIXANGLE_RELATIVE);

	// Pongles [

	// HACKHACK: pass thru's for CINSPlayer
	virtual bool IsProned(void) const { return false; }
	virtual bool IsCrouched(void) const { return false; }
	virtual bool IsStanding(void) const { return false; }
	virtual bool IsMoving(void) const { return false; }

	virtual int GetTeamID(void) const { return 0; }

	virtual void SetFreeAimAngles(const QAngle &angAngles) { }
	virtual const QAngle &GetFreeAimAngles(void) const { return vec3_angle; }

	virtual const QAngle &GetSupportedFireAngles(void) const  { return vec3_angle; }

	virtual bool IsFreeaimEnabled(void) const { return false; }
	virtual float GetFreeaimDistance( void ) const { return 0.0f; }
	virtual float GetFreeaimScreenWeaponRelation(void) const { return 0.0f; }
	virtual bool UseFreeaimSWRFraction(void) const { return false; }

	virtual bool IsSprinting(void) const { return false; }

	// Pongles ]

public:
	int m_StuckLast;
	
	// Data for only the local player
	CNetworkVarEmbedded( CPlayerLocalData, m_Local );

	// Data common to all other players, too
	CPlayerState			pl;

	// Player FOV values
	int						m_iFOV;			 	    // field of view
	int						m_iFOVStart;	  	    // starting value of the FOV changing over time (client only)
	float					m_flFOVTime;	    	// starting time of the FOV zoom

    // Viewmodel FOV values
    int                     m_iViewmodelFOV;
    int                     m_iViewmodelFOVStart;
    float                   m_flViewmodelFOVTime;


	char			m_szAnimExtension[32];

	int				m_afButtonLast;
	int				m_afButtonPressed;
	int				m_afButtonReleased;

	int				m_nButtons;

	CUserCmd		*m_pCurrentCommand;

	// Movement constraints
	EHANDLE			m_hConstraintEntity;
	Vector			m_vecConstraintCenter;
	float			m_flConstraintRadius;
	float			m_flConstraintWidth;
	float			m_flConstraintSpeedFactor;

protected:
	
	void				CalcPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void				CalcVehicleView( IClientVehicle *pVehicle, Vector& eyeOrigin, QAngle& eyeAngles, float& zNear, float& zFar, float& fov );
	void				CalcObserverView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void				CalcChaseCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void				CalcInEyeCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	virtual void		CalcDeathCamView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov );
	void				CalcRoamingView(Vector& eyeOrigin, QAngle& eyeAngles, float& fov);

	virtual void		ApplyPlayerView( Vector& eyeOrigin, QAngle& eyeAngles, float& fov ) { }

	// Check to see if we're in vgui input mode...
	void DetermineVguiInputMode( CUserCmd *pCmd );

	// Used by prediction, sets the view angles for the player
	void SetLocalViewAngles( const QAngle &viewAngles );


	// used by client side player footsteps 
	surfacedata_t* GetGroundSurface();

protected:

	// Did we just enter a vehicle this frame?
	bool			JustEnteredVehicle();

// DATA
	int				m_iObserverMode;	// if in spectator mode != 0
	EHANDLE			m_hObserverTarget;	// current observer target
	float			m_flObserverChaseDistance; // last distance to observer traget
	float			m_flDeathTime;		// last time player died

	float			m_flStepSoundTime;

	// eye angle management
	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

private:
	// Make sure no one calls this...
	C_BasePlayer& operator=( const C_BasePlayer& src );
	C_BasePlayer( const C_BasePlayer & ); // not defined, not accessible

	// Vehicle stuff.
	EHANDLE			m_hVehicle;
	EHANDLE			m_hOldVehicle;
	EHANDLE			m_hUseEntity;
	
	float			m_flMaxspeed;

	CInterpolatedVar< Vector >	m_iv_vecViewOffset;

	// Not replicated
	Vector			m_vecWaterJumpVel;
	float			m_flWaterJumpTime;  // used to be called teleport_time
	int				m_nImpulse;

	float			m_flSwimSoundTime;
	Vector			m_vecLadderNormal;
	
	QAngle			m_vecOldViewAngles;

	bool			m_bWasFrozen;
	int				m_flPhysics;

	int				m_nTickBase;
	int				m_nFinalPredictedTick;

	EHANDLE			m_pCurrentVguiScreen;

	// Player flashlight dynamic light pointers
	CFlashlightEffect *m_pFlashlight;

	typedef CHandle<C_BaseCombatWeapon> CBaseCombatWeaponHandle;
	CNetworkVar( CBaseCombatWeaponHandle, m_hLastWeapon );

#if !defined( NO_ENTITY_PREDICTION )
	CUtlVector< CHandle< C_BaseEntity > > m_SimulatedByThisPlayer;
#endif

	// players own view models, left & right hand
	CHandle< C_BaseViewModel >	m_hViewModel;
	// view models of other player how is spectated in eyes
	CHandle< C_BaseViewModel >	m_hObserverViewModel;

	float					m_flOldPlayerZ;
	float					m_flOldPlayerViewOffsetZ;
	
	C_CommandContext		m_CommandContext;

	// For underwater effects
	float							m_flWaterSurfaceZ;
	bool							m_bResampleWaterSurface;
	TimedEvent						m_tWaterParticleTimer;
	CSmartPtr<WaterDebrisEffect>	m_pWaterEmitter;

	friend class CPrediction;

	friend class CGameMovement;
	friend class CINSGameMovement;

	// Accessors for gamemovement
	float GetStepSize( void ) const { return m_Local.m_flStepSize; }

	float m_flNextAvoidanceTime;
	float m_flAvoidanceRight;
	float m_flAvoidanceForward;
	float m_flAvoidanceDotForward;
	float m_flAvoidanceDotRight;

protected:
	/*virtual bool IsDucked( void ) const { return m_Local.m_bDucked; }
	virtual bool IsDucking( void ) const { return m_Local.m_bDucking; }*/
	virtual float GetFallVelocity( void ) { return m_Local.m_flFallVelocity; }

	float m_flLaggedMovementValue;

	// These are used to smooth out prediction corrections. They're most useful when colliding with
	// vphysics objects. The server will be sending constant prediction corrections, and these can help
	// the errors not be so jerky.
	Vector m_vecPredictionError;
	float m_flPredictionErrorTime;
	char m_szLastPlaceName[16];	// received from the server

	// Texture names and surface data, used by CGameMovement
	int				m_surfaceProps;
	surfacedata_t*	m_pSurfaceData;
	float			m_surfaceFriction;
	char			m_chTextureType;

	// spawn interp stuff
	int	  m_iSpawnInterpCounter;
	int	  m_iSpawnInterpCounterCache;

	float m_flNextAttack;

	CHandle< C_BaseCombatWeapon > m_hNextActiveWeapon;
	float m_flNextActiveWeapon;

	CHandle<C_BaseCombatWeapon>		m_hMyWeapons[MAX_PWEAPONS];
	CHandle< C_BaseCombatWeapon > m_hActiveWeapon;

public:

	float GetLaggedMovementValue( void ){ return m_flLaggedMovementValue; }
	bool  ShouldGoSouth( Vector vNPCForward, Vector vNPCRight ); //Such a bad name.

	void SetOldPlayerZ( float flOld ) { m_flOldPlayerZ = flOld;	}
};

EXTERN_RECV_TABLE(DT_BasePlayer);

//-----------------------------------------------------------------------------
// Inline methods
//-----------------------------------------------------------------------------
inline C_BasePlayer *ToBasePlayer( C_BaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#if _DEBUG
	Assert( dynamic_cast<C_BasePlayer *>( pEntity ) != NULL );
#endif

	return static_cast<C_BasePlayer *>( pEntity );
}

inline IClientVehicle *C_BasePlayer::GetVehicle() 
{ 
	C_BaseEntity *pVehicleEnt = m_hVehicle.Get();
	return pVehicleEnt ? pVehicleEnt->GetClientVehicle() : NULL;
}

inline bool C_BasePlayer::IsObserver() const 
{ 
	return (GetObserverMode() != OBS_MODE_NONE); 
}

inline int C_BasePlayer::GetImpulse( void ) const 
{ 
	return m_nImpulse; 
}

inline C_CommandContext* C_BasePlayer::GetCommandContext()
{
	return &m_CommandContext;
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

#endif // C_BASEPLAYER_H
