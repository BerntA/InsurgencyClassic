//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PHYSICS_NPC_SOLVER_H
#define PHYSICS_NPC_SOLVER_H

#ifdef _WIN32
#pragma once
#endif

extern CBaseEntity* EntityPhysics_CreateSolver(CBaseEntity* pMovingEntity, CBaseEntity* pPhysicsBlocker, bool disableCollisions, float separationDuration);

#endif // PHYSICS_NPC_SOLVER_H