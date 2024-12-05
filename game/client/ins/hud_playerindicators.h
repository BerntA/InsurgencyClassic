//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HUD_PLAYERINDICATORS_H
#define HUD_PLAYERINDICATORS_H
#ifdef _WIN32
#pragma once
#endif

#include "hud_indicators.h"
#include "materialsystem/imaterial.h"

#include "commander_shared.h"

//=========================================================
//=========================================================
class C_INSPlayer;
class C_INSSquad;

//=========================================================
//=========================================================
enum PlayerSpecialIcons_t
{
	PICONSPECIAL_INVALID = -1,
	PICONSPECIAL_BLEED = 0,
	PICONSPECIAL_VOICE,
	//PICONSPECIAL_AMMO,
	//PICONSPECIAL_TARGET,
	PICONSPECIAL_COUNT,
};

//=========================================================
//=========================================================
class CHUDPlayerIndicators : public CHUDIndicators
{
	DECLARE_CLASS_SIMPLE(CHUDPlayerIndicators, CHUDIndicators);

public:
	CHUDPlayerIndicators(const char *pElementName);
	~CHUDPlayerIndicators();

	void Draw(void);

private:
	void Init(void);
	void LevelShutdown(void);
	void LoadRankIcons(void);

	void DrawPlayer(C_INSPlayer *pPlayer, int &iDrawList);
	float *GetAlpha(Vector &vecOrigin, float &flDot) const;

	int FindSpecial(C_INSPlayer *pPlayer);
	bool FindInjuredSpecial(C_INSPlayer *pPlayer);
	bool FindVoiceSpecial(C_INSPlayer *pPlayer);

private:
	bool m_bLoadedRankIcons;

	CMaterialReference m_RankIcons[MAX_PLAY_TEAMS][RANK_COUNT];
	CMaterialReference m_SpecialIcons[PICONSPECIAL_COUNT];

	typedef bool (CHUDPlayerIndicators::*SpecialIcon_t)(C_INSPlayer *pPlayer);
	SpecialIcon_t m_SpecialIconHelpers[PICONSPECIAL_COUNT];
};

//=========================================================
//=========================================================
extern CHUDPlayerIndicators *g_pPlayerIndicators;

#endif // HUD_PLAYERINDICATORS_H