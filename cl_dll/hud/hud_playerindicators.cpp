//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "hud_playerindicators.h"

#include "play_team_shared.h"
#include "ins_squad_shared.h"
#include "voice_status.h"

#include "ins_gamerules.h"

#include "view.h"

#include "vguimatsurface/imatsystemsurface.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterialvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define ICON_SIZE 6.0f

const char *g_pszRankTexNames[RANK_COUNT] = {
	"prv",		// RANK_PRIVATE
	"lpv",		// RANK_LPRIVATE
	"crp",		// RANK_CORPORAL
	"sgt",		// RANK_SERGEANT
	"lt",		// RANK_LIEUTENANT
};

const char *g_pszSpecialTexNames[PICONSPECIAL_COUNT] = {
	"bleed",	// PICONSPECIAL_BLEED
	"voice",	// PICONSPECIAL_VOICE
	//"ammo",	// PICONSPECIAL_AMMO
	//"target",	// PICONSPECIAL_TARGET
};

//=========================================================
//=========================================================
DECLARE_HUDELEMENT(CHUDPlayerIndicators);

CHUDPlayerIndicators *g_pPlayerIndicators = NULL;

CHUDPlayerIndicators::CHUDPlayerIndicators(const char *pszElementName)
	: CHUDIndicators(pszElementName, "HudPlayerIndicators")
{
	g_pPlayerIndicators = this;

	m_bLoadedRankIcons = false;
}

//=========================================================
//=========================================================
CHUDPlayerIndicators::~CHUDPlayerIndicators()
{
	g_pPlayerIndicators = NULL;
}

//=========================================================
//=========================================================
void CHUDPlayerIndicators::Init(void)
{
	// load materials
	char szPath[128];

	for(int i = 0; i < PICONSPECIAL_COUNT; i++)
	{
		Q_strncpy(szPath, "sprites/hud/playerindi/", sizeof(szPath));
		Q_strncat(szPath, g_pszSpecialTexNames[i], sizeof(szPath), COPY_ALL_CHARACTERS);

		m_SpecialIcons[i].Init(szPath, TEXTURE_GROUP_OTHER);
	}

	// setup helpers
	for(int i = 0; i < PICONSPECIAL_COUNT; i++)
	{
		SpecialIcon_t &SpecialIconHelper = m_SpecialIconHelpers[i];

		switch(i)
		{
		case PICONSPECIAL_BLEED:
			SpecialIconHelper = FindInjuredSpecial;
			break;
		case PICONSPECIAL_VOICE:
			SpecialIconHelper = FindVoiceSpecial;
			break;
		}
	}
}

//=========================================================
//=========================================================
void CHUDPlayerIndicators::LevelShutdown(void)
{
	for(int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		int iTeamID = TeamToPlayTeam(i);

		for(int j = 0; j < RANK_COUNT; j++)
		{
			CMaterialReference &MatRef = m_RankIcons[iTeamID][j];

			if(MatRef)
				MatRef->DecrementReferenceCount();
		}
	}

	m_bLoadedRankIcons = false;
}

//=========================================================
//=========================================================
void CHUDPlayerIndicators::LoadRankIcons(void)
{
	m_bLoadedRankIcons = true;

	char szPath[128], szSecPath[128];

	for(int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		C_PlayTeam *pTeam = GetGlobalPlayTeam(i);
		int iTeamID = TeamToPlayTeam(i);

		if(!pTeam)
			continue;

		for(int j = 0; j < RANK_COUNT; j++)
		{
			GetVGUITeamPath(pTeam, TEAMVGUI_RANKICONS, szPath, sizeof(szPath));

			Q_snprintf(szSecPath, sizeof(szSecPath), "/%s", g_pszRankTexNames[j]);
			Q_strncat(szPath, szSecPath, sizeof(szPath), COPY_ALL_CHARACTERS);

			m_RankIcons[iTeamID][j].Init(szPath, TEXTURE_GROUP_OTHER);
		}
	}
}

//=========================================================
//=========================================================
void CHUDPlayerIndicators::Draw(void)
{
	C_INSPlayer *pPlayer = C_INSPlayer::GetLocalPlayer();

	if(!pPlayer || !pPlayer->OnPlayTeam())
		return;

	C_PlayTeam *pTeam = GetLocalPlayTeam();
	C_INSSquad *pSquad = GetLocalSquad();

	if(!pTeam || !pSquad)
		return;

	// load rankicons
	if(!m_bLoadedRankIcons)
		LoadRankIcons();

	// draw fireteam or squad
	int i, iDrawList;
	iDrawList = 0;

	for(i = 0; i < MAX_SQUAD_SLOTS; i++)
		DrawPlayer(pSquad->GetPlayer(i), iDrawList);

	// draw target
	int iEntIndex = pPlayer->GetIDTarget();

	if(iEntIndex != 0)
		DrawPlayer(ToINSPlayer(ClientEntityList().GetEnt(iEntIndex)), iDrawList);
}

//=========================================================
//=========================================================
void CHUDPlayerIndicators::DrawPlayer(C_INSPlayer *pPlayer, int &iDrawList)
{
	if(!pPlayer || pPlayer->IsDormant() || pPlayer->IsPlayerDead() || pPlayer == C_INSPlayer::GetLocalPlayer())
		return;

	if(iDrawList & (1 << (pPlayer->entindex() - 1)))
		return;

	Vector vecOrigin;
	QAngle angAngles;

	pPlayer->GetAttachment(pPlayer->LookupAttachment("anim_attachment_head"), vecOrigin, angAngles);
	vecOrigin.z += ICON_SIZE + 12.0f;

	float flDot = 0.0f;
	float *pAlpha = GetAlpha(vecOrigin, flDot);

	// if we can't see it, draw the arrow
	if(flDot < 0.0f)
	{
		Color ArrowColor = INSRules()->TeamColor(pPlayer);
		DrawArrow(vecOrigin, ArrowColor);
		return;
	}

	// find indicator to draw
	CMaterialReference *pMat = NULL;

	int iIconSpecialID = FindSpecial(pPlayer);

	if(iIconSpecialID != PICONSPECIAL_INVALID)
	{
		pMat = &m_SpecialIcons[iIconSpecialID];
	}
	else
	{
		int iRank = pPlayer->GetRank();

		if(iRank != RANK_INVALID)
			pMat = &m_RankIcons[TeamToPlayTeam(pPlayer->GetTeamID())][iRank];
	}

	// no material, no drawing.
	if(!pMat)
		return;
	
	// draw indicator
	static Vector vecUp(0, 0, 1);
	Vector vecRight = CurrentViewRight();

	materials->Bind(*pMat, TEXTURE_GROUP_OTHER);

	CMeshBuilder Builder;
	IMesh *pMesh = materials->GetDynamicMesh();

	Builder.Begin(pMesh, MATERIAL_QUADS, 1);

	Builder.Color4fv(pAlpha);
	Builder.TexCoord2f(0.0f, 0.0f, 0.0f);
	Builder.Position3fv((vecOrigin + (vecRight * -ICON_SIZE) + (vecUp * ICON_SIZE)).Base());
	Builder.AdvanceVertex();

	Builder.Color4fv(pAlpha);
	Builder.TexCoord2f(0.0f, 1.0f, 0.0f);
	Builder.Position3fv((vecOrigin + (vecRight * ICON_SIZE) + (vecUp * ICON_SIZE)).Base());
	Builder.AdvanceVertex();

	Builder.Color4fv(pAlpha);
	Builder.TexCoord2f(0.0f, 1.0f, 1.0f);
	Builder.Position3fv((vecOrigin + (vecRight * ICON_SIZE) + (vecUp * -ICON_SIZE)).Base());
	Builder.AdvanceVertex();

	Builder.Color4fv(pAlpha);
	Builder.TexCoord2f(0.0f, 0.0f, 1.0f);
	Builder.Position3fv((vecOrigin + (vecRight * -ICON_SIZE) + (vecUp * -ICON_SIZE)).Base());
	Builder.AdvanceVertex();

	Builder.End();

	pMesh->Draw();

	// set to drawn
	iDrawList |= (1 << (pPlayer->entindex() - 1));
}

//=========================================================
//=========================================================
float *CHUDPlayerIndicators::GetAlpha(Vector &vecOrigin, float &flDot) const
{
	static Vector4D Alpha(1.0f, 1.0f, 1.0f, 1.0f);

	Vector vecView, vecDir;
	vecView = CurrentViewForward();
	vecDir = vecOrigin - CurrentViewOrigin();

	VectorNormalize(vecDir);

	flDot = DotProduct(vecView, vecDir);
	Alpha.w = RemapValClamped(flDot, 0.5f, 1.0f, 0.85f, 0.5f);

	return Alpha.Base();
}

//=========================================================
//=========================================================
int CHUDPlayerIndicators::FindSpecial(C_INSPlayer *pPlayer)
{
	for(int i = 0; i < PICONSPECIAL_COUNT; i++)
	{
		Assert(m_SpecialIconHelpers[i]);

		if(!(*this.*m_SpecialIconHelpers[i])(pPlayer))
			continue;

		return i;
	}

	return PICONSPECIAL_INVALID;
}

//=========================================================
//=========================================================
bool CHUDPlayerIndicators::FindInjuredSpecial(C_INSPlayer *pPlayer)
{
	return (pPlayer->GetHealthType() == HEALTHTYPE_SERIOUS);
}

//=========================================================
//=========================================================
bool CHUDPlayerIndicators::FindVoiceSpecial(C_INSPlayer *pPlayer)
{
	CVoiceStatus *pVoiceStatus = GetClientVoiceMgr();

	if(!pVoiceStatus)
		return false;

	return pVoiceStatus->IsPlayerSpeaking(pPlayer->entindex());
}