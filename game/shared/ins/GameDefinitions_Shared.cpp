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
		{ "ACH_TUTORIAL_COMPLETE", "", 0, ACHIEVEMENT_TYPE_MAP, 0, false, NULL },

		// Level
		{ "ACH_LEVEL_5", "BBX_ST_LEVEL", 5, ACHIEVEMENT_TYPE_DEFAULT, 25, false, NULL },
		{ "ACH_LEVEL_50", "BBX_ST_LEVEL", 50, ACHIEVEMENT_TYPE_DEFAULT, 500, false, NULL },
		{ "ACH_LEVEL_100", "BBX_ST_LEVEL", 100, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },
		{ "ACH_LEVEL_150", "BBX_ST_LEVEL", 150, ACHIEVEMENT_TYPE_DEFAULT, 2500, false, NULL },
		{ "ACH_LEVEL_350", "BBX_ST_LEVEL", 350, ACHIEVEMENT_TYPE_DEFAULT, 5000, false, NULL },
		{ "ACH_LEVEL_500", "BBX_ST_LEVEL", 500, ACHIEVEMENT_TYPE_DEFAULT, 0, false, NULL },

		// Objectives / Sweetness
		{ "ACH_SURVIVOR_CAPTURE_BRIEFCASE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 50, false, NULL },
		{ "ACH_SURVIVOR_KILL_FRED", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 100, false, NULL },
		{ "ACH_SURVIVOR_KILL_FRED_HALLOWEEN", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },
		{ "ACH_SURVIVOR_INFECTED", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 50, false, NULL },
		{ "ACH_ZOMBIE_KILL_HUMAN", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 50, false, NULL },
		{ "ACH_ZOMBIE_KILL_HUMANOIDS", "BBX_KI_HUMANS", 25, ACHIEVEMENT_TYPE_DEFAULT, 300, false, NULL },
		{ "ACH_ZOMBIE_FIRST_BLOOD", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 25, false, NULL },

		// Perks
		{ "ACH_SKILL_PERK_ROCKET", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 25, false, NULL },
		{ "ACH_SKILL_PERK_BLOODRAGE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 25, false, NULL },
		{ "ACH_SKILL_PERK_GUNSLINGER", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 25, false, NULL },
		{ "ACH_SKILL_PERK_ZOMBIERAGE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 50, false, NULL },

		// Zombie Demolishing
		{ "ACH_SURVIVOR_KILL_25_ZOMBIES", "BBX_KI_ZOMBIES", 25, ACHIEVEMENT_TYPE_DEFAULT, 50, false, NULL },
		{ "ACH_SURVIVOR_KILL_100_ZOMBIES", "BBX_KI_ZOMBIES", 100, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		{ "ACH_SURVIVOR_KILL_1000_ZOMBIES", "BBX_KI_ZOMBIES", 1000, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },
		{ "ACH_SURVIVOR_KILL_20000_ZOMBIES", "BBX_KI_ZOMBIES", 20000, ACHIEVEMENT_TYPE_DEFAULT, 10000, false, NULL },

		// Bandit Carnage
		{ "ACH_BANDIT_KILL_500", "BBX_KI_BANDITS", 500, ACHIEVEMENT_TYPE_DEFAULT, 750, false, NULL },
		{ "ACH_BANDIT_KILL_1000", "BBX_KI_BANDITS", 1000, ACHIEVEMENT_TYPE_DEFAULT, 1250, false, NULL },

		// Luck
		{ "ACH_GM_SURVIVAL_PROPANE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 70, false, NULL },
		{ "ACH_WEAPON_GRENADE_FAIL", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 25, false, NULL },

		// Pistols & Revolvers
		{ "ACH_WEP_BERETTA", "BBX_KI_BERETTA", 150, ACHIEVEMENT_TYPE_DEFAULT, 250, false, NULL },
		{ "ACH_WEP_DEAGLE", "BBX_KI_DEAGLE", 200, ACHIEVEMENT_TYPE_DEFAULT, 400, false, NULL },
		//{ "ACH_WEP_1911", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		//{ "ACH_WEP_SAUER", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		//{ "ACH_WEP_HK45", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		{ "ACH_WEP_GLOCK", "BBX_KI_GLOCK17", 200, ACHIEVEMENT_TYPE_DEFAULT, 300, false, NULL },
		//{ "ACH_WEP_USP", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		//{ "ACH_WEP_357", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		{ "ACH_WEP_DUAL", "BBX_KI_AKIMBO", 500, ACHIEVEMENT_TYPE_DEFAULT, 750, false, NULL },

		// SMGs
		{ "ACH_WEP_UZI", "BBX_KI_UZI", 300, ACHIEVEMENT_TYPE_DEFAULT, 285, false, NULL },
		{ "ACH_WEP_HKMP5", "BBX_KI_HKMP5", 300, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		//{ "ACH_WEP_HKSMG2", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		{ "ACH_WEP_HKMP7", "BBX_KI_HKMP7", 300, ACHIEVEMENT_TYPE_DEFAULT, 250, false, NULL },
		{ "ACH_WEP_MAC11", "BBX_KI_MAC11", 300, ACHIEVEMENT_TYPE_DEFAULT, 350, false, NULL },

		// Rifles
		{ "ACH_WEP_FAMAS", "BBX_KI_FAMAS", 240, ACHIEVEMENT_TYPE_DEFAULT, 150, false, NULL },
		{ "ACH_WEP_AK74", "BBX_KI_AK74", 300, ACHIEVEMENT_TYPE_DEFAULT, 150, false, NULL },
		{ "ACH_WEP_G36C", "BBX_KI_G36C", 300, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		//{ "ACH_WEP_M4", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },

		// Shotguns
		{ "ACH_WEP_870", "BBX_KI_870", 210, ACHIEVEMENT_TYPE_DEFAULT, 150, false, NULL },
		{ "ACH_WEP_BENELLIM4", "BBX_KI_BENELLIM4", 275, ACHIEVEMENT_TYPE_DEFAULT, 185, false, NULL },
		{ "ACH_WEP_DOUBLEBARREL", "BBX_KI_SAWOFF", 200, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		{ "ACH_WEP_WINCHESTER", "BBX_KI_TRAPPER", 240, ACHIEVEMENT_TYPE_DEFAULT, 400, false, NULL },

		// Special & Snipers
		{ "ACH_WEP_MINIGUN", "BBX_KI_MINIGUN", 12000, ACHIEVEMENT_TYPE_DEFAULT, 3000, false, NULL },
		{ "ACH_WEP_FLAMETHROWER", "BBX_KI_FLAMETHROWER", 340, ACHIEVEMENT_TYPE_DEFAULT, 750, false, NULL },
		//{ "ACH_WEP_50CAL", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		{ "ACH_WEP_700", "BBX_KI_REM700", 150, ACHIEVEMENT_TYPE_DEFAULT, 500, false, NULL },
		//{ "ACH_WEP_CROSSBOW", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		//{ "ACH_WEP_STONER", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },

		// Explosives
		{ "ACH_WEP_EXPLOSIVE", "BBX_KI_EXPLOSIVES", 200, ACHIEVEMENT_TYPE_DEFAULT, 450, false, NULL },

		// Melee
		{ "ACH_WEAPON_AXE", "BBX_KI_FIREAXE", 50, ACHIEVEMENT_TYPE_DEFAULT, 300, false, NULL },
		{ "ACH_WEAPON_KNIFE", "BBX_KI_M9PHROBIS", 35, ACHIEVEMENT_TYPE_DEFAULT, 150, false, NULL },
		{ "ACH_WEP_BASEBALLBAT", "BBX_KI_BASEBALLBAT", 80, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		{ "ACH_WEP_HATCHET", "BBX_KI_HATCHET", 120, ACHIEVEMENT_TYPE_DEFAULT, 240, false, NULL },
		{ "ACH_WEP_SLEDGE", "BBX_KI_SLEDGEHAMMER", 280, ACHIEVEMENT_TYPE_DEFAULT, 500, false, NULL },
		{ "ACH_WEP_MACHETE", "BBX_KI_MACHETE", 280, ACHIEVEMENT_TYPE_DEFAULT, 300, false, NULL },
		{ "ACH_WEP_BRICKED", "BBX_KI_BRICK", 99, ACHIEVEMENT_TYPE_DEFAULT, 250, false, NULL },
		{ "ACH_WEP_TRICKSTER", "BBX_KI_KICK", 35, ACHIEVEMENT_TYPE_DEFAULT, 300, false, NULL },
		{ "ACH_WEP_FISTS", "BBX_KI_FISTS", 100, ACHIEVEMENT_TYPE_DEFAULT, 400, false, NULL },

		// Special End Game:
		{ "ACH_GM_ARENA_WIN", "", 0, ACHIEVEMENT_TYPE_MAP, 250, false, NULL },
		{ "ACH_ENDGAME_NOFIREARMS", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },
		{ "ACH_OBJ_NOFIREARMS", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },

		// Objectives & Quests Progression, + story.
		{ "ACH_QUEST_PROG_5", "BBX_QUESTS", 5, ACHIEVEMENT_TYPE_DEFAULT, 500, false, NULL },
		{ "ACH_QUEST_PROG_25", "BBX_QUESTS", 25, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },

		// Misc, Custom, Community Recommendations...
		{ "ACH_RAZ_HEALTH_ADDICT", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		{ "ACH_RAZ_SWEEPER", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 200, false, NULL },
		{ "ACH_RAZ_MAZELTOV", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, NULL },
		{ "ACH_RAZ_KUNG_FLU", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, NULL },
		{ "ACH_RAZ_TRAUMA", "BBX_RZ_HEADSHOT", 250, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, NULL },
		{ "ACH_RAZ_PRESCRIBED_PAIN", "BBX_RZ_PAIN", 1000000, ACHIEVEMENT_TYPE_DEFAULT, 2500, false, NULL },

		// Maps:
		{ "ACH_MAP_LASTSTAND", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bbc_laststand" },
		{ "ACH_MAP_TERMOIL", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bbc_termoil" },
		{ "ACH_MAP_MECKLENBURG", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 750, false, "bbc_mecklenburg" },
		{ "ACH_MAP_COMPOUND", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 250, false, "bbc_compound" },
		{ "ACH_MAP_NIGHTCLUB", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 750, false, "bbc_nightclub" },
		{ "ACH_MAP_SWAMPTROUBLE_OBJ", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 750, false, "bbc_swamptrouble" },
		{ "ACH_MAP_COLTEC_C", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 750, false, "bbc_coltec" },
		{ "ACH_MAP_FEVER", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 1000, false, "bbc_fever" },
		{ "ACH_MAP_DAYCITY", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bbc_daycity" },
		{ "ACH_MAP_FORGOTTEN_MANSION", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 1500, false, "bbs_forgotten_mansion" },

		{ "ACH_MAP_ROOFTOP", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_rooftop" },
		{ "ACH_MAP_COLOSSEUM", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_colosseum" },
		{ "ACH_MAP_BARRACKS_ARENA", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_barracks" },
		{ "ACH_MAP_CARGO", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_cargo" },
		{ "ACH_MAP_DEVILSCRYPT", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_devilscrypt" },
		{ "ACH_MAP_SWAMPTROUBLE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_swamptrouble" },
		{ "ACH_MAP_SALVAGE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 2000, false, "bba_salvage" },
		{ "ACH_MAP_CARNAGE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_carnage" },
		{ "ACH_MAP_COLTEC_A", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_coltec" },
		{ "ACH_MAP_ISLAND_A", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 250, false, "bba_island" },
		{ "ACH_MAP_SURGERY_A", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 250, false, "bba_surgery" },
		{ "ACH_MAP_AMBIT", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 750, false, "bba_ambit" },
		{ "ACH_MAP_BACKYARD_A", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_backyard" },
		{ "ACH_MAP_MECKLENBURG_A", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 500, false, "bba_mecklenburg" },
		{ "ACH_MAP_RISHIKA_A", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 3500, false, "bba_rishika" },

		// Hidden:
		{ "ACH_SECRET_WATCHYOURSTEP", "", 0, ACHIEVEMENT_TYPE_MAP, 0, true, NULL },
		{ "ACH_SECRET_TFO", "", 0, ACHIEVEMENT_TYPE_MAP, 0, true, NULL },
		//{ "ACH_AREA_MACHETE", "", 0, ACHIEVEMENT_TYPE_MAP, 0, true, NULL },
		//{ "ACH_AREA_TEA", "", 0, ACHIEVEMENT_TYPE_MAP, 0, true, NULL },
		{ "ACH_SECRET_TURTLE", "", 0, ACHIEVEMENT_TYPE_MAP, 0, true, NULL },
		{ "ACH_SECRET_GIVEALL", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		{ "ACH_WEP_BRICK", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 150, true, NULL },
		{ "ACH_TURTLE_RAVAGE", "", 0, ACHIEVEMENT_TYPE_DEFAULT, 350, true, NULL },

		// Stats Only + hidden (no achievs) - used for leaderboards...
		{ "", "BBX_ST_KILLS", 9999999, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
		{ "", "BBX_ST_DEATHS", 9999999, ACHIEVEMENT_TYPE_DEFAULT, 0, true, NULL },
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