//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "c_ins_combineball.h"
#include "materialsystem/IMaterial.h"
#include "model_types.h"
#include "c_physicsprop.h"
#include "c_te_effect_dispatch.h"
#include "fx_quad.h"
#include "fx.h"
#include "ClientEffectPrecacheSystem.h"
#include "view.h"
#include "view_scene.h"
#include "beamdraw.h"

//=========================================================
//=========================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCombineBall )
	CLIENTEFFECT_MATERIAL( "effects/ar2_altfire1" )
	CLIENTEFFECT_MATERIAL( "effects/ar2_altfire1b" )
	CLIENTEFFECT_MATERIAL( "effects/combinemuzzle1_nocull" )
	CLIENTEFFECT_MATERIAL( "effects/combinemuzzle2_nocull" )
	CLIENTEFFECT_MATERIAL( "effects/combinemuzzle1" )
	CLIENTEFFECT_MATERIAL( "effects/ar2_altfire1" )
	CLIENTEFFECT_MATERIAL( "effects/ar2_altfire1b" )
CLIENTEFFECT_REGISTER_END( )

IMPLEMENT_CLIENTCLASS_DT( C_INSCombineBall, DT_INSCombineBall, CINSCombineBall )
	RecvPropBool( RECVINFO( m_bEmit ) ),
	RecvPropFloat( RECVINFO( m_flRadius ) ),
END_RECV_TABLE()

//=========================================================
//=========================================================
C_INSCombineBall::C_INSCombineBall( void )
{
	m_pFlickerMaterial = NULL;
	m_pBodyMaterial = NULL;
	m_pBlurMaterial = NULL;
}

//=========================================================
//=========================================================
void C_INSCombineBall::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_vecLastOrigin = GetAbsOrigin( );
		InitMaterials( );
	}
}

//=========================================================
//=========================================================
RenderGroup_t C_INSCombineBall::GetRenderGroup( void )
{
	return RENDER_GROUP_TRANSLUCENT_ENTITY;
}

//=========================================================
//=========================================================
bool C_INSCombineBall::InitMaterials( void )
{
	// motion blur
	if ( m_pBlurMaterial == NULL )
	{
		m_pBlurMaterial = materials->FindMaterial( "effects/ar2_altfire1b", NULL, false );

		if( m_pBlurMaterial == NULL )
			return false;
	}

	// main body of the ball
	if ( m_pBodyMaterial == NULL )
	{
		m_pBodyMaterial = materials->FindMaterial( "effects/ar2_altfire1", NULL, false );

		if( m_pBodyMaterial == NULL )
			return false;
	}

	// flicker material
	if ( m_pFlickerMaterial == NULL )
	{
		m_pFlickerMaterial = materials->FindMaterial( "effects/combinemuzzle1", NULL, false );

		if( m_pFlickerMaterial == NULL )
			return false;
	}

	return true;
}

//=========================================================
//=========================================================
void C_INSCombineBall::DrawMotionBlur( void )
{
	float flColor[ 3 ];

	Vector vecDir = GetAbsOrigin( ) - m_vecLastOrigin;
	float flSpeed = VectorNormalize( vecDir );
	
	flSpeed = clamp( flSpeed, 0, 32 );
	
	float flStepSize = min( ( flSpeed * 0.5f ), 4.0f );

	Vector vecSpawnPos = GetAbsOrigin( );
	Vector vecSpawnStep = -vecDir * flStepSize;

	float flBase = RemapValClamped( flSpeed, 4, 32, 0.0f, 1.0f );

	materials->Bind( m_pBlurMaterial );

	// draw the motion blurred trail
	for( int i = 0; i < 8; i++ )
	{
		vecSpawnPos += vecSpawnStep;

		flColor[ 0 ] = flColor[ 1 ] = flColor[ 2 ] = flBase * ( 1.0f - ( ( float ) i / 12.0f ) );

		DrawHalo( m_pBlurMaterial, vecSpawnPos, m_flRadius, flColor );
	}
}

//=========================================================
//=========================================================
void C_INSCombineBall::DrawFlicker( void )
{
	float flRand1 = random->RandomFloat( 0.2f, 0.3f );
	float flRand2 = random->RandomFloat( 1.5f, 2.5f );

	if( gpGlobals->frametime == 0.0f )
	{
		flRand1 = 0.2f;
		flRand2 = 1.5f;
	}

	float flColor[ 3 ];
	flColor[ 0 ] = flColor[ 1 ] = flColor[ 2 ] = flRand1;

	// draw the flickering glow
	materials->Bind( m_pFlickerMaterial );
	DrawHalo( m_pFlickerMaterial, GetAbsOrigin( ), m_flRadius * flRand2, flColor );
}

//=========================================================
//=========================================================
void DrawHaloOriented( const Vector &vecSource, float flScale, float const *pColor, float flRoll )
{
	Vector vecPoint, vecScreen;
	
	IMesh *pMesh = materials->GetDynamicMesh( );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	// transform source into screen space
	ScreenTransform( vecSource, vecScreen );

	Vector vecRight, vecUp;
	float flSR, flCR;

	SinCos( flRoll, &flSR, &flCR );

	for( int i = 0; i < 3; i++ )
	{
		vecRight[ i ] = CurrentViewRight( )[ i ] * flCR + CurrentViewUp( )[ i ] * flSR;
		vecUp[ i ] = CurrentViewRight( )[ i ] * -flSR + CurrentViewUp( )[ i ] * flCR;
	}

	meshBuilder.Color3fv( pColor );
	meshBuilder.TexCoord2f( 0, 0, 1 );
	VectorMA( vecSource, -flScale, vecUp, vecPoint );
	VectorMA( vecPoint, -flScale, vecRight, vecPoint );
	meshBuilder.Position3fv( vecPoint.Base( ) );
	meshBuilder.AdvanceVertex( );

	meshBuilder.Color3fv( pColor );
	meshBuilder.TexCoord2f( 0, 0, 0 );
	VectorMA( vecSource, flScale, vecUp, vecPoint );
	VectorMA( vecPoint, -flScale, vecRight, vecPoint );
	meshBuilder.Position3fv( vecPoint.Base( ) );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv( pColor );
	meshBuilder.TexCoord2f( 0, 1, 0 );
	VectorMA( vecSource, flScale, vecUp, vecPoint );
	VectorMA( vecPoint, flScale, vecRight, vecPoint );
	meshBuilder.Position3fv( vecPoint.Base( ) );
	meshBuilder.AdvanceVertex( );

	meshBuilder.Color3fv( pColor );
	meshBuilder.TexCoord2f( 0, 1, 1 );
	VectorMA( vecSource, -flScale, vecUp, vecPoint );
	VectorMA( vecPoint, flScale, vecRight, vecPoint );
	meshBuilder.Position3fv( vecPoint.Base( ) );
	meshBuilder.AdvanceVertex( );
	
	meshBuilder.End( );
	pMesh->Draw( );
}

//=========================================================
//=========================================================
int C_INSCombineBall::DrawModel( int iFlags )
{
	if( !m_bEmit )
		return 0;
	
	// make sure our materials are cached
	if( !InitMaterials( ) )
	{
		AssertOnce( false );
		return 0;
	}

	// draw the flickering overlay
	DrawFlicker( );
	
	// Draw the motion blur from movement
	DrawMotionBlur( );

	float flColor[ 3 ];
	flColor[ 0 ] = flColor[ 1 ] = flColor[ 2 ] = 1.0f;

	float flSinOffs = 1.0f * sin( gpGlobals->curtime * 25 );

	float flRoll = SpawnTime( );

	// Draw the main ball body
	materials->Bind( m_pBodyMaterial, ( C_BaseEntity* ) this );
	DrawHaloOriented( GetAbsOrigin( ), m_flRadius + flSinOffs, flColor, flRoll );
	
	m_vecLastOrigin = GetAbsOrigin( );

	return 1;
}

//=========================================================
//=========================================================
void CombineBallImpactCallback( const CEffectData &data )
{
	// quick flash
	FX_AddQuad( data.m_vOrigin,
				data.m_vNormal,
				data.m_flRadius * 10.0f,
				0,
				0.75f, 
				1.0f,
				0.0f,
				0.4f,
				random->RandomInt( 0, 360 ), 
				0,
				Vector( 1.0f, 1.0f, 1.0f ), 
				0.25f, 
				"effects/combinemuzzle1_nocull",
				( FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA ) );

	// lingering burn
	FX_AddQuad( data.m_vOrigin,
				data.m_vNormal, 
				data.m_flRadius * 2.0f,
				data.m_flRadius * 4.0f,
				0.75f, 
				1.0f,
				0.0f,
				0.4f,
				random->RandomInt( 0, 360 ), 
				0,
				Vector( 1.0f, 1.0f, 1.0f ), 
				0.5f, 
				"effects/combinemuzzle2_nocull",
				( FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA ) );

	// throw sparks
	FX_ElectricSpark( data.m_vOrigin, 2, 1, &data.m_vNormal );
}

DECLARE_CLIENT_EFFECT( "cball_bounce", CombineBallImpactCallback );

//=========================================================
//=========================================================
void CombineBallExplosionCallback( const CEffectData &data )
{
	Vector vecNormal( 0, 0, 1 );

	// throw sparks
	FX_ElectricSpark( data.m_vOrigin, 4, 1, &vecNormal );
}

DECLARE_CLIENT_EFFECT( "cball_explode", CombineBallExplosionCallback );