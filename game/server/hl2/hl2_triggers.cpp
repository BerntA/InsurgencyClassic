//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hl2_shared_misc.h"
#include "triggers.h"

//-----------------------------------------------------------------------------
// Weapon-dissolve trigger; all weapons in this field (sans the physcannon) are destroyed!
//-----------------------------------------------------------------------------
class CTriggerWeaponDissolve : public CTriggerMultiple
{
	DECLARE_CLASS( CTriggerWeaponDissolve, CTriggerMultiple );
	DECLARE_DATADESC();

public:
				~CTriggerWeaponDissolve( void );

	virtual void Spawn( void );
	virtual void Precache( void );
	virtual void Activate( void );
	virtual void StartTouch( CBaseEntity *pOther );

	inline bool HasWeapon( CBaseCombatWeapon *pWeapon );

	Vector	GetConduitPoint( CBaseEntity *pTarget );

	void	InputStopSound( inputdata_t &inputdata );

	void	AddWeapon( CBaseCombatWeapon *pWeapon );
	void	CreateBeam( const Vector &vecSource, CBaseEntity *pDest, float flLifetime );
	void	DissolveThink( void );

private:

	COutputEvent	m_OnDissolveWeapon;
	COutputEvent	m_OnChargingPhyscannon;

	CUtlVector< CHandle<CBaseCombatWeapon> >	m_pWeapons;
	CUtlVector< CHandle<CBaseEntity> >			m_pConduitPoints;
	string_t									m_strEmitterName;
	int											m_spriteTexture;
};

LINK_ENTITY_TO_CLASS( trigger_weapon_dissolve, CTriggerWeaponDissolve );

BEGIN_DATADESC( CTriggerWeaponDissolve )

	DEFINE_KEYFIELD( m_strEmitterName,	FIELD_STRING, "emittername" ),

	DEFINE_OUTPUT( m_OnDissolveWeapon, "OnDissolveWeapon" ),
	DEFINE_OUTPUT( m_OnChargingPhyscannon, "OnChargingPhyscannon" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "StopSound", InputStopSound ),

	DEFINE_THINKFUNC( DissolveThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
CTriggerWeaponDissolve::~CTriggerWeaponDissolve( void )
{
	m_pWeapons.Purge();
	m_pConduitPoints.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: Call precache for our sprite texture
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::Spawn( void )
{
	BaseClass::Spawn();
	Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Precache our sprite texture
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::Precache( void )
{
	BaseClass::Precache();

	m_spriteTexture = PrecacheModel( "sprites/lgtning.vmt" );

	PrecacheScriptSound( "WeaponDissolve.Dissolve" );
	PrecacheScriptSound( "WeaponDissolve.Charge" );
	PrecacheScriptSound( "WeaponDissolve.Beam" );
}

static const char *s_pDissolveThinkContext = "DissolveThinkContext";

//-----------------------------------------------------------------------------
// Purpose: Collect all our known conduit points
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::Activate( void )
{
	BaseClass::Activate();

	CBaseEntity *pEntity = NULL;

	while ( ( pEntity = gEntList.FindEntityByName( pEntity, m_strEmitterName ) ) != NULL )
	{
		m_pConduitPoints.AddToTail( pEntity );
	}

	SetContextThink( &CTriggerWeaponDissolve::DissolveThink, gpGlobals->curtime + 0.1f, s_pDissolveThinkContext );
}

//-----------------------------------------------------------------------------
// Purpose: Checks to see if a weapon is already known
// Input  : *pWeapon - weapon to check for
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTriggerWeaponDissolve::HasWeapon( CBaseCombatWeapon *pWeapon )
{
	if ( m_pWeapons.Find( pWeapon ) == m_pWeapons.InvalidIndex() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Adds a weapon to the known weapon list
// Input  : *pWeapon - weapon to add
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::AddWeapon( CBaseCombatWeapon *pWeapon )
{
	if ( HasWeapon( pWeapon ) )
		return;

	m_pWeapons.AddToTail( pWeapon );
}

//-----------------------------------------------------------------------------
// Purpose: Collect any weapons inside our volume
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::StartTouch( CBaseEntity *pOther )
{
	BaseClass::StartTouch( pOther );

	if ( PassesTriggerFilters( pOther ) == false )
		return;

	CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon *>(pOther);

	if ( pWeapon == NULL )
		return;

	AddWeapon( pWeapon );
}

//-----------------------------------------------------------------------------
// Purpose: Creates a beam between a conduit point and a weapon
// Input  : &vecSource - conduit point
//			*pDest - weapon
//			flLifetime - amount of time
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::CreateBeam( const Vector &vecSource, CBaseEntity *pDest, float flLifetime )
{
	CBroadcastRecipientFilter filter;

	te->BeamEntPoint( filter, 0.0,
		0,
		&vecSource,
		pDest->entindex(), 
		&(pDest->WorldSpaceCenter()),
		m_spriteTexture,
		0,				// No halo
		1,				// Frame
		30,
		flLifetime,
		16.0f,			// Start width
		4.0f,			// End width
		0,				// No fade
		8,				// Amplitude
		255,
		255,
		255,
		255,
		16 );			// Speed
}

//-----------------------------------------------------------------------------
// Purpose: Returns the closest conduit point to a weapon
// Input  : *pTarget - weapon to check for
// Output : Vector - position of the conduit
//-----------------------------------------------------------------------------
Vector CTriggerWeaponDissolve::GetConduitPoint( CBaseEntity *pTarget )
{
	float	nearDist = 9999999.0f;
	Vector	bestPoint = vec3_origin;
	float	testDist;

	// Find the nearest conduit to the target
	for ( int i = 0; i < m_pConduitPoints.Count(); i++ )
	{
		testDist = ( m_pConduitPoints[i]->GetAbsOrigin() - pTarget->GetAbsOrigin() ).LengthSqr();

		if ( testDist < nearDist )
		{
			bestPoint = m_pConduitPoints[i]->GetAbsOrigin();
			nearDist = testDist;
		}
	}

	return bestPoint;
}

//-----------------------------------------------------------------------------
// Purpose: Dissolve all weapons within our volume
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::DissolveThink( void )
{
	int	numWeapons = m_pWeapons.Count();

	// Dissolve all the items within the volume
	for ( int i = 0; i < numWeapons; i++ )
	{
		CBaseCombatWeapon *pWeapon = m_pWeapons[i];
		Vector vecConduit = GetConduitPoint( pWeapon );

		// Randomly dissolve them all
		float flLifetime = random->RandomFloat( 2.5f, 4.0f );
		CreateBeam( vecConduit, pWeapon, flLifetime );
		pWeapon->Dissolve( NULL, gpGlobals->curtime + ( 3.0f - flLifetime ), false );

		m_OnDissolveWeapon.FireOutput( this, this );

		CPASAttenuationFilter filter( pWeapon );
		EmitSound( filter, pWeapon->entindex(), "WeaponDissolve.Dissolve" );
		
		// Beam looping sound
		EmitSound( "WeaponDissolve.Beam" );

		m_pWeapons.Remove( i );
		SetContextThink( &CTriggerWeaponDissolve::DissolveThink, gpGlobals->curtime + random->RandomFloat( 0.5f, 1.5f ), s_pDissolveThinkContext );
		return;
	}

	SetContextThink( &CTriggerWeaponDissolve::DissolveThink, gpGlobals->curtime + 0.1f, s_pDissolveThinkContext );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &inputdata - 
//-----------------------------------------------------------------------------
void CTriggerWeaponDissolve::InputStopSound( inputdata_t &inputdata )
{
	StopSound( "WeaponDissolve.Beam" );
	StopSound( "WeaponDissolve.Charge" );
}

//-----------------------------------------------------------------------------
// Teleport trigger
//-----------------------------------------------------------------------------
class CTriggerPhysicsTrap : public CTriggerMultiple
{
	DECLARE_CLASS( CTriggerPhysicsTrap, CTriggerMultiple );
	DECLARE_DATADESC();

public:
	void Touch( CBaseEntity *pOther );

private:
	void InputEnable( inputdata_t &inputdata );
	void InputDisable( inputdata_t &inputdata );
	void InputToggle( inputdata_t &inputdata );

	int m_nDissolveType;
};

//-----------------------------------------------------------------------------
// Save/load
//-----------------------------------------------------------------------------
LINK_ENTITY_TO_CLASS( trigger_physics_trap, CTriggerPhysicsTrap );

BEGIN_DATADESC( CTriggerPhysicsTrap )

	DEFINE_KEYFIELD( m_nDissolveType,	FIELD_INTEGER,	"dissolvetype" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),
	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),

END_DATADESC()

//------------------------------------------------------------------------------
// Inputs
//------------------------------------------------------------------------------
void CTriggerPhysicsTrap::InputToggle( inputdata_t &inputdata )
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

void CTriggerPhysicsTrap::InputEnable( inputdata_t &inputdata )
{
	if ( m_bDisabled )
	{
		Enable();
	}
}

void CTriggerPhysicsTrap::InputDisable( inputdata_t &inputdata )
{
	if ( !m_bDisabled )
	{
		Disable();
	}
}

//-----------------------------------------------------------------------------
// Traps the entities
//-----------------------------------------------------------------------------
#define JOINTS_TO_CONSTRAIN 1

void CTriggerPhysicsTrap::Touch( CBaseEntity *pOther )
{
	if ( !PassesTriggerFilters(pOther) )
		return;

	CBaseAnimating *pAnim = pOther->GetBaseAnimating();
	if ( !pAnim )
		return;

	pAnim->Dissolve( NULL, gpGlobals->curtime, false, m_nDissolveType );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------

class CWateryDeathLeech : public CBaseAnimating
{
	DECLARE_CLASS( CWateryDeathLeech, CBaseAnimating );
public:
	DECLARE_DATADESC();

	void Spawn( void );
	void Precache( void );
	void LeechThink( void );

	int m_iFadeState;
};

LINK_ENTITY_TO_CLASS( ent_watery_leech, CWateryDeathLeech );

BEGIN_DATADESC( CWateryDeathLeech )
	DEFINE_THINKFUNC( LeechThink ),
END_DATADESC()

void CWateryDeathLeech::Precache( void )
{
	//Ugh this is temporary until Jakob finishes the animations and doesn't need the command anymore.
	bool allowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );

	BaseClass::Precache();

	PrecacheModel( "models/leech.mdl" );
	CBaseEntity::SetAllowPrecache( allowPrecache );
}

void CWateryDeathLeech::Spawn( void )
{
	Precache();
	BaseClass::Spawn();

	SetSolid ( SOLID_NONE );

	SetMoveType( MOVETYPE_NONE );
	AddEffects( EF_NOSHADOW );
	
	SetModel( "models/leech.mdl" );

	SetThink( &CWateryDeathLeech::LeechThink );
	SetNextThink( gpGlobals->curtime + 0.1 );

	m_flPlaybackRate = random->RandomFloat( 0.5, 1.5 );
	SetCycle( random->RandomFloat( 0.0f, 0.9f ) );

	QAngle vAngle;
	vAngle[YAW] = random->RandomFloat( 0, 360 );
	SetAbsAngles( vAngle );

	m_iFadeState = 1;
	SetRenderColorA( 1 );
}

void CWateryDeathLeech::LeechThink( void )
{
	if ( IsMarkedForDeletion() )
		 return;

	StudioFrameAdvance();
	SetNextThink( gpGlobals->curtime + 0.1 );

	if ( m_iFadeState != 0 )
	{
		float dt = gpGlobals->frametime;
		if ( dt > 0.1f )
		{
			dt = 0.1f;
		}
		m_nRenderMode = kRenderTransTexture;
		int speed = MAX(1,256*dt); // fade out over 1 second

		if ( m_iFadeState == -1 )
			 SetRenderColorA( UTIL_Approach( 0, m_clrRender->a, speed ) );
		else
			 SetRenderColorA( UTIL_Approach( 255, m_clrRender->a, speed ) );

		if ( m_clrRender->a == 0 )
		{
			UTIL_Remove(this);
		}
		else if ( m_clrRender->a == 255 )
		{
			m_iFadeState = 0;
		}
		else
		{
			SetNextThink( gpGlobals->curtime );
		}
	}


	if ( GetOwnerEntity() )
	{
		if ( GetOwnerEntity()->GetWaterLevel() < 3 )
		{
			AddEffects( EF_NODRAW );
		}
		else
		{
			RemoveEffects( EF_NODRAW );
		}

		SetAbsOrigin( GetOwnerEntity()->GetAbsOrigin() + GetOwnerEntity()->GetViewOffset() );
	}
}

class CTriggerWateryDeath : public CBaseTrigger
{
	DECLARE_CLASS( CTriggerWateryDeath, CBaseTrigger );
public:

	void Spawn( void );
	void Precache( void );
	void Touch( CBaseEntity *pOther );
	void SpawnLeeches( CBaseEntity *pOther );
	
	// Ignore non-living entities
	virtual bool PassesTriggerFilters(CBaseEntity *pOther)
	{
		if ( !BaseClass::PassesTriggerFilters(pOther) )
			return false;

		return (pOther->m_takedamage == DAMAGE_YES);
	}

	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

private:

	CUtlVector< EHANDLE > m_hLeeches;

	// Kill times for entities I'm touching
	CUtlVector< float >	m_flEntityKillTimes;
	float				m_flNextPullSound;
	float				m_flPainValue;
};

LINK_ENTITY_TO_CLASS( trigger_waterydeath, CTriggerWateryDeath );

// Stages of the waterydeath trigger, in time offsets from the initial touch
#define WD_KILLTIME_NEXT_BITE	0.3
#define WD_PAINVALUE_STEP 2.0
#define WD_MAX_DAMAGE 15.0f

//-----------------------------------------------------------------------------
// Purpose: Called when spawning, after keyvalues have been handled.
//-----------------------------------------------------------------------------
void CTriggerWateryDeath::Spawn( void )
{
	BaseClass::Spawn();
	Precache();

	m_flNextPullSound = 0;
	m_flPainValue = 0;
	InitTrigger();
}

void CTriggerWateryDeath::Precache( void )
{
	//Ugh this is temporary until Jakob finishes the animations and doesn't need the command anymore.
	BaseClass::Precache();
	PrecacheModel( "models/leech.mdl" );
	
	PrecacheScriptSound( "coast.leech_bites_loop" );
	PrecacheScriptSound( "coast.leech_water_churn_loop" );
}

void CTriggerWateryDeath::SpawnLeeches( CBaseEntity *pOther )
{
	if ( pOther	== NULL )
		 return;

	if ( m_hLeeches.Count() > 0 )
		 return;

	int iMaxLeeches = 12;
	
	for ( int i = 0; i < iMaxLeeches; i++ )
	{
		CWateryDeathLeech *pLeech = (CWateryDeathLeech*)CreateEntityByName( "ent_watery_leech" );

		if ( pLeech )
		{
			m_hLeeches.AddToTail( pLeech );

			pLeech->Spawn();
			pLeech->SetAbsOrigin( pOther->GetAbsOrigin() );
			pLeech->SetOwnerEntity( pOther );

			if ( i <= 8 )
				 pLeech->SetSequence( i % 4 );
			else 
				 pLeech->SetSequence( ( i % 4 ) + 4 ) ;
			pLeech->ResetSequenceInfo();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTriggerWateryDeath::Touch( CBaseEntity *pOther )
{	
	if (!PassesTriggerFilters(pOther))
		return;

	// Find our index
	EHANDLE hOther;
	hOther = pOther;
	int iIndex = m_hTouchingEntities.Find( hOther );
	if ( iIndex == m_hTouchingEntities.InvalidIndex() )
		return;

	float flKillTime = m_flEntityKillTimes[iIndex];
	
	// Time to kill it?
	if ( gpGlobals->curtime > flKillTime )
	{
		//EmitSound( filter, entindex(), "WateryDeath.Bite", &pOther->GetAbsOrigin() );
		// Kill it
		if ( pOther->IsPlayer() )
		{
			m_flPainValue = MIN( m_flPainValue + WD_PAINVALUE_STEP, WD_MAX_DAMAGE );
		}
		else
		{
			m_flPainValue = WD_MAX_DAMAGE;
		}

		// Use DMG_GENERIC & make the target inflict the damage on himself.
		// This ensures that if the target is the player, the damage isn't modified by skill
		CTakeDamageInfo info = CTakeDamageInfo( pOther, pOther, m_flPainValue, DMG_GENERIC );

		GuessDamageForce( &info, (pOther->GetAbsOrigin() - GetAbsOrigin()), pOther->GetAbsOrigin() );
		pOther->TakeDamage( info );

		m_flEntityKillTimes[iIndex] = gpGlobals->curtime + WD_KILLTIME_NEXT_BITE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Called when an entity starts touching us.
// Input  : pOther - The entity that is touching us.
//-----------------------------------------------------------------------------
void CTriggerWateryDeath::StartTouch(CBaseEntity *pOther)
{
	BaseClass::StartTouch( pOther );

	m_flPainValue = 0.0f;

	// If we added him to our list, store the start time
	EHANDLE hOther;
	hOther = pOther;
	if ( m_hTouchingEntities.Find( hOther ) != m_hTouchingEntities.InvalidIndex() )
	{
		// Always added to the end
		// Players get warned, everything else gets et quick.
		if ( pOther->IsPlayer() )
		{
			m_flEntityKillTimes.AddToTail( gpGlobals->curtime + WD_KILLTIME_NEXT_BITE );
		}
		else
		{
			m_flEntityKillTimes.AddToTail( gpGlobals->curtime + WD_KILLTIME_NEXT_BITE );
		}
	}

#ifdef HL2_DLL
	if ( pOther->IsPlayer() )
	{
		SpawnLeeches( pOther );
	}
#endif
	
}


//-----------------------------------------------------------------------------
// Purpose: Called when an entity stops touching us.
// Input  : pOther - The entity that was touching us.
//-----------------------------------------------------------------------------
void CTriggerWateryDeath::EndTouch( CBaseEntity *pOther )
{
	if ( IsTouching( pOther ) )
	{
		EHANDLE hOther;
		hOther = pOther;

		// Remove the time from our list
		int iIndex = m_hTouchingEntities.Find( hOther );
		if ( iIndex != m_hTouchingEntities.InvalidIndex() )
		{
			m_flEntityKillTimes.Remove( iIndex );
		}
	}

#ifdef HL2_DLL
	if ( pOther->IsPlayer() )
	{
		for (int i = 0; i < m_hLeeches.Count(); i++ )
		{
			CWateryDeathLeech *pLeech = dynamic_cast<CWateryDeathLeech*>( m_hLeeches[i].Get() );

			if ( pLeech )
			{
				pLeech->m_iFadeState = -1;
			}
		}

		if ( m_hLeeches.Count() > 0 )
			 m_hLeeches.Purge();
	}
#endif

	BaseClass::EndTouch( pOther );
}
