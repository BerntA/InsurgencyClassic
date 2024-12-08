//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PB_RULES_H
#define PB_RULES_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_gamerules.h"

//=========================================================
//=========================================================
#define POWERBALL_MODEL "models/effects/combineball.mdl"

//=========================================================
//=========================================================
class CINSObjective;
class CPowerBall;

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CPBRules C_PBRules

#endif

class CPBRules : public CINSRules
{
	DECLARE_CLASS( CPBRules, CINSRules );

public:
	int GetGameType( void ) const { return GAMETYPE_POWERBALL; }

#ifdef GAME_DLL

private:
	void Precache( void );

	bool IsEnoughObjectives( void ) const;
	int FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient );

	bool ForceTimerType( void ) { return false; }

	void UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn );
	void CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder );
	bool ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID ) { return true; }

	int ObjectiveCapturePoints( bool bFollowingOrders ) const;
	int ObjectiveLeaderPoints( void ) const;

	bool CheckForWinner( int &iWinningTeam );
	int GetDefaultWinner( void ) const;

	void PlayerAddWeapons( CINSPlayer *pPlayer );

#endif

private:
	CPowerBall *m_pPowerBall;
};

#endif // PB_RULES_H
