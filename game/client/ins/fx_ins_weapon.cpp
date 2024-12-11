//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
//=============================================================================//

#include "cbase.h"
#include "fx.h"
#include "fx_discreetline.h"
#include "dlight.h"
#include "iefx.h"
#include "clientsideeffects.h"
#include "clienteffectprecachesystem.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"
#include "particles_simple.h"
#include "particles_localspace.h"
#include "engine/ienginesound.h"
#include "view.h"
#include "weapon_defines.h"
#include "collisionutils.h"
#include "soundemittersystem/isoundemittersystembase.h"
#include "ins_player_shared.h"
#include "ins_fx.h"
#include "ins_shared.h"
#include "pain_helper.h"
#include "tier0/vprof.h"

#include "tier0/memdbgon.h"

//=========================================================
//=========================================================

CLIENTEFFECT_REGISTER_BEGIN(PrecacheINSWeapponFX)
CLIENTEFFECT_MATERIAL("particles/dust1")
CLIENTEFFECT_MATERIAL("particles/dust2")
CLIENTEFFECT_MATERIAL("effects/muzzleflash1")
CLIENTEFFECT_MATERIAL("effects/muzzleflash2")
CLIENTEFFECT_MATERIAL("effects/muzzleflash3")
CLIENTEFFECT_MATERIAL("effects/muzzleflash4")
CLIENTEFFECT_MATERIAL("effects/muzzleflash1_noz")
CLIENTEFFECT_MATERIAL("effects/muzzleflash2_noz")
CLIENTEFFECT_MATERIAL("effects/muzzleflash3_noz")
CLIENTEFFECT_MATERIAL("effects/muzzleflash4_noz")
CLIENTEFFECT_MATERIAL("effects/spark")
CLIENTEFFECT_MATERIAL("effects/redflare")
CLIENTEFFECT_MATERIAL("effects/greenflare")
CLIENTEFFECT_MATERIAL("effects/gunshiptracer")
CLIENTEFFECT_REGISTER_END()

//=========================================================
//=========================================================
C_BaseCombatWeapon* FX_INS_GetWeapon(ClientEntityHandle_t hViewModelEntity)
{
	C_BaseViewModel* pViewModel = dynamic_cast<C_BaseViewModel*>(cl_entitylist->GetBaseEntityFromHandle(hViewModelEntity));
	return (pViewModel ? pViewModel->GetOwningWeapon() : NULL);
}

//=========================================================
//=========================================================
C_BaseCombatWeapon* FX_INS_GetSource(ClientEntityHandle_t hEntity, Vector& vecSource, QAngle& angSource, int iAttachIndex, bool bFirstPerson)
{
	C_BaseCombatWeapon* pWeapon = FX_INS_GetWeapon(hEntity);

	if (!pWeapon)
		return NULL;

	if (bFirstPerson)
	{
		C_INSPlayer* pPlayer = C_INSPlayer::GetLocalPlayer();

		if (!pPlayer)
			return NULL;

		pPlayer->GetMuzzle(vecSource, angSource);
	}
	else
	{
		if (!pWeapon->GetAttachment(iAttachIndex, vecSource, angSource))
			return NULL;
	}

	return pWeapon;
}

//=========================================================
//=========================================================
void FX_INS_MuzzleFlash(C_BaseCombatWeapon* pWeapon, const Vector& vecOrigin, const QAngle& vecAngles, bool bFirstPerson)
{
	if (!pWeapon)
		return;

	C_BasePlayer* pPlayer = pWeapon->GetOwner();

	if (pPlayer && pPlayer->IsZoomed())
		return;

	int iEntIndex = pWeapon->entindex();
	int iFlags = (bFirstPerson) ? FX_FLAGS_FIRSTPERSON : 0;

	switch (pWeapon->GetWeaponClass())
	{
	case WEAPONCLASS_PISTOL:
		FX_INS_Default_MuzzleEffect(vecOrigin, vecAngles, iEntIndex, 0.5f, iFlags);
		break;

	case WEAPONCLASS_RIFLE:
	case WEAPONCLASS_SHOTGUN:
	case WEAPONCLASS_SNIPER:
		FX_INS_Default_MuzzleEffect(vecOrigin, vecAngles, iEntIndex, 1.0f, iFlags);
		break;

	case WEAPONCLASS_RPG:
		FX_INS_Default_MuzzleEffect(vecOrigin, vecAngles, iEntIndex, 2.00f, iFlags | FX_FLAGS_NOFLASH);
		break;

	case WEAPONCLASS_LMG:
		FX_INS_Default_MuzzleEffect(vecOrigin, vecAngles, iEntIndex, 0.75f, iFlags);
		break;

	case WEAPONCLASS_PLASMA:
		FX_INS_Combine_MuzzleEffect(vecOrigin, vecAngles, iEntIndex);
		break;

	default:
		DevWarning("Invalid Weapon Class!\n");
		break;
	}
}

//=========================================================
//=========================================================
#define MAX_LUM_FLASH 0.05f
#define MUZZLE_VELOCITY 15

void FX_INS_Default_MuzzleEffect(const Vector& pos, const QAngle& angles, int entindex, float scale, int flags)
{
	VPROF_BUDGET("FX_INS_Default_MuzzleEffect", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	Vector vecForward;
	AngleVectors(angles, &vecForward);

	bool bFirstPerson = flags & FX_FLAGS_FIRSTPERSON;

	// init mats
	PMaterialHandle DustParticle = ParticleMgr()->GetPMaterial("particle/particle_smokegrenade");

	PMaterialHandle MuzzleParticles[4];

	for (int i = 0; i < 4; i++)
		MuzzleParticles[i] = ParticleMgr()->GetPMaterial(VarArgs("effects/muzzleflash%d%s", i + 1, bFirstPerson ? "_noz" : ""));

	// smoke
	CSmartPtr< CSimpleEmitter > Smoke = CSimpleEmitter::Create("Smoke");

	Smoke->SetSortOrigin(pos);
	Smoke->SetDrawBeforeViewModel(true);

	int iSmokeCount = random->RandomInt(MUZZLE_VELOCITY * 0.8f, MUZZLE_VELOCITY);

	if (!bFirstPerson)
		iSmokeCount *= 0.5f;

	for (int i = 0; i < iSmokeCount; i++)
	{
		SimpleParticle* pPart = (SimpleParticle*)Smoke->AddParticle(sizeof(SimpleParticle), DustParticle, pos);

		if (!pPart)
			continue;

		float flScaleMuzzle = random->RandomFloat(1.0f, MUZZLE_VELOCITY * scale);

		pPart->m_Pos += (vecForward * (flScaleMuzzle * 0.75f));

		pPart->m_uchColor[0] = 64;
		pPart->m_uchColor[1] = 64;
		pPart->m_uchColor[2] = 68;

		pPart->m_uchStartAlpha = 52;
		pPart->m_uchEndAlpha = 16;

		pPart->m_flLifetime = 0.0f;
		pPart->m_flDieTime = random->RandomFloat(0.1f, 0.4f);

		pPart->m_vecVelocity.Random(-3.0f, 3.0f);
		VectorMA(pPart->m_vecVelocity, random->RandomFloat(24.0f, 32.0f), vecForward, pPart->m_vecVelocity);

		pPart->m_uchStartSize = flScaleMuzzle * (scale * 0.5f);
		pPart->m_uchEndSize = flScaleMuzzle * (scale * 1.5f);

		pPart->m_flRoll = random->RandomInt(0, 360);
		pPart->m_flRollDelta = random->RandomFloat(-1.0f, 2.0f);
	}

	// don't do flash or light when too bright
	float flLuminosity;
	UTIL_GetNormalizedColorTintAndLuminosity(WorldGetLightForPoint(pos, true), NULL, &flLuminosity);

	if (flLuminosity > MAX_LUM_FLASH)
	{
		flags |= FX_FLAGS_NOLIGHT;

		if (RandomInt(0, RoundFloatToInt(50 * flLuminosity)))
			flags |= FX_FLAGS_NOFLASH;
	}

	// make lights
	if ((flags & FX_FLAGS_NOLIGHT) == 0)
	{
		dlight_t* pDLight = effects->CL_AllocDlight(LIGHT_INDEX_MUZZLEFLASH + entindex);
		pDLight->origin = pos;
		pDLight->radius = random->RandomInt(45, 80);
		pDLight->decay = pDLight->radius / 0.05f;
		pDLight->die = gpGlobals->curtime + 0.05f;
		pDLight->color.r = 255;
		pDLight->color.g = 192;
		pDLight->color.b = 64;
		pDLight->color.exponent = 5;

		dlight_t* pELight = effects->CL_AllocElight(LIGHT_INDEX_MUZZLEFLASH + entindex);
		pELight->origin = pos;
		pELight->radius = random->RandomInt(32, 64);
		pELight->decay = pELight->radius / 0.05f;
		pELight->die = gpGlobals->curtime + 0.05f;
		pELight->color.r = 255;
		pELight->color.g = 192;
		pELight->color.b = 64;
		pELight->color.exponent = 5;
	}

	// flash 
	if ((flags & FX_FLAGS_NOFLASH) == 0)
	{
		CSmartPtr< CLocalSpaceEmitter > Flash = CLocalSpaceEmitter::Create("Flash", entindex, 1);

		Flash->SetSortOrigin(pos);
		Flash->SetDrawBeforeViewModel(true);

		for (int i = 1; i < 6; i++)
		{
			Vector vecOffset = pos + (vecForward * (i * 2.0f * scale));

			SimpleParticle* pPart = (SimpleParticle*)Flash->AddParticle(sizeof(SimpleParticle), MuzzleParticles[random->RandomInt(0, 3)], vecOffset);

			if (!pPart)
				continue;

			pPart->m_flLifetime = 0.0f;
			pPart->m_flDieTime = random->RandomFloat(0.035f, 0.085f);

			pPart->m_vecVelocity.Init();

			pPart->m_uchColor[0] = pPart->m_uchColor[1] = pPart->m_uchColor[2] = 255;

			pPart->m_uchStartAlpha = 64;
			pPart->m_uchEndAlpha = 128;

			pPart->m_uchStartSize = ((random->RandomFloat(6.0f, 8.0f) * (8 - i) / 8) * scale) * (((flags & FX_FLAGS_FIRSTPERSON) != 0) ? 1.0f : 0.75f);
			pPart->m_uchEndSize = pPart->m_uchStartSize;

			pPart->m_flRoll = random->RandomInt(0, 360);
			pPart->m_flRollDelta = 0.0f;
		}
	}
}

//=========================================================
//=========================================================
void FX_INS_Combine_MuzzleEffect(const Vector& vecOrigin, const QAngle& vecAngles, int iEntIndex)
{
	VPROF_BUDGET("FX_INS_Combine_MuzzleEffect", VPROF_BUDGETGROUP_PARTICLE_RENDERING);
	CSmartPtr<CLocalSpaceEmitter> pSimple = CLocalSpaceEmitter::Create("MuzzleFlash", iEntIndex, 1, FLE_VIEWMODEL);

	SimpleParticle* pParticle;
	Vector			offset; //NOTENOTE: All coords are in local space

	float flScale = random->RandomFloat(2.0f, 2.25f);

	Vector vecForward;
	AngleVectors(vecAngles, &vecForward);

	pSimple->SetDrawBeforeViewModel(true);

	// Flash
	for (int i = 1; i < 6; i++)
	{
		offset = vecOrigin + (vecForward * (i * 8.0f * flScale));

		pParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial(VarArgs("effects/combinemuzzle%d_noz", random->RandomInt(1, 2))), offset);

		if (pParticle == NULL)
			return;

		pParticle->m_flLifetime = 0.0f;
		pParticle->m_flDieTime = 0.025f;

		pParticle->m_vecVelocity.Init();

		pParticle->m_uchColor[0] = 255;
		pParticle->m_uchColor[1] = 255;
		pParticle->m_uchColor[2] = 200 + random->RandomInt(0, 55);

		pParticle->m_uchStartAlpha = 255;
		pParticle->m_uchEndAlpha = 255;

		pParticle->m_uchStartSize = ((random->RandomFloat(6.0f, 8.0f) * (12 - (i)) / 12) * flScale);
		pParticle->m_uchEndSize = pParticle->m_uchStartSize;
		pParticle->m_flRoll = random->RandomInt(0, 360);
		pParticle->m_flRollDelta = 0.0f;
	}

	// Tack on the smoke
	pParticle = (SimpleParticle*)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial(VarArgs("effects/combinemuzzle%d_noz", random->RandomInt(1, 2))), vec3_origin);

	if (pParticle == NULL)
		return;

	pParticle->m_flLifetime = 0.0f;
	pParticle->m_flDieTime = 0.025f;

	pParticle->m_vecVelocity.Init();

	pParticle->m_uchColor[0] = 255;
	pParticle->m_uchColor[1] = 255;
	pParticle->m_uchColor[2] = 255;

	pParticle->m_uchStartAlpha = random->RandomInt(64, 128);
	pParticle->m_uchEndAlpha = 32;

	pParticle->m_uchStartSize = random->RandomFloat(10.0f, 16.0f);
	pParticle->m_uchEndSize = pParticle->m_uchStartSize;

	pParticle->m_flRoll = random->RandomInt(0, 360);
	pParticle->m_flRollDelta = 0.0f;
}

//=========================================================
//=========================================================
void FX_INS_Default_Tracer(const Vector& start, const Vector& end, int type, float velocity)
{
	VPROF_BUDGET("FX_INS_Default_Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	const char* pMaterials[MAX_TRACERTYPES] =
	{
		"",							// TRACERTYPE_NONE
		"effects/spark",			// TRACERTYPE_DEFAULT
		"effects/gunshiptracer",	// TRACERTYPE_WATER
		"effects/redflare",			// TRACERTYPE_RED
		"effects/greenflare",		// TRACERTYPE_GREEN
		"effects/gunshiptracer"		// TRACERTYPE_PLASMA
	};

	if (type < 0 || type >= MAX_TRACERTYPES)
	{
		AssertMsg(false, "Invalid Tracer Type Passed");
		type = 0;
	}

	// make sound
	FX_INS_Default_TracerSound(start, end, type);

	// only show on some types
	if (type != TRACERTYPE_NONE)
	{
		Vector dir;
		VectorSubtract(end, start, dir);

		float dist = VectorNormalize(dir);

		if (dist < 128 && type != TRACERTYPE_WATER)
			return;

		if (velocity == 0.0f)
			velocity = 5000.0f;

		float length = random->RandomFloat(128.0f, 256.0f);
		float life = (dist + length) / velocity;

		FX_AddDiscreetLine(start, dir, velocity, length, dist, random->RandomFloat(0.5f, 1.5f), life, pMaterials[type]);
	}
}

//=========================================================
//=========================================================
#define	TRACER_MAX_HEAR_DIST	(6*12)
#define TRACER_SOUND_TIME_MIN	0.1f
#define TRACER_SOUND_TIME_MAX	0.1f

class CBulletWhizTimer : public CAutoGameSystem
{
public:
	CBulletWhizTimer(char const* name)
		: CAutoGameSystem(name)
	{
	}

	void LevelInitPreEntity(void)
	{
		m_flNextWhizTime = 0.0f;
	}

public:
	float m_flNextWhizTime;
};

CBulletWhizTimer g_BulletWhiz("CBulletWhizTimer");

#define LISTENER_HEIGHT 24
#define BULLET_SUPERSONIC 288 // about 24ft

void FX_INS_Default_TracerSound(const Vector& start, const Vector& end, int type)
{
	const char* pszSoundName = NULL;
	float flWhizDist, flMinWhizTime, flMaxWhizTime;
	Vector vecListenOrigin = MainViewOrigin();

	if (type == TRACERTYPE_WATER)
	{
		pszSoundName = "Underwater.BulletImpact";
		flWhizDist = 48.0f;
		flMinWhizTime = 0.3f;
		flMaxWhizTime = 0.6f;
	}
	else
	{
		pszSoundName = "Bullets.SonicNearmiss";
		flWhizDist = 24.0f;
		flMinWhizTime = 0.1f;
		flMaxWhizTime = 0.1f;

		Vector vecDiff = start - end;

		if (vecDiff.Length() >= BULLET_SUPERSONIC)
			pszSoundName = "Bullets.DefaultNearmiss";

		Ray_t bullet, listener;
		bullet.Init(start, end);

		Vector vecLower = vecListenOrigin;
		vecLower.z -= LISTENER_HEIGHT;
		listener.Init(vecListenOrigin, vecLower);

		float s, t;
		IntersectRayWithRay(bullet, listener, s, t);
		t = clamp(t, 0, 1);
		vecListenOrigin.z -= t * LISTENER_HEIGHT;
	}

	if (!pszSoundName)
		return;

	// is it time yet?
	float dt = g_BulletWhiz.m_flNextWhizTime - gpGlobals->curtime;

	if (dt > 0)
		return;

	// did the thing pass close enough to our head?
	float vDist = CalcDistanceSqrToLineSegment(vecListenOrigin, start, end);

	if (vDist >= (flWhizDist * flWhizDist))
		return;

	// play the sound
	CSoundParameters params;

	if (C_BaseEntity::GetParametersForSound(pszSoundName, params, NULL))
	{
		// get shot direction
		Vector shotDir;
		VectorSubtract(end, start, shotDir);
		VectorNormalize(shotDir);

		CLocalPlayerFilter filter;
		enginesound->EmitSound(filter, SOUND_FROM_WORLD, CHAN_STATIC, params.soundname,
			params.volume, SNDLVL_TO_ATTN(params.soundlevel), 0, params.pitch, 0, &start, &shotDir, NULL, false);
	}

	// make an effect
	g_PainHelper.CreatePain(PAINTYPE_BWHIZ);

	// don't play another bullet whiz for this client until this time has run out
	g_BulletWhiz.m_flNextWhizTime = gpGlobals->curtime + random->RandomFloat(flMinWhizTime, flMaxWhizTime);
}

//=========================================================
//=========================================================
Vector UTIL_GetTracerOrigin(const CEffectData& data)
{
	Vector vecPos = data.m_vStart;
	QAngle angAng = vec3_angle;

	if (data.m_nHitBox == 0)
		return vecPos;

	C_BaseEntity* pEnt = ClientEntityList().GetBaseEntityFromHandle(data.m_hEntity);

	if (!pEnt || pEnt->IsDormant())
		return vecPos;

	C_INSPlayer* pPlayer = C_INSPlayer::GetLocalPlayer();
	C_BaseCombatWeapon* pWeapon = dynamic_cast<C_BaseCombatWeapon*>(pEnt);

	if (pWeapon && pPlayer && pWeapon->IsCarriedByLocalPlayer() && pPlayer->GetViewModel() != NULL)
		pPlayer->GetMuzzle(vecPos, angAng);
	else
		pEnt->GetAttachment(1, vecPos, angAng);

	return vecPos;
}

//=========================================================
//=========================================================
void TracerCallback(const CEffectData& data)
{
	Vector start = UTIL_GetTracerOrigin(data);
	Vector end = data.m_vOrigin;

	FX_INS_Default_Tracer(start, end, data.m_nDamageType, 0.0f);
}

DECLARE_CLIENT_EFFECT("Tracer", TracerCallback);

//=========================================================
//=========================================================
void TracerSoundCallback(const CEffectData& data)
{
	Vector start = UTIL_GetTracerOrigin(data);
	Vector end = data.m_vOrigin;

	FX_INS_Default_TracerSound(start, end, data.m_fFlags);
}

DECLARE_CLIENT_EFFECT("TracerSound", TracerSoundCallback);

//=========================================================
//=========================================================
void INS_EjectBrass(const CEffectData& data, int iType)
{
	C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return;

	tempents->CSEjectBrass(data.m_vOrigin, data.m_vAngles, data.m_fFlags, INS_SHELL_RIFLE, pPlayer);
}

//=========================================================
//=========================================================
void INS_FX_EjectBrass_9mmNato_Callback(const CEffectData& data)
{
	INS_EjectBrass(data, INS_SHELL_PISTOL);
}

//=========================================================
//=========================================================
void INS_FX_EjectBrass_12Gauge_Callback(const CEffectData& data)
{
	INS_EjectBrass(data, INS_SHELL_SHOTGUN);
}

//=========================================================
//=========================================================
void INS_FX_EjectBrass_762Nato_Callback(const CEffectData& data)
{
	INS_EjectBrass(data, INS_SHELL_RIFLE);
}

//=========================================================
//=========================================================
void INS_FX_EjectBrass_556Nato_Callback(const CEffectData& data)
{
	INS_EjectBrass(data, INS_SHELL_RIFLE);
}

//=========================================================
//=========================================================

DECLARE_CLIENT_EFFECT("EjectBrass_9mmNato", INS_FX_EjectBrass_9mmNato_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_12Gauge", INS_FX_EjectBrass_12Gauge_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_762Nato", INS_FX_EjectBrass_762Nato_Callback);
DECLARE_CLIENT_EFFECT("EjectBrass_556Nato", INS_FX_EjectBrass_556Nato_Callback);