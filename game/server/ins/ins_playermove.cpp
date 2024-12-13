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
#include "ipredictionsystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static CMoveData g_MoveData;
CMoveData* g_pMoveData = &g_MoveData;

IPredictionSystem* IPredictionSystem::g_pPredictionSystems = NULL;

class CINSPlayerMove : public CPlayerMove
{
	DECLARE_CLASS(CINSPlayerMove, CPlayerMove);
};

static CINSPlayerMove g_PlayerMove;

CPlayerMove* PlayerMove()
{
	return &g_PlayerMove;
}