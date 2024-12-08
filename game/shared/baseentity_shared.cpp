//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"

#include "decals.h"
#include "effect_dispatch_data.h"
#include "model_types.h"
#include "gamestringpool.h"
#include "ammodef.h"
#include "takedamageinfo.h"
#include "shot_manipulator.h"
#include "ai_debug_shared.h"
#include "mapentities_shared.h"
#include "debugoverlay_shared.h"
#include "coordsize.h"
#include "vphysics/performance.h"
#include "GameBase_Shared.h"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
    #include "c_hl2mp_player.h"
#else
	#include "te_effect_dispatch.h"
	#include "soundent.h"
	#include "player_pickup.h"
	#include "waterbullet.h"
	#include "func_break.h"
    #include "hl2mp_player.h"
    #include "te_bullet_shot.h"
	#include "ilagcompensationmanager.h"
#endif

#include "rumble_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL
	ConVar ent_debugkeys( "ent_debugkeys", "" );
	extern bool ParseKeyvalue( void *pObject, typedescription_t *pFields, int iNumFields, const char *szKeyName, const char *szValue );
	extern bool ExtractKeyvalue( void *pObject, typedescription_t *pFields, int iNumFields, const char *szKeyName, char *szValue, int iMaxLen );
#endif

bool CBaseEntity::m_bAllowPrecache = false;

bool CBaseEntity::m_bDontSuppressEvents = false;
bool ShouldIgnorePredictionSuppression() { return CBaseEntity::m_bDontSuppressEvents; }

// Set default max values for entities based on the existing constants from elsewhere
float k_flMaxEntityPosCoord = MAX_COORD_FLOAT;
float k_flMaxEntityEulerAngle = 360.0 * 1000.0f; // really should be restricted to +/-180, but some code doesn't adhere to this.  let's just trap NANs, etc
// Sometimes the resulting computed speeds are legitimately above the original
// constants; use bumped up versions for the downstream validation logic to
// account for this.
float k_flMaxEntitySpeed = k_flMaxVelocity * 2.0f;
float k_flMaxEntitySpinRate = k_flMaxAngularVelocity * 10.0f;

ConVar	ai_shot_bias_min( "ai_shot_bias_min", "-1.0", FCVAR_REPLICATED );
ConVar	ai_shot_bias_max( "ai_shot_bias_max", "1.0", FCVAR_REPLICATED );
ConVar	ai_debug_shoot_positions( "ai_debug_shoot_positions", "0", FCVAR_REPLICATED | FCVAR_CHEAT );

// Utility func to throttle rate at which the "reasonable position" spew goes out
static double s_LastEntityReasonableEmitTime;
bool CheckEmitReasonablePhysicsSpew()
{

	// Reported recently?
	double now = Plat_FloatTime();
	if ( now >= s_LastEntityReasonableEmitTime && now < s_LastEntityReasonableEmitTime + 5.0 )
	{
		// Already reported recently
		return false;
	}

	// Not reported recently.  Report it now
	s_LastEntityReasonableEmitTime = now;
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: Spawn some blood particles
//-----------------------------------------------------------------------------
void SpawnBlood(Vector vecSpot, const Vector &vecDir, int bloodColor, float flDamage, int hitbox)
{
	UTIL_BloodDrips( vecSpot, vecDir, bloodColor, (int)flDamage, hitbox );
}

#if !defined( NO_ENTITY_PREDICTION )
//-----------------------------------------------------------------------------
// The player drives simulation of this entity
//-----------------------------------------------------------------------------
void CBaseEntity::SetPlayerSimulated( CBasePlayer *pOwner )
{
	m_bIsPlayerSimulated = true;
	pOwner->AddToPlayerSimulationList( this );
	m_hPlayerSimulationOwner = pOwner;
}

void CBaseEntity::UnsetPlayerSimulated( void )
{
	if ( m_hPlayerSimulationOwner != NULL )
	{
		m_hPlayerSimulationOwner->RemoveFromPlayerSimulationList( this );
	}
	m_hPlayerSimulationOwner = NULL;
	m_bIsPlayerSimulated = false;
}
#endif

// position of eyes
Vector CBaseEntity::EyePosition( void )
{ 
	return GetAbsOrigin() + GetViewOffset(); 
}

const QAngle &CBaseEntity::EyeAngles( void )
{
	return GetAbsAngles();
}

const QAngle &CBaseEntity::LocalEyeAngles( void )
{
	return GetLocalAngles();
}

// position of ears
Vector CBaseEntity::EarPosition( void )
{ 
	return EyePosition(); 
}

void CBaseEntity::SetViewOffset( const Vector& v ) 
{ 
	m_vecViewOffset = v; 
}

const Vector& CBaseEntity::GetViewOffset() const 
{ 
	return m_vecViewOffset; 
}


//-----------------------------------------------------------------------------
// center point of entity
//-----------------------------------------------------------------------------
const Vector &CBaseEntity::WorldSpaceCenter( ) const 
{
	return CollisionProp()->WorldSpaceCenter();
}

#if !defined( CLIENT_DLL )
#define CHANGE_FLAGS(flags,newFlags) { unsigned int old = flags; flags = (newFlags); gEntList.ReportEntityFlagsChanged( this, old, flags ); }
#else
#define CHANGE_FLAGS(flags,newFlags) (flags = (newFlags))
#endif

void CBaseEntity::AddFlag( int flags )
{
	CHANGE_FLAGS( m_fFlags, m_fFlags | flags );
}

void CBaseEntity::RemoveFlag( int flagsToRemove )
{
	CHANGE_FLAGS( m_fFlags, m_fFlags & ~flagsToRemove );
}

void CBaseEntity::ClearFlags( void )
{
	CHANGE_FLAGS( m_fFlags, 0 );
}

void CBaseEntity::ToggleFlag( int flagToToggle )
{
	CHANGE_FLAGS( m_fFlags, m_fFlags ^ flagToToggle );
}

void CBaseEntity::SetEffects( int nEffects )
{
	if ( nEffects != m_fEffects )
	{
		m_fEffects = nEffects;

#ifndef CLIENT_DLL
		DispatchUpdateTransmitState();
#else
		UpdateVisibility();
#endif
	}
}

void CBaseEntity::AddEffects( int nEffects ) 
{ 
	m_fEffects |= nEffects; 

	if ( nEffects & EF_NODRAW)
	{
#ifndef CLIENT_DLL
		DispatchUpdateTransmitState();
#else
		UpdateVisibility();
#endif
	}
}

void CBaseEntity::SetBlocksLOS( bool bBlocksLOS )
{
	if ( bBlocksLOS )
	{
		RemoveEFlags( EFL_DONTBLOCKLOS );
	}
	else
	{
		AddEFlags( EFL_DONTBLOCKLOS );
	}
}

bool CBaseEntity::BlocksLOS( void ) 
{ 
	return !IsEFlagSet(EFL_DONTBLOCKLOS); 
}

void CBaseEntity::SetAIWalkable( bool bBlocksLOS )
{
	if ( bBlocksLOS )
	{
		RemoveEFlags( EFL_DONTWALKON );
	}
	else
	{
		AddEFlags( EFL_DONTWALKON );
	}
}

bool CBaseEntity::IsAIWalkable( void ) 
{ 
	return !IsEFlagSet(EFL_DONTWALKON);
}


//-----------------------------------------------------------------------------
// Purpose: Handles keys and outputs from the BSP.
// Input  : mapData - Text block of keys and values from the BSP.
//-----------------------------------------------------------------------------
void CBaseEntity::ParseMapData( CEntityMapData *mapData )
{
	char keyName[MAPKEY_MAXLENGTH];
	char value[MAPKEY_MAXLENGTH];

	#ifdef _DEBUG
	#ifdef GAME_DLL
	ValidateDataDescription();
	#endif // GAME_DLL
	#endif // _DEBUG

	// loop through all keys in the data block and pass the info back into the object
	if ( mapData->GetFirstKey(keyName, value) )
	{
		do 
		{
			KeyValue( keyName, value );
		} 
		while ( mapData->GetNextKey(keyName, value) );
	}
}

//-----------------------------------------------------------------------------
// Parse data from a map file
//-----------------------------------------------------------------------------
bool CBaseEntity::KeyValue( const char *szKeyName, const char *szValue ) 
{
	//!! temp hack, until worldcraft is fixed
	// strip the # tokens from (duplicate) key names
	char *s = (char *)strchr( szKeyName, '#' );
	if ( s )
	{
		*s = '\0';
	}

	if ( FStrEq( szKeyName, "rendercolor" ) || FStrEq( szKeyName, "rendercolor32" ))
	{
		color32 tmp;
		UTIL_StringToColor32( &tmp, szValue );
		SetRenderColor( tmp.r, tmp.g, tmp.b );
		// don't copy alpha, legacy support uses renderamt
		return true;
	}
	
	if ( FStrEq( szKeyName, "renderamt" ) )
	{
		SetRenderColorA( atoi( szValue ) );
		return true;
	}

	if ( FStrEq( szKeyName, "disableshadows" ))
	{
		int val = atoi( szValue );
		if (val)
		{
			AddEffects( EF_NOSHADOW );
		}
		return true;
	}

	if ( FStrEq( szKeyName, "mins" ))
	{
		Vector mins;
		UTIL_StringToVector( mins.Base(), szValue );
		CollisionProp()->SetCollisionBounds( mins, CollisionProp()->OBBMaxs() );
		return true;
	}

	if ( FStrEq( szKeyName, "maxs" ))
	{
		Vector maxs;
		UTIL_StringToVector( maxs.Base(), szValue );
		CollisionProp()->SetCollisionBounds( CollisionProp()->OBBMins(), maxs );
		return true;
	}

	if ( FStrEq( szKeyName, "disablereceiveshadows" ))
	{
		int val = atoi( szValue );
		if (val)
		{
			AddEffects( EF_NORECEIVESHADOW );
		}
		return true;
	}

	if ( FStrEq( szKeyName, "nodamageforces" ))
	{
		int val = atoi( szValue );
		if (val)
		{
			AddEFlags( EFL_NO_DAMAGE_FORCES );
		}
		return true;
	}

	// Fix up single angles
	if( FStrEq( szKeyName, "angle" ) )
	{
		static char szBuf[64];

		float y = atof( szValue );
		if (y >= 0)
		{
			Q_snprintf( szBuf,sizeof(szBuf), "%f %f %f", GetLocalAngles()[0], y, GetLocalAngles()[2] );
		}
		else if ((int)y == -1)
		{
			Q_strncpy( szBuf, "-90 0 0", sizeof(szBuf) );
		}
		else
		{
			Q_strncpy( szBuf, "90 0 0", sizeof(szBuf) );
		}

		// Do this so inherited classes looking for 'angles' don't have to bother with 'angle'
		return KeyValue( szKeyName, szBuf );
	}

	// NOTE: Have to do these separate because they set two values instead of one
	if( FStrEq( szKeyName, "angles" ) )
	{
		QAngle angles;
		UTIL_StringToVector( angles.Base(), szValue );

		// If you're hitting this assert, it's probably because you're
		// calling SetLocalAngles from within a KeyValues method.. use SetAbsAngles instead!
		Assert( (GetMoveParent() == NULL) && !IsEFlagSet( EFL_DIRTY_ABSTRANSFORM ) );
		SetAbsAngles( angles );
		return true;
	}

	if( FStrEq( szKeyName, "origin" ) )
	{
		Vector vecOrigin;
		UTIL_StringToVector( vecOrigin.Base(), szValue );

		// If you're hitting this assert, it's probably because you're
		// calling SetLocalOrigin from within a KeyValues method.. use SetAbsOrigin instead!
		Assert( (GetMoveParent() == NULL) && !IsEFlagSet( EFL_DIRTY_ABSTRANSFORM ) );
		SetAbsOrigin( vecOrigin );
		return true;
	}

#ifdef GAME_DLL	
	
	if ( FStrEq( szKeyName, "targetname" ) )
	{
		m_iName = AllocPooledString( szValue );
		return true;
	}

	// loop through the data description, and try and place the keys in
	if ( !*ent_debugkeys.GetString() )
	{
		for ( datamap_t *dmap = GetDataDescMap(); dmap != NULL; dmap = dmap->baseMap )
		{
			if ( ::ParseKeyvalue(this, dmap->dataDesc, dmap->dataNumFields, szKeyName, szValue) )
				return true;
		}
	}
	else
	{
		// debug version - can be used to see what keys have been parsed in
		bool printKeyHits = false;
		const char *debugName = "";

		if ( *ent_debugkeys.GetString() && !Q_stricmp(ent_debugkeys.GetString(), STRING(m_iClassname)) )
		{
			// Msg( "-- found entity of type %s\n", STRING(m_iClassname) );
			printKeyHits = true;
			debugName = STRING(m_iClassname);
		}

		// loop through the data description, and try and place the keys in
		for ( datamap_t *dmap = GetDataDescMap(); dmap != NULL; dmap = dmap->baseMap )
		{
			if ( !printKeyHits && *ent_debugkeys.GetString() && !Q_stricmp(dmap->dataClassName, ent_debugkeys.GetString()) )
			{
				// Msg( "-- found class of type %s\n", dmap->dataClassName );
				printKeyHits = true;
				debugName = dmap->dataClassName;
			}

			if ( ::ParseKeyvalue(this, dmap->dataDesc, dmap->dataNumFields, szKeyName, szValue) )
			{
				if ( printKeyHits )
					Msg( "(%s) key: %-16s value: %s\n", debugName, szKeyName, szValue );
				
				return true;
			}
		}

		if ( printKeyHits )
			Msg( "!! (%s) key not handled: \"%s\" \"%s\"\n", STRING(m_iClassname), szKeyName, szValue );
	}

#endif

	// key hasn't been handled
	return false;
}

bool CBaseEntity::KeyValue( const char *szKeyName, float flValue ) 
{
	char	string[256];

	Q_snprintf(string,sizeof(string), "%f", flValue );

	return KeyValue( szKeyName, string );
}

bool CBaseEntity::KeyValue( const char *szKeyName, const Vector &vecValue ) 
{
	char	string[256];

	Q_snprintf(string,sizeof(string), "%f %f %f", vecValue.x, vecValue.y, vecValue.z );

	return KeyValue( szKeyName, string );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :
// Output :
//-----------------------------------------------------------------------------

bool CBaseEntity::GetKeyValue( const char *szKeyName, char *szValue, int iMaxLen )
{
	if ( FStrEq( szKeyName, "rendercolor" ) || FStrEq( szKeyName, "rendercolor32" ))
	{
		color32 tmp = GetRenderColor();
		Q_snprintf( szValue, iMaxLen, "%d %d %d %d", tmp.r, tmp.g, tmp.b, tmp.a );
		return true;
	}
	
	if ( FStrEq( szKeyName, "renderamt" ) )
	{
		color32 tmp = GetRenderColor();
		Q_snprintf( szValue, iMaxLen, "%d", tmp.a );
		return true;
	}

	if ( FStrEq( szKeyName, "disableshadows" ))
	{
		Q_snprintf( szValue, iMaxLen, "%d", IsEffectActive( EF_NOSHADOW ) );
		return true;
	}

	if ( FStrEq( szKeyName, "mins" ))
	{
		Assert( 0 );
		return false;
	}

	if ( FStrEq( szKeyName, "maxs" ))
	{
		Assert( 0 );
		return false;
	}

	if ( FStrEq( szKeyName, "disablereceiveshadows" ))
	{
		Q_snprintf( szValue, iMaxLen, "%d", IsEffectActive( EF_NORECEIVESHADOW ) );
		return true;
	}

	if ( FStrEq( szKeyName, "nodamageforces" ))
	{
		Q_snprintf( szValue, iMaxLen, "%d", IsEffectActive( EFL_NO_DAMAGE_FORCES ) );
		return true;
	}

	// Fix up single angles
	if( FStrEq( szKeyName, "angle" ) )
	{
		return false;
	}

	// NOTE: Have to do these separate because they set two values instead of one
	if( FStrEq( szKeyName, "angles" ) )
	{
		QAngle angles = GetAbsAngles();

		Q_snprintf( szValue, iMaxLen, "%f %f %f", angles.x, angles.y, angles.z );
		return true;
	}

	if( FStrEq( szKeyName, "origin" ) )
	{
		Vector vecOrigin = GetAbsOrigin();
		Q_snprintf( szValue, iMaxLen, "%f %f %f", vecOrigin.x, vecOrigin.y, vecOrigin.z );
		return true;
	}

#ifdef GAME_DLL	
	
	if ( FStrEq( szKeyName, "targetname" ) )
	{
		Q_snprintf( szValue, iMaxLen, "%s", STRING( GetEntityName() ) );
		return true;
	}

	if ( FStrEq( szKeyName, "classname" ) )
	{
		Q_snprintf( szValue, iMaxLen, "%s", GetClassname() );
		return true;
	}

	for ( datamap_t *dmap = GetDataDescMap(); dmap != NULL; dmap = dmap->baseMap )
	{
		if ( ::ExtractKeyvalue( this, dmap->dataDesc, dmap->dataNumFields, szKeyName, szValue, iMaxLen ) )
			return true;
	}
#endif

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : collisionGroup - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseEntity::ShouldCollide( int collisionGroup, int contentsMask ) const
{
	if ( m_CollisionGroup == COLLISION_GROUP_DEBRIS )
	{
		if ( ! (contentsMask & CONTENTS_DEBRIS) )
			return false;
	}
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : seed - 
//-----------------------------------------------------------------------------
void CBaseEntity::SetPredictionRandomSeed( const CUserCmd *cmd )
{
	if ( !cmd )
	{
		m_nPredictionRandomSeed = -1;
#ifdef GAME_DLL
		m_nPredictionRandomSeedServer = -1;
#endif		
		return;
	}

	m_nPredictionRandomSeed = ( cmd->random_seed );
	
#ifdef GAME_DLL
	m_nPredictionRandomSeedServer = ( cmd->server_random_seed );
#endif	
}


//------------------------------------------------------------------------------
// Purpose : Base implimentation for entity handling decals
//------------------------------------------------------------------------------
void CBaseEntity::DecalTrace( trace_t *pTrace, char const *decalName )
{
	int index = decalsystem->GetDecalIndexForName( decalName );
	if ( index < 0 )
		return;

	Assert( pTrace->m_pEnt );

	CBroadcastRecipientFilter filter;
	te->Decal( filter, 0.0, &pTrace->endpos, &pTrace->startpos,
		pTrace->GetEntityIndex(), pTrace->hitbox, index );
}

//-----------------------------------------------------------------------------
// Purpose: Base handling for impacts against entities
//-----------------------------------------------------------------------------
void CBaseEntity::ImpactTrace( trace_t *pTrace, int iDamageType, const char *pCustomImpactName )
{
	VPROF( "CBaseEntity::ImpactTrace" );
	Assert( pTrace->m_pEnt );

	CBaseEntity *pEntity = pTrace->m_pEnt;
 
	// Build the impact data
	CEffectData data;
	data.m_vOrigin = pTrace->endpos;
	data.m_vStart = pTrace->startpos;
	data.m_nSurfaceProp = pTrace->surface.surfaceProps;
	data.m_nDamageType = iDamageType;
	data.m_nHitBox = pTrace->hitbox;
#ifdef CLIENT_DLL
	data.m_hEntity = ClientEntityList().EntIndexToHandle( pEntity->entindex() );
#else
	data.m_nEntIndex = pEntity->entindex();
#endif

	// Send it on its way
	if ( !pCustomImpactName )
	{
		DispatchEffect( "Impact", data );
	}
	else
	{
		DispatchEffect( pCustomImpactName, data );
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the damage decal to use, given a damage type
// Input  : bitsDamageType - the damage type
// Output : the index of the damage decal to use
//-----------------------------------------------------------------------------
char const *CBaseEntity::DamageDecal( int bitsDamageType, int gameMaterial ) // BB2_Warn - Remember!
{
	if ( m_nRenderMode == kRenderTransAlpha )
		return "";

	if ( m_nRenderMode != kRenderNormal && gameMaterial == 'G' )
		return "BulletProof";

	if (bitsDamageType & DMG_RICOCHET)
		return "Ricochet";

	// This will get translated at a lower layer based on game material
	return "Impact.Concrete";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int	CBaseEntity::GetIndexForThinkContext( const char *pszContext )
{
	for ( int i = 0; i < m_aThinkFunctions.Size(); i++ )
	{
		if ( !Q_strncmp( STRING( m_aThinkFunctions[i].m_iszContext ), pszContext, MAX_CONTEXT_LENGTH ) )
			return i;
	}

	return NO_THINK_CONTEXT;
}

//-----------------------------------------------------------------------------
// Purpose: Get a fresh think context for this entity
//-----------------------------------------------------------------------------
int CBaseEntity::RegisterThinkContext( const char *szContext )
{
	int iIndex = GetIndexForThinkContext( szContext );
	if ( iIndex != NO_THINK_CONTEXT )
		return iIndex;

	// Make a new think func
	thinkfunc_t sNewFunc;
	Q_memset( &sNewFunc, 0, sizeof( sNewFunc ) );
	sNewFunc.m_pfnThink = NULL;
	sNewFunc.m_nNextThinkTick = 0;
	sNewFunc.m_iszContext = AllocPooledString(szContext);

	// Insert it into our list
	return m_aThinkFunctions.AddToTail( sNewFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
BASEPTR	CBaseEntity::ThinkSet( BASEPTR func, float thinkTime, const char *szContext )
{
#if !defined( CLIENT_DLL )
#ifdef _DEBUG
#ifdef GNUC
	COMPILE_TIME_ASSERT( sizeof(func) == 8 );
#else
	COMPILE_TIME_ASSERT( sizeof(func) == 4 );
#endif
#endif
#endif

	// Old system?
	if ( !szContext )
	{
		m_pfnThink = func;
#if !defined( CLIENT_DLL )
#ifdef _DEBUG
		FunctionCheck( *(reinterpret_cast<void **>(&m_pfnThink)), "BaseThinkFunc" ); 
#endif
#endif
		return m_pfnThink;
	}

	// Find the think function in our list, and if we couldn't find it, register it
	int iIndex = GetIndexForThinkContext( szContext );
	if ( iIndex == NO_THINK_CONTEXT )
	{
		iIndex = RegisterThinkContext( szContext );
	}

	m_aThinkFunctions[ iIndex ].m_pfnThink = func;
#if !defined( CLIENT_DLL )
#ifdef _DEBUG
	FunctionCheck( *(reinterpret_cast<void **>(&m_aThinkFunctions[ iIndex ].m_pfnThink)), szContext ); 
#endif
#endif

	if ( thinkTime != 0 )
	{
		int thinkTick = ( thinkTime == TICK_NEVER_THINK ) ? TICK_NEVER_THINK : TIME_TO_TICKS( thinkTime );
		m_aThinkFunctions[ iIndex ].m_nNextThinkTick = thinkTick;
		CheckHasThinkFunction( thinkTick == TICK_NEVER_THINK ? false : true );
	}
	return func;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseEntity::SetNextThink( float thinkTime, const char *szContext )
{
	int thinkTick = ( thinkTime == TICK_NEVER_THINK ) ? TICK_NEVER_THINK : TIME_TO_TICKS( thinkTime );

	// Are we currently in a think function with a context?
	int iIndex = 0;
	if ( !szContext )
	{
#ifdef _DEBUG
		if ( m_iCurrentThinkContext != NO_THINK_CONTEXT )
		{
			Msg( "Warning: Setting base think function within think context %s\n", STRING(m_aThinkFunctions[m_iCurrentThinkContext].m_iszContext) );
		}
#endif

		// Old system
		m_nNextThinkTick = thinkTick;
		CheckHasThinkFunction( thinkTick == TICK_NEVER_THINK ? false : true );
		return;
	}
	else
	{
		// Find the think function in our list, and if we couldn't find it, register it
		iIndex = GetIndexForThinkContext( szContext );
		if ( iIndex == NO_THINK_CONTEXT )
		{
			iIndex = RegisterThinkContext( szContext );
		}
	}

	// Old system
	m_aThinkFunctions[ iIndex ].m_nNextThinkTick = thinkTick;
	CheckHasThinkFunction( thinkTick == TICK_NEVER_THINK ? false : true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CBaseEntity::GetNextThink( const char *szContext )
{
	// Are we currently in a think function with a context?
	int iIndex = 0;
	if ( !szContext )
	{
#ifdef _DEBUG
		if ( m_iCurrentThinkContext != NO_THINK_CONTEXT )
		{
			Msg( "Warning: Getting base nextthink time within think context %s\n", STRING(m_aThinkFunctions[m_iCurrentThinkContext].m_iszContext) );
		}
#endif

		if ( m_nNextThinkTick == TICK_NEVER_THINK )
			return TICK_NEVER_THINK;

		// Old system
		return TICK_INTERVAL * (m_nNextThinkTick );
	}
	else
	{
		// Find the think function in our list
		iIndex = GetIndexForThinkContext( szContext );
	}

	if ( iIndex == m_aThinkFunctions.InvalidIndex() )
		return TICK_NEVER_THINK;

	if ( m_aThinkFunctions[ iIndex ].m_nNextThinkTick == TICK_NEVER_THINK )
	{
		return TICK_NEVER_THINK;
	}
	return TICK_INTERVAL * (m_aThinkFunctions[ iIndex ].m_nNextThinkTick );
}

int	CBaseEntity::GetNextThinkTick( const char *szContext /*= NULL*/ )
{
	// Are we currently in a think function with a context?
	int iIndex = 0;
	if ( !szContext )
	{
#ifdef _DEBUG
		if ( m_iCurrentThinkContext != NO_THINK_CONTEXT )
		{
			Msg( "Warning: Getting base nextthink time within think context %s\n", STRING(m_aThinkFunctions[m_iCurrentThinkContext].m_iszContext) );
		}
#endif

		if ( m_nNextThinkTick == TICK_NEVER_THINK )
			return TICK_NEVER_THINK;

		// Old system
		return m_nNextThinkTick;
	}
	else
	{
		// Find the think function in our list
		iIndex = GetIndexForThinkContext( szContext );

		// Looking up an invalid think context!
		Assert( iIndex != -1 );
	}

	if ( ( iIndex == -1 ) || ( m_aThinkFunctions[ iIndex ].m_nNextThinkTick == TICK_NEVER_THINK ) )
	{
		return TICK_NEVER_THINK;
	}

	return m_aThinkFunctions[ iIndex ].m_nNextThinkTick;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CBaseEntity::GetLastThink( const char *szContext )
{
	// Are we currently in a think function with a context?
	int iIndex = 0;
	if ( !szContext )
	{
#ifdef _DEBUG
		if ( m_iCurrentThinkContext != NO_THINK_CONTEXT )
		{
			Msg( "Warning: Getting base lastthink time within think context %s\n", STRING(m_aThinkFunctions[m_iCurrentThinkContext].m_iszContext) );
		}
#endif
		// Old system
		return m_nLastThinkTick * TICK_INTERVAL;
	}
	else
	{
		// Find the think function in our list
		iIndex = GetIndexForThinkContext( szContext );
	}

	return m_aThinkFunctions[ iIndex ].m_nLastThinkTick * TICK_INTERVAL;
}
	
int CBaseEntity::GetLastThinkTick( const char *szContext /*= NULL*/ )
{
	// Are we currently in a think function with a context?
	int iIndex = 0;
	if ( !szContext )
	{
#ifdef _DEBUG
		if ( m_iCurrentThinkContext != NO_THINK_CONTEXT )
		{
			Msg( "Warning: Getting base lastthink time within think context %s\n", STRING(m_aThinkFunctions[m_iCurrentThinkContext].m_iszContext) );
		}
#endif
		// Old system
		return m_nLastThinkTick;
	}
	else
	{
		// Find the think function in our list
		iIndex = GetIndexForThinkContext( szContext );
	}

	return m_aThinkFunctions[ iIndex ].m_nLastThinkTick;
}

bool CBaseEntity::WillThink()
{
	if ( m_nNextThinkTick > 0 )
		return true;

	for ( int i = 0; i < m_aThinkFunctions.Count(); i++ )
	{
		if ( m_aThinkFunctions[i].m_nNextThinkTick > 0 )
			return true;
	}

	return false;
}

// returns the first tick the entity will run any think function
// returns TICK_NEVER_THINK if no think functions are scheduled
int CBaseEntity::GetFirstThinkTick()
{
	int minTick = TICK_NEVER_THINK;
	if ( m_nNextThinkTick > 0 )
	{
		minTick = m_nNextThinkTick;
	}

	for ( int i = 0; i < m_aThinkFunctions.Count(); i++ )
	{
		int next = m_aThinkFunctions[i].m_nNextThinkTick;
		if ( next > 0 )
		{
			if ( next < minTick || minTick == TICK_NEVER_THINK )
			{
				minTick = next;
			}
		}
	}
	return minTick;
}

// NOTE: pass in the isThinking hint so we have to search the think functions less
void CBaseEntity::CheckHasThinkFunction( bool isThinking )
{
	if ( IsEFlagSet( EFL_NO_THINK_FUNCTION ) && isThinking )
	{
		RemoveEFlags( EFL_NO_THINK_FUNCTION );
	}
	else if ( !isThinking && !IsEFlagSet( EFL_NO_THINK_FUNCTION ) && !WillThink() )
	{
		AddEFlags( EFL_NO_THINK_FUNCTION );
	}
#if !defined( CLIENT_DLL )
	SimThink_EntityChanged( this );
#endif
}

bool CBaseEntity::WillSimulateGamePhysics()
{
	// players always simulate game physics
	if ( !IsPlayer() )
	{
		MoveType_t movetype = GetMoveType();
		
		if ( movetype == MOVETYPE_NONE || movetype == MOVETYPE_VPHYSICS )
			return false;

#if !defined( CLIENT_DLL )
		// MOVETYPE_PUSH not supported on the client
		if ( movetype == MOVETYPE_PUSH && GetMoveDoneTime() <= 0 )
			return false;
#endif
	}

	return true;
}

void CBaseEntity::CheckHasGamePhysicsSimulation()
{
	bool isSimulating = WillSimulateGamePhysics();
	if ( isSimulating != IsEFlagSet(EFL_NO_GAME_PHYSICS_SIMULATION) )
		return;
	if ( isSimulating )
	{
		RemoveEFlags( EFL_NO_GAME_PHYSICS_SIMULATION );
	}
	else
	{
		AddEFlags( EFL_NO_GAME_PHYSICS_SIMULATION );
	}
#if !defined( CLIENT_DLL )
	SimThink_EntityChanged( this );
#endif
}

//-----------------------------------------------------------------------------
// Sets/Gets the next think based on context index
//-----------------------------------------------------------------------------
void CBaseEntity::SetNextThink( int nContextIndex, float thinkTime )
{
	int thinkTick = ( thinkTime == TICK_NEVER_THINK ) ? TICK_NEVER_THINK : TIME_TO_TICKS( thinkTime );

	if (nContextIndex < 0)
	{
		SetNextThink( thinkTime );
	}
	else
	{
		m_aThinkFunctions[nContextIndex].m_nNextThinkTick = thinkTick;
	}
	CheckHasThinkFunction( thinkTick == TICK_NEVER_THINK ? false : true );
}

void CBaseEntity::SetLastThink( int nContextIndex, float thinkTime )
{
	int thinkTick = ( thinkTime == TICK_NEVER_THINK ) ? TICK_NEVER_THINK : TIME_TO_TICKS( thinkTime );

	if (nContextIndex < 0)
	{
		m_nLastThinkTick = thinkTick;
	}
	else
	{
		m_aThinkFunctions[nContextIndex].m_nLastThinkTick = thinkTick;
	}
}

float CBaseEntity::GetNextThink( int nContextIndex ) const
{
	if (nContextIndex < 0)
		return m_nNextThinkTick * TICK_INTERVAL;

	return m_aThinkFunctions[nContextIndex].m_nNextThinkTick * TICK_INTERVAL; 
}

int	CBaseEntity::GetNextThinkTick( int nContextIndex ) const
{
	if (nContextIndex < 0)
		return m_nNextThinkTick;

	return m_aThinkFunctions[nContextIndex].m_nNextThinkTick; 
}


int CheckEntityVelocity( Vector &v )
{
	float r = k_flMaxEntitySpeed;
	if (
		v.x > -r && v.x < r &&
		v.y > -r && v.y < r &&
		v.z > -r && v.z < r)
	{
		// The usual case.  It's totally reasonable
		return 1;
	}
	float speed = v.Length();
	if ( speed < k_flMaxEntitySpeed * 100.0f )
	{
		// Sort of suspicious.  Clamp it
		v *= k_flMaxEntitySpeed / speed;
		return 0;
	}

	// A terrible, horrible, no good, very bad velocity.
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: My physics object has been updated, react or extract data
//-----------------------------------------------------------------------------
void CBaseEntity::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	switch( GetMoveType() )
	{
	case MOVETYPE_VPHYSICS:
		{
			if ( GetMoveParent() )
			{
				DevWarning("Updating physics on object in hierarchy %s!\n", GetClassname());
				return;
			}
			Vector origin;
			QAngle angles;

			pPhysics->GetPosition( &origin, &angles );

			if ( !IsEntityQAngleReasonable( angles ) )
			{
				if ( CheckEmitReasonablePhysicsSpew() )
				{
					Warning( "Ignoring bogus angles (%f,%f,%f) from vphysics! (entity %s)\n", angles.x, angles.y, angles.z, GetDebugName() );
				}
				angles = vec3_angle;
			}
#ifndef CLIENT_DLL 
			Vector prevOrigin = GetAbsOrigin();
#endif

			if ( IsEntityPositionReasonable( origin ) )
			{
				SetAbsOrigin( origin );
			}
			else
			{
				if ( CheckEmitReasonablePhysicsSpew() )
				{
					Warning( "Ignoring unreasonable position (%f,%f,%f) from vphysics! (entity %s)\n", origin.x, origin.y, origin.z, GetDebugName() );
				}
			}

			for ( int i = 0; i < 3; ++i )
			{
				angles[ i ] = AngleNormalize( angles[ i ] );
			}
			SetAbsAngles( angles );

			// Interactive debris converts back to debris when it comes to rest
			if ( pPhysics->IsAsleep() && GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS )
			{
				SetCollisionGroup( COLLISION_GROUP_DEBRIS );
			}

#ifndef CLIENT_DLL 
			PhysicsTouchTriggers( &prevOrigin );
			PhysicsRelinkChildren(gpGlobals->frametime);
#endif
		}
	break;

	case MOVETYPE_STEP:
		break;

	case MOVETYPE_PUSH:
#ifndef CLIENT_DLL
		VPhysicsUpdatePusher( pPhysics );
#endif
	break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Init this object's physics as a static
//-----------------------------------------------------------------------------
IPhysicsObject *CBaseEntity::VPhysicsInitStatic( void )
{
	if ( !VPhysicsInitSetup() )
		return NULL;

#ifndef CLIENT_DLL
	// If this entity has a move parent, it needs to be shadow, not static
	if ( GetMoveParent() )
	{
		// must be SOLID_VPHYSICS if in hierarchy to solve collisions correctly
		if ( GetSolid() == SOLID_BSP && GetRootMoveParent()->GetSolid() != SOLID_BSP )
		{
			SetSolid( SOLID_VPHYSICS );
		}

		return VPhysicsInitShadow( false, false );
	}
#endif

	// No physics
	if ( GetSolid() == SOLID_NONE )
		return NULL;

	// create a static physics objct
	IPhysicsObject *pPhysicsObject = NULL;
	if ( GetSolid() == SOLID_BBOX )
	{
		pPhysicsObject = PhysModelCreateBox( this, WorldAlignMins(), WorldAlignMaxs(), GetAbsOrigin(), true );
	}
	else
	{
		pPhysicsObject = PhysModelCreateUnmoveable( this, GetModelIndex(), GetAbsOrigin(), GetAbsAngles() );
	}
	VPhysicsSetObject( pPhysicsObject );
	return pPhysicsObject;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPhysics - 
//-----------------------------------------------------------------------------
void CBaseEntity::VPhysicsSetObject( IPhysicsObject *pPhysics )
{
	if ( m_pPhysicsObject && pPhysics )
	{
		Warning( "Overwriting physics object for %s\n", GetClassname() );
	}
	m_pPhysicsObject = pPhysics;
	if ( pPhysics && !m_pPhysicsObject )
	{
		CollisionRulesChanged();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseEntity::VPhysicsDestroyObject( void )
{
	if ( m_pPhysicsObject )
	{
#ifndef CLIENT_DLL
		PhysRemoveShadow( this );
#endif
		PhysDestroyObject( m_pPhysicsObject, this );
		m_pPhysicsObject = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseEntity::VPhysicsInitSetup()
{
#ifndef CLIENT_DLL
	// don't support logical ents
	if ( !edict() || IsMarkedForDeletion() )
		return false;
#endif

	// If this entity already has a physics object, then it should have been deleted prior to making this call.
	Assert(!m_pPhysicsObject);
	VPhysicsDestroyObject();

	// make sure absorigin / absangles are correct
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: This creates a normal vphysics simulated object
//			physics alone determines where it goes (gravity, friction, etc)
//			and the entity receives updates from vphysics.  SetAbsOrigin(), etc do not affect the object!
//-----------------------------------------------------------------------------
IPhysicsObject *CBaseEntity::VPhysicsInitNormal( SolidType_t solidType, int nSolidFlags, bool createAsleep, solid_t *pSolid )
{
	if ( !VPhysicsInitSetup() )
		return NULL;

	// NOTE: This has to occur before PhysModelCreate because that call will
	// call back into ShouldCollide(), which uses solidtype for rules.
	SetSolid( solidType );
	SetSolidFlags( nSolidFlags );

	// No physics
	if ( solidType == SOLID_NONE )
	{
		return NULL;
	}

	// create a normal physics object
	IPhysicsObject *pPhysicsObject = PhysModelCreate( this, GetModelIndex(), GetAbsOrigin(), GetAbsAngles(), pSolid );
	if ( pPhysicsObject )
	{
		VPhysicsSetObject( pPhysicsObject );
		SetMoveType( MOVETYPE_VPHYSICS );

		if ( !createAsleep )
		{
			pPhysicsObject->Wake();
		}
	}

	return pPhysicsObject;
}

// This creates a vphysics object with a shadow controller that follows the AI
IPhysicsObject *CBaseEntity::VPhysicsInitShadow( bool allowPhysicsMovement, bool allowPhysicsRotation, solid_t *pSolid )
{
	if ( !VPhysicsInitSetup() )
		return NULL;

	// No physics
	if ( GetSolid() == SOLID_NONE )
		return NULL;

	const Vector &origin = GetAbsOrigin();
	QAngle angles = GetAbsAngles();
	IPhysicsObject *pPhysicsObject = NULL;

	if ( GetSolid() == SOLID_BBOX )
	{
		// adjust these so the game tracing epsilons match the physics minimum separation distance
		// this will shrink the vphysics version of the model by the difference in epsilons
		float radius = 0.25f - DIST_EPSILON;
		Vector mins = WorldAlignMins() + Vector(radius, radius, radius);
		Vector maxs = WorldAlignMaxs() - Vector(radius, radius, radius);
		pPhysicsObject = PhysModelCreateBox( this, mins, maxs, origin, false );
		angles = vec3_angle;
	}
	else if ( GetSolid() == SOLID_OBB )
	{
		pPhysicsObject = PhysModelCreateOBB( this, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), origin, angles, false );
	}
	else
	{
		pPhysicsObject = PhysModelCreate( this, GetModelIndex(), origin, angles, pSolid );
	}
	if ( !pPhysicsObject )
		return NULL;

	VPhysicsSetObject( pPhysicsObject );
	// UNDONE: Tune these speeds!!!
	pPhysicsObject->SetShadow( 1e4, 1e4, allowPhysicsMovement, allowPhysicsRotation );
	pPhysicsObject->UpdateShadow( origin, angles, false, 0 );
	return pPhysicsObject;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CBaseEntity::CreateVPhysics()
{
	return false;
}

bool CBaseEntity::IsStandable() const
{
	if (GetSolidFlags() & FSOLID_NOT_STANDABLE) 
		return false;

	if ( GetSolid() == SOLID_BSP || GetSolid() == SOLID_VPHYSICS || GetSolid() == SOLID_BBOX )
		return true;

	return IsBSPModel( ); 
}

bool CBaseEntity::IsBSPModel() const
{
	if ( GetSolid() == SOLID_BSP )
		return true;
	
	const model_t *model = modelinfo->GetModel( GetModelIndex() );

	if ( GetSolid() == SOLID_VPHYSICS && modelinfo->GetModelType( model ) == mod_brush )
		return true;

	return false;
}


//-----------------------------------------------------------------------------
// Invalidates the abs state of all children
//-----------------------------------------------------------------------------
void CBaseEntity::InvalidatePhysicsRecursive( int nChangeFlags )
{
	// Main entry point for dirty flag setting for the 90% case
	// 1) If the origin changes, then we have to update abstransform, Shadow projection, PVS, KD-tree, 
	//    client-leaf system.
	// 2) If the angles change, then we have to update abstransform, Shadow projection,
	//    shadow render-to-texture, client-leaf system, and surrounding bounds. 
	//	  Children have to additionally update absvelocity, KD-tree, and PVS.
	//	  If the surrounding bounds actually update, when we also need to update the KD-tree and the PVS.
	// 3) If it's due to attachment, then all children who are attached to an attachment point
	//    are assumed to have dirty origin + angles.

	// Other stuff:
	// 1) Marking the surrounding bounds dirty will automatically mark KD tree + PVS dirty.
	
	int nDirtyFlags = 0;

	if ( nChangeFlags & VELOCITY_CHANGED )
	{
		nDirtyFlags |= EFL_DIRTY_ABSVELOCITY;
	}

	if ( nChangeFlags & POSITION_CHANGED )
	{
		nDirtyFlags |= EFL_DIRTY_ABSTRANSFORM;

#ifndef CLIENT_DLL
		NetworkProp()->MarkPVSInformationDirty();
#endif

		// NOTE: This will also mark shadow projection + client leaf dirty
		CollisionProp()->MarkPartitionHandleDirty();
	}

	// NOTE: This has to be done after velocity + position are changed
	// because we change the nChangeFlags for the child entities
	if ( nChangeFlags & ANGLES_CHANGED )
	{
		nDirtyFlags |= EFL_DIRTY_ABSTRANSFORM;
		if ( CollisionProp()->DoesRotationInvalidateSurroundingBox() )
		{
			// NOTE: This will handle the KD-tree, surrounding bounds, PVS
			// render-to-texture shadow, shadow projection, and client leaf dirty
			CollisionProp()->MarkSurroundingBoundsDirty();
		}
		else
		{
#ifdef CLIENT_DLL
			MarkRenderHandleDirty();
			g_pClientShadowMgr->AddToDirtyShadowList( this );
			g_pClientShadowMgr->MarkRenderToTextureShadowDirty( GetShadowHandle() );
#endif
		}

		// This is going to be used for all children: children
		// have position + velocity changed
		nChangeFlags |= POSITION_CHANGED | VELOCITY_CHANGED;
	}

	AddEFlags( nDirtyFlags );

	// Set flags for children
	bool bOnlyDueToAttachment = false;
	if ( nChangeFlags & ANIMATION_CHANGED )
	{
#ifdef CLIENT_DLL
		g_pClientShadowMgr->MarkRenderToTextureShadowDirty( GetShadowHandle() );
#endif

		// Only set this flag if the only thing that changed us was the animation.
		// If position or something else changed us, then we must tell all children.
		if ( !( nChangeFlags & (POSITION_CHANGED | VELOCITY_CHANGED | ANGLES_CHANGED) ) )
		{
			bOnlyDueToAttachment = true;
		}

		nChangeFlags = POSITION_CHANGED | ANGLES_CHANGED | VELOCITY_CHANGED;
	}

	for (CBaseEntity *pChild = FirstMoveChild(); pChild; pChild = pChild->NextMovePeer())
	{
		// If this is due to the parent animating, only invalidate children that are parented to an attachment
		// Entities that are following also access attachments points on parents and must be invalidated.
		if ( bOnlyDueToAttachment )
		{
#ifdef CLIENT_DLL
			if ( (pChild->GetParentAttachment() == 0) && !pChild->IsFollowingEntity() )
				continue;
#else
			if ( pChild->GetParentAttachment() == 0 )
				continue;
#endif
		}
		pChild->InvalidatePhysicsRecursive( nChangeFlags );
	}

	//
	// This code should really be in here, or the bone cache should not be in world space.
	// Since the bone transforms are in world space, if we move or rotate the entity, its
	// bones should be marked invalid.
	//
	// As it is, we're near ship, and don't have time to setup a good A/B test of how much
	// overhead this fix would add. We've also only got one known case where the lack of
	// this fix is screwing us, and I just fixed it, so I'm leaving this commented out for now.
	//
	// Hopefully, we'll put the bone cache in entity space and remove the need for this fix.
	//
	//#ifdef CLIENT_DLL
	//	if ( nChangeFlags & (POSITION_CHANGED | ANGLES_CHANGED | ANIMATION_CHANGED) )
	//	{
	//		C_BaseAnimating *pAnim = GetBaseAnimating();
	//		if ( pAnim )
	//			pAnim->InvalidateBoneCache();		
	//	}
	//#endif
}



//-----------------------------------------------------------------------------
// Returns the highest parent of an entity
//-----------------------------------------------------------------------------
CBaseEntity *CBaseEntity::GetRootMoveParent()
{
	CBaseEntity *pEntity = this;
	CBaseEntity *pParent = this->GetMoveParent();
	while ( pParent )
	{
		pEntity = pParent;
		pParent = pEntity->GetMoveParent();
	}

	return pEntity;
}

//-----------------------------------------------------------------------------
// Purpose: static method
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseEntity::IsPrecacheAllowed()
{
	return m_bAllowPrecache;
}

//-----------------------------------------------------------------------------
// Purpose: static method
// Input  : allow - 
//-----------------------------------------------------------------------------
void CBaseEntity::SetAllowPrecache( bool allow )
{
	m_bAllowPrecache = allow;
}

/*
================
FireBullets

Go to the trouble of combining multiple pellets into a single damage call.
================
*/

CBulletsTraceFilter::CBulletsTraceFilter(const IHandleEntity *passentity, int collisionGroup, int team) : CTraceFilterSimpleList(collisionGroup)
{
	m_iTeamLink = team;
	SetPassEntity(passentity);
}

bool CBulletsTraceFilter::ShouldHitEntity(IHandleEntity *pHandleEntity, int contentsMask)
{
	extern ConVar friendlyfire;

	CBaseEntity *pEntity = EntityFromEntityHandle(pHandleEntity);
	if (pEntity)
	{
		if ((pEntity->GetCollisionGroup() == COLLISION_GROUP_WEAPON) || pEntity->IsBaseCombatWeapon()) // Don't hit items, armor, etc...
			return false;

		if ((m_iTeamLink != TEAM_INVALID) && pEntity->IsPlayer() && (pEntity->GetTeamNumber() == m_iTeamLink) && !friendlyfire.GetBool())
			return false; // Bullets shouldn't trace against teammates. (plrs)

		if (dynamic_cast<CBaseViewModel*>(pEntity) != NULL) // Don't hit viewmodel!
			return false;
	}

#if defined( GAME_DLL )
	if (m_PassEntities.Count())
	{
		CBaseEntity *pPassEntity = EntityFromEntityHandle(m_PassEntities[0]);
		if (pEntity && pPassEntity && pEntity->GetOwnerEntity() == pPassEntity &&
			pPassEntity->IsSolidFlagSet(FSOLID_NOT_SOLID) && pPassEntity->IsSolidFlagSet(FSOLID_CUSTOMBOXTEST) &&
			pPassEntity->IsSolidFlagSet(FSOLID_CUSTOMRAYTEST))
		{
			// It's a bone follower of the entity to ignore (toml 8/3/2007)
			return false;
		}
	}
#endif

	return CTraceFilterSimpleList::ShouldHitEntity(pHandleEntity, contentsMask);
}

void CBaseEntity::FireBullets( const FireBulletsInfo_t &info )
{
	// TODO
}

ITraceFilter* CBaseEntity::GetBeamTraceFilter( void )
{
	return NULL;
}

void CBaseEntity::DispatchTraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr)
{
#ifdef GAME_DLL
	// Make sure our damage filter allows the damage.
	if ( !PassesDamageFilter( info ))
	{
		return;
	}
#endif

	TraceAttack( info, vecDir, ptr );
}

void CBaseEntity::TraceAttack( const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr )
{
	Vector vecOrigin = ptr->endpos - vecDir * 4;

	if ( m_takedamage )
	{
		AddMultiDamage(info, this);

		int blood = BloodColor();		
		if ( blood != DONT_BLEED )
		{
			SpawnBlood( vecOrigin, vecDir, blood, info.GetDamage(), ptr->hitgroup);// a little surface blood.
			TraceBleed( info.GetDamage(), vecDir, ptr, info.GetDamageType() );
		}
	}
}


//-----------------------------------------------------------------------------
// Allows the shooter to change the impact effect of his bullets
//-----------------------------------------------------------------------------
void CBaseEntity::DoImpactEffect( trace_t &tr, int nDamageType )
{
	// give shooter a chance to do a custom impact.
	UTIL_ImpactTrace( &tr, nDamageType );
} 


//-----------------------------------------------------------------------------
// Computes the tracer start position
//-----------------------------------------------------------------------------
void CBaseEntity::ComputeTracerStartPosition( const Vector &vecShotSrc, Vector *pVecTracerStart )
{
#ifndef HL2MP
	if ( g_pGameRules->IsMultiplayer() )
	{
		// NOTE: we do this because in MakeTracer, we force it to use the attachment position
		// in multiplayer, so the results from this function should never actually get used.
		pVecTracerStart->Init( 999, 999, 999 );
		return;
	}
#endif
	
	if ( IsPlayer() )
	{
		// adjust tracer position for player
		Vector forward, right;
		CBasePlayer *pPlayer = ToBasePlayer( this );
		pPlayer->EyeVectors( &forward, &right, NULL );
		*pVecTracerStart = vecShotSrc + Vector ( 0 , 0 , -4 ) + right * 2 + forward * 16;
	}
	else
	{
		*pVecTracerStart = vecShotSrc;

		CBaseCombatCharacter *pBCC = MyCombatCharacterPointer();
		if ( pBCC != NULL )
		{
			CBaseCombatWeapon *pWeapon = pBCC->GetActiveWeapon();

			if ( pWeapon != NULL )
			{
				Vector vecMuzzle;
				QAngle vecMuzzleAngles;

				if ( pWeapon->GetAttachment( 1, vecMuzzle, vecMuzzleAngles ) )
				{
					*pVecTracerStart = vecMuzzle;
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: Virtual function allows entities to handle tracer presentation
//			as they see fit.
//
// Input  : vecTracerSrc - the point at which to start the tracer (not always the
//			same spot as the traceline!
//
//			tr - the entire trace result for the shot.
//
// Output :
//-----------------------------------------------------------------------------
void CBaseEntity::MakeTracer( const Vector &vecTracerSrc, const trace_t &tr, int iTracerType )
{
	//const char *pszTracerName = GetTracerType();

	//Vector vNewSrc = vecTracerSrc;

	//int iAttachment = GetTracerAttachment();

	//switch ( iTracerType )
	//{
	//case TRACER_LINE:
	//	UTIL_Tracer( vNewSrc, tr.endpos, entindex(), iAttachment, 0.0f, false, pszTracerName );
	//	break;

	//case TRACER_LINE_AND_WHIZ:
	//	UTIL_Tracer( vNewSrc, tr.endpos, entindex(), iAttachment, 0.0f, true, pszTracerName );
	//	break;
	//}
}

//-----------------------------------------------------------------------------
// Default tracer attachment
//-----------------------------------------------------------------------------
int CBaseEntity::GetTracerAttachment( void )
{
	return 1;
}

int CBaseEntity::BloodColor()
{
	return DONT_BLEED; 
}

float CBaseEntity::HealthFraction() const
{
	if ( GetMaxHealth() == 0 )
		return 1.0f;

	float flFraction = ( float )GetHealth() / ( float )GetMaxHealth();
	flFraction = clamp( flFraction, 0.0f, 1.0f );
	return flFraction;
}

void CBaseEntity::TraceBleed( float flDamage, const Vector &vecDir, trace_t *ptr, int bitsDamageType )
{
	if ((BloodColor() == DONT_BLEED) || (BloodColor() == BLOOD_COLOR_MECH) || (flDamage == 0))
		return;

	if (!(bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB | DMG_BUCKSHOT)))
		return;

#ifdef GAME_DLL
	if (!IsAlive())
	{
		// dealing with a dead npc.
		if (GetMaxHealth() <= 0)
		{
			// no blood decal for a npc that has already decalled its limit.
			return;
		}
		else
		{
			m_iMaxHealth -= 1;
		}
	}
#endif

	UTIL_BloodSpawn(entindex(), ptr->endpos, vecDir, flDamage, ptr->hitgroup);
}


const char* CBaseEntity::GetTracerType()
{
	return NULL;
}

void CBaseEntity::ModifyEmitSoundParams( EmitSound_t &params )
{
}

//-----------------------------------------------------------------------------
// These methods encapsulate MOVETYPE_FOLLOW, which became obsolete
//-----------------------------------------------------------------------------
void CBaseEntity::AttachmentFollow(CBaseEntity *pBaseEntity, bool bBoneMerge, int iAttachment)
{
	SetParent(pBaseEntity, iAttachment);
	SetMoveType(MOVETYPE_NONE);

	if (bBoneMerge)
		AddEffects(EF_BONEMERGE);
	else
		RemoveEffects(EF_BONEMERGE);

	AddSolidFlags(FSOLID_NOT_SOLID);
	SetLocalOrigin(vec3_origin);
	SetLocalAngles(vec3_angle);
}

void CBaseEntity::FollowEntity( CBaseEntity *pBaseEntity, bool bBoneMerge, int iAttachment )
{
	if (pBaseEntity)
	{
		SetParent( pBaseEntity, iAttachment );
		SetMoveType( MOVETYPE_NONE );
		
		if ( bBoneMerge )
			AddEffects( EF_BONEMERGE );

		AddSolidFlags( FSOLID_NOT_SOLID );
		SetLocalOrigin( vec3_origin );
		SetLocalAngles( vec3_angle );
	}
	else
	{
		StopFollowingEntity();
	}
}

void CBaseEntity::SetEffectEntity( CBaseEntity *pEffectEnt )
{
	if ( m_hEffectEntity.Get() != pEffectEnt )
	{
		m_hEffectEntity = pEffectEnt;
	}
}

void CBaseEntity::ApplyLocalVelocityImpulse( const Vector &inVecImpulse )
{
	// NOTE: Don't have to use GetVelocity here because local values
	// are always guaranteed to be correct, unlike abs values which may 
	// require recomputation
	if ( inVecImpulse != vec3_origin )
	{
		Vector vecImpulse = inVecImpulse;

		// Safety check against receive a huge impulse, which can explode physics
		switch ( CheckEntityVelocity( vecImpulse ) )
		{
			case -1:
				Warning( "Discarding ApplyLocalVelocityImpulse(%f,%f,%f) on %s\n", vecImpulse.x, vecImpulse.y, vecImpulse.z, GetDebugName() );
				Assert( false );
				return;
			case 0:
				if ( CheckEmitReasonablePhysicsSpew() )
				{
					Warning( "Clamping ApplyLocalVelocityImpulse(%f,%f,%f) on %s\n", inVecImpulse.x, inVecImpulse.y, inVecImpulse.z, GetDebugName() );
				}
				break;
		}

		if ( GetMoveType() == MOVETYPE_VPHYSICS )
		{
			Vector worldVel;
			VPhysicsGetObject()->LocalToWorld( &worldVel, vecImpulse );
			VPhysicsGetObject()->AddVelocity( &worldVel, NULL );
		}
		else
		{
			InvalidatePhysicsRecursive( VELOCITY_CHANGED );
			m_vecVelocity += vecImpulse;
		}
	}
}

void CBaseEntity::ApplyAbsVelocityImpulse(const Vector &inVecImpulse, bool bNoLimit)
{
	if (inVecImpulse != vec3_origin)
	{
		Vector vecImpulse = inVecImpulse;

		// Safety check against receive a huge impulse, which can explode physics
		switch (CheckEntityVelocity(vecImpulse))
		{
		case -1:
			Warning("Discarding ApplyAbsVelocityImpulse(%f,%f,%f) on %s\n", vecImpulse.x, vecImpulse.y, vecImpulse.z, GetDebugName());
			Assert(false);
			return;
		case 0:
			if (CheckEmitReasonablePhysicsSpew())
			{
				Warning("Clamping ApplyAbsVelocityImpulse(%f,%f,%f) on %s\n", inVecImpulse.x, inVecImpulse.y, inVecImpulse.z, GetDebugName());
			}
			break;
		}

		// BB2: Prevents extreme pushback values...
		if ((IsPlayer() || IsNPC()) && (bNoLimit == false))
			vecImpulse *= 0.1;

		PreAbsVelocityImpulse(bNoLimit);

		if (GetMoveType() == MOVETYPE_VPHYSICS)
		{
			VPhysicsGetObject()->AddVelocity(&vecImpulse, NULL);
		}
		else
		{
			// NOTE: Have to use GetAbsVelocity here to ensure it's the correct value
			Vector vecResult;
			VectorAdd(GetAbsVelocity(), vecImpulse, vecResult);
			SetAbsVelocity(vecResult);
		}

		PostAbsVelocityImpulse(bNoLimit);
	}
}

void CBaseEntity::ApplyLocalAngularVelocityImpulse( const AngularImpulse &angImpulse )
{
	if (angImpulse != vec3_origin )
	{
		// Safety check against receive a huge impulse, which can explode physics
		if ( !IsEntityAngularVelocityReasonable( angImpulse ) )
		{
			Warning( "Bad ApplyLocalAngularVelocityImpulse(%f,%f,%f) on %s\n", angImpulse.x, angImpulse.y, angImpulse.z, GetDebugName() );
			Assert( false );
			return;
		}

		if ( GetMoveType() == MOVETYPE_VPHYSICS )
		{
			VPhysicsGetObject()->AddVelocity( NULL, &angImpulse );
		}
		else
		{
			QAngle vecResult;
			AngularImpulseToQAngle( angImpulse, vecResult );
			VectorAdd( GetLocalAngularVelocity(), vecResult, vecResult );
			SetLocalAngularVelocity( vecResult );
		}
	}
}

void CBaseEntity::SetCollisionGroup( int collisionGroup )
{
	if ( (int)m_CollisionGroup != collisionGroup )
	{
		m_CollisionGroup = collisionGroup;
		CollisionRulesChanged();
	}
}


void CBaseEntity::CollisionRulesChanged()
{
	// ivp maintains state based on recent return values from the collision filter, so anything
	// that can change the state that a collision filter will return (like m_Solid) needs to call RecheckCollisionFilter.
	if ( VPhysicsGetObject() )
	{
		extern bool PhysIsInCallback();
		if ( PhysIsInCallback() )
		{
			Warning("Changing collision rules within a callback is likely to cause crashes!\n");
			Assert(0);
		}
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = VPhysicsGetObjectList( pList, ARRAYSIZE(pList) );

		for ( int i = 0; i < count; i++ )
		{
			if ( pList[i] != NULL ) //this really shouldn't happen, but it does >_<
				pList[i]->RecheckCollisionFilter();
		}
	}
}

int CBaseEntity::GetWaterType() const
{
	int out = 0;
	if ( m_nWaterType & 1 )
		out |= CONTENTS_WATER;
	if ( m_nWaterType & 2 )
		out |= CONTENTS_SLIME;
	return out;
}

void CBaseEntity::SetWaterType( int nType )
{
	m_nWaterType = 0;
	if ( nType & CONTENTS_WATER )
		m_nWaterType |= 1;
	if ( nType & CONTENTS_SLIME )
		m_nWaterType |= 2;
}

ConVar	sv_alternateticks( "sv_alternateticks", ( IsX360() ) ? "1" : "0", FCVAR_SPONLY, "If set, server only simulates entities on even numbered ticks.\n" );

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseEntity::IsSimulatingOnAlternateTicks()
{
	if ( gpGlobals->maxClients != 1 )
	{
		return false;
	}
	
	return sv_alternateticks.GetBool();
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CBaseEntity::IsToolRecording() const
{
#ifndef NO_TOOLFRAMEWORK
	return m_bToolRecording;
#else
	return false;
#endif
}
#endif
