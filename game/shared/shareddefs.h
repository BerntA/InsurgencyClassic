//========= Copyright Valve Corporation, All rights reserved. ============//
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
#include "basic_colors.h"

#define TICK_INTERVAL			(gpGlobals->interval_per_tick)

#define TIME_TO_TICKS( dt )		( (int)( 0.5f + (float)(dt) / TICK_INTERVAL ) )
#define TICKS_TO_TIME( t )		( TICK_INTERVAL *( t ) )
#define ROUND_TO_TICKS( t )		( TICK_INTERVAL * TIME_TO_TICKS( t ) )
#define TICK_NEVER_THINK		(-1)

#define ANIMATION_CYCLE_BITS		15
#define ANIMATION_CYCLE_MINFRAC		(1.0f / (1<<ANIMATION_CYCLE_BITS))

// Each mod defines these for itself.
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
		Vector vDeadViewHeight
		)
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

#define VEC_HULL_MIN			g_pGameRules->GetViewVectors()->m_vHullMin
#define VEC_HULL_MAX			g_pGameRules->GetViewVectors()->m_vHullMax
#define VEC_VIEW				g_pGameRules->GetViewVectors()->m_vView

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

// If the player (enemy bots) are scaled, adjust the hull
#define VEC_HULL_MIN_SCALED( player )			( VEC_HULL_MIN * player->GetModelScale() )
#define VEC_HULL_MAX_SCALED( player )			( VEC_HULL_MAX * player->GetModelScale() )
#define VEC_VIEW_SCALED( player )				( VEC_VIEW * player->GetModelScale() )

#define VEC_DUCK_HULL_MIN_SCALED( player )		( VEC_DUCK_HULL_MIN * player->GetModelScale() )
#define VEC_DUCK_HULL_MAX_SCALED( player )		( VEC_DUCK_HULL_MAX * player->GetModelScale() )
#define VEC_DUCK_VIEW_SCALED( player )			( VEC_DUCK_VIEW * player->GetModelScale() )

#define VEC_PRONE_HULL_MIN_SCALED( player )		( VEC_PRONE_HULL_MIN * player->GetModelScale() )
#define VEC_PRONE_HULL_MAX_SCALED( player )		( VEC_PRONE_HULL_MAX * player->GetModelScale() )
#define VEC_PRONE_VIEW_SCALED( player )			( VEC_PRONE_VIEW * player->GetModelScale() )

#define VEC_OBS_HULL_MIN_SCALED( player )		( VEC_OBS_HULL_MIN * player->GetModelScale() )
#define VEC_OBS_HULL_MAX_SCALED( player )		( VEC_OBS_HULL_MAX * player->GetModelScale() )

#define VEC_DEAD_VIEWHEIGHT_SCALED( player )	( VEC_DEAD_VIEWHEIGHT * player->GetModelScale() )

#define WATERJUMP_HEIGHT	8
#define MAX_CLIMB_SPEED		200

#define HUD_PRINTNOTIFY		1
#define HUD_PRINTCONSOLE	2
#define HUD_PRINTTALK		3
#define HUD_PRINTCENTER		4

#define INVALID_TEAM		-1
#define MAX_TEAM_NAME_LENGTH 18

enum TeamList_t
{
	TEAM_UNASSIGNED = 0,	// not assigned to a team
	TEAM_ONE,				// team one
	TEAM_TWO,				// team two
	TEAM_SPECTATOR,			// spectator team
	MAX_TEAMS				// max teams
};

//===================================================================================================================
// Close caption flags
#define CLOSE_CAPTION_WARNIFMISSING	( 1<<0 )
#define CLOSE_CAPTION_FROMPLAYER	( 1<<1 )
#define CLOSE_CAPTION_GENDER_MALE	( 1<<2 )
#define CLOSE_CAPTION_GENDER_FEMALE	( 1<<3 )

//===================================================================================================================
// Hud Element hiding flags
#define	HIDEHUD_WEAPONSELECTION		( 1<<0 )	// Hide ammo count & weapon selection
#define	HIDEHUD_ALL					( 1<<1 )
#define HIDEHUD_HEALTH				( 1<<2 )	// Hide health & armor / suit battery
#define HIDEHUD_PLAYERDEAD			( 1<<3 )	// Hide when local player's dead
#define HIDEHUD_MISCSTATUS			( 1<<4 )	// Hide miscellaneous status elements (trains, pickup history, death notices, etc)
#define HIDEHUD_CHAT				( 1<<5 )	// Hide all communication elements (saytext, voice icon, etc)
#define HIDEHUD_SCOREBOARD		    ( 1<<6 )	// Hide HUD when the scoreboard is visible.

#define HIDEHUD_BITCOUNT			7

// BB2 
#define MAX_TEAMMATE_DISTANCE 500.0f
#define MAX_GLOW_RADIUS_DIST 100.0f

//===================================================================================================================
// Player Defines

// Max number of players in a game ( see const.h for ABSOLUTE_PLAYER_LIMIT (256 ) )
// The Source engine is really designed for 32 or less players.  If you raise this number above 32, you better know what you are doing
//  and have a good answer for a bunch of perf question related to player simulation, thinking logic, tracelines, networking overhead, etc.
// But if you are brave or are doing something interesting, go for it...   ywb 9/22/03

//You might be wondering why these aren't multiple of 2. Well the reason is that if servers decide to have HLTV or Replay enabled we need the extra slot.
//This is ok since MAX_PLAYERS is used for code specific things like arrays and loops, but it doesn't really means that this is the max number of players allowed
//Since this is decided by the gamerules (and it can be whatever number as long as its less than MAX_PLAYERS).

#define MAX_PLAYERS				65  // Absolute max players supported

#define MAX_FOV						90
#define MAX_PLACE_NAME_LENGTH	18

//===================================================================================================================
// Team Defines
#define TEAM_ANY				-2
#define	TEAM_INVALID			-1

#define WEAPON_NOT_CARRIED				0	// Weapon is on the ground
#define WEAPON_IS_CARRIED_BY_PLAYER		1	// This client is carrying this weapon.
#define WEAPON_IS_ACTIVE				2	// This client is carrying this weapon and it's the currently held weapon

#define MAX_BEAM_ENTS			10

#define TRACER_TYPE_DEFAULT		0x00000001
#define TRACER_TYPE_GUNSHIP		0x00000002
#define TRACER_TYPE_STRIDER		0x00000004 // Here ya go, Jay!
#define TRACER_TYPE_GAUSS		0x00000008
#define TRACER_TYPE_WATERBULLET	0x00000010

#define MUZZLEFLASH_TYPE_DEFAULT	0x00000001
#define MUZZLEFLASH_TYPE_GUNSHIP	0x00000002
#define MUZZLEFLASH_TYPE_STRIDER	0x00000004

// Muzzle flash definitions (for the flags field of the "MuzzleFlash" DispatchEffect)
enum
{
	MUZZLEFLASH_AR2				= 0,
	MUZZLEFLASH_SHOTGUN,
	MUZZLEFLASH_SMG1,
	MUZZLEFLASH_SMG2,
	MUZZLEFLASH_PISTOL,
	MUZZLEFLASH_COMBINE,
	MUZZLEFLASH_357,
	MUZZLEFLASH_RPG,
	MUZZLEFLASH_COMBINE_TURRET,

	MUZZLEFLASH_FIRSTPERSON		= 0x100,
};

// Tracer Flags
#define TRACER_FLAG_WHIZ			0x0001
#define TRACER_FLAG_USEATTACHMENT	0x0002

#define TRACER_DONT_USE_ATTACHMENT	-1

// Entity Dissolve types
enum
{
	ENTITY_DISSOLVE_NORMAL = 0,
	ENTITY_DISSOLVE_ELECTRICAL,
	ENTITY_DISSOLVE_ELECTRICAL_LIGHT,
	ENTITY_DISSOLVE_CORE,

	// NOTE: Be sure to up the bits if you make more dissolve types
	ENTITY_DISSOLVE_BITS = 3
};

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

// LEGACY
#define	HITGROUP_CHEST		2
#define	HITGROUP_STOMACH	3
#define HITGROUP_LEFTARM	4	
#define HITGROUP_RIGHTARM	5
#define HITGROUP_LEFTLEG	6
#define HITGROUP_RIGHTLEG	7

// HL2 has 600 gravity by default
// NOTE: The discrete ticks can have quantization error, so these numbers are biased a little to
// make the heights more exact
#define PLAYER_FATAL_FALL_SPEED		1024.0f // approx 60 feet sqrt( 2 * gravity * 60 * 12 )
#define PLAYER_MAX_SAFE_FALL_SPEED	580.0f // approx 20 feet sqrt( 2 * gravity * 20 * 12 )
#define PLAYER_LAND_ON_FLOATING_OBJECT	200.0f // Can fall another 173 in/sec without getting hurt
#define PLAYER_MIN_BOUNCE_SPEED		200.0f
#define PLAYER_FALL_PUNCH_THRESHOLD 350.0f // won't punch player's screen/make scrape noise unless player falling at least this fast - at least a 76" fall (sqrt( 2 * g * 76))
#define DAMAGE_FOR_FALL_SPEED		100.0f / ( PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED ) // damage per unit per second.

// instant damage

#define DMG_GENERIC						0			// generic damage was done
#define DMG_CRUSH						(1 << 0)	// crushed by falling or moving object. 
#define DMG_BULLET						(1 << 1)	// shot
#define DMG_BUCKSHOT					(1 << 2)	// shot
#define DMG_SLASH						(1 << 3)	// cut, clawed, stabbed
#define DMG_BURN						(1 << 4)	// heat burned
#define DMG_VEHICLE						(1 << 5)	// hit by a vehicle
#define DMG_FALL						(1 << 6)	// fell too far
#define DMG_BLAST						(1 << 7)	// explosive blast damage
#define DMG_CLUB						(1 << 8)	// crowbar, punch, headbutt
#define DMG_SHOCK						(1 << 9)	// electric shock
#define DMG_DROWN						(1 << 10)	// Drowning
#define DMG_PHYSGUN						(1 << 11)	// physics gun
#define DMG_PLASMA						(1 << 12)	// plasma!
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

// NOTE: DO NOT ADD ANY MORE CUSTOM DMG_ TYPES. MODS USE THE DMG_LASTGENERICFLAG BELOW, AND
//		 IF YOU ADD NEW DMG_ TYPES, THEIR TYPES WILL BE HOSED. WE NEED A BETTER SOLUTION.

// TODO: keep this up to date so all the mod-specific flags don't overlap anything.
#define DMG_LASTGENERICFLAG	DMG_RICOCHET

#define DMG_NO_PHYSICS_FORCE	(DMG_FALL | DMG_BURN | DMG_DROWN | DMG_CRUSH | DMG_PREVENT_PHYSICS_FORCE)
#define DMG_VALIDFORCE			(DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_PLASMA)

// settings for m_takedamage
#define	DAMAGE_NO				0
#define DAMAGE_EVENTS_ONLY		1		// Call damage functions, but don't modify health
#define	DAMAGE_YES				2
#define	DAMAGE_AIM				3

// Spectator Movement modes
enum SpectatorMovementModes_t
{
	OBS_MODE_NONE = 0,	// not in spectator mode
	OBS_MODE_DEATHCAM,	// special mode for death cam animation
	OBS_MODE_FREEZECAM,	// zooms to a target, and freeze-frames on them
	OBS_MODE_FIXED,		// view from a fixed camera position
	OBS_MODE_IN_EYE,	// follow a player in first person view
	OBS_MODE_CHASE,		// follow a player in third person view
	OBS_MODE_ROAMING,	// free roaming

	NUM_OBSERVER_MODES,
};

#define LAST_PLAYER_OBSERVERMODE	OBS_MODE_ROAMING

// Force Camera Restrictions with mp_forcecamera
enum {
	OBS_ALLOW_ALL = 0,	// allow all modes, all targets
	OBS_ALLOW_TEAM,		// allow only own team & first person, no PIP
	OBS_ALLOW_NONE,		// don't allow any spectating after death (fixed & fade to black)

	OBS_ALLOW_NUM_MODES,
};

enum
{
	TYPE_TEXT = 0,	// just display this plain text
	TYPE_INDEX,		// lookup text & title in stringtable
	TYPE_URL,		// show this URL
	TYPE_FILE,		// show this local file
} ;

// VGui Screen Flags
enum
{
	VGUI_SCREEN_ACTIVE = 0x1,
	VGUI_SCREEN_VISIBLE_TO_TEAMMATES = 0x2,
	VGUI_SCREEN_ATTACHED_TO_VIEWMODEL = 0x4,
	VGUI_SCREEN_TRANSPARENT = 0x8,
	VGUI_SCREEN_ONLY_USABLE_BY_OWNER = 0x10,

	VGUI_SCREEN_MAX_BITS = 5
};

typedef enum
{
	USE_OFF = 0, 
	USE_ON = 1, 
	USE_SET = 2, 
	USE_TOGGLE = 3
} USE_TYPE;

// All NPCs need this data
enum
{
	DONT_BLEED = -1,
	BLOOD_COLOR_RED = 0,
	BLOOD_COLOR_YELLOW,
	BLOOD_COLOR_GREEN,
	BLOOD_COLOR_MECH,
};

//-----------------------------------------------------------------------------
// Water splash effect flags
//-----------------------------------------------------------------------------
enum
{
	FX_WATER_IN_SLIME = 0x1,
};

// Shared think context stuff
#define	MAX_CONTEXT_LENGTH		32
#define NO_THINK_CONTEXT		-1

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
//	UNUSED						= (1<<16),

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
	EFL_NO_WATER_VELOCITY_CHANGE  =	(1<<29),	// Don't adjust this entity's velocity when transitioning into water
	EFL_NO_PHYSCANNON_INTERACTION =	(1<<30),	// Physcannon can't pick these up or punt them
	EFL_NO_DAMAGE_FORCES =		(1<<31),	// Doesn't accept forces from physics damage
};

//-----------------------------------------------------------------------------
// EFFECTS
//-----------------------------------------------------------------------------
const int FX_BLOODSPRAY_DROPS	= 0x01;
const int FX_BLOODSPRAY_GORE	= 0x02;
const int FX_BLOODSPRAY_CLOUD	= 0x04;
const int FX_BLOODSPRAY_ALL		= 0xFF;

//-----------------------------------------------------------------------------
#define MAX_SCREEN_OVERLAYS		10

// These are the types of data that hang off of CBaseEntities and the flag bits used to mark their presence
enum
{
	GROUNDLINK = 0,
	TOUCHLINK,
	STEPSIMULATION,
	MODELSCALE,
	POSITIONWATCHER,
	PHYSICSPUSHLIST,
	VPHYSICSUPDATEAI,
	VPHYSICSWATCHER,

	// Must be last and <= 32
	NUM_DATAOBJECT_TYPES,
};

class CBaseEntity;

//-----------------------------------------------------------------------------
// Bullet firing information
//-----------------------------------------------------------------------------
class CBaseEntity;

enum FireBulletsFlags_t
{
	FIRE_BULLETS_FIRST_SHOT_ACCURATE = 0x1,		// Pop the first shot with perfect accuracy
	FIRE_BULLETS_DONT_HIT_UNDERWATER = 0x2,		// If the shot hits its target underwater, don't damage it
	FIRE_BULLETS_ALLOW_WATER_SURFACE_IMPACTS = 0x4,	// If the shot hits water surface, still call DoImpactEffect
	FIRE_BULLETS_TEMPORARY_DANGER_SOUND = 0x8,		// Danger sounds added from this impact can be stomped immediately if another is queued
};

struct FireBulletsInfo_t
{
	FireBulletsInfo_t()
	{
		m_iShots = 1;
		m_vecSpread.Init( 0, 0, 0 );
		m_vecFirstStartPos.Init(0, 0, 0);
		m_flDistance = 8192;
		m_iTracerFreq = 4;
		m_flDamage = 0;
		m_iPlayerDamage = 0;
		m_pAttacker = NULL;
		m_nFlags = 0;
		m_pAdditionalIgnoreEnt = NULL;
		m_flDamageForceScale = 1.0f;
		m_bPrimaryAttack = true;
		m_flDropOffDist = 0.0f;
		m_bUseServerRandomSeed = false;
	}

	FireBulletsInfo_t( int nShots, const Vector &vecSrc, const Vector &vecDir, const Vector &vecSpread, float flDistance, int nAmmoType, bool bPrimaryAttack = true )
	{
		m_iShots = nShots;
		m_vecSrc = vecSrc;
		m_vecDirShooting = vecDir;
		m_vecSpread = vecSpread;
		m_vecFirstStartPos.Init(0, 0, 0);
		m_flDistance = flDistance;
		m_iAmmoType = nAmmoType;
		m_iTracerFreq = 4;
		m_flDamage = 0;
		m_iPlayerDamage = 0;
		m_pAttacker = NULL;
		m_nFlags = 0;
		m_pAdditionalIgnoreEnt = NULL;
		m_flDamageForceScale = 1.0f;
		m_bPrimaryAttack = bPrimaryAttack;
		m_flDropOffDist = 0.0f;
		m_bUseServerRandomSeed = false;
	}

	int m_iShots;
	Vector m_vecSrc;
	Vector m_vecDirShooting;
	Vector m_vecSpread;
	Vector m_vecFirstStartPos;
	float m_flDropOffDist;
	float m_flDistance;
	int m_iAmmoType;
	int m_iTracerFreq;
	float m_flDamage;
	int m_iPlayerDamage;	// Damage to be used instead of m_flDamage if we hit a player
	int m_nFlags;			// See FireBulletsFlags_t
	float m_flDamageForceScale;
	CBaseEntity *m_pAttacker;
	CBaseEntity *m_pAdditionalIgnoreEnt;
	bool m_bPrimaryAttack;
	bool m_bUseServerRandomSeed;
};

//-----------------------------------------------------------------------------
// Purpose: Data for making the MOVETYPE_STEP entities appear to simulate every frame
//  We precompute the simulation and then meter it out each tick during networking of the 
//  entities origin and orientation.  Uses a bit more bandwidth, but it solves the NPCs interacting
//  with elevators/lifts bugs.
//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
// Purpose: Simple state tracking for changing model sideways shrinkage during barnacle swallow
//-----------------------------------------------------------------------------
struct ModelScale
{
	float		m_flModelScaleStart;
	float		m_flModelScaleGoal;
	float		m_flModelScaleFinishTime;
	float		m_flModelScaleStartTime;
};

#include "soundflags.h"

struct CSoundParameters;
typedef short HSOUNDSCRIPTHANDLE;

//-----------------------------------------------------------------------------
// Purpose: Aggregates and sets default parameters for EmitSound function calls
//-----------------------------------------------------------------------------
struct EmitSound_t
{
	EmitSound_t() :
		m_nChannel( 0 ),
		m_pSoundName( 0 ),
		m_flVolume( VOL_NORM ),
		m_SoundLevel( SNDLVL_NONE ),
		m_nFlags( 0 ),
		m_nPitch( PITCH_NORM ),
		m_nSpecialDSP( 0 ),
		m_pOrigin( 0 ),
		m_flSoundTime( 0.0f ),
		m_pflSoundDuration( 0 ),
		m_bEmitCloseCaption( true ),
		m_bWarnOnMissingCloseCaption( false ),
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
	int							m_nSpecialDSP;
	const Vector				*m_pOrigin;
	float						m_flSoundTime; ///< NOT DURATION, but rather, some absolute time in the future until which this sound should be delayed
	float						*m_pflSoundDuration;
	bool						m_bEmitCloseCaption;
	bool						m_bWarnOnMissingCloseCaption;
	bool						m_bWarnOnDirectWaveReference;
	int							m_nSpeakerEntity;
	mutable CUtlVector< Vector >	m_UtlVecSoundOrigin;  ///< Actual sound origin(s) (can be multiple if sound routed through speaker entity(ies) )
	mutable HSOUNDSCRIPTHANDLE		m_hSoundScriptHandle;
};

enum
{
	SIMULATION_TIME_WINDOW_BITS = 8,
};

enum
{
	HILL_TYPE_NONE = 0,
	HILL_TYPE_UPHILL,
	HILL_TYPE_DOWNHILL,
};

#define NOINTERP_PARITY_MAX			4
#define NOINTERP_PARITY_MAX_BITS	2

//-----------------------------------------------------------------------------
// Generic activity lookup support
//-----------------------------------------------------------------------------
enum
{
	kActivityLookup_Unknown = -2,			// hasn't been searched for
	kActivityLookup_Missing = -1,			// has been searched for but wasn't found
};

// BB2
#define GLOWS_ENABLE

enum
{
	GLOW_MODE_NONE = 0,
	GLOW_MODE_RADIUS, // Glow a normal item which can be picked up / can be consumed. Like weapons, ammo, etc... You can't force glow items using this type / change'em. (it is client side)
	GLOW_MODE_GLOBAL, // Forces Glowing on any entity.
	// PLAYER STUFF:
	GLOW_MODE_TEAMMATE, // Others can glow me if I'm X amount away from them, even if I'm not visible.
};

enum EntityObstructionType
{
	ENTITY_OBSTRUCTION_NONE = 0,
	ENTITY_OBSTRUCTION_FUNC_BREAKABLE,
	ENTITY_OBSTRUCTION_PROP_BREAKABLE,
	ENTITY_OBSTRUCTION_DOOR,
	ENTITY_OBSTRUCTION_NPC_OBSTACLE,
};

enum ItemPriorityType
{
	ITEM_PRIORITY_NO = 0,
	ITEM_PRIORITY_GENERIC,
	ITEM_PRIORITY_OBJECTIVE,
};

#define DEFAULT_PLAYER_MODEL(team) ( ( team == TEAM_DECEASED ) ? ( "models/characters/player/marine_zombie.mdl" ) : ( "models/characters/player/marine.mdl" ) )

#endif // SHAREDDEFS_H