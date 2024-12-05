//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player_command.h"
#include "igamemovement.h"
#include "in_buttons.h"
#include "ipredictionsystem.h"
#include "iservervehicle.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
static CMoveData g_MoveData;
CMoveData *g_pMoveData = &g_MoveData;

IPredictionSystem *IPredictionSystem::g_pPredictionSystems = NULL;

//=========================================================
//=========================================================
class CINSPlayerMove : public CPlayerMove
{
	DECLARE_CLASS( CINSPlayerMove, CPlayerMove );

public:
	void StartCommand( CBasePlayer *player, CUserCmd *cmd );
	void SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move );
	void FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move );
};

//=========================================================
//=========================================================
static CINSPlayerMove g_PlayerMove;

//=========================================================
//=========================================================
CPlayerMove *PlayerMove( void )
{
	return &g_PlayerMove;
}

//=========================================================
//=========================================================
void CINSPlayerMove::StartCommand( CBasePlayer *player, CUserCmd *cmd )
{
	BaseClass::StartCommand( player, cmd );
}

//=========================================================
//=========================================================
void CINSPlayerMove::SetupMove( CBasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move )
{
	// zero [ 

	// PNOTE: this was a fix todo with players
	// getting stuck on barrels

	// PNOTE: commenting out because I'm scared of vphysics client crash

	/*extern ConVar sv_turbophysics;

	if ( sv_turbophysics.GetBool( ) )
		player->AvoidPhysicsProps( ucmd );*/

	// zero ]

	BaseClass::SetupMove( player, ucmd, pHelper, move );

	IServerVehicle *pVehicle = player->GetVehicle( );

	if( pVehicle && gpGlobals->frametime != 0 )
		pVehicle->SetupMove( player, ucmd, pHelper, move ); 
}

//=========================================================
//=========================================================
void CINSPlayerMove::FinishMove( CBasePlayer *player, CUserCmd *ucmd, CMoveData *move )
{
	BaseClass::FinishMove( player, ucmd, move );

	IServerVehicle *pVehicle = player->GetVehicle( );

	if( pVehicle && gpGlobals->frametime != 0 )
		pVehicle->FinishMove( player, ucmd, move );
}
