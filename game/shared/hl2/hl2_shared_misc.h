//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Shared HL2 Stuff...
//
//========================================================================================//

#ifndef HL2_SHARED_MISC_H
#define HL2_SHARED_MISC_H
#ifdef _WIN32
#pragma once
#endif 

#ifdef CLIENT_DLL
#include "c_baseentity.h"
#include "iviewrender_beams.h"
#else
#include "baseentity.h"
#include "Sprite.h"
#include "npcevent.h"
#include "beam_shared.h"
#endif

bool PlayerPickupControllerIsHoldingEntity(CBaseEntity *pPickupController, CBaseEntity *pHeldEntity);
float PlayerPickupGetHeldObjectMass(CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject);

#endif // HL2_SHARED_MISC_H