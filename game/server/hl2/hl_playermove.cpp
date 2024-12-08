//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "player_command.h"
#include "player.h"
#include "igamemovement.h"
#include "hl_movedata.h"
#include "ipredictionsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHLPlayerMove : public CPlayerMove
{
	DECLARE_CLASS(CHLPlayerMove, CPlayerMove);
public:
	CHLPlayerMove()
	{
	}

	void SetupMove(CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move);
	void FinishMove(CBasePlayer *player, CUserCmd *ucmd, CMoveData *move);
};

//
//
// PlayerMove Interface

static CHLPlayerMove g_PlayerMove;

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
CPlayerMove *PlayerMove()
{
	return &g_PlayerMove;
}

static CHLMoveData g_HLMoveData;
CMoveData *g_pMoveData = &g_HLMoveData;

IPredictionSystem *IPredictionSystem::g_pPredictionSystems = NULL;

void CHLPlayerMove::SetupMove(CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move)
{
	extern ConVar sv_turbophysics;

	if (sv_turbophysics.GetBool())
		player->AvoidPhysicsProps(ucmd);

	BaseClass::SetupMove(player, ucmd, pHelper, move);
}

void CHLPlayerMove::FinishMove(CBasePlayer *player, CUserCmd *ucmd, CMoveData *move)
{
	BaseClass::FinishMove(player, ucmd, move);
}