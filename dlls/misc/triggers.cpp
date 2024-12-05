//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Spawn and use functions for editor-placed triggers.
//
//=============================================================================//

#include "cbase.h"
#include "saverestore.h"
#include "gamerules.h"
#include "entityapi.h"
#include "entitylist.h"
#include "ndebugoverlay.h"
#include "globalstate.h"
#include "filters.h"
#include "vstdlib/random.h"
#include "triggers.h"
#include "saverestoretypes.h"
#include "hierarchy.h"
#include "bspfile.h"
#include "saverestore_utlvector.h"
#include "physics_saverestore.h"
#include "te_effect_dispatch.h"
#include "ammodef.h"
#include "iservervehicle.h"
#include "movevars_shared.h"
#include "physics_prop_ragdoll.h"
#include "props.h"
#include "entityparticletrail.h"
#include "in_buttons.h"
#include "gameinterface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DEBUG_TRANSITIONS_VERBOSE	2
ConVar g_debug_transitions( "g_debug_transitions", "0", FCVAR_NONE, "Set to 1 and restart the map to be warned if the map has no trigger_transition volumes. Set to 2 to see a dump of all entities & associated results during a transition." );

// Global list of triggers that care about weapon fire
// Doesn't need saving, the triggers re-add themselves on restore.
CUtlVector< CHandle<CTriggerMultiple> >	g_hWeaponFireTriggers;

extern CServerGameDLL	g_ServerGameDLL;
extern bool				g_fGameOver;
ConVar showtriggers( "showtriggers", "0", FCVAR_CHEAT, "Shows trigger brushes" );

bool IsTriggerClass( CBaseEntity *pEntity );

// Command to dynamically toggle trigger visibility
void Cmd_ShowtriggersToggle_f( void )
{
	// Loop through the entities in the game and make visible anything derived from CBaseTrigger
	CBaseEntity *pEntity = gEntList.FirstEnt();
	while ( pEntity )
	{
		if ( IsTriggerClass(pEntity) )
		{
			// If a classname is specified, only show triggles of that type
			if ( engine->Cmd_Argc() > 1 )
			{
				const char *sClassname = engine->Cmd_Argv(1);
				if ( sClassname && sClassname[0] )
				{
					if ( !FClassnameIs( pEntity, sClassname ) )
					{
						pEntity = gEntList.NextEnt( pEntity );
						continue;
					}
				}
			}

			if ( pEntity->IsEffectActive( EF_NODRAW ) )
			{
				pEntity->RemoveEffects( EF_NODRAW );
			}
			else
			{
				pEntity->AddEffects( EF_NODRAW );
			}
		}

		pEntity = gEntList.NextEnt( pEntity );
	}
}

static ConCommand showtriggers_toggle( "showtriggers_toggle", Cmd_ShowtriggersToggle_f, "Toggle show triggers", FCVAR_CHEAT );

// Global Savedata for base trigger
BEGIN_DATADESC( CBaseTrigger )

	// Keyfields
	DEFINE_KEYFIELD( m_iFilterName,	FIELD_STRING,	"filtername" ),
	DEFINE_FIELD( m_hFilter,	FIELD_EHANDLE ),
	DEFINE_KEYFIELD( m_bDisabled,		FIELD_BOOLEAN,	"StartDisabled" ),
	DEFINE_UTLVECTOR( m_hTouchingEntities, FIELD_EHANDLE ),

	// Inputs	
	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

	DEFINE_INPUTFUNC( FIELD_VOID, "StartTouch", InputStartTouch ),
	DEFINE_INPUTFUNC( FIELD_VOID, "EndTouch", InputEndTouch ),

	// Outputs
	DEFINE_OUTPUT( m_OnStartTouch, "OnStartTouch"),
	DEFINE_OUTPUT( m_OnEndTouch, "OnEndTouch"),
	DEFINE_OUTPUT( m_OnEndTouchAll, "OnEndTouchAll"),

END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger, CBaseTrigger );


CBaseTrigger::CBaseTrigger()
{
	AddEFlags( EFL_USE_PARTITION_WHEN_NOT_SOLID );
}

//------------------------------------------------------------------------------
// Purpose: Input handler to turn on this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::InputEnable( inputdata_t &inputdata )
{ 
	Enable();
}


//------------------------------------------------------------------------------
// Purpose: Input handler to turn off this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::InputDisable( inputdata_t &inputdata )
{ 
	Disable();
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CBaseTrigger::Spawn()
{
	if( HasSpawnFlags( SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS ) )
	{
		// Automatically set this trigger to work with NPC's.
		AddSpawnFlags( SF_TRIGGER_ALLOW_NPCS );
	}

	if ( HasSpawnFlags( SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES ) )
	{
		AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS );
	}

	if ( HasSpawnFlags( SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES ) )
	{
		AddSpawnFlags( SF_TRIGGER_ALLOW_CLIENTS );
	}

	BaseClass::Spawn();
}

//------------------------------------------------------------------------------
// Create VPhysics
//------------------------------------------------------------------------------
bool CBaseTrigger::CreateVPhysics( void )
{
	if ( !HasSpawnFlags( SF_TRIG_TOUCH_DEBRIS ) )
		return false;

	IPhysicsObject *pPhysics;
	pPhysics = VPhysicsInitShadow( false, false );
	if ( pPhysics )
	{
		pPhysics->BecomeTrigger();
	}
	return true;
}

//------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------
void CBaseTrigger::UpdateOnRemove( void )
{
	if ( VPhysicsGetObject())
	{
		VPhysicsGetObject()->RemoveTrigger();
	}

	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
// Purpose: Turns on this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::Enable( void )
{
	m_bDisabled = false;

	if ( VPhysicsGetObject())
	{
		VPhysicsGetObject()->EnableCollisions( true );
	}

	if (!IsSolidFlagSet( FSOLID_TRIGGER ))
	{
		AddSolidFlags( FSOLID_TRIGGER ); 
		PhysicsTouchTriggers();
	}
}

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CBaseTrigger::Activate( void ) 
{ 
	// Get a handle to my filter entity if there is one
	if (m_iFilterName != NULL_STRING)
	{
		m_hFilter = dynamic_cast<CBaseFilter *>(gEntList.FindEntityByName( NULL, m_iFilterName ));
	}

	BaseClass::Activate();
}


//-----------------------------------------------------------------------------
// Purpose: Called after player becomes active in the game
//-----------------------------------------------------------------------------
void CBaseTrigger::PostClientActive( void )
{
	BaseClass::PostClientActive();

	if ( !m_bDisabled )
	{
		PhysicsTouchTriggers();
	}
}

//------------------------------------------------------------------------------
// Purpose: Turns off this trigger.
//------------------------------------------------------------------------------
void CBaseTrigger::Disable( void )
{ 
	m_bDisabled = true;

	if ( VPhysicsGetObject())
	{
		VPhysicsGetObject()->EnableCollisions( false );
	}

	if (IsSolidFlagSet(FSOLID_TRIGGER))
	{
		RemoveSolidFlags( FSOLID_TRIGGER ); 
		PhysicsTouchTriggers();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CBaseTrigger::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		// --------------
		// Print Target
		// --------------
		char tempstr[255];
		if (IsSolidFlagSet(FSOLID_TRIGGER)) 
		{
			Q_strncpy(tempstr,"State: Enabled",sizeof(tempstr));
		}
		else
		{
			Q_strncpy(tempstr,"State: Disabled",sizeof(tempstr));
		}
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseTrigger::InitTrigger( )
{
	SetSolid( GetParent() ? SOLID_VPHYSICS : SOLID_BSP );	
	AddSolidFlags( FSOLID_NOT_SOLID );
	if (m_bDisabled)
	{
		RemoveSolidFlags( FSOLID_TRIGGER );	
	}
	else
	{
		AddSolidFlags( FSOLID_TRIGGER );	
	}

	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) );    // set size and link into world
	if ( showtriggers.GetInt() == 0 )
	{
		AddEffects( EF_NODRAW );
	}

	m_hTouchingEntities.Purge();

	if ( HasSpawnFlags( SF_TRIG_TOUCH_DEBRIS ) )
	{
		CreateVPhysics();
	}
}


//-----------------------------------------------------------------------------
// Purpose: Returns true if this entity passes the filter criterea, false if not.
// Input  : pOther - The entity to be filtered.
//-----------------------------------------------------------------------------
bool CBaseTrigger::PassesTriggerFilters(CBaseEntity *pOther)
{
	// First test spawn flag filters
	if ( HasSpawnFlags(SF_TRIGGER_ALLOW_ALL) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS) && (pOther->GetFlags() & FL_CLIENT)) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_NPCS) && (pOther->GetFlags() & FL_NPC)) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_PUSHABLES) && FClassnameIs(pOther, "func_pushable")) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_PHYSICS) && pOther->GetMoveType() == MOVETYPE_VPHYSICS))
	{
		bool bOtherIsPlayer = pOther->IsPlayer();

		// Pongles [
		/*if( HasSpawnFlags(SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS) && !bOtherIsPlayer )
		{
			CAI_BaseNPC *pNPC = pOther->MyNPCPointer();

			if( !pNPC || !pNPC->IsPlayerAlly() )
			{
				return false;
			}
		}*/
		// Pongles ]

		if ( HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES) && bOtherIsPlayer )
		{
			if ( !((CBasePlayer*)pOther)->IsInAVehicle() )
				return false;
		}

		if ( HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES) && bOtherIsPlayer )
		{
			if ( ((CBasePlayer*)pOther)->IsInAVehicle() )
				return false;
		}

		CBaseFilter *pFilter = m_hFilter.Get();
		return (!pFilter) ? true : pFilter->PassesFilter( this, pOther );
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Called to simulate what happens when an entity touches the trigger.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::InputStartTouch( inputdata_t &inputdata )
{
	//Pretend we just touched the trigger.
	StartTouch( inputdata.pCaller );
}
//-----------------------------------------------------------------------------
// Purpose: Called to simulate what happens when an entity leaves the trigger.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::InputEndTouch( inputdata_t &inputdata )
{
	//And... pretend we left the trigger.
	EndTouch( inputdata.pCaller );	
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::StartTouch(CBaseEntity *pOther)
{
	if ( HasSpawnFlags( SF_TRIG_TOUCH_DEBRIS ) )
	{
		triggerevent_t event;
		if ( PhysGetTriggerEvent( &event, this ) )
		{
			// We've been called due a vphysics touch.
			// If we're not debris, abort. The normal game code will call touch for us.
			if ( pOther->GetCollisionGroup() != COLLISION_GROUP_DEBRIS )
				return;
		}
	}

	if (PassesTriggerFilters(pOther) )
	{
		EHANDLE hOther;
		hOther = pOther;

		m_hTouchingEntities.AddToTail( hOther );
		m_OnStartTouch.FireOutput(pOther, this);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CBaseTrigger::EndTouch(CBaseEntity *pOther)
{
	if ( IsTouching( pOther ) )
	{
		EHANDLE hOther;
		hOther = pOther;
		m_hTouchingEntities.FindAndRemove( hOther );
		
		//FIXME: Without this, triggers fire their EndTouch outputs when they are disabled!
		//if ( !m_bDisabled )
		//{
			m_OnEndTouch.FireOutput(pOther, this);
		//}

		// If there are no more entities touching this trigger, fire the lost all touches
		// Loop through the touching entities backwards. Clean out old ones, and look for existing
		bool bFoundOtherTouchee = false;
		int iSize = m_hTouchingEntities.Count();
		for ( int i = iSize-1; i >= 0; i-- )
		{
			EHANDLE hOther;
			hOther = m_hTouchingEntities[i];

			if ( !hOther )
			{
				m_hTouchingEntities.Remove( i );
			}
			else
			{
				bFoundOtherTouchee = true;
			}
		}

		//FIXME: Without this, triggers fire their EndTouch outputs when they are disabled!
		// Didn't find one?
		if ( !bFoundOtherTouchee /*&& !m_bDisabled*/ )
		{
			m_OnEndTouchAll.FireOutput(pOther, this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return true if the specified entity is touching us
//-----------------------------------------------------------------------------
bool CBaseTrigger::IsTouching( CBaseEntity *pOther )
{
	EHANDLE hOther;
	hOther = pOther;
	return ( m_hTouchingEntities.Find( hOther ) != m_hTouchingEntities.InvalidIndex() );
}

//-----------------------------------------------------------------------------
// Purpose: Return a pointer to the first entity of the specified type being touched by this trigger
//-----------------------------------------------------------------------------
CBaseEntity *CBaseTrigger::GetTouchedEntityOfType( const char *sClassName )
{
	int iCount = m_hTouchingEntities.Count();
	for ( int i = 0; i < iCount; i++ )
	{
		CBaseEntity *pEntity = m_hTouchingEntities[i];
		if ( FClassnameIs( pEntity, sClassName ) )
			return pEntity;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Toggles this trigger between enabled and disabled.
//-----------------------------------------------------------------------------
void CBaseTrigger::InputToggle( inputdata_t &inputdata )
{
	if (IsSolidFlagSet( FSOLID_TRIGGER ))
	{
		RemoveSolidFlags(FSOLID_TRIGGER);
	}
	else
	{
		AddSolidFlags(FSOLID_TRIGGER);
	}

	PhysicsTouchTriggers();
}


//-----------------------------------------------------------------------------
// Purpose: Removes anything that touches it. If the trigger has a targetname,
//			firing it will toggle state.
//-----------------------------------------------------------------------------
class CTriggerRemove : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerRemove, CBaseTrigger );

	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	
	DECLARE_DATADESC();
	
	// Outputs
	COutputEvent m_OnRemove;
};

BEGIN_DATADESC( CTriggerRemove )

	// Outputs
	DEFINE_OUTPUT( m_OnRemove, "OnRemove" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_remove, CTriggerRemove );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerRemove::Spawn( void )
{
	BaseClass::Spawn();
	InitTrigger();
}


//-----------------------------------------------------------------------------
// Purpose: Trigger hurt that causes radiation will do a radius check and set
//			the player's geiger counter level according to distance from center
//			of trigger.
//-----------------------------------------------------------------------------
void CTriggerRemove::Touch( CBaseEntity *pOther )
{
	if (!PassesTriggerFilters(pOther))
		return;

	UTIL_Remove( pOther );
}



//-----------------------------------------------------------------------------
// Purpose: Hurts anything that touches it. If the trigger has a targetname,
//			firing it will toggle state.
//-----------------------------------------------------------------------------
class CTriggerHurt : public CBaseTrigger
{
public:
	CTriggerHurt()
	{
		// This field came along after levels were built so the field defaults to 20 here in the constructor.
		m_flDamageCap = 20.0f;
	}

	DECLARE_CLASS( CTriggerHurt, CBaseTrigger );

	void Spawn( void );
	// Pongles [
	//void RadiationThink( void );
	// Pongles ]
	void HurtThink( void );
	void Touch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );
	bool HurtEntity( CBaseEntity *pOther, float damage );
	int HurtAllTouchers( float dt );
	
	DECLARE_DATADESC();
	
	float	m_flOriginalDamage;	// Damage as specified by the level designer.
	float	m_flDamage;			// Damage per second.
	float	m_flDamageCap;		// Maximum damage per second.
	float	m_flLastDmgTime;	// Time that we last applied damage.
	float	m_flDmgResetTime;	// For forgiveness, the time to reset the counter that accumulates damage.
	int		m_bitsDamageInflict;	// DMG_ damage type that the door or tigger does
	int		m_damageModel;

	enum
	{
		DAMAGEMODEL_NORMAL = 0,
		DAMAGEMODEL_DOUBLE_FORGIVENESS,
	};

	// Outputs
	COutputEvent m_OnHurt;
	COutputEvent m_OnHurtPlayer;

	CUtlVector<EHANDLE>	m_hurtEntities;
};

BEGIN_DATADESC( CTriggerHurt )

	// Function Pointers
	// Pongles [
	//DEFINE_FUNCTION( RadiationThink ),
	// Pongles ]
	DEFINE_FUNCTION( HurtThink ),

	// Fields
	DEFINE_FIELD( m_flOriginalDamage, FIELD_FLOAT ),
	DEFINE_KEYFIELD( m_flDamage, FIELD_FLOAT, "damage" ),
	DEFINE_KEYFIELD( m_flDamageCap, FIELD_FLOAT, "damagecap" ),
	DEFINE_KEYFIELD( m_bitsDamageInflict, FIELD_INTEGER, "damagetype" ),
	DEFINE_KEYFIELD( m_damageModel, FIELD_INTEGER, "damagemodel" ),

	DEFINE_FIELD( m_flLastDmgTime, FIELD_TIME ),
	DEFINE_FIELD( m_flDmgResetTime, FIELD_TIME ),
	DEFINE_UTLVECTOR( m_hurtEntities, FIELD_EHANDLE ),

	// Inputs
	DEFINE_INPUT( m_flDamage, FIELD_FLOAT, "SetDamage" ),

	// Outputs
	DEFINE_OUTPUT( m_OnHurt, "OnHurt" ),
	DEFINE_OUTPUT( m_OnHurtPlayer, "OnHurtPlayer" ),

END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_hurt, CTriggerHurt );


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerHurt::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();

	m_flOriginalDamage = m_flDamage;

	SetNextThink( TICK_NEVER_THINK );
	SetThink( NULL );

	// Pongles [
	/*if (m_bitsDamageInflict & DMG_RADIATION)
	{
		SetThink ( &CTriggerHurt::RadiationThink );
		SetNextThink( gpGlobals->curtime + random->RandomFloat(0.0, 0.5) );
	}*/
	// Pongles ]
}

// Pongles [

//-----------------------------------------------------------------------------
// Purpose: Trigger hurt that causes radiation will do a radius check and set
//			the player's geiger counter level according to distance from center
//			of trigger.
//-----------------------------------------------------------------------------
/*void CTriggerHurt::RadiationThink( void )
{
	// check to see if a player is in pvs
	// if not, continue	
	Vector vecSurroundMins, vecSurroundMaxs;
	CollisionProp()->WorldSpaceSurroundingBounds( &vecSurroundMins, &vecSurroundMaxs );
	CBasePlayer *pPlayer = static_cast<CBasePlayer *>(UTIL_FindClientInPVS( vecSurroundMins, vecSurroundMaxs ));

	if (pPlayer)
	{
		// get range to player;
		float flRange = CollisionProp()->CalcDistanceFromPoint( pPlayer->WorldSpaceCenter() );
		flRange *= 3.0f;
		pPlayer->NotifyNearbyRadiationSource(flRange);
	}

	float dt = gpGlobals->curtime - m_flLastDmgTime;
	if ( dt >= 0.5 )
	{
		HurtAllTouchers( dt );
	}

	SetNextThink( gpGlobals->curtime + 0.25 );
}*/

// Pongles ]


//-----------------------------------------------------------------------------
// Purpose: When touched, a hurt trigger does m_flDamage points of damage each half-second.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
bool CTriggerHurt::HurtEntity( CBaseEntity *pOther, float damage )
{
	if ( !pOther->m_takedamage || !PassesTriggerFilters(pOther) )
		return false;

	// Pongles [

	// PNOTE: it was annoying flying around karkar and dying :P
	if( pOther->GetMoveType( ) == MOVETYPE_NOCLIP )
		return false;

	// Pongles ]

	if ( damage < 0 )
	{
		pOther->TakeHealth( -damage, m_bitsDamageInflict );
	}
	else
	{
		// The damage position is the nearest point on the damaged entity
		// to the trigger's center. Not perfect, but better than nothing.
		Vector vecCenter = CollisionProp()->WorldSpaceCenter();

		Vector vecDamagePos;
		pOther->CollisionProp()->CalcNearestPoint( vecCenter, &vecDamagePos );

		CTakeDamageInfo info( this, this, damage, m_bitsDamageInflict );
		info.SetDamagePosition( vecDamagePos );
		GuessDamageForce( &info, ( vecDamagePos - vecCenter ), vecDamagePos );
		pOther->TakeDamage( info );
	}

	if (pOther->IsPlayer())
	{
		m_OnHurtPlayer.FireOutput(pOther, this);
	}
	else
	{
		m_OnHurt.FireOutput(pOther, this);
	}
	m_hurtEntities.AddToTail( EHANDLE(pOther) );
	//NDebugOverlay::Box( pOther->GetAbsOrigin(), pOther->WorldAlignMins(), pOther->WorldAlignMaxs(), 255,0,0,0,0.5 );
	return true;
}

void CTriggerHurt::HurtThink()
{
	// if I hurt anyone, think again
	if ( HurtAllTouchers( 0.5 ) <= 0 )
	{
		SetThink(NULL);
	}
	else
	{
		SetNextThink( gpGlobals->curtime + 0.5f );
	}
}

void CTriggerHurt::EndTouch( CBaseEntity *pOther )
{
	if (PassesTriggerFilters(pOther))
	{
		EHANDLE hOther;
		hOther = pOther;

		// if this guy has never taken damage, hurt him now
		if ( !m_hurtEntities.HasElement( hOther ) )
		{
			HurtEntity( pOther, m_flDamage * 0.5 );
		}
	}
	BaseClass::EndTouch( pOther );
}

//-----------------------------------------------------------------------------
// Purpose: called from RadiationThink() as well as HurtThink()
//			This function applies damage to any entities currently touching the
//			trigger
// Input  : dt - time since last call
// Output : int - number of entities actually hurt
//-----------------------------------------------------------------------------
#define TRIGGER_HURT_FORGIVE_TIME	3.0f	// time in seconds
int CTriggerHurt::HurtAllTouchers( float dt )
{
	int hurtCount = 0;
	// half second worth of damage
	float fldmg = m_flDamage * dt;
	m_flLastDmgTime = gpGlobals->curtime;

	m_hurtEntities.RemoveAll();

	touchlink_t *root = ( touchlink_t * )GetDataObject( TOUCHLINK );
	if ( root )
	{
		for ( touchlink_t *link = root->nextLink; link != root; link = link->nextLink )
		{
			CBaseEntity *pTouch = link->entityTouched;
			if ( pTouch )
			{
				if ( HurtEntity( pTouch, fldmg ) )
				{
					hurtCount++;
				}
			}
		}
	}

	if( m_damageModel == DAMAGEMODEL_DOUBLE_FORGIVENESS )
	{
		if( hurtCount == 0 )
		{
			if( gpGlobals->curtime > m_flDmgResetTime  )
			{
				// Didn't hurt anyone. Reset the damage if it's time. (hence, the forgiveness)
				m_flDamage = m_flOriginalDamage;
			}
		}
		else
		{
			// Hurt someone! double the damage
			m_flDamage *= 2.0f;

			if( m_flDamage > m_flDamageCap )
			{
				// Clamp
				m_flDamage = m_flDamageCap;
			}

			// Now, put the damage reset time into the future. The forgive time is how long the trigger
			// must go without harming anyone in order that its accumulated damage be reset to the amount
			// set by the level designer. This is a stop-gap for an exploit where players could hop through
			// slime and barely take any damage because the trigger would reset damage anytime there was no
			// one in the trigger when this function was called. (sjb)
			m_flDmgResetTime = gpGlobals->curtime + TRIGGER_HURT_FORGIVE_TIME;
		}
	}

	return hurtCount;
}

void CTriggerHurt::Touch( CBaseEntity *pOther )
{
	if ( m_pfnThink == NULL )
	{
		SetThink( &CTriggerHurt::HurtThink );
		SetNextThink( gpGlobals->curtime );
	}
}


// ##################################################################################
//	>> TriggerMultiple
// ##################################################################################
LINK_ENTITY_TO_CLASS( trigger_multiple, CTriggerMultiple );


BEGIN_DATADESC( CTriggerMultiple )

	// Function Pointers
	DEFINE_FUNCTION(MultiTouch),
	DEFINE_FUNCTION(MultiWaitOver ),

	// Outputs
	DEFINE_OUTPUT(m_OnTrigger, "OnTrigger")

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerMultiple::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();

	if (m_flWait == 0)
	{
		m_flWait = 0.2;
	}

	ASSERTSZ(m_iHealth == 0, "trigger_multiple with health");
	SetTouch( &CTriggerMultiple::MultiTouch );
}


//-----------------------------------------------------------------------------
// Purpose: Touch function. Activates the trigger.
// Input  : pOther - The thing that touched us.
//-----------------------------------------------------------------------------
void CTriggerMultiple::MultiTouch(CBaseEntity *pOther)
{
	if (PassesTriggerFilters(pOther))
	{
		ActivateMultiTrigger( pOther );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pActivator - 
//-----------------------------------------------------------------------------
void CTriggerMultiple::ActivateMultiTrigger(CBaseEntity *pActivator)
{
	if (GetNextThink() > gpGlobals->curtime)
		return;         // still waiting for reset time

	m_hActivator = pActivator;

	m_OnTrigger.FireOutput(m_hActivator, this);

	if (m_flWait > 0)
	{
		SetThink( &CTriggerMultiple::MultiWaitOver );
		SetNextThink( gpGlobals->curtime + m_flWait );
	}
	else
	{
		// we can't just remove (self) here, because this is a touch function
		// called while C code is looping through area links...
		SetTouch( NULL );
		SetNextThink( gpGlobals->curtime + 0.1f );
		SetThink(  &CTriggerMultiple::SUB_Remove );
	}
}


//-----------------------------------------------------------------------------
// Purpose: The wait time has passed, so set back up for another activation
//-----------------------------------------------------------------------------
void CTriggerMultiple::MultiWaitOver( void )
{
	SetThink( NULL );
}

// ##################################################################################
//	>> TriggerOnce
// ##################################################################################
class CTriggerOnce : public CTriggerMultiple
{
	DECLARE_CLASS( CTriggerOnce, CTriggerMultiple );
public:

	void Spawn( void );
};

LINK_ENTITY_TO_CLASS( trigger_once, CTriggerOnce );

void CTriggerOnce::Spawn( void )
{
	BaseClass::Spawn();

	m_flWait = -1;
}

// ##################################################################################
//	>> TriggerVolume
// ##################################################################################
class CTriggerVolume : public CPointEntity	// Derive from point entity so this doesn't move across levels
{
public:
	DECLARE_CLASS( CTriggerVolume, CPointEntity );

	void		Spawn( void );
};

LINK_ENTITY_TO_CLASS( trigger_transition, CTriggerVolume );

// Define space that travels across a level transition
void CTriggerVolume::Spawn( void )
{
	SetSolid( SOLID_BSP );
	AddSolidFlags( FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) );    // set size and link into world
	if ( showtriggers.GetInt() == 0 )
	{
		AddEffects( EF_NODRAW );
	}
}

#define SF_CHANGELEVEL_NOTOUCH		0x0002
#define SF_CHANGELEVEL_CHAPTER		0x0004

#define cchMapNameMost 32

enum
{
	TRANSITION_VOLUME_SCREENED_OUT = 0,
	TRANSITION_VOLUME_NOT_FOUND = 1,
	TRANSITION_VOLUME_PASSED = 2,
};


//-----------------------------------------------------------------------------
// Purpose: A trigger that pushes the player, NPCs, or objects.
//-----------------------------------------------------------------------------
class CTriggerPush : public CBaseTrigger
{
public:
	DECLARE_CLASS( CTriggerPush, CBaseTrigger );

	void Spawn( void );
	void Touch( CBaseEntity *pOther );
	void Untouch( CBaseEntity *pOther );

	Vector m_vecPushDir;

	DECLARE_DATADESC();
};

BEGIN_DATADESC( CTriggerPush )
	DEFINE_KEYFIELD( m_vecPushDir, FIELD_VECTOR, "pushdir" ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_push, CTriggerPush );


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerPush::Spawn( )
{
	// Convert pushdir from angles to a vector
	Vector vecAbsDir;
	QAngle angPushDir = QAngle(m_vecPushDir.x, m_vecPushDir.y, m_vecPushDir.z);
	AngleVectors(angPushDir, &vecAbsDir);

	// Transform the vector into entity space
	VectorIRotate( vecAbsDir, EntityToWorldTransform(), m_vecPushDir );

	BaseClass::Spawn();

	InitTrigger();

	if (m_flSpeed == 0)
	{
		m_flSpeed = 100;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CTriggerPush::Touch( CBaseEntity *pOther )
{
	if ( !pOther->IsSolid() || (pOther->GetMoveType() == MOVETYPE_PUSH || pOther->GetMoveType() == MOVETYPE_NONE ) )
		return;

	if (!PassesTriggerFilters(pOther))
		return;

	// FIXME: If something is hierarchically attached, should we try to push the parent?
	if (pOther->GetMoveParent())
		return;

	// Transform the push dir into global space
	Vector vecAbsDir;
	VectorRotate( m_vecPushDir, EntityToWorldTransform(), vecAbsDir );

	// Instant trigger, just transfer velocity and remove
	if (HasSpawnFlags(SF_TRIG_PUSH_ONCE))
	{
		pOther->ApplyAbsVelocityImpulse( m_flSpeed * vecAbsDir );

		if ( vecAbsDir.z > 0 )
		{
			pOther->SetGroundEntity( NULL );
		}
		UTIL_Remove( this );
		return;
	}

	switch( pOther->GetMoveType() )
	{
	case MOVETYPE_NONE:
	case MOVETYPE_PUSH:
	case MOVETYPE_NOCLIP:
		break;

	case MOVETYPE_VPHYSICS:
		{
			IPhysicsObject *pPhys = pOther->VPhysicsGetObject();
			if ( pPhys )
			{
				// UNDONE: Assume the velocity is for a 100kg object, scale with mass
				pPhys->ApplyForceCenter( m_flSpeed * vecAbsDir * 100.0f * gpGlobals->frametime );
				return;
			}
		}
		break;

	default:
		{
#if defined( HL2_DLL )
			// HACK HACK  HL2 players on ladders will only be disengaged if the sf is set, otherwise no push occurs.
			if ( pOther->IsPlayer() && 
				 pOther->GetMoveType() == MOVETYPE_LADDER )
			{
				if ( !HasSpawnFlags(SF_TRIG_PUSH_AFFECT_PLAYER_ON_LADDER) )
				{
					// Ignore the push
					return;
				}
			}
#endif

			Vector vecPush = (m_flSpeed * vecAbsDir);
			if ( pOther->GetFlags() & FL_BASEVELOCITY )
			{
				vecPush = vecPush + pOther->GetBaseVelocity();
			}
			if ( vecPush.z > 0 && (pOther->GetFlags() & FL_ONGROUND) )
			{
				pOther->SetGroundEntity( NULL );
				Vector origin = pOther->GetAbsOrigin();
				origin.z += 1.0f;
				pOther->SetAbsOrigin( origin );
			}

#ifdef HL1_DLL
			// Apply the z velocity as a force so it counteracts gravity properly
			Vector vecImpulse( 0, 0, vecPush.z * 0.025 );//magic hack number

			pOther->ApplyAbsVelocityImpulse( vecImpulse );

			// apply x, y as a base velocity so we travel at constant speed on conveyors
			vecPush.z = 0;
#endif			

			pOther->SetBaseVelocity( vecPush );
			pOther->AddFlag( FL_BASEVELOCITY );
		}
		break;
	}
}

// ##################################################################################
//	>> TriggerWind
//
//  Blows physics objects in the trigger
//
// ##################################################################################

#define MAX_WIND_CHANGE		5.0f

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
class CPhysicsWind : public IMotionEvent
{
	DECLARE_SIMPLE_DATADESC();

public:
	simresult_e Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
	{
		// If we have no windspeed, we're not doing anything
		if ( !m_flWindSpeed )
			return IMotionEvent::SIM_NOTHING;

		// Get a cosine modulated noise between 5 and 20 that is object specific
		int nNoiseMod = 5+(int)pObject%15; // 

		// Turn wind yaw direction into a vector and add noise
		QAngle vWindAngle = vec3_angle;	
		vWindAngle[1] = m_nWindYaw+(30*cos(nNoiseMod * gpGlobals->curtime + nNoiseMod));
		Vector vWind;
		AngleVectors(vWindAngle,&vWind);

		// Add lift with noise
		vWind.z = 1.1 + (1.0 * sin(nNoiseMod * gpGlobals->curtime + nNoiseMod));

		linear = 3*vWind*m_flWindSpeed;
		angular = vec3_origin;
		return IMotionEvent::SIM_GLOBAL_FORCE;	
	}

	int		m_nWindYaw;
	float	m_flWindSpeed;
};

BEGIN_SIMPLE_DATADESC( CPhysicsWind )

	DEFINE_FIELD( m_nWindYaw,		FIELD_INTEGER ),
	DEFINE_FIELD( m_flWindSpeed,	FIELD_FLOAT ),

END_DATADESC()


extern short g_sModelIndexSmoke;
extern float	GetFloorZ(const Vector &origin);
#define WIND_THINK_CONTEXT		"WindThinkContext"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTriggerWind : public CBaseVPhysicsTrigger
{
	DECLARE_CLASS( CTriggerWind, CBaseVPhysicsTrigger );
public:
	DECLARE_DATADESC();

	void	Spawn( void );
	bool	KeyValue( const char *szKeyName, const char *szValue );
	void	OnRestore();
	void	UpdateOnRemove();
	bool	CreateVPhysics();
	void	StartTouch( CBaseEntity *pOther );
	void	EndTouch( CBaseEntity *pOther );
	void	WindThink( void );
	int		DrawDebugTextOverlays( void );

	// Input handlers
	void	InputEnable( inputdata_t &inputdata );
	void	InputSetSpeed( inputdata_t &inputdata );

private:
	int 	m_nSpeedBase;	// base line for how hard the wind blows
	int		m_nSpeedNoise;	// noise added to wind speed +/-
	int		m_nSpeedCurrent;// current wind speed
	int		m_nSpeedTarget;	// wind speed I'm approaching

	int		m_nDirBase;		// base line for direction the wind blows (yaw)
	int		m_nDirNoise;	// noise added to wind direction
	int		m_nDirCurrent;	// the current wind direction
	int		m_nDirTarget;	// wind direction I'm approaching

	int		m_nHoldBase;	// base line for how long to wait before changing wind
	int		m_nHoldNoise;	// noise added to how long to wait before changing wind

	bool	m_bSwitch;		// when does wind change

	IPhysicsMotionController*	m_pWindController;
	CPhysicsWind				m_WindCallback;

};

LINK_ENTITY_TO_CLASS( trigger_wind, CTriggerWind );

BEGIN_DATADESC( CTriggerWind )

	DEFINE_FIELD( m_nSpeedCurrent, FIELD_INTEGER),
	DEFINE_FIELD( m_nSpeedTarget,	FIELD_INTEGER),
	DEFINE_FIELD( m_nDirBase,		FIELD_INTEGER),
	DEFINE_FIELD( m_nDirCurrent,	FIELD_INTEGER),
	DEFINE_FIELD( m_nDirTarget,	FIELD_INTEGER),
	DEFINE_FIELD( m_bSwitch,		FIELD_BOOLEAN),

	DEFINE_FIELD( m_nSpeedBase,		FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_nSpeedNoise,	FIELD_INTEGER, "SpeedNoise"),
	DEFINE_KEYFIELD( m_nDirNoise,	FIELD_INTEGER, "DirectionNoise"),
	DEFINE_KEYFIELD( m_nHoldBase,	FIELD_INTEGER, "HoldTime"),
	DEFINE_KEYFIELD( m_nHoldNoise,	FIELD_INTEGER, "HoldNoise"),

	DEFINE_PHYSPTR( m_pWindController ),
	DEFINE_EMBEDDED( m_WindCallback ),

	DEFINE_FUNCTION( WindThink ),

	DEFINE_INPUTFUNC( FIELD_INTEGER, "SetSpeed", InputSetSpeed ),

END_DATADESC()


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::Spawn( void )
{
	m_bSwitch = true;
	m_nDirBase = GetLocalAngles().y;

	BaseClass::Spawn();

	m_nSpeedCurrent = m_nSpeedBase;
	m_nDirCurrent = m_nDirBase;

	SetContextThink( &CTriggerWind::WindThink, gpGlobals->curtime, WIND_THINK_CONTEXT );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTriggerWind::KeyValue( const char *szKeyName, const char *szValue )
{
	// Done here to avoid collision with CBaseEntity's speed key
	if ( FStrEq(szKeyName, "Speed") )
	{
		m_nSpeedBase = atoi( szValue );
	}
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}

//------------------------------------------------------------------------------
// Create VPhysics
//------------------------------------------------------------------------------
bool CTriggerWind::CreateVPhysics()
{
	BaseClass::CreateVPhysics();

	m_pWindController = physenv->CreateMotionController( &m_WindCallback );
	return true;
}

//------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------
void CTriggerWind::UpdateOnRemove()
{
	if ( m_pWindController )
	{
		physenv->DestroyMotionController( m_pWindController );
		m_pWindController = NULL;
	}

	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::OnRestore()
{
	BaseClass::OnRestore();
	if ( m_pWindController )
	{
		m_pWindController->SetEventHandler( &m_WindCallback );
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::StartTouch(CBaseEntity *pOther)
{
	if ( !PassesTriggerFilters(pOther) )
		return;
	if ( pOther->IsPlayer() )
		return;

	IPhysicsObject *pPhys = pOther->VPhysicsGetObject();
	if ( pPhys)
	{
		m_pWindController->AttachObject( pPhys, false );
		pPhys->Wake();
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::EndTouch(CBaseEntity *pOther)
{
	if ( !PassesTriggerFilters(pOther) )
		return;
	if ( pOther->IsPlayer() )
		return;

	IPhysicsObject *pPhys = pOther->VPhysicsGetObject();
	if ( pPhys && m_pWindController )
	{
		m_pWindController->DetachObject( pPhys );
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::InputEnable( inputdata_t &inputdata )
{
	BaseClass::InputEnable( inputdata );
	SetContextThink( &CTriggerWind::WindThink, gpGlobals->curtime + 0.1f, WIND_THINK_CONTEXT );
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::WindThink( void )
{
	// By default...
	SetContextThink( &CTriggerWind::WindThink, gpGlobals->curtime + 0.1, WIND_THINK_CONTEXT );

	// Is it time to change the wind?
	if (m_bSwitch)
	{
		m_bSwitch = false;

		// Set new target direction and speed
		m_nSpeedTarget = m_nSpeedBase + random->RandomInt( -m_nSpeedNoise, m_nSpeedNoise );
		m_nDirTarget = UTIL_AngleMod( m_nDirBase + random->RandomInt(-m_nDirNoise, m_nDirNoise) );
	}
	else
	{
		bool bDone = true;
		// either ramp up, or sleep till change
		if (abs(m_nSpeedTarget - m_nSpeedCurrent) > MAX_WIND_CHANGE)
		{
			m_nSpeedCurrent += (m_nSpeedTarget > m_nSpeedCurrent) ? MAX_WIND_CHANGE : -MAX_WIND_CHANGE;
			bDone = false;
		}

		if (abs(m_nDirTarget - m_nDirCurrent) > MAX_WIND_CHANGE)
		{

			m_nDirCurrent = UTIL_ApproachAngle( m_nDirTarget, m_nDirCurrent, MAX_WIND_CHANGE );
			bDone = false;
		}
		
		if (bDone)
		{
			m_nSpeedCurrent = m_nSpeedTarget;
			SetContextThink( &CTriggerWind::WindThink, m_nHoldBase + random->RandomFloat(-m_nHoldNoise,m_nHoldNoise), WIND_THINK_CONTEXT );
			m_bSwitch = true;
		}
	}

	// If we're starting to blow, where we weren't before, wake up all our objects
	if ( m_nSpeedCurrent )
	{
		m_pWindController->WakeObjects();
	}

	// store the wind data in the controller callback
	m_WindCallback.m_nWindYaw = m_nDirCurrent;
	if ( m_bDisabled )
	{
		m_WindCallback.m_flWindSpeed = 0;
	}
	else
	{
		m_WindCallback.m_flWindSpeed = m_nSpeedCurrent;
	}
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerWind::InputSetSpeed( inputdata_t &inputdata )
{
	// Set new speed and mark to switch
	m_nSpeedBase = inputdata.value.Int();
	m_bSwitch = true;
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CTriggerWind::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		// --------------
		// Print Target
		// --------------
		char tempstr[255];
		Q_snprintf(tempstr,sizeof(tempstr),"Dir: %i (%i)",m_nDirCurrent,m_nDirTarget);
		EntityText(text_offset,tempstr,0);
		text_offset++;

		Q_snprintf(tempstr,sizeof(tempstr),"Speed: %i (%i)",m_nSpeedCurrent,m_nSpeedTarget);
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}


// ##################################################################################
//	>> TriggerImpact
//
//  Blows physics objects in the trigger
//
// ##################################################################################
#define TRIGGERIMPACT_VIEWKICK_SCALE 0.1

class CTriggerImpact : public CTriggerMultiple
{
	DECLARE_CLASS( CTriggerImpact, CTriggerMultiple );
public:
	DECLARE_DATADESC();

	float	m_flMagnitude;
	float	m_flNoise;
	float	m_flViewkick;

	void	Spawn( void );
	void	StartTouch( CBaseEntity *pOther );

	// Inputs
	void InputSetMagnitude( inputdata_t &inputdata );
	void InputImpact( inputdata_t &inputdata );

	// Outputs
	COutputVector	m_pOutputForce;		// Output force in case anyone else wants to use it

	// Debug
	int		DrawDebugTextOverlays(void);
};

LINK_ENTITY_TO_CLASS( trigger_impact, CTriggerImpact );

BEGIN_DATADESC( CTriggerImpact )

	DEFINE_KEYFIELD( m_flMagnitude,	FIELD_FLOAT, "Magnitude"),
	DEFINE_KEYFIELD( m_flNoise,		FIELD_FLOAT, "Noise"),
	DEFINE_KEYFIELD( m_flViewkick,	FIELD_FLOAT, "Viewkick"),

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID,  "Impact", InputImpact ),
	DEFINE_INPUTFUNC( FIELD_FLOAT, "SetMagnitude", InputSetMagnitude ),

	// Outputs
	DEFINE_OUTPUT(m_pOutputForce, "ImpactForce"),

	// Function Pointers
	DEFINE_FUNCTION( Disable ),


END_DATADESC()


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerImpact::Spawn( void )
{	
	// Clamp date in case user made an error
	m_flNoise = clamp(m_flNoise,0,1);
	m_flViewkick = clamp(m_flViewkick,0,1);

	// Always start disabled
	m_bDisabled = true;
	BaseClass::Spawn();
}


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerImpact::InputImpact( inputdata_t &inputdata )
{
	// Output the force vector in case anyone else wants to use it
	Vector vDir;
	AngleVectors( GetLocalAngles(),&vDir );
	m_pOutputForce.Set( m_flMagnitude * vDir, inputdata.pActivator, inputdata.pCaller);

	// Enable long enough to throw objects inside me
	Enable();
	SetNextThink( gpGlobals->curtime + 0.1f );
	SetThink(&CTriggerImpact::Disable);
}

// Pongles [

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerImpact::StartTouch(CBaseEntity *pOther)
{
	//If the entity is valid and has physics, hit it
	if ( ( pOther != NULL  ) && ( pOther->VPhysicsGetObject() != NULL ) )
	{
		Vector vDir;
		AngleVectors( GetLocalAngles(),&vDir );
		vDir += RandomVector(-m_flNoise,m_flNoise);
		pOther->VPhysicsGetObject()->ApplyForceCenter( m_flMagnitude * vDir );
	}

	// If the player, so a view kick
	if (pOther->IsPlayer() && fabs(m_flMagnitude)>0 )
	{
		Vector vDir;
		AngleVectors( GetLocalAngles(),&vDir );

		float flPunch = -m_flViewkick*m_flMagnitude*TRIGGERIMPACT_VIEWKICK_SCALE;

		CBasePlayer *pPlayer = ToBasePlayer( pOther );
		pPlayer->ViewPunch( QAngle( vDir.y * flPunch, 0, vDir.x * flPunch ) );
	}
}

// Pongles ]


//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CTriggerImpact::InputSetMagnitude( inputdata_t &inputdata )
{
	m_flMagnitude = inputdata.value.Float();
}


//-----------------------------------------------------------------------------
// Purpose: Draw any debug text overlays
// Output : Current text offset from the top
//-----------------------------------------------------------------------------
int CTriggerImpact::DrawDebugTextOverlays(void) 
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT) 
	{
		char tempstr[255];
		Q_snprintf(tempstr,sizeof(tempstr),"Magnitude: %3.2f",m_flMagnitude);
		EntityText(text_offset,tempstr,0);
		text_offset++;
	}
	return text_offset;
}

//-----------------------------------------------------------------------------
// Purpose: Disables auto movement on players that touch it
//-----------------------------------------------------------------------------

const int SF_TRIGGER_MOVE_AUTODISABLE				= 0x80; // disable auto movement

class CTriggerPlayerMovement : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerPlayerMovement, CBaseTrigger );
public:

	void Spawn( void );
	void StartTouch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );
	
	DECLARE_DATADESC();

};

BEGIN_DATADESC( CTriggerPlayerMovement )

END_DATADESC()


LINK_ENTITY_TO_CLASS( trigger_playermovement, CTriggerPlayerMovement );


//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerPlayerMovement::Spawn( void )
{
	if( HasSpawnFlags( SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS ) )
	{
		// @Note (toml 01-07-04): fix up spawn flag collision coding error. Remove at some point once all maps fixed up please!
		DevMsg("*** trigger_playermovement using obsolete spawnflag. Remove and reset with new value for \"Disable auto player movement\"\n" );
		RemoveSpawnFlags(SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS);
		AddSpawnFlags(SF_TRIGGER_MOVE_AUTODISABLE);
	}
	BaseClass::Spawn();

	InitTrigger();
}


// UNDONE: This will not support a player touching more than one of these
// UNDONE: Do we care?  If so, ref count automovement in the player?
void CTriggerPlayerMovement::StartTouch( CBaseEntity *pOther )
{	
	if (!PassesTriggerFilters(pOther))
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOther );

	if ( !pPlayer )
		return;

	// UNDONE: Currently this is the only operation this trigger can do
	if ( HasSpawnFlags(SF_TRIGGER_MOVE_AUTODISABLE) )
	{
		pPlayer->m_Local.m_bAllowAutoMovement = false;
	}
}

void CTriggerPlayerMovement::EndTouch( CBaseEntity *pOther )
{
	if (!PassesTriggerFilters(pOther))
		return;

	CBasePlayer *pPlayer = ToBasePlayer( pOther );

	if ( !pPlayer )
		return;

	if ( HasSpawnFlags(SF_TRIGGER_MOVE_AUTODISABLE) )
	{
		pPlayer->m_Local.m_bAllowAutoMovement = true;
	}
}

//------------------------------------------------------------------------------
// Base VPhysics trigger implementation
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Save/load
//------------------------------------------------------------------------------
BEGIN_DATADESC( CBaseVPhysicsTrigger )
	DEFINE_KEYFIELD( m_bDisabled,		FIELD_BOOLEAN,	"StartDisabled" ),

	// Pongles [
	//DEFINE_KEYFIELD( m_iFilterName,	FIELD_STRING,	"filtername" ),
	//DEFINE_FIELD( m_hFilter,	FIELD_EHANDLE ),
	// Pongles ]

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
END_DATADESC()

//------------------------------------------------------------------------------
// Spawn
//------------------------------------------------------------------------------
void CBaseVPhysicsTrigger::Spawn()
{
	Precache();

	SetSolid( SOLID_VPHYSICS );	
	AddSolidFlags( FSOLID_NOT_SOLID );

	// NOTE: Don't make yourself FSOLID_TRIGGER here or you'll get game 
	// collisions AND vphysics collisions.  You don't want any game collisions
	// so just use FSOLID_NOT_SOLID

	SetMoveType( MOVETYPE_NONE );
	SetModel( STRING( GetModelName() ) );    // set size and link into world
	if ( showtriggers.GetInt() == 0 )
	{
		AddEffects( EF_NODRAW );
	}

	CreateVPhysics();
}

//------------------------------------------------------------------------------
// Create VPhysics
//------------------------------------------------------------------------------
bool CBaseVPhysicsTrigger::CreateVPhysics()
{
	IPhysicsObject *pPhysics;
	if ( !HasSpawnFlags( SF_VPHYSICS_MOTION_MOVEABLE ) )
	{
		pPhysics = VPhysicsInitStatic();
	}
	else
	{
		pPhysics = VPhysicsInitShadow( false, false );
	}

	pPhysics->BecomeTrigger();
	return true;
}

//------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------
void CBaseVPhysicsTrigger::UpdateOnRemove()
{
	if ( VPhysicsGetObject())
	{
		VPhysicsGetObject()->RemoveTrigger();
	}

	BaseClass::UpdateOnRemove();
}

//------------------------------------------------------------------------------
// Activate
//------------------------------------------------------------------------------
void CBaseVPhysicsTrigger::Activate( void ) 
{ 
	// Get a handle to my filter entity if there is one
	if (m_iFilterName != NULL_STRING)
	{
		m_hFilter = dynamic_cast<CBaseFilter *>(gEntList.FindEntityByName( NULL, m_iFilterName ));
	}

	BaseClass::Activate();
}

//------------------------------------------------------------------------------
// Inputs
//------------------------------------------------------------------------------
void CBaseVPhysicsTrigger::InputToggle( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		InputEnable( inputdata );
	}
	else
	{
		InputDisable( inputdata );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseVPhysicsTrigger::InputEnable( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		m_bDisabled = false;
		if ( VPhysicsGetObject())
		{
			VPhysicsGetObject()->EnableCollisions( true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseVPhysicsTrigger::InputDisable( inputdata_t &inputdata )
{
	if ( !m_bDisabled )
	{
		m_bDisabled = true;
		if ( VPhysicsGetObject())
		{
			VPhysicsGetObject()->EnableCollisions( false );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseVPhysicsTrigger::StartTouch( CBaseEntity *pOther )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseVPhysicsTrigger::EndTouch( CBaseEntity *pOther )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseVPhysicsTrigger::PassesTriggerFilters( CBaseEntity *pOther )
{
	if ( pOther->GetMoveType() != MOVETYPE_VPHYSICS && !pOther->IsPlayer() )
		return false;

	// First test spawn flag filters
	if ( HasSpawnFlags(SF_TRIGGER_ALLOW_ALL) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_CLIENTS) && (pOther->GetFlags() & FL_CLIENT)) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_NPCS) && (pOther->GetFlags() & FL_NPC)) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_PUSHABLES) && FClassnameIs(pOther, "func_pushable")) ||
		(HasSpawnFlags(SF_TRIGGER_ALLOW_PHYSICS) && pOther->GetMoveType() == MOVETYPE_VPHYSICS))
	{
		bool bOtherIsPlayer = pOther->IsPlayer();

		/*// Pongles [
		if( HasSpawnFlags(SF_TRIGGER_ONLY_PLAYER_ALLY_NPCS) && !bOtherIsPlayer )
		{
			CAI_BaseNPC *pNPC = pOther->MyNPCPointer();

			if( !pNPC || !pNPC->IsPlayerAlly() )
			{
				return false;
			}
		}*/
		// Pongles ]

		if ( HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_IN_VEHICLES) && bOtherIsPlayer )
		{
			if ( !((CBasePlayer*)pOther)->IsInAVehicle() )
				return false;
		}

		if ( HasSpawnFlags(SF_TRIGGER_ONLY_CLIENTS_OUT_OF_VEHICLES) && bOtherIsPlayer )
		{
			if ( ((CBasePlayer*)pOther)->IsInAVehicle() )
				return false;
		}

		CBaseFilter *pFilter = m_hFilter.Get();
		return (!pFilter) ? true : pFilter->PassesFilter( this, pOther );
	}

	return false;
}

//=====================================================================================================================
//-----------------------------------------------------------------------------
// Purpose: VPhysics trigger that changes the motion of vphysics objects that touch it
//-----------------------------------------------------------------------------
class CTriggerVPhysicsMotion : public CBaseVPhysicsTrigger, public IMotionEvent
{
	DECLARE_CLASS( CTriggerVPhysicsMotion, CBaseVPhysicsTrigger );

public:
	void Spawn();
	void Precache();
	virtual void UpdateOnRemove();
	bool CreateVPhysics();
	void OnRestore();

	// UNDONE: Pass trigger event in or change Start/EndTouch.  Add ITriggerVPhysics perhaps?
	// BUGBUG: If a player touches two of these, his movement will screw up.
	// BUGBUG: If a player uses crouch/uncrouch it will generate touch events and clear the motioncontroller flag
	void StartTouch( CBaseEntity *pOther );
	void EndTouch( CBaseEntity *pOther );

	void InputSetVelocityLimitTime( inputdata_t &inputdata );

	float LinearLimit();

	inline bool HasGravityScale() { return m_gravityScale != 1.0 ? true : false; }
	inline bool HasAirDensity() { return m_addAirDensity != 0 ? true : false; }
	inline bool HasLinearLimit() { return LinearLimit() != 0.0f; }
	inline bool HasLinearScale() { return m_linearScale != 1.0 ? true : false; }
	inline bool HasAngularLimit() { return m_angularLimit != 0 ? true : false; }
	inline bool HasAngularScale() { return m_angularScale != 1.0 ? true : false; }
	inline bool HasLinearForce() { return m_linearForce != 0.0 ? true : false; }

	DECLARE_DATADESC();

	virtual simresult_e	Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );

private:
	IPhysicsMotionController	*m_pController;

#ifndef _XBOX
	EntityParticleTrailInfo_t	m_ParticleTrail;
#endif //!_XBOX

	float						m_gravityScale;
	float						m_addAirDensity;
	float						m_linearLimit;
	float						m_linearLimitDelta;
	float						m_linearLimitTime;
	float						m_linearLimitStart;
	float						m_linearLimitStartTime;
	float						m_linearScale;
	float						m_angularLimit;
	float						m_angularScale;
	float						m_linearForce;
	QAngle						m_linearForceAngles;
};


//------------------------------------------------------------------------------
// Save/load
//------------------------------------------------------------------------------
BEGIN_DATADESC( CTriggerVPhysicsMotion )
	DEFINE_PHYSPTR( m_pController ),
#ifndef _XBOX
	DEFINE_EMBEDDED( m_ParticleTrail ),
#endif //!_XBOX
	DEFINE_INPUT( m_gravityScale, FIELD_FLOAT, "SetGravityScale" ),
	DEFINE_INPUT( m_addAirDensity, FIELD_FLOAT, "SetAdditionalAirDensity" ),
	DEFINE_INPUT( m_linearLimit, FIELD_FLOAT, "SetVelocityLimit" ),
	DEFINE_INPUT( m_linearLimitDelta, FIELD_FLOAT, "SetVelocityLimitDelta" ),
	DEFINE_FIELD( m_linearLimitTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_linearLimitStart, FIELD_TIME ),
	DEFINE_FIELD( m_linearLimitStartTime, FIELD_TIME ),
	DEFINE_INPUT( m_linearScale, FIELD_FLOAT, "SetVelocityScale" ),
	DEFINE_INPUT( m_angularLimit, FIELD_FLOAT, "SetAngVelocityLimit" ),
	DEFINE_INPUT( m_angularScale, FIELD_FLOAT, "SetAngVelocityScale" ),
	DEFINE_INPUT( m_linearForce, FIELD_FLOAT, "SetLinearForce" ),
	DEFINE_INPUT( m_linearForceAngles, FIELD_VECTOR, "SetLinearForceAngles" ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetVelocityLimitTime", InputSetVelocityLimitTime ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( trigger_vphysics_motion, CTriggerVPhysicsMotion );


//------------------------------------------------------------------------------
// Spawn
//------------------------------------------------------------------------------
void CTriggerVPhysicsMotion::Spawn()
{
	Precache();

	BaseClass::Spawn();
}

//------------------------------------------------------------------------------
// Precache
//------------------------------------------------------------------------------
void CTriggerVPhysicsMotion::Precache()
{
#ifndef _XBOX
	if ( m_ParticleTrail.m_strMaterialName != NULL_STRING )
	{
		PrecacheMaterial( STRING(m_ParticleTrail.m_strMaterialName) ); 
	}
#endif //!_XBOX
}

//------------------------------------------------------------------------------
// Create VPhysics
//------------------------------------------------------------------------------
float CTriggerVPhysicsMotion::LinearLimit()
{
	if ( m_linearLimitTime == 0.0f )
		return m_linearLimit;

	float dt = gpGlobals->curtime - m_linearLimitStartTime;
	if ( dt >= m_linearLimitTime )
	{
		m_linearLimitTime = 0.0;
		return m_linearLimit;
	}

	dt /= m_linearLimitTime;
	float flLimit = RemapVal( dt, 0.0f, 1.0f, m_linearLimitStart, m_linearLimit );
	return flLimit;
}

	
//------------------------------------------------------------------------------
// Create VPhysics
//------------------------------------------------------------------------------
bool CTriggerVPhysicsMotion::CreateVPhysics()
{
	m_pController = physenv->CreateMotionController( this );
	BaseClass::CreateVPhysics();

	return true;
}


//------------------------------------------------------------------------------
// Cleanup
//------------------------------------------------------------------------------
void CTriggerVPhysicsMotion::UpdateOnRemove()
{
	if ( m_pController )
	{
		physenv->DestroyMotionController( m_pController );
		m_pController = NULL;
	}

	BaseClass::UpdateOnRemove();
}


//------------------------------------------------------------------------------
// Restore
//------------------------------------------------------------------------------
void CTriggerVPhysicsMotion::OnRestore()
{
	BaseClass::OnRestore();
	if ( m_pController )
	{
		m_pController->SetEventHandler( this );
	}
}

//------------------------------------------------------------------------------
// Start/End Touch
//------------------------------------------------------------------------------
// UNDONE: Pass trigger event in or change Start/EndTouch.  Add ITriggerVPhysics perhaps?
// BUGBUG: If a player touches two of these, his movement will screw up.
// BUGBUG: If a player uses crouch/uncrouch it will generate touch events and clear the motioncontroller flag
void CTriggerVPhysicsMotion::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );

	if ( !PassesTriggerFilters(pOther) )
		return;

	// Pongles [

	CBasePlayer *pPlayer = ToBasePlayer( pOther );

	if( pPlayer )
		pPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, true );

	// Pongles ]

	triggerevent_t event;
	PhysGetTriggerEvent( &event, this );
	if ( event.pObject )
	{
		// these all get done again on save/load, so check
		m_pController->AttachObject( event.pObject, true );
	}

	// Don't show these particles on the XBox
	if ( m_ParticleTrail.m_strMaterialName != NULL_STRING )
	{
		CEntityParticleTrail::Create( pOther, m_ParticleTrail, this ); 
	}

	// Pongles [
	/*if ( pOther->GetBaseAnimating() && pOther->GetBaseAnimating()->IsRagdoll() )
	{
		CRagdollBoogie::IncrementSuppressionCount( pOther );
	}*/
	// Pongles ]
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerVPhysicsMotion::EndTouch( CBaseEntity *pOther )
{
	BaseClass::EndTouch( pOther );

	if ( !PassesTriggerFilters(pOther) )
		return;

	// Pongles [

	CBasePlayer *pPlayer = ToBasePlayer( pOther );

	if( pPlayer )
		pPlayer->SetPhysicsFlag( PFLAG_VPHYSICS_MOTIONCONTROLLER, false );

	// Pongles ]

	triggerevent_t event;
	PhysGetTriggerEvent( &event, this );
	if ( event.pObject && m_pController )
	{
		m_pController->DetachObject( event.pObject );
	}

	if ( m_ParticleTrail.m_strMaterialName != NULL_STRING )
	{
		CEntityParticleTrail::Destroy( pOther, m_ParticleTrail ); 
	}

	// Pongles [
	/*if ( pOther->GetBaseAnimating() && pOther->GetBaseAnimating()->IsRagdoll() )
	{
		CRagdollBoogie::DecrementSuppressionCount( pOther );
	}*/
	// Pongles ]
}


//------------------------------------------------------------------------------
// Inputs
//------------------------------------------------------------------------------
void CTriggerVPhysicsMotion::InputSetVelocityLimitTime( inputdata_t &inputdata )
{
	m_linearLimitStart = LinearLimit();
	m_linearLimitStartTime = gpGlobals->curtime;

	float args[2];
	UTIL_StringToFloatArray( args, 2, inputdata.value.String() );
	m_linearLimit = args[0];
	m_linearLimitTime = args[1];
}

//------------------------------------------------------------------------------
// Apply the forces to the entity
//------------------------------------------------------------------------------
IMotionEvent::simresult_e CTriggerVPhysicsMotion::Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular )
{
	if ( m_bDisabled )
		return SIM_NOTHING;

	linear.Init();
	angular.Init();

	if ( HasGravityScale() )
	{
		// assume object already has 1.0 gravities applied to it, so apply the additional amount
		linear.z -= (m_gravityScale-1) * sv_gravity.GetFloat();
	}

	if ( HasLinearForce() )
	{
		Vector vecForceDir;
		AngleVectors( m_linearForceAngles, &vecForceDir );
		VectorMA( linear, m_linearForce, vecForceDir, linear );
	}

	if ( HasAirDensity() || HasLinearLimit() || HasLinearScale() || HasAngularLimit() || HasAngularScale() )
	{
		Vector vel;
		AngularImpulse angVel;
		pObject->GetVelocity( &vel, &angVel );
		vel += linear * deltaTime; // account for gravity scale

		Vector unitVel = vel;
		Vector unitAngVel = angVel;

		float speed = VectorNormalize( unitVel );
		float angSpeed = VectorNormalize( unitAngVel );

		float speedScale = 0.0;
		float angSpeedScale = 0.0;

		if ( HasAirDensity() )
		{
			float linearDrag = -0.5 * m_addAirDensity * pObject->CalculateLinearDrag( unitVel ) * deltaTime;
			if ( linearDrag < -1 )
			{
				linearDrag = -1;
			}
			speedScale += linearDrag / deltaTime;
			float angDrag = -0.5 * m_addAirDensity * pObject->CalculateAngularDrag( unitAngVel ) * deltaTime;
			if ( angDrag < -1 )
			{
				angDrag = -1;
			}
			angSpeedScale += angDrag / deltaTime;
		}

		if ( HasLinearLimit() && speed > m_linearLimit )
		{
			float flDeltaVel = (LinearLimit() - speed) / deltaTime;
			if ( m_linearLimitDelta != 0.0f )
			{
				float flMaxDeltaVel = -m_linearLimitDelta / deltaTime;
				if ( flDeltaVel < flMaxDeltaVel )
				{
					flDeltaVel = flMaxDeltaVel;
				}
			}
			VectorMA( linear, flDeltaVel, unitVel, linear );
		}
		if ( HasAngularLimit() && angSpeed > m_angularLimit )
		{
			angular += ((m_angularLimit - angSpeed)/deltaTime) * unitAngVel;
		}
		if ( HasLinearScale() )
		{
			speedScale = ( (speedScale+1) * m_linearScale ) - 1;
		}
		if ( HasAngularScale() )
		{
			angSpeedScale = ( (angSpeedScale+1) * m_angularScale ) - 1;
		}
		linear += vel * speedScale;
		angular += angVel * angSpeedScale;
	}

	return SIM_GLOBAL_ACCELERATION;
}

// Pongles [

/*class CServerRagdollTrigger : public CBaseTrigger
{
	DECLARE_CLASS( CServerRagdollTrigger, CBaseTrigger );

public:

	virtual void StartTouch( CBaseEntity *pOther );
	virtual void EndTouch( CBaseEntity *pOther );
	virtual void Spawn( void );

};

LINK_ENTITY_TO_CLASS( trigger_serverragdoll, CServerRagdollTrigger );

void CServerRagdollTrigger::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();
}

void CServerRagdollTrigger::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch( pOther );

	if ( pOther->IsPlayer() )
		return;

	CBaseCombatCharacter *pCombatChar = pOther->MyCombatCharacterPointer();

	if ( pCombatChar )
	{
		pCombatChar->m_bForceServerRagdoll = true;
	}
}

void CServerRagdollTrigger::EndTouch(CBaseEntity *pOther)
{
	BaseClass::EndTouch( pOther );

	if ( pOther->IsPlayer() )
		return;

	CBaseCombatCharacter *pCombatChar = pOther->MyCombatCharacterPointer();

	if ( pCombatChar )
	{
		pCombatChar->m_bForceServerRagdoll = false;
	}
}

#ifdef HL1_DLL
//----------------------------------------------------------------------------------
// func_friction
//----------------------------------------------------------------------------------
class CFrictionModifier : public CBaseTrigger
{
	DECLARE_CLASS( CFrictionModifier, CBaseTrigger );

public:
	void		Spawn( void );
	bool		KeyValue( const char *szKeyName, const char *szValue );

	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

	virtual int	ObjectCaps( void ) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	float		m_frictionFraction;

	DECLARE_DATADESC();

};

LINK_ENTITY_TO_CLASS( func_friction, CFrictionModifier );

BEGIN_DATADESC( CFrictionModifier )
	DEFINE_FIELD( m_frictionFraction, FIELD_FLOAT ),
END_DATADESC()

// Modify an entity's friction
void CFrictionModifier::Spawn( void )
{
	BaseClass::Spawn();

	InitTrigger();
}

// Sets toucher's friction to m_frictionFraction (1.0 = normal friction)
bool CFrictionModifier::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "modifier"))
	{
		m_frictionFraction = atof(szValue) / 100.0;
	}
	else
	{
		BaseClass::KeyValue( szKeyName, szValue );
	}
	return true;
}

void CFrictionModifier::StartTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )		// ignore player
	{
		pOther->SetFriction( m_frictionFraction );
	}
}

void CFrictionModifier::EndTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )		// ignore player
	{
		pOther->SetFriction( 1.0f );
	}
}

#endif //HL1_DLL*/

// Pongles ]

bool IsTriggerClass( CBaseEntity *pEntity )
{
	if ( NULL != dynamic_cast<CBaseTrigger *>(pEntity) )
		return true;

	if ( NULL != dynamic_cast<CTriggerVPhysicsMotion *>(pEntity) )
		return true;

	if ( NULL != dynamic_cast<CTriggerVolume *>(pEntity) )
		return true;
	
	return false;
}
