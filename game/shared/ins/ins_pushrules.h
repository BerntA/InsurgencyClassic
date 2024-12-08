//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef PUSH_RULES_H
#define PUSH_RULES_H
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

#define CPushRulesProxy C_PushRulesProxy

#endif

class CPushRulesProxy : public CINSRulesProxy
{
	DECLARE_CLASS( CPushRulesProxy, CINSRulesProxy );
	DECLARE_NETWORKCLASS( );
};

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CPushRules C_PushRules

#endif

class CPushRules : public CINSRules
{
	DECLARE_CLASS( CPushRules, CINSRules );
	DECLARE_NETWORKCLASS_NOBASE( );

public:
	CPushRules( );

	virtual int GetGameType( void ) const { return GAMETYPE_PUSH; }

#ifdef GAME_DLL

	CINSObjective *GetNextPossibleObjective( void );

#else

	void SetAttackableObjID( int iObjID ) { m_iAttackableObjID = iObjID; }

#endif

private:

	bool ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID );
	void ObjectiveCaptured( CINSObjective *pObjective );

#ifdef GAME_DLL

	void CreateProxy( void );

	bool IsEnoughObjectives( void ) const;
	int FinalSetup( int &iT1Spawn, int &iT2Spawn, int &iT1Orient );

	bool ForceTimerType( void );

	void ResetSpawnPoints( void );
	void UpdateSpawnPoints( CINSObjective *pCapturedObj, int &iT1Spawn, int &iT2Spawn );
	void CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder );

	int ObjectiveCapturePoints( bool bFollowingOrders ) const;
	int ObjectiveLeaderPoints( void ) const;	

	bool CheckForWinner( int &iWinningTeamID );
	int GetDefaultWinner( void ) const { return m_iAttackingTeamID; }

#else

	int GetAttackableObjID( void ) const { return m_iAttackableObjID; }

#endif

private:

#ifdef GAME_DLL

	int m_iAttackingTeamID;

	bool m_bUseInitialAttackingObj;
	CINSObjective *m_pInitialAttackingObjID;

#else

	int m_iAttackableObjID;

#endif

};

#endif // PUSH_RULES_H