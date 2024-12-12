//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Shared HL2 Stuff...
//
//========================================================================================//

#include "cbase.h"
#include "hl2_shared_misc.h"

#ifdef CLIENT_DLL
#include "view.h"
#else
#include "soundent.h"
#include "explode.h"
#include "smoke_trail.h"
#include "collisionutils.h"
#include "te_effect_dispatch.h"
#endif

#include "gamerules.h"
#include "in_buttons.h"
#include "vphysics/friction.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar player_throwforce("player_throwforce", "1000", FCVAR_REPLICATED | FCVAR_CHEAT);
ConVar player_pickup_maxmass("player_pickup_maxmass", "250", FCVAR_REPLICATED | FCVAR_CHEAT);

//-----------------------------------------------------------------------------
// Purpose: Computes a local matrix for the player clamped to valid carry ranges
//-----------------------------------------------------------------------------
// when looking level, hold bottom of object 8 inches below eye level
#define PLAYER_HOLD_LEVEL_EYES	-8

// when looking down, hold bottom of object 0 inches from feet
#define PLAYER_HOLD_DOWN_FEET	2

// when looking up, hold bottom of object 24 inches above eye level
#define PLAYER_HOLD_UP_EYES		24

// use a +/-30 degree range for the entire range of motion of pitch
#define PLAYER_LOOK_PITCH_RANGE	30

// player can reach down 2ft below his feet (otherwise he'll hold the object above the bottom)
#define PLAYER_REACH_DOWN_DISTANCE	24

static void ComputePlayerMatrix(CBasePlayer *pPlayer, matrix3x4_t &out)
{
	if (!pPlayer)
		return;

	QAngle angles = pPlayer->EyeAngles();
	Vector origin = pPlayer->EyePosition();

	// 0-360 / -180-180
	//angles.x = init ? 0 : AngleDistance( angles.x, 0 );
	//angles.x = clamp( angles.x, -PLAYER_LOOK_PITCH_RANGE, PLAYER_LOOK_PITCH_RANGE );
	angles.x = 0;

	float feet = pPlayer->GetAbsOrigin().z + pPlayer->WorldAlignMins().z;
	float eyes = origin.z;
	float zoffset = 0;
	// moving up (negative pitch is up)
	if (angles.x < 0)
	{
		zoffset = RemapVal(angles.x, 0, -PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_UP_EYES);
	}
	else
	{
		zoffset = RemapVal(angles.x, 0, PLAYER_LOOK_PITCH_RANGE, PLAYER_HOLD_LEVEL_EYES, PLAYER_HOLD_DOWN_FEET + (feet - eyes));
	}
	origin.z += zoffset;
	angles.x = 0;
	AngleMatrix(angles, origin, out);
}

static void MatrixOrthogonalize(matrix3x4_t &matrix, int column)
{
	Vector columns[3];
	int i;

	for (i = 0; i < 3; i++)
	{
		MatrixGetColumn(matrix, i, columns[i]);
	}

	int index0 = column;
	int index1 = (column + 1) % 3;
	int index2 = (column + 2) % 3;

	columns[index2] = CrossProduct(columns[index0], columns[index1]);
	columns[index1] = CrossProduct(columns[index2], columns[index0]);
	VectorNormalize(columns[index2]);
	VectorNormalize(columns[index1]);
	MatrixSetColumn(columns[index1], index1, matrix);
	MatrixSetColumn(columns[index2], index2, matrix);
}

#define SIGN(x) ( (x) < 0 ? -1 : 1 )

static QAngle AlignAngles(const QAngle &angles, float cosineAlignAngle)
{
	matrix3x4_t alignMatrix;
	AngleMatrix(angles, alignMatrix);

	// NOTE: Must align z first
	for (int j = 3; --j >= 0;)
	{
		Vector vec;
		MatrixGetColumn(alignMatrix, j, vec);
		for (int i = 0; i < 3; i++)
		{
			if (fabs(vec[i]) > cosineAlignAngle)
			{
				vec[i] = SIGN(vec[i]);
				vec[(i + 1) % 3] = 0;
				vec[(i + 2) % 3] = 0;
				MatrixSetColumn(vec, j, alignMatrix);
				MatrixOrthogonalize(alignMatrix, j);
				break;
			}
		}
	}

	QAngle out;
	MatrixAngles(alignMatrix, out);
	return out;
}

//-----------------------------------------------------------------------------
// Purpose: Grab Controller
//-----------------------------------------------------------------------------

// derive from this so we can add save/load data to it
struct game_shadowcontrol_params_t : public hlshadowcontrol_params_t
{
};

//-----------------------------------------------------------------------------
class CGrabController : public IMotionEvent
{
public:

	CGrabController(void);
	~CGrabController(void);
	void AttachEntity(CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, bool bIsMegaPhysCannon, const Vector &vGrabPosition, bool bUseGrabPosition);
	void DetachEntity(bool bClearVelocity);

	bool UpdateObject(CBasePlayer *pPlayer, float flError);

	void SetTargetPosition(const Vector &target, const QAngle &targetOrientation);
	float ComputeError();
	float GetLoadWeight(void) const { return m_flLoadWeight; }
	void SetAngleAlignment(float alignAngleCosine) { m_angleAlignment = alignAngleCosine; }
	void SetIgnorePitch(bool bIgnore) { m_bIgnoreRelativePitch = bIgnore; }
	QAngle TransformAnglesToPlayerSpace(const QAngle &anglesIn, CBasePlayer *pPlayer);
	QAngle TransformAnglesFromPlayerSpace(const QAngle &anglesIn, CBasePlayer *pPlayer);

	CBaseEntity *GetAttached() { return (CBaseEntity *)m_attachedEntity; }

	IMotionEvent::simresult_e Simulate(IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular);
	float GetSavedMass(IPhysicsObject *pObject);

	QAngle			m_attachedAnglesPlayerSpace;
	Vector			m_attachedPositionObjectSpace;

private:
	// Compute the max speed for an attached object
	void ComputeMaxSpeed(CBaseEntity *pEntity, IPhysicsObject *pPhysics);

	game_shadowcontrol_params_t	m_shadow;
	float			m_timeToArrive;
	float			m_errorTime;
	float			m_error;
	float			m_contactAmount;
	float			m_angleAlignment;
	bool			m_bCarriedEntityBlocksLOS;
	bool			m_bIgnoreRelativePitch;

	float			m_flLoadWeight;
	float			m_savedRotDamping[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	float			m_savedMass[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	EHANDLE			m_attachedEntity;
	QAngle			m_vecPreferredCarryAngles;
	bool			m_bHasPreferredCarryAngles;


	IPhysicsMotionController *m_controller;
	int				m_frameCount;
	friend class CWeaponPhysCannon;
};

const float DEFAULT_MAX_ANGULAR = 360.0f * 10.0f;
const float REDUCED_CARRY_MASS = 1.0f;

CGrabController::CGrabController(void)
{
	m_shadow.dampFactor = 1.0;
	m_shadow.teleportDistance = 0;
	m_errorTime = 0;
	m_error = 0;
	// make this controller really stiff!
	m_shadow.maxSpeed = 1000;
	m_shadow.maxAngular = DEFAULT_MAX_ANGULAR;
	m_shadow.maxDampSpeed = m_shadow.maxSpeed * 2;
	m_shadow.maxDampAngular = m_shadow.maxAngular;
	m_attachedEntity = NULL;
	m_vecPreferredCarryAngles = vec3_angle;
	m_bHasPreferredCarryAngles = false;
}

CGrabController::~CGrabController(void)
{
	DetachEntity(false);
}

void CGrabController::SetTargetPosition(const Vector &target, const QAngle &targetOrientation)
{
	m_shadow.targetPosition = target;
	m_shadow.targetRotation = targetOrientation;

	m_timeToArrive = gpGlobals->frametime;

	CBaseEntity *pAttached = GetAttached();
	if (pAttached)
	{
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();

		if (pObj != NULL)
		{
			pObj->Wake();
		}
		else
		{
			DetachEntity(false);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float CGrabController::ComputeError()
{
	if (m_errorTime <= 0)
		return 0;

	CBaseEntity *pAttached = GetAttached();
	if (pAttached)
	{
		Vector pos;
		IPhysicsObject *pObj = pAttached->VPhysicsGetObject();

		if (pObj)
		{
			pObj->GetShadowPosition(&pos, NULL);

			float error = (m_shadow.targetPosition - pos).Length();
			if (m_errorTime > 0)
			{
				if (m_errorTime > 1)
				{
					m_errorTime = 1;
				}
				float speed = error / m_errorTime;
				if (speed > m_shadow.maxSpeed)
				{
					error *= 0.5;
				}
				m_error = (1 - m_errorTime) * m_error + error * m_errorTime;
			}
		}
		else
		{
			DevMsg("Object attached to Physcannon has no physics object\n");
			DetachEntity(false);
			return 9999; // force detach
		}
	}

	m_errorTime = 0;

	return m_error;
}

#define MASS_SPEED_SCALE	60
#define MAX_MASS			40

void CGrabController::ComputeMaxSpeed(CBaseEntity *pEntity, IPhysicsObject *pPhysics)
{
#ifndef CLIENT_DLL
	m_shadow.maxSpeed = 1000;
	m_shadow.maxAngular = DEFAULT_MAX_ANGULAR;

	// Compute total mass...
	float flMass = PhysGetEntityMass(pEntity);
	float flMaxMass = player_pickup_maxmass.GetFloat();
	if (flMass <= flMaxMass)
		return;

	float flLerpFactor = clamp(flMass, flMaxMass, 500.0f);
	flLerpFactor = SimpleSplineRemapVal(flLerpFactor, flMaxMass, 500.0f, 0.0f, 1.0f);

	float invMass = pPhysics->GetInvMass();
	float invInertia = pPhysics->GetInvInertia().Length();

	float invMaxMass = 1.0f / MAX_MASS;
	float ratio = invMaxMass / invMass;
	invMass = invMaxMass;
	invInertia *= ratio;

	float maxSpeed = invMass * MASS_SPEED_SCALE * 200;
	float maxAngular = invInertia * MASS_SPEED_SCALE * 360;

	m_shadow.maxSpeed = Lerp(flLerpFactor, m_shadow.maxSpeed, maxSpeed);
	m_shadow.maxAngular = Lerp(flLerpFactor, m_shadow.maxAngular, maxAngular);
#endif
}

QAngle CGrabController::TransformAnglesToPlayerSpace(const QAngle &anglesIn, CBasePlayer *pPlayer)
{
	if (m_bIgnoreRelativePitch)
	{
		matrix3x4_t test;
		QAngle angleTest = pPlayer->EyeAngles();
		angleTest.x = 0;
		AngleMatrix(angleTest, test);
		return TransformAnglesToLocalSpace(anglesIn, test);
	}
	return TransformAnglesToLocalSpace(anglesIn, pPlayer->EntityToWorldTransform());
}

QAngle CGrabController::TransformAnglesFromPlayerSpace(const QAngle &anglesIn, CBasePlayer *pPlayer)
{
	if (m_bIgnoreRelativePitch)
	{
		matrix3x4_t test;
		QAngle angleTest = pPlayer->EyeAngles();
		angleTest.x = 0;
		AngleMatrix(angleTest, test);
		return TransformAnglesToWorldSpace(anglesIn, test);
	}
	return TransformAnglesToWorldSpace(anglesIn, pPlayer->EntityToWorldTransform());
}

void CGrabController::AttachEntity(CBasePlayer *pPlayer, CBaseEntity *pEntity, IPhysicsObject *pPhys, bool bIsMegaPhysCannon, const Vector &vGrabPosition, bool bUseGrabPosition)
{
	// play the impact sound of the object hitting the player
	// used as feedback to let the player know he picked up the object
#ifndef CLIENT_DLL
	PhysicsImpactSound(pPlayer, pPhys, CHAN_STATIC, pPhys->GetMaterialIndex(), pPlayer->VPhysicsGetObject() ? pPlayer->VPhysicsGetObject()->GetMaterialIndex() : pPhys->GetMaterialIndex(), 1.0, 64);
#endif
	Vector position;
	QAngle angles;
	pPhys->GetPosition(&position, &angles);
	// If it has a preferred orientation, use that instead.
#ifndef CLIENT_DLL
	Pickup_GetPreferredCarryAngles(pEntity, pPlayer, pPlayer->EntityToWorldTransform(), angles);
#endif

	//	ComputeMaxSpeed( pEntity, pPhys );

	// Carried entities can never block LOS
	m_bCarriedEntityBlocksLOS = pEntity->BlocksLOS();
	pEntity->SetBlocksLOS(false);
	m_controller = physenv->CreateMotionController(this);
	m_controller->AttachObject(pPhys, true);
	// Don't do this, it's causing trouble with constraint solvers.
	//m_controller->SetPriority( IPhysicsMotionController::HIGH_PRIORITY );

	pPhys->Wake();
	PhysSetGameFlags(pPhys, FVPHYSICS_PLAYER_HELD);
	SetTargetPosition(position, angles);
	m_attachedEntity = pEntity;
	IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
	int count = pEntity->VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
	m_flLoadWeight = 0;
	float damping = 10;
	float flFactor = count / 7.5f;
	if (flFactor < 1.0f)
	{
		flFactor = 1.0f;
	}
	for (int i = 0; i < count; i++)
	{
		float mass = pList[i]->GetMass();
		pList[i]->GetDamping(NULL, &m_savedRotDamping[i]);
		m_flLoadWeight += mass;
		m_savedMass[i] = mass;

		// reduce the mass to prevent the player from adding crazy amounts of energy to the system
		pList[i]->SetMass(REDUCED_CARRY_MASS / flFactor);
		pList[i]->SetDamping(NULL, &damping);
	}

	// Give extra mass to the phys object we're actually picking up
	pPhys->SetMass(REDUCED_CARRY_MASS);
	pPhys->EnableDrag(false);

	m_errorTime = -1.0f; // 1 seconds until error starts accumulating
	m_error = 0;
	m_contactAmount = 0;

	m_attachedAnglesPlayerSpace = TransformAnglesToPlayerSpace(angles, pPlayer);
	if (m_angleAlignment != 0)
	{
		m_attachedAnglesPlayerSpace = AlignAngles(m_attachedAnglesPlayerSpace, m_angleAlignment);
	}

	VectorITransform(pEntity->WorldSpaceCenter(), pEntity->EntityToWorldTransform(), m_attachedPositionObjectSpace);

#ifndef CLIENT_DLL
	// If it's a prop, see if it has desired carry angles
	CPhysicsProp *pProp = dynamic_cast<CPhysicsProp *>(pEntity);
	if (pProp)
	{
		m_bHasPreferredCarryAngles = pProp->GetPropDataAngles("preferred_carryangles", m_vecPreferredCarryAngles);
	}
	else
	{
		m_bHasPreferredCarryAngles = false;
	}
#else

	m_bHasPreferredCarryAngles = false;
#endif
}

static void ClampPhysicsVelocity(IPhysicsObject *pPhys, float linearLimit, float angularLimit)
{
	Vector vel;
	AngularImpulse angVel;
	pPhys->GetVelocity(&vel, &angVel);
	float speed = VectorNormalize(vel) - linearLimit;
	float angSpeed = VectorNormalize(angVel) - angularLimit;
	speed = speed < 0 ? 0 : -speed;
	angSpeed = angSpeed < 0 ? 0 : -angSpeed;
	vel *= speed;
	angVel *= angSpeed;
	pPhys->AddVelocity(&vel, &angVel);
}

void CGrabController::DetachEntity(bool bClearVelocity)
{
	CBaseEntity *pEntity = GetAttached();
	if (pEntity)
	{
		// Restore the LS blocking state
		pEntity->SetBlocksLOS(m_bCarriedEntityBlocksLOS);
		IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
		int count = pEntity->VPhysicsGetObjectList(pList, ARRAYSIZE(pList));

		for (int i = 0; i < count; i++)
		{
			IPhysicsObject *pPhys = pList[i];
			if (!pPhys)
				continue;

			// on the odd chance that it's gone to sleep while under anti-gravity
			pPhys->EnableDrag(true);
			pPhys->Wake();
			pPhys->SetMass(m_savedMass[i]);
			pPhys->SetDamping(NULL, &m_savedRotDamping[i]);
			PhysClearGameFlags(pPhys, FVPHYSICS_PLAYER_HELD);
			if (bClearVelocity)
			{
				PhysForceClearVelocity(pPhys);
			}
			else
			{
#ifndef CLIENT_DLL
				ClampPhysicsVelocity(pPhys, 150 * 1.5f, 2.0f * 360.0f);
#endif
			}

		}
	}

	m_attachedEntity = NULL;
	if (physenv)
	{
		physenv->DestroyMotionController(m_controller);
	}
	m_controller = NULL;
}

static bool InContactWithHeavyObject(IPhysicsObject *pObject, float heavyMass)
{
	bool contact = false;
	IPhysicsFrictionSnapshot *pSnapshot = pObject->CreateFrictionSnapshot();
	while (pSnapshot->IsValid())
	{
		IPhysicsObject *pOther = pSnapshot->GetObject(1);
		if (!pOther->IsMoveable() || pOther->GetMass() > heavyMass)
		{
			contact = true;
			break;
		}
		pSnapshot->NextFrictionData();
	}
	pObject->DestroyFrictionSnapshot(pSnapshot);
	return contact;
}

IMotionEvent::simresult_e CGrabController::Simulate(IPhysicsMotionController *pController, IPhysicsObject *pObject, float deltaTime, Vector &linear, AngularImpulse &angular)
{
	game_shadowcontrol_params_t shadowParams = m_shadow;
	if (InContactWithHeavyObject(pObject, GetLoadWeight()))
	{
		m_contactAmount = Approach(0.1f, m_contactAmount, deltaTime*2.0f);
	}
	else
	{
		m_contactAmount = Approach(1.0f, m_contactAmount, deltaTime*2.0f);
	}
	shadowParams.maxAngular = m_shadow.maxAngular * m_contactAmount * m_contactAmount * m_contactAmount;
#ifndef CLIENT_DLL
	m_timeToArrive = pObject->ComputeShadowControl(shadowParams, m_timeToArrive, deltaTime);
#else
	m_timeToArrive = pObject->ComputeShadowControl(shadowParams, (TICK_INTERVAL * 2), deltaTime);
#endif

	// Slide along the current contact points to fix bouncing problems
	Vector velocity;
	AngularImpulse angVel;
	pObject->GetVelocity(&velocity, &angVel);
	PhysComputeSlideDirection(pObject, velocity, angVel, &velocity, &angVel, GetLoadWeight());
	pObject->SetVelocityInstantaneous(&velocity, NULL);

	linear.Init();
	angular.Init();
	m_errorTime += deltaTime;

	return SIM_LOCAL_ACCELERATION;
}

float CGrabController::GetSavedMass(IPhysicsObject *pObject)
{
	CBaseEntity *pHeld = m_attachedEntity;
	if (pHeld)
	{
		if (pObject->GetGameData() == (void*)pHeld)
		{
			IPhysicsObject *pList[VPHYSICS_MAX_OBJECT_LIST_COUNT];
			int count = pHeld->VPhysicsGetObjectList(pList, ARRAYSIZE(pList));
			for (int i = 0; i < count; i++)
			{
				if (pList[i] == pObject)
					return m_savedMass[i];
			}
		}
	}
	return 0.0f;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CGrabController::UpdateObject(CBasePlayer *pPlayer, float flError)
{
	CBaseEntity *pEntity = GetAttached();
	if (!pEntity)
		return false;
	if (ComputeError() > flError)
		return false;
	if (pPlayer->GetGroundEntity() == pEntity)
		return false;
	if (!pEntity->VPhysicsGetObject())
		return false;

	//Adrian: Oops, our object became motion disabled, let go!
	IPhysicsObject *pPhys = pEntity->VPhysicsGetObject();
	if (pPhys && pPhys->IsMoveable() == false)
	{
		return false;
	}

	if (m_frameCount == gpGlobals->framecount)
	{
		return true;
	}
	m_frameCount = gpGlobals->framecount;
	Vector forward, right, up;
	QAngle playerAngles = pPlayer->EyeAngles();

	float pitch = AngleDistance(playerAngles.x, 0);
	playerAngles.x = clamp(pitch, -75, 75);
	AngleVectors(playerAngles, &forward, &right, &up);

	// Now clamp a sphere of object radius at end to the player's bbox
	Vector radial = physcollision->CollideGetExtent(pPhys->GetCollide(), vec3_origin, pEntity->GetAbsAngles(), -forward);
	Vector player2d = pPlayer->CollisionProp()->OBBMaxs();
	float playerRadius = player2d.Length2D();
	float flDot = DotProduct(forward, radial);

	float radius = playerRadius + fabs(flDot);

	float distance = 24 + (radius * 2.0f);

	Vector start = pPlayer->Weapon_ShootPosition();
	Vector end = start + (forward * distance);

	trace_t	tr;
	CTraceFilterSkipTwoEntities traceFilter(pPlayer, pEntity, COLLISION_GROUP_NONE);
	Ray_t ray;
	ray.Init(start, end);
	enginetrace->TraceRay(ray, MASK_SOLID_BRUSHONLY, &traceFilter, &tr);

	if (tr.fraction < 0.5)
	{
		end = start + forward * (radius*0.5f);
	}
	else if (tr.fraction <= 1.0f)
	{
		end = start + forward * (distance - radius);
	}

	Vector playerMins, playerMaxs, nearest;
	pPlayer->CollisionProp()->WorldSpaceAABB(&playerMins, &playerMaxs);
	Vector playerLine = pPlayer->CollisionProp()->WorldSpaceCenter();
	CalcClosestPointOnLine(end, playerLine + Vector(0, 0, playerMins.z), playerLine + Vector(0, 0, playerMaxs.z), nearest, NULL);

	Vector delta = end - nearest;
	float len = VectorNormalize(delta);
	if (len < radius)
	{
		end = nearest + radius * delta;
	}

	QAngle angles = TransformAnglesFromPlayerSpace(m_attachedAnglesPlayerSpace, pPlayer);

#ifndef CLIENT_DLL
	// If it has a preferred orientation, update to ensure we're still oriented correctly.
	Pickup_GetPreferredCarryAngles(pEntity, pPlayer, pPlayer->EntityToWorldTransform(), angles);

	// We may be holding a prop that has preferred carry angles
	if (m_bHasPreferredCarryAngles)
	{
		matrix3x4_t tmp;
		ComputePlayerMatrix(pPlayer, tmp);
		angles = TransformAnglesToWorldSpace(m_vecPreferredCarryAngles, tmp);
	}
#endif

	matrix3x4_t attachedToWorld;
	Vector offset;
	AngleMatrix(angles, attachedToWorld);
	VectorRotate(m_attachedPositionObjectSpace, attachedToWorld, offset);

	SetTargetPosition(end - offset, angles);

	return true;
}

//-----------------------------------------------------------------------------
// Player pickup controller
//-----------------------------------------------------------------------------

class CPlayerPickupController : public CBaseEntity
{
	DECLARE_CLASS(CPlayerPickupController, CBaseEntity);
public:
	void Init(CBasePlayer *pPlayer, CBaseEntity *pObject);
	void Shutdown(bool bThrown = false);
	bool OnControls(CBaseEntity *pControls) { return true; }
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void VPhysicsUpdate(IPhysicsObject *pPhysics){}
	void VPhysicsShadowUpdate(IPhysicsObject *pPhysics) {}

	bool IsHoldingEntity(CBaseEntity *pEnt);
	CGrabController &GetGrabController() { return m_grabController; }

private:
	CGrabController		m_grabController;
	CBasePlayer			*m_pPlayer;
};

LINK_ENTITY_TO_CLASS(player_pickup, CPlayerPickupController);

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pPlayer - 
//			*pObject - 
//-----------------------------------------------------------------------------
void CPlayerPickupController::Init(CBasePlayer *pPlayer, CBaseEntity *pObject)
{
#ifndef CLIENT_DLL
	// Holster player's weapon
	if (pPlayer->GetActiveWeapon())
	{
		if (!pPlayer->GetActiveWeapon()->Holster())
		{
			Shutdown();
			return;
		}
	}

	// If the target is debris, convert it to non-debris
	if (pObject->GetCollisionGroup() == COLLISION_GROUP_DEBRIS)
	{
		// Interactive debris converts back to debris when it comes to rest
		pObject->SetCollisionGroup(COLLISION_GROUP_INTERACTIVE_DEBRIS);
	}

	// done so I'll go across level transitions with the player
	SetParent(pPlayer);
	m_grabController.SetIgnorePitch(true);
	m_grabController.SetAngleAlignment(DOT_30DEGREE);
	m_pPlayer = pPlayer;
	IPhysicsObject *pPhysics = pObject->VPhysicsGetObject();
	Pickup_OnPhysGunPickup(pObject, m_pPlayer);

	m_grabController.AttachEntity(pPlayer, pObject, pPhysics, false, vec3_origin, false);

	m_pPlayer->SetUseEntity(this);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void CPlayerPickupController::Shutdown(bool bThrown)
{
#ifndef CLIENT_DLL
	CBaseEntity *pObject = m_grabController.GetAttached();

	bool bClearVelocity = false;
	if (!bThrown && pObject && pObject->VPhysicsGetObject() && pObject->VPhysicsGetObject()->GetContactPoint(NULL, NULL))
	{
		bClearVelocity = true;
	}

	m_grabController.DetachEntity(bClearVelocity);

	if (pObject != NULL)
	{
		Pickup_OnPhysGunDrop(pObject, m_pPlayer, bThrown ? THROWN_BY_PLAYER : DROPPED_BY_PLAYER);
	}

	if (m_pPlayer)
	{
		m_pPlayer->SetUseEntity(NULL);

		if (m_pPlayer->GetActiveWeapon())
		{
			m_pPlayer->GetActiveWeapon()->Deploy();
		}
	}
	Remove();

#endif
}

void CPlayerPickupController::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (ToBasePlayer(pActivator) == m_pPlayer)
	{
		CBaseEntity *pAttached = m_grabController.GetAttached();

		// UNDONE: Use vphysics stress to decide to drop objects
		// UNDONE: Must fix case of forcing objects into the ground you're standing on (causes stress) before that will work
		if (!pAttached || useType == USE_OFF || (m_pPlayer->m_nButtons & IN_SPECIAL1) || m_grabController.ComputeError() > 12)
		{
			Shutdown();
			return;
		}

		//Adrian: Oops, our object became motion disabled, let go!
		IPhysicsObject *pPhys = pAttached->VPhysicsGetObject();
		if (pPhys && pPhys->IsMoveable() == false)
		{
			Shutdown();
			return;
		}

#if STRESS_TEST
		vphysics_objectstress_t stress;
		CalculateObjectStress(pPhys, pAttached, &stress);
		if (stress.exertedStress > 250)
		{
			Shutdown();
			return;
		}
#endif
		// +ATTACK will throw phys objects
		if (m_pPlayer->m_nButtons & IN_ATTACK)
		{
			Shutdown(true);
			Vector vecLaunch;
			m_pPlayer->EyeVectors(&vecLaunch);
			// JAY: Scale this with mass because some small objects really go flying
			if (!pPhys)
				return;

			float flNewMass = pPhys->GetMass();
			if (flNewMass < 0.5 || flNewMass > 15)
				flNewMass = 10;

			float massFactor = clamp(flNewMass, 0.5, 15);
			massFactor = RemapVal(massFactor, 0.5, 15, 0.5, 4);
			vecLaunch *= player_throwforce.GetFloat() * massFactor;

			pPhys->ApplyForceCenter(vecLaunch);
			AngularImpulse aVel = RandomAngularImpulse(-10, 10) * massFactor;
			pPhys->ApplyTorqueCenter(aVel);
			return;
		}

		if (useType == USE_SET)
		{
			// update position
			m_grabController.UpdateObject(m_pPlayer, 12);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pEnt - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPlayerPickupController::IsHoldingEntity(CBaseEntity *pEnt)
{
	return (m_grabController.GetAttached() == pEnt);
}

bool PlayerPickupControllerIsHoldingEntity(CBaseEntity *pPickupControllerEntity, CBaseEntity *pHeldEntity)
{
	CPlayerPickupController *pController = dynamic_cast<CPlayerPickupController *>(pPickupControllerEntity);
	return pController ? pController->IsHoldingEntity(pHeldEntity) : false;
}

float PlayerPickupGetHeldObjectMass(CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject)
{
	float mass = 0.0f;
	CPlayerPickupController *pController = dynamic_cast<CPlayerPickupController *>(pPickupControllerEntity);
	if (pController)
	{
		CGrabController &grab = pController->GetGrabController();
		mass = grab.GetSavedMass(pHeldObject);
	}
	return mass;
}

void PlayerPickupObject(CBasePlayer *pPlayer, CBaseEntity *pObject)
{
#ifndef CLIENT_DLL
	//Don't pick up if we don't have a phys object.
	if (pObject->VPhysicsGetObject() == NULL)
		return;

	CPlayerPickupController *pController = (CPlayerPickupController *)CBaseEntity::Create("player_pickup", pObject->GetAbsOrigin(), vec3_angle, pPlayer);
	if (!pController)
		return;

	pController->Init(pPlayer, pObject);
#endif
}