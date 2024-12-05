//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "prediction.h"
#include "c_baseplayer.h"
#include "igamemovement.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
static CMoveData g_MoveData;
CMoveData *g_pMoveData = &g_MoveData;

//=========================================================
//=========================================================
class CINSPrediction : public CPrediction
{
	DECLARE_CLASS( CINSPrediction, CPrediction );

public:
	void SetupMove( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	void FinishMove( C_BasePlayer *player, CUserCmd *ucmd, CMoveData *move );
};

//=========================================================
//=========================================================
void CINSPrediction::SetupMove( C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	// zero (#0000386 "Rolling barrel = getting stuck") [ 
	extern ConVar sv_turbophysics;

	if ( sv_turbophysics.GetBool( ) )
		player->AvoidPhysicsProps( ucmd );
	// zero ]

	// call the default SetupMove code.
	BaseClass::SetupMove( player, ucmd, pHelper, move );
}

//=========================================================
//=========================================================
void CINSPrediction::FinishMove( C_BasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	// call the default FinishMove code
	BaseClass::FinishMove( player, ucmd, move );
}

//=========================================================
//=========================================================

// expose interface to engine
static CINSPrediction g_Prediction;

//=========================================================
//=========================================================
EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CINSPrediction, IPrediction, VCLIENT_PREDICTION_INTERFACE_VERSION, g_Prediction );

CPrediction *prediction = &g_Prediction;
