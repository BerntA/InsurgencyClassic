//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "team_spawnpoint.h"
#include "ins_obj.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
int ConvertCSToINSTeam(int iCSTeamID)
{
	if(iCSTeamID == 3)
		return TEAM_ONE;

	return TEAM_TWO;
}

//=========================================================
//=========================================================
class CCSSpawnPoint : public CSpawnPoint
{
public:
	DECLARE_CLASS(CCSSpawnPoint, CSpawnPoint);

protected:
	void Activate(void);
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(info_player_counterterrorist, CCSSpawnPoint);
LINK_ENTITY_TO_CLASS(info_player_terrorist, CCSSpawnPoint);

//=========================================================
//=========================================================
void CCSSpawnPoint::Activate(void)
{
	if(ClassMatches("info_player_counterterrorist"))
	{
		m_iTeam = TEAM_ONE;
		m_iParentObjective = 1;
	}
	else
	{
		m_iTeam = TEAM_TWO;
		m_iParentObjective = 2;
	}

	BaseClass::Activate();
}

//=========================================================
//=========================================================
class CCSObj : public CINSObjective
{
public:
	DECLARE_CLASS(CCSObj, CINSObjective);

protected:
	bool KeyValue(const char *szKeyName, const char *szValue);
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(func_buyzone, CCSObj);

//=========================================================
//=========================================================
bool CCSObj::KeyValue(const char *szKeyName, const char *szValue)
{
	if(FStrEq(szKeyName, "TeamNum"))
	{
		if(ConvertCSToINSTeam(atoi(szValue)) == TEAM_ONE)
			m_iID = 1;
		else
			m_iID = 2;
		
		return true;
	}

	return BaseClass::KeyValue(szKeyName, szValue);
}