//=========       Copyright © Reperio Studios 2015 @ Bernt Andreas Eide!       ============//
//
// Purpose: Shared Data Handler Class : Keeps information about the player and npc sound sets.
//
//========================================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "KeyValues.h"
#include "GameDefinitions_Shared.h"
#include "GameBase_Shared.h"
#include "gibs_shared.h"
#include "decals.h"
#include "icommandline.h"

#ifdef CLIENT_DLL
#include "hud.h"
#include "hudelement.h"
#include "vgui/ISurface.h"
#include "c_world.h"

// Misc
#define CLIENT_MODEL_START_INDEX 8192 // Make sure we don't collide with anything edict related! Must be > 4096!
struct ClientModelItem
{
	ClientModelItem(int id, const model_t *mdl)
	{
		this->id = id;
		this->model = mdl;
	}

	~ClientModelItem()
	{
		model = NULL;
	}

	int id;
	const model_t *model;
};
static CUtlVector<ClientModelItem> m_pClientModelList;

static const model_t *LoadClientModel(const char *path)
{
	int newID = (CLIENT_MODEL_START_INDEX + m_pClientModelList.Count());

	const model_t *model = engine->LoadModel(path);
	m_pClientModelList.AddToTail(ClientModelItem(newID, model));
	return model;
}

int LookupClientModelIndex(const model_t *model)
{
	int count = m_pClientModelList.Count();
	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			ClientModelItem *item = &m_pClientModelList[i];
			if (item->model == model)
				return item->id;
		}
	}

	return -1;
}

const model_t *LookupClientModelPointer(int index)
{
	int count = m_pClientModelList.Count();
	if (count && (index >= CLIENT_MODEL_START_INDEX))
	{
		for (int i = 0; i < count; i++)
		{
			ClientModelItem *item = &m_pClientModelList[i];
			if (item->id == index)
				return item->model;
		}
	}

	return modelinfo->GetModel(index); // Revert to default!
}
#else
#include "GameBase_Server.h"
#endif

CGameDefinitionsShared::CGameDefinitionsShared()
{
	LoadData();
}

CGameDefinitionsShared::~CGameDefinitionsShared()
{
	Cleanup();
}

void CGameDefinitionsShared::Cleanup(void)
{
#ifdef CLIENT_DLL
	m_pClientModelList.Purge();
#endif
}

bool CGameDefinitionsShared::LoadData(void)
{
	Cleanup();
	return true;
}

// Precache Stuff
bool CGameDefinitionsShared::Precache(void)
{
#ifdef CLIENT_DLL
	if (!engine->IsInGame())
		return false;
#endif

	// Legacy Particles
	PrecacheParticleSystem("blood_impact_red_01");
	PrecacheParticleSystem("blood_impact_green_01");
	PrecacheParticleSystem("blood_impact_yellow_01");

	// Gore
	PrecacheParticleSystem("blood_impact_red_01_extreme");
	PrecacheParticleSystem("blood_impact_headshot_01");
	PrecacheParticleSystem("blood_impact_headshot_01_extreme");
	PrecacheParticleSystem("blood_bleedout");
	PrecacheParticleSystem("blood_mist");

	// Other
	PrecacheParticleSystem("headshot");
	PrecacheParticleSystem("Rocket");
	PrecacheParticleSystem("water_splash_01"); // TFO Rain Effects
	PrecacheParticleSystem("bb2_item_spawn");

	// Helms
	PrecacheParticleSystem("helm_aurora_parent_green");
	PrecacheParticleSystem("helm_aurora_parent_orange");
	PrecacheParticleSystem("helm_aurora_parent_purple");
	PrecacheParticleSystem("helm_halo01");

	// Guns
	PrecacheParticleSystem("generic_tracer");
	PrecacheParticleSystem("generic_tracer_nosmoke");
	PrecacheParticleSystem("weapon_smoke_trail_01");
	PrecacheParticleSystem("muzzleflash_pistol");
	PrecacheParticleSystem("muzzleflash_pistol_tp");

	// Impacts
	PrecacheParticleSystem("impact_concrete");
	PrecacheParticleSystem("impact_dirt");
	PrecacheParticleSystem("impact_metal");
	PrecacheParticleSystem("impact_computer");
	PrecacheParticleSystem("impact_wood");
	PrecacheParticleSystem("impact_glass");

	PrecacheParticleSystem("water_splash_01");
	PrecacheParticleSystem("water_splash_02");
	PrecacheParticleSystem("water_splash_03");

	PrecacheParticleSystem("weapon_muzzle_smoke_b");

	PrecacheParticleSystem("flame_tiny");
	PrecacheParticleSystem("weapon_flame_fire_1");

	// Explosions
	PrecacheParticleSystem("explosion_grenade");

	return true;
}

#ifdef CLIENT_DLL
void CGameDefinitionsShared::LoadClientModels(void)
{
	m_pClientModelList.Purge();
}
#endif

const char *CGameDefinitionsShared::GetBloodParticle(bool bExtremeGore)
{
	return (bExtremeGore ? "blood_impact_red_01_extreme" : "blood_impact_red_01");
}

const char *CGameDefinitionsShared::GetHeadshotParticle(bool bExtremeGore)
{
	return (bExtremeGore ? "blood_impact_headshot_01_extreme" : "blood_impact_headshot_01");
}

const char *CGameDefinitionsShared::GetBleedoutParticle(bool bExtremeGore)
{
	return "blood_bleedout";
}

const char *CGameDefinitionsShared::GetBloodExplosionMist(bool bExtremeGore)
{
	return (bExtremeGore ? "blood_mist" : "blood_impact_red_01_extreme");
}

const char *CGameDefinitionsShared::GetGibParticleForLimb(const char *limb, bool bExtremeGore)
{
	return (bExtremeGore ? "blood_impact_red_01_extreme" : "blood_impact_red_01");
}

namespace ACHIEVEMENTS
{
	static const achievementStatItem_t GAME_STAT_AND_ACHIEVEMENT_DATA[] =
	{
		{ "ACH_BOOTCAMP", "", 0, ACHIEVEMENT_TYPE_MAP, false, NULL },

		// Challenges
		{ "ACH_GRENADE_CARNAGE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, false, NULL },

		// Rifles
		{ "ACH_WEP_AK47", "INS_KI_AK47", 300, ACHIEVEMENT_TYPE_DEFAULT, false, NULL },

		// Maps:
		{ "ACH_MAP_BAGHDAD", "", 0, ACHIEVEMENT_TYPE_DEFAULT, false, "ins_baghdad" },

		// Stats Only + hidden (no achievs) - used for leaderboards...
		{ "", "INS_ST_MORALE", 9999999, ACHIEVEMENT_TYPE_DEFAULT, true, NULL },
		{ "", "INS_ST_KILLS", 9999999, ACHIEVEMENT_TYPE_DEFAULT, true, NULL },
		{ "", "INS_ST_DEATHS", 9999999, ACHIEVEMENT_TYPE_DEFAULT, true, NULL },
	};

	const achievementStatItem_t *GetAchievementItem(int index)
	{
		if ((index < 0) || (index >= _ARRAYSIZE(GAME_STAT_AND_ACHIEVEMENT_DATA)))
			return NULL;
		return &GAME_STAT_AND_ACHIEVEMENT_DATA[index];
	}

	const achievementStatItem_t *GetAchievementItem(const char *str)
	{
		if (str && str[0])
		{
			for (int i = 0; i < _ARRAYSIZE(GAME_STAT_AND_ACHIEVEMENT_DATA); i++)
			{
				if (!strcmp(GAME_STAT_AND_ACHIEVEMENT_DATA[i].szAchievement, str))
					return &GAME_STAT_AND_ACHIEVEMENT_DATA[i];
			}
		}
		return NULL;
	}

	int GetNumAchievements(void) { return _ARRAYSIZE(GAME_STAT_AND_ACHIEVEMENT_DATA); }
}

#define PENETRATION_DATA_SIZE 12
DataPenetrationItem_t PENETRATION_DATA_LIST[PENETRATION_DATA_SIZE] =
{
	{ CHAR_TEX_WOOD, 9.0f },
	{ CHAR_TEX_GRATE, 6.0f },
	{ CHAR_TEX_CONCRETE, 4.0f },
	{ CHAR_TEX_TILE, 5.0f },
	{ CHAR_TEX_COMPUTER, 5.0f },
	{ CHAR_TEX_GLASS, 8.0f },
	{ CHAR_TEX_VENT, 4.0f },
	{ CHAR_TEX_METAL, 5.0f },
	{ CHAR_TEX_PLASTIC, 8.0f },
	{ CHAR_TEX_BLOODYFLESH, 16.0f },
	{ CHAR_TEX_FLESH, 16.0f },
	{ CHAR_TEX_DIRT, 6.0f },
};

const DataPenetrationItem_t *GetPenetrationDataForMaterial(unsigned short material)
{
	for (int i = 0; i < PENETRATION_DATA_SIZE; i++)
	{
		if (PENETRATION_DATA_LIST[i].material == material)
			return &PENETRATION_DATA_LIST[i];
	}

	return NULL;
}

Vector TryPenetrateSurface(trace_t *tr, ITraceFilter *filter)
{
	if (tr && filter && strcmp(tr->surface.name, "tools/toolsblockbullets"))
	{
		surfacedata_t *p_penetrsurf = physprops->GetSurfaceData(tr->surface.surfaceProps);
		if (p_penetrsurf)
		{
			const DataPenetrationItem_t *penetrationInfo = GetPenetrationDataForMaterial(p_penetrsurf->game.material);
			if (penetrationInfo)
			{
				Vector vecDir = (tr->endpos - tr->startpos);
				VectorNormalize(vecDir);

				Vector vecNewStart = tr->endpos + vecDir * penetrationInfo->depth;
				trace_t trPeneTest;
				UTIL_TraceLine(vecNewStart, vecNewStart + vecDir * MAX_TRACE_LENGTH, MASK_SHOT, filter, &trPeneTest);
				if (!trPeneTest.startsolid)
					return vecNewStart;
			}
		}
	}

	return vec3_invalid;
}

const char* COM_GetModDirectory()
{
	static char modDir[MAX_PATH];
	if (Q_strlen(modDir) == 0)
	{
		const char* gamedir = CommandLine()->ParmValue("-game", CommandLine()->ParmValue("-defaultgamedir", "hl2"));
		Q_strncpy(modDir, gamedir, sizeof(modDir));
		if (strchr(modDir, '/') || strchr(modDir, '\\'))
		{
			Q_StripLastDir(modDir, sizeof(modDir));
			int dirlen = Q_strlen(modDir);
			Q_strncpy(modDir, gamedir + dirlen, sizeof(modDir) - dirlen);
		}
	}

	return modDir;
}