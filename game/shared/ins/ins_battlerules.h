//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef BATTLE_RULES_H
#define BATTLE_RULES_H
#ifdef _WIN32
#pragma once
#endif

#include "ins_ffrules.h"

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CBattleRulesProxy C_BattleRulesProxy

#endif

class CBattleRulesProxy : public CINSRulesProxy
{
public:
	DECLARE_CLASS( CBattleRulesProxy, CINSRulesProxy );
	DECLARE_NETWORKCLASS( );
};

//=========================================================
//=========================================================
#ifdef CLIENT_DLL

#define CBattleRules C_BattleRules

#endif

class CBattleRules : public CFFRules
{
	DECLARE_CLASS( CBattleRules, CFFRules );
	DECLARE_NETWORKCLASS_NOBASE( );

public:
	int GetGameType( void ) const { return GAMETYPE_BATTLE; }

	bool ObjectiveCaptureAllowed( const CINSObjective *pObjective, int iTeamID );

public:
	void SetCaptureObj( int iTeamID, CINSObjective *pObjective );
	CINSObjective *GetAllowedObj( int iTeamID ) const;

private:
	CINSObjective *m_pCaptureObjs[ MAX_PLAY_TEAMS ];

#ifdef GAME_DLL

public:
	CBattleRules( );

private:
	void CreateProxy( void );

	bool IsEnoughObjectives( void ) const;

	void RoundReset( void );

	void ResetAllowedCaptureObjs( void );
	void CalculateCaptureObjs( void );
	void ObjectiveCaptured( CINSObjective *pObjective );
	bool IsCutupsAllowed( void ) const;
	void CalculateDefaultOrders( int iTeamID, CObjOrder &ObjOrder );

#endif
};

#endif // BATTLE_RULES_H
