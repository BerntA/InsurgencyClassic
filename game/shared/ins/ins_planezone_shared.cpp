//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"

#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

#include "ins_planezone_shared.h"

#ifdef GAME_DLL

#include "ins_player_shared.h"
#include "ins_gamerules.h"
#include "model_types.h"
#include "ins_utils.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef GAME_DLL

//=========================================================
//=========================================================
enum FindSurfaceRets_t
{
	FSRET_VALID = 0,
	FSRET_BADINIT,
	FSRET_INTERNAL,
	FSRET_MISSING,
	FSRET_BADTEXTURE,
	FSRET_BADTRACE,
	FSRET_COUNT
};

//=========================================================
//=========================================================
void CPlaneZone::Spawn( void )
{
	BaseClass::Spawn( );

	int iFSRet = FSRET_BADINIT;

	// init
	if( Init( ) )
		iFSRet = DetermineProtectedSurfaces( );

	// remove if failed
	if( iFSRet != FSRET_VALID )
	{
		static const char *pszError[ FSRET_COUNT ] = {
			"",											// FSRET_VALID
			"init failed",								// FSRET_BADINIT
			"internal error",							// FSRET_INTERNAL
			"cannot find surface",						// FSRET_MISSING
			"cannot find a marked texture",				// FSRET_BADTEXTURE
			"internal error - didn't hit a surface",	// FSRET_BADTRACE
		};

		Warning( "%s - %s\n", GetClassname( ), pszError[ iFSRet ] );

		// PNOTE: when not testing, remove the broken entity
		// but otherwise keep it - because a mapper
		// shouldn't be placing wrongly configured entities
		UTIL_Remove(this);

		return;
	}

	MakeNotSolid( );
	EnableTouch( );
}

//=========================================================
//=========================================================
void CPlaneZone::PlayerStartTouch( CINSPlayer *pPlayer )
{
	UTIL_SendHint( pPlayer, GetHintDisplay( ) );
}

//=========================================================
//=========================================================
int CPlaneZone::GetHintDisplay( void ) const
{
	return HINT_INVALID;
}

//=========================================================
//=========================================================
class CTraceFilterPlaneZone : public CTraceFilter
{
public:
	CTraceFilterPlaneZone( CPlaneZone *pPlaneZone )
	{
		m_pPlaneZone = pPlaneZone;
	}

	bool ShouldHitEntity( IHandleEntity *pEntity, int contentsMask )
	{
		return ( m_pPlaneZone == pEntity );
	}

private:
	CPlaneZone *m_pPlaneZone;
};

//=========================================================
//=========================================================
int CPlaneZone::DetermineProtectedSurfaces( void )
{
	static const char *pszMarkedSide = "TOOLS/SPAWNPROTECT2";

	model_t *pModel = GetModel( );

	if( !pModel )
		return FSRET_INTERNAL;

	if ( modelinfo->GetModelType( pModel ) != mod_brush )
		return FSRET_INTERNAL;

	// find the texture in the list before doing any traces
	int iMaterialCount = modelinfo->GetModelMaterialCount( pModel );

	if( iMaterialCount == 0 )
		return FSRET_INTERNAL;

	bool bFound = false;

	IMaterial *pBaseMaterial = NULL;
	IMaterial **pMaterials = &pBaseMaterial;

	modelinfo->GetModelMaterials( pModel, iMaterialCount, pMaterials );

	for( int i = 0; i < iMaterialCount; i++ )
	{	
		IMaterial *pMaterial = pMaterials[ i ];
		Assert( pMaterial );

		if( !pMaterial )
			continue;

		const char *pszName = pMaterial->GetName( );

		if( Q_strcmp( pszName, pszMarkedSide ) == 0 )
		{
			bFound = true;
			break;
		}
	}

	if( !bFound )
		return FSRET_BADTEXTURE;

	// get the mins and maxs
	Vector vecMins, vecMaxs;
	modelinfo->GetModelBounds( pModel, vecMins, vecMaxs );

	// work out the lengths
	float flLengths[ 3 ];

	for( int i = 0; i < 3; i++ )
		flLengths[ i ] = abs( vecMaxs[ i ] - vecMins[ i ] );

	// get the origin
	Vector vecOrigin = Vector( 
		vecMins[ 0 ] + ( flLengths[ 0 ] * 0.5f ),
		vecMins[ 1 ] + ( flLengths[ 1 ] * 0.5f ),
		vecMins[ 2 ] + ( flLengths[ 2 ] * 0.5f ) );

	Vector vecDir[ 3 ] = {
		Vector( 1.0f, 0.0f, 0.0f ),
		Vector( 0.0f, 1.0f, 0.0f ),
		Vector( 0.0f, 0.0f, 1.0f )
	};

#ifdef _DEBUG

	static Color DebugColorPos( 0, 255, 0 );
	static Color DebugColorNeg( 255, 0, 0 );

#endif

	Vector vecStart;
	trace_t tr;

	// now do some tracelines
	CTraceFilterPlaneZone TraceFilterPlaneZone( this );
	bFound = false;

	for( int i = 0; i < 3; i++ )
	{
		for( int j = 0; j < 2; j++ )
		{
			VectorMA( vecOrigin, flLengths[ i ] * 0.75f, vecDir[ i ] * ( ( j != 0 ) ? 1.0f : -1.0f ), vecStart );

			UTIL_TraceLine( vecStart, vecOrigin, MASK_ALL, &TraceFilterPlaneZone, &tr );

			if( !tr.DidHit( ) )
			{
			#ifdef _DEBUG

				UTIL_AddLineList( vecStart, vecOrigin, DebugColorNeg );

			#endif

				return FSRET_BADTRACE;
			}

		#ifdef _DEBUG

			UTIL_AddLineList( vecStart, vecOrigin, DebugColorPos );

		#endif

			csurface_t surface = tr.surface;

			if( Q_strcmp( surface.name, pszMarkedSide ) != 0 )
				continue;

			FoundProtectedSurface( tr.plane.normal, flLengths[ i ], tr.endpos );

			if( StopFirstProtectedSurface( ) )
				return FSRET_VALID;

			bFound = true;
		}
	}

	if( !bFound && MustFindProtectedSurface( ) )
		return FSRET_MISSING;

	return FSRET_VALID;
}

#endif

//=========================================================
//=========================================================
float CPlaneZone::GetInternalPlaneDistance( const Vector &vecNormal, const Vector &vecCentre, const Vector &vecPoint )
{
	float flDistance = DotProduct( -1.0f * vecNormal, vecPoint - vecCentre );
	float flNormalMag = vecNormal.Length();

	if( flNormalMag == 0 )
		return 0.0f;

	if( flNormalMag == 1 )
		return flDistance;

	return flDistance / flNormalMag;
}

//=========================================================
//=========================================================
float CPlaneZone::GetInternalPlaneDistance( const ZonedPlaneData_t &PlaneData, const Vector &vecPoint )
{
	return GetInternalPlaneDistance( PlaneData.m_vecNormal, PlaneData.m_vecCentre, vecPoint );
}
