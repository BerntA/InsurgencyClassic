//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "physics_saverestore.h"
#include "vphysics/friction.h"
//#include "ai_basenpc.h"
#include "movevars_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


class CPhysicsEntitySolver : public CLogicalEntity//, public IMotionEvent
{
	DECLARE_CLASS( CPhysicsEntitySolver, CLogicalEntity );
public:
	DECLARE_DATADESC();
	void Init( CBaseEntity *pMovingEntity, CBaseEntity *pPhysicsBlocker, float separationTime );
	static CPhysicsEntitySolver *Create( CBaseEntity *pMovingEntity, CBaseEntity *pPhysicsBlocker, float separationTime );

	// CBaseEntity
	virtual void Spawn();
	virtual void UpdateOnRemove();
	virtual void Think();

	// IMotionEvent
	//virtual simresult_e	Simulate( IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular );

private:
	// locals
	void ResetCancelTime();
	void BecomePenetrationSolver();
	//bool IsIntersecting();
	//bool IsTouching();

	EHANDLE						m_hMovingEntity;
	EHANDLE						m_hPhysicsBlocker;
	//IPhysicsMotionController	*m_pController;
	float						m_separationDuration;
	float						m_cancelTime;
	int							m_savedCollisionGroup;
};

LINK_ENTITY_TO_CLASS( physics_entity_solver, CPhysicsEntitySolver );

BEGIN_DATADESC( CPhysicsEntitySolver )

	DEFINE_FIELD( m_hMovingEntity, FIELD_EHANDLE ),
	DEFINE_FIELD( m_hPhysicsBlocker, FIELD_EHANDLE ),
	DEFINE_FIELD( m_separationDuration, FIELD_FLOAT ),
	DEFINE_FIELD( m_cancelTime, FIELD_TIME ),
	DEFINE_FIELD( m_savedCollisionGroup, FIELD_INTEGER ),
	//DEFINE_PHYSPTR( m_pController ),

END_DATADESC()

CPhysicsEntitySolver *CPhysicsEntitySolver::Create( CBaseEntity *pMovingEntity, CBaseEntity *pPhysicsBlocker, float separationTime )
{
	CPhysicsEntitySolver *pSolver = (CPhysicsEntitySolver *)CBaseEntity::CreateNoSpawn( "physics_entity_solver", vec3_origin, vec3_angle, NULL );
	pSolver->Init( pMovingEntity, pPhysicsBlocker, separationTime );
	pSolver->Spawn();
	//NDebugOverlay::EntityBounds(pNPC, 255, 255, 0, 64, 0.5f );
	return pSolver;
}

void CPhysicsEntitySolver::Init( CBaseEntity *pMovingEntity, CBaseEntity *pPhysicsBlocker, float separationTime )
{
	m_hMovingEntity = pMovingEntity;
	m_hPhysicsBlocker = pPhysicsBlocker;
	//m_pController = NULL;
	m_separationDuration = separationTime;
}

void CPhysicsEntitySolver::Spawn()
{
	SetNextThink( gpGlobals->curtime + m_separationDuration );
	PhysDisableEntityCollisions( m_hMovingEntity, m_hPhysicsBlocker );
	m_savedCollisionGroup = m_hPhysicsBlocker->GetCollisionGroup();
	m_hPhysicsBlocker->SetCollisionGroup( COLLISION_GROUP_DEBRIS );
	m_hPhysicsBlocker->VPhysicsGetObject()->RecheckContactPoints();
}

void CPhysicsEntitySolver::Think()
{
	UTIL_Remove(this);
}

void CPhysicsEntitySolver::UpdateOnRemove()
{
	//physenv->DestroyMotionController( m_pController );
	//m_pController = NULL;
	CBaseEntity *pEntity = m_hMovingEntity.Get();
	CBaseEntity *pPhysics = m_hPhysicsBlocker.Get();
	if ( pEntity && pPhysics )
	{
		PhysEnableEntityCollisions( pEntity, pPhysics );
	}
	if ( pPhysics )
	{
		pPhysics->SetCollisionGroup( m_savedCollisionGroup );
	}
	BaseClass::UpdateOnRemove();
}


CBaseEntity *EntityPhysics_CreateSolver( CBaseEntity *pMovingEntity, CBaseEntity *pPhysicsObject, bool disableCollisions, float separationDuration )
{
	if ( PhysEntityCollisionsAreDisabled( pMovingEntity, pPhysicsObject ) )
		return NULL;

	return CPhysicsEntitySolver::Create( pMovingEntity, pPhysicsObject, separationDuration );
}

