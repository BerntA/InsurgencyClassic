//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_RULES_H
#define FF_RULES_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_gamerules.h"

//=========================================================
//=========================================================
class CINSObjective;

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CFFRules C_FFRules

#endif

class CFFRules : public CINSRules
{
	DECLARE_CLASS( CFFRules, CINSRules );

protected:
	virtual int GetGameType( void ) const { return GAMETYPE_FIREFIGHT; }

#ifdef GAME_DLL
	
protected:
	virtual bool IsEnoughObjectives( void ) const;

	virtual bool IsCutupsAllowed( void ) const;

	virtual void CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder );

private:
	int FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient );
	int FindStartSpawns( int &iT1Spawn, int &iT2Spawn );

	bool ForceTimerType( void ) { return false; }

	void UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn );
	int FindObjForSpawn( int iTeamID );
	void UpdateCutups( CINSObjective *pCapturedObj );

	int ObjectiveCapturePoints( bool bFollowingOrders ) const;
	int ObjectiveLeaderPoints( void ) const;

	bool CheckForWinner( int &iWinningTeam );
	int GetDefaultWinner( void ) const;

#endif
};

#endif // FF_RULES_H