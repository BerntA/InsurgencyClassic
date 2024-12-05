//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Definitions that are shared by the game DLL and the client DLL.
//
// $NoKeywords: $
//=============================================================================//

#ifndef SHAREDDEFS_H
#define SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_shared_global.h"

//=========================================================
//=========================================================
#define TICK_INTERVAL			(gpGlobals->interval_per_tick)	

#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )
#define ROUND_TO_TICKS( t )		( TICK_INTERVAL * TIME_TO_TICKS( t ) )
#define TICK_NEVER_THINK		(-1)

#define ANIMATION_CYCLE_BITS		15
#define ANIMATION_CYCLE_MINFRAC		(1.0f / (1<<ANIMATION_CYCLE_BITS))

//=========================================================
//=========================================================
class CViewVectors
{
public:
	CViewVectors() {}

	CViewVectors( 
		Vector vHullMin,
		Vector vHullMax,
		Vector vView,
		Vector vDuckHullMin,
		Vector vDuckHullMax,
		Vector vDuckView,
		Vector vProneHullMin,
		Vector vProneHullMax,
		Vector vProneView,
		Vector vObsHullMin,
		Vector vObsHullMax,
		Vector vDeadViewHeight )
	{
		m_vHullMin = vHullMin;
		m_vHullMax = vHullMax;
		m_vView = vView;
		m_vDuckHullMin = vDuckHullMin;
		m_vDuckHullMax = vDuckHullMax;
		m_vDuckView = vDuckView;
		m_vProneHullMin = vProneHullMin;
		m_vProneHullMax = vProneHullMax;
		m_vProneView = vProneView;
		m_vObsHullMin = vObsHullMin;
		m_vObsHullMax = vObsHullMax;
		m_vDeadViewHeight = vDeadViewHeight;
	}

	Vector m_vHullMin;
	Vector m_vHullMax;
	Vector m_vView;
	
	Vector m_vDuckHullMin;
	Vector m_vDuckHullMax;
	Vector m_vDuckView;

	Vector m_vProneHullMin;
	Vector m_vProneHullMax;
	Vector m_vProneView;
	
	Vector m_vObsHullMin;
	Vector m_vObsHullMax;
	
	Vector m_vDeadViewHeight;
};

#define VEC_VIEW				g_pGameRules->GetViewVectors()->m_vView
#define VEC_HULL_MIN			g_pGameRules->GetViewVectors()->m_vHullMin
#define VEC_HULL_MAX			g_pGameRules->GetViewVectors()->m_vHullMax

#define VEC_DUCK_HULL_MIN		g_pGameRules->GetViewVectors()->m_vDuckHullMin
#define VEC_DUCK_HULL_MAX		g_pGameRules->GetViewVectors()->m_vDuckHullMax
#define VEC_DUCK_VIEW			g_pGameRules->GetViewVectors()->m_vDuckView

#define VEC_PRONE_HULL_MIN		g_pGameRules->GetViewVectors()->m_vProneHullMin
#define VEC_PRONE_HULL_MAX		g_pGameRules->GetViewVectors()->m_vProneHullMax
#define VEC_PRONE_VIEW			g_pGameRules->GetViewVectors()->m_vProneView
#define VEC_PRONE_DIRECT_VIEW	g_pGameRules->GetViewVectors()->m_vProneView

#define VEC_OBS_HULL_MIN		g_pGameRules->GetViewVectors()->m_vObsHullMin
#define VEC_OBS_HULL_MAX		g_pGameRules->GetViewVectors()->m_vObsHullMax

#define VEC_DEAD_VIEWHEIGHT		g_pGameRules->GetViewVectors()->m_vDeadViewHeight

//=========================================================
//=========================================================
#define WATERJUMP_HEIGHT 8

#define MAX_CLIMB_SPEED 200

//=========================================================
//=========================================================
#define HUD_PRINTNOTIFY		1
#define HUD_PRINTCONSOLE	2
#define HUD_PRINTTALK		3
#define HUD_PRINTCENTER		4

//=========================================================
//=========================================================

// NOTE: max number of players in a game ( see const.h for ABSOLUTE_PLAYER_LIMIT )
#define MAX_PLAYERS				MAX_STATS_PLAYERS

//=========================================================
//=========================================================
#define MAX_PLACE_NAME_LENGTH	18

//=========================================================
//=========================================================
#define INVALID_TEAM			-1

enum TeamList_t
{
	TEAM_UNASSIGNED = 0,	// not assigned to a team
	TEAM_ONE,				// team one
	TEAM_TWO,				// team two
	TEAM_SPECTATOR,			// spectator team
	MAX_TEAMS				// max teams
};

#define MAX_TEAM_NAME_LENGTH 18

//=========================================================
//=========================================================
enum WeaponStatus_t
{
	WEAPON_NOT_CARRIED = 0,			// weapon is on the ground
	WEAPON_IS_CARRIED_BY_PLAYER,	// this client is carrying this weapon
	WEAPON_IS_ACTIVE				// this client is carrying this weapon and it's the currently held weapon
};

//=========================================================
//=========================================================
#define MUZZLEFLASH_FIRSTPERSON 0x80

//=========================================================
//=========================================================
#define MAX_BEAM_ENTS			10

//=========================================================
//=========================================================
enum
{
	ENTITY_DISSOLVE_NORMAL = 0,
	ENTITY_DISSOLVE_ELECTRICAL,
	ENTITY_DISSOLVE_ELECTRICAL_LIGHT,
	ENTITY_DISSOLVE_CORE,

	// NOTE: Be sure to up the bits if you make more dissolve types
	ENTITY_DISSOLVE_BITS = 2
};

//=========================================================
//=========================================================
enum HitGroups_t
{
	HITGROUP_GENERIC = 0,
	HITGROUP_HEAD,
	HITGROUP_NECK,
	HITGROUP_SPINE,
	HITGROUP_PELVIS,
	HITGROUP_LEFTTHIGH,
	HITGROUP_RIGHTTHIGH,
	HITGROUP_LEFTCALF,
	HITGROUP_RIGHTCALF,
	HITGROUP_LEFTFOOT,
	HITGROUP_RIGHTFOOT,
	HITGROUP_LEFTUPPERARM,
	HITGROUP_RIGHTUPPERARM,
	HITGROUP_LEFTFOREARM,
	HITGROUP_RIGHTFOREARM,
	HIRGROUP_COUNT
};

//=========================================================
//=========================================================
enum PLAYER_ANIM
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	PLAYER_IN_VEHICLE,

	// TF Player animations
	PLAYER_RELOAD,
	PLAYER_START_AIMING,
	PLAYER_LEAVE_AIMING,
};

//=========================================================
//=========================================================
#define PLAYER_FATAL_FALL_SPEED		1024 // approx 60 feet
#define PLAYER_MAX_SAFE_FALL_SPEED	580 // approx 20 feet
#define PLAYER_LAND_ON_FLOATING_OBJECT	200 // Can go another 200 units without getting hurt
#define PLAYER_MIN_BOUNCE_SPEED		200
#define PLAYER_FALL_PUNCH_THRESHOLD (float)350 // won't punch player's screen/make scrape noise unless player falling at least this fast.
#define DAMAGE_FOR_FALL_SPEED		100.0f / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED ) // damage per unit per second.

//=========================================================
//=========================================================

// instant damage
#define DMG_GENERIC			0			// generic damage was done
#define DMG_CRUSH			(1 << 0)	// crushed by falling or moving object. 
#define DMG_BULLET			(1 << 1)	// shot
#define DMG_BUCKSHOT		(1 << 2)	// shot
#define DMG_SLASH			(1 << 3)	// cut, clawed, stabbed
#define DMG_BURN			(1 << 4)	// heat burned
#define DMG_VEHICLE			(1 << 5)	// hit by a vehicle
#define DMG_FALL			(1 << 6)	// fell too far
#define DMG_BLAST			(1 << 7)	// explosive blast damage
#define DMG_CLUB			(1 << 8)	// crowbar, punch, headbutt
#define DMG_SHOCK			(1 << 9)	// electric shock
#define DMG_DROWN			(1 << 10)	// Drowning
#define DMG_PHYSGUN			(1 << 11)	// physics gun
#define DMG_PLASMA			(1 << 12)	// plasma!

// modifiers
#define DMG_PREVENT_PHYSICS_FORCE		(1 << 13)	// Prevent a physics force 
#define DMG_REMOVENORAGDOLL				(1 << 14)		// with this bit OR'd in, no ragdoll will be created, and the target will be quietly removed.
#define DMG_DISSOLVE					(1 << 15)		// Dissolving!
#define DMG_BLAST_SURFACE				(1 << 16)		// A blast on the surface of water that cannot harm things underwater
#define DMG_DIRECT						(1 << 17)
#define DMG_DEVELOPER					(1 << 18)		// damage from a developer
#define DMG_TELEFRAG					(1 << 19)		// damage from telefraggin
#define DMG_ENGINEER					(1 << 20)		// damage from an engineer
#define DMG_MINEFIELD					(1 << 21)		// damage from a minefield
#define DMG_SNIPERZONE					(1 << 22)		// damage from a sniperzone
#define DMG_INSTANT						(1 << 23)		// kill instally
#define DMG_RICOCHET					(1 << 24)		// ricochet

// these are the damage types that don't have to supply a physics force & position
#define DMG_NO_PHYSICS_FORCE	(DMG_FALL | DMG_BURN | DMG_DROWN | DMG_CRUSH | DMG_PREVENT_PHYSICS_FORCE)

#define DMG_VALIDFORCE	(DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_PLASMA)

// settings for m_takedamage
#define	DAMAGE_NO				0
#define DAMAGE_EVENTS_ONLY		1		// Call damage functions, but don't modify health
#define	DAMAGE_YES				2
#define	DAMAGE_AIM				3

//=========================================================
//=========================================================
enum SpectatorMovementModes_t
{
	OBS_MODE_NONE = 0,	// not in spectator mode
	OBS_MODE_DEATHCAM,	// special mode for detah cam animation
	OBS_MODE_FIXED,		// view from a fixed camera position
	OBS_MODE_IN_EYE,	// follow a player in first perosn view
	OBS_MODE_CHASE,		// follow a player in third person view
	OBS_MODE_ROAMING,	// free roaming
};

//=========================================================
//=========================================================
enum VGUIScreenFlags_t
{
	VGUI_SCREEN_ACTIVE = 0x1,
	VGUI_SCREEN_VISIBLE_TO_TEAMMATES = 0x2,
	VGUI_SCREEN_ATTACHED_TO_VIEWMODEL=0x4,

	VGUI_SCREEN_MAX_BITS = 3
};

//=========================================================
//=========================================================
typedef enum
{
	USE_OFF = 0, 
	USE_ON = 1, 
	USE_SET = 2, 
	USE_TOGGLE = 3
} USE_TYPE;

//=========================================================
//=========================================================
#define		DONT_BLEED			-1
#define		BLOOD_COLOR_RED		(byte)247
#define		BLOOD_COLOR_YELLOW	(byte)195
#define		BLOOD_COLOR_GREEN	BLOOD_COLOR_YELLOW
#define		BLOOD_COLOR_MECH	(byte)20

//=========================================================
//=========================================================
enum PassengerRole_t
{
	VEHICLE_ROLE_NONE = -1,

	VEHICLE_ROLE_DRIVER = 0,	// Only one driver
	
	LAST_SHARED_VEHICLE_ROLE,
};

//=========================================================
//=========================================================
enum
{
	FX_WATER_IN_SLIME = 0x1,
};


//=========================================================
//=========================================================
#define	MAX_CONTEXT_LENGTH		32
#define NO_THINK_CONTEXT	-1

//=========================================================
//=========================================================

// entity flags, CBaseEntity::m_iEFlags
enum
{
	EFL_KILLME	=				(1<<0),	// This entity is marked for death -- This allows the game to actually delete ents at a safe time
	EFL_DORMANT	=				(1<<1),	// Entity is dormant, no updates to client
	EFL_NOCLIP_ACTIVE =			(1<<2),	// Lets us know when the noclip command is active.
	EFL_SETTING_UP_BONES =		(1<<3),	// Set while a model is setting up its bones.
	EFL_KEEP_ON_RECREATE_ENTITIES = (1<<4), // This is a special entity that should not be deleted when we restart entities only

	EFL_HAS_PLAYER_CHILD=		(1<<4),	// One of the child entities is a player.

	EFL_DIRTY_SHADOWUPDATE =	(1<<5),	// Client only- need shadow manager to update the shadow...
	EFL_NOTIFY =				(1<<6),	// Another entity is watching events on this entity (used by teleport)

	// The default behavior in ShouldTransmit is to not send an entity if it doesn't
	// have a model. Certain entities want to be sent anyway because all the drawing logic
	// is in the client DLL. They can set this flag and the engine will transmit them even
	// if they don't have a model.
	EFL_FORCE_CHECK_TRANSMIT =	(1<<7),

	EFL_BOT_FROZEN =			(1<<8),	// This is set on bots that are frozen.
	EFL_SERVER_ONLY =			(1<<9),	// Non-networked entity.
	EFL_NO_AUTO_EDICT_ATTACH =	(1<<10), // Don't attach the edict; we're doing it explicitly
	
	// Some dirty bits with respect to abs computations
	EFL_DIRTY_ABSTRANSFORM =	(1<<11),
	EFL_DIRTY_ABSVELOCITY =		(1<<12),
	EFL_DIRTY_ABSANGVELOCITY =	(1<<13),
	EFL_DIRTY_SURROUNDING_COLLISION_BOUNDS	= (1<<14),
	EFL_DIRTY_SPATIAL_PARTITION = (1<<15),
	EFL_DIRTY_PVS_INFORMATION = (1<<16),

	EFL_IN_SKYBOX =				(1<<17),	// This is set if the entity detects that it's in the skybox.
											// This forces it to pass the "in PVS" for transmission.
	EFL_USE_PARTITION_WHEN_NOT_SOLID = (1<<18),	// Entities with this flag set show up in the partition even when not solid
	EFL_TOUCHING_FLUID =		(1<<19),	// Used to determine if an entity is floating

	// FIXME: Not really sure where I should add this...
	EFL_IS_BEING_LIFTED_BY_BARNACLE = (1<<20),
	EFL_NO_ROTORWASH_PUSH =		(1<<21),		// I shouldn't be pushed by the rotorwash
	EFL_NO_THINK_FUNCTION =		(1<<22),
	EFL_NO_GAME_PHYSICS_SIMULATION = (1<<23),

	EFL_CHECK_UNTOUCH =			(1<<24),
	EFL_DONTBLOCKLOS =			(1<<25),		// I shouldn't block NPC line-of-sight
	EFL_DONTWALKON =			(1<<26),		// NPC;s should not walk on this entity
	EFL_NO_DISSOLVE =			(1<<27),		// These guys shouldn't dissolve
	EFL_NO_MEGAPHYSCANNON_RAGDOLL = (1<<28),	// Mega physcannon can't ragdoll these guys.
	EFL_NO_WEAPON_PICKUP =		(1<<29),		// Characters can't pick up weapons
	EFL_NO_PHYSCANNON_INTERACTION =	(1<<30),	// Physcannon can't pick these up or punt them
	EFL_NO_DAMAGE_FORCES =		(1<<31),	// Doesn't accept forces from physics damage
};

//=========================================================
//=========================================================
const int FX_BLOODSPRAY_DROPS	= 0x01;
const int FX_BLOODSPRAY_GORE	= 0x02;
const int FX_BLOODSPRAY_CLOUD	= 0x04;
const int FX_BLOODSPRAY_ALL		= 0xFF;

//=========================================================
//=========================================================
#define MAX_SCREEN_OVERLAYS		10

//=========================================================
//=========================================================

// these are the types of data that hang off of CBaseEntities and the flag bits used to mark their presence
enum
{
	GROUNDLINK = 0,
	TOUCHLINK,
	STEPSIMULATION,
	MODELWIDTHSCALE,
	POSITIONWATCHER,
	PHYSICSPUSHLIST,

	// must be last
	NUM_DATAOBJECT_TYPES,
};


//=========================================================
//=========================================================

// NOTE: Data for making the MOVETYPE_STEP entities appear to simulate every frame
//  We precompute the simulation and then meter it out each tick during networking of the 
//  entities origin and orientation.  Uses a bit more bandwidth, but it solves the NPCs interacting
//  with elevators/lifts bugs.
struct StepSimulationStep
{
	int			nTickCount;
	Vector		vecOrigin;
	Quaternion	qRotation;
};

struct StepSimulationData
{
	// Are we using the Step Simulation Data
	bool		m_bOriginActive;
	bool		m_bAnglesActive;

	// This is the pre-pre-Think position, orientation (Quaternion) and tick count
	StepSimulationStep	m_Previous2;

	// This is the pre-Think position, orientation (Quaternion) and tick count
	StepSimulationStep	m_Previous;

	// This is a potential mid-think position, orientation (Quaternion) and tick count
	// Used to mark motion discontinuities that happen between thinks
	StepSimulationStep	m_Discontinuity;

	// This is the goal or post-Think position and orientation (and Quaternion for blending) and next think time tick
	StepSimulationStep	m_Next;
	QAngle		m_angNextRotation;

	// This variable is used so that we only compute networked origin/angles once per tick
	int			m_nLastProcessTickCount;
	// The computed/interpolated network origin/angles to use
	Vector		m_vecNetworkOrigin;
	QAngle		m_angNetworkAngles;
};

//=========================================================
//=========================================================
struct ModelWidthScale
{
	float		m_flModelWidthScaleStart;
	float		m_flModelWidthScaleGoal;
	float		m_flModelWidthScaleFinishTime;
	float		m_flModelWidthScaleStartTime;
};

//=========================================================
//=========================================================
#include "soundflags.h"

struct CSoundParameters;
typedef short HSOUNDSCRIPTHANDLE;

struct EmitSound_t
{
	EmitSound_t() :
		m_nChannel( 0 ),
		m_pSoundName( 0 ),
		m_flVolume( VOL_NORM ),
		m_SoundLevel( SNDLVL_NONE ),
		m_nFlags( 0 ),
		m_nPitch( PITCH_NORM ),
		m_pOrigin( 0 ),
		m_flSoundTime( 0.0f ),
		m_pflSoundDuration( 0 ),
		m_bWarnOnDirectWaveReference( false ),
		m_nSpeakerEntity( -1 ),
		m_UtlVecSoundOrigin(),
		m_hSoundScriptHandle( -1 )
	{
	}

	EmitSound_t( const CSoundParameters &src );

	int							m_nChannel;
	char const					*m_pSoundName;
	float						m_flVolume;
	soundlevel_t				m_SoundLevel;
	int							m_nFlags;
	int							m_nPitch;
	const Vector				*m_pOrigin;
	float						m_flSoundTime;
	float						*m_pflSoundDuration;
	bool						m_bEmitCloseCaption;
	bool						m_bWarnOnMissingCloseCaption;
	bool						m_bWarnOnDirectWaveReference;
	int							m_nSpeakerEntity;
	mutable CUtlVector< Vector >	m_UtlVecSoundOrigin;  // Actual sound origin(s) (can be multiple if sound routed through speaker entity(ies) )
	mutable HSOUNDSCRIPTHANDLE		m_hSoundScriptHandle;
};

#endif // SHAREDDEFS_H