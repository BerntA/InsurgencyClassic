//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_COMBINEBALL_H
#define INS_COMBINEBALL_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
class CSpriteTrail;

//=========================================================
//=========================================================
class CINSCombineBall : public CBaseAnimating
{
	DECLARE_CLASS( CINSCombineBall, CBaseAnimating );
	DECLARE_DATADESC( );
	DECLARE_SERVERCLASS( );

public:

	static void CreateBall( const Vector &vecOrigin, const Vector &vecVelocity, CBaseEntity *pOwner );

	void Precache( void );
	void Spawn( void );
	void UpdateOnRemove( void );

	void VPhysicsCollision( int iIndex, gamevcollisionevent_t *pEvent );

	bool OverridePropdata( void );
	bool CreateVPhysics( void );

	void ExplodeThink( void );

	void SetRadius( float flRadius );
	void SetSpeed( float flSpeed ) { m_flSpeed = flSpeed; }
	float GetSpeed( void ) const { return m_flSpeed; }

	void StartLifetime( float flDuration );
	void SetMass( float flMass );

	CBasePlayer *HasPhysicsAttacker( float flDT );

private:

	void SetPlayerLaunched( CBasePlayer *pOwner );

	// Pow!
	void DoExplosion( );

	void StartAnimating( void );
	void StopAnimating( void );

	void SetBallAsLaunched( void );

	void CollisionEventToTrace( int index, gamevcollisionevent_t *pEvent, trace_t &tr );
	bool DissolveEntity( CBaseEntity *pEntity );
	bool OnHitEntity( CBaseEntity *pHitEntity, float flSpeed, int index, gamevcollisionevent_t *pEvent );
	void DoImpactEffect( const Vector &preVelocity, int index, gamevcollisionevent_t *pEvent );

	// Bounce inside the spawner: 
	void BounceInSpawner( float flSpeed, int index, gamevcollisionevent_t *pEvent );

	bool IsAttractiveTarget( CBaseEntity *pEntity );

	// Deflects the ball toward enemies in case of a collision 
	void DeflectTowardEnemy( float flSpeed, int index, gamevcollisionevent_t *pEvent );

	// Is this something we can potentially dissolve? 
	bool IsHittableEntity( CBaseEntity *pHitEntity );

	// Sucky. 
	void DieThink();
	void AnimThink( void );

	void FadeOut( float flDuration );

private:

	float	m_flLastBounceTime;

	float	m_flSpeed;

	CSpriteTrail *m_pGlowTrail;
	CSoundPatch *m_pHoldingSound;

	float	m_flNextDamageTime;
	float	m_flLastCaptureTime;

	CNetworkVar( bool, m_bEmit );
	CNetworkVar( float, m_flRadius );
};

#endif // PROP_COMBINE_BALL_H
