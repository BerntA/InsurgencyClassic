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
int ConvertDODToINSTeam(int iDODTeamID)
{
	if(iDODTeamID == 2)
		return TEAM_ONE;

	return TEAM_TWO;
}

//=========================================================
//=========================================================
class CDODSpawnPoint : public CSpawnPoint
{
public:
	DECLARE_CLASS(CDODSpawnPoint, CSpawnPoint);

protected:
	void Activate(void);
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(info_player_allies, CDODSpawnPoint);
LINK_ENTITY_TO_CLASS(info_player_axis, CDODSpawnPoint);

//=========================================================
//=========================================================
void CDODSpawnPoint::Activate(void)
{
	if(ClassMatches("info_player_allies"))
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
class CDODControlPoint : public CBaseEntity
{
public:
	DECLARE_CLASS(CDODControlPoint, CBaseEntity);
	DECLARE_DATADESC();

public:
	int GetPointID(void) const { return m_iPointID; }

private:
	int m_iPointID;
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(dod_control_point, CDODControlPoint);

BEGIN_DATADESC(CDODControlPoint)

	DEFINE_KEYFIELD(m_iPointID, FIELD_INTEGER, "point_index")

END_DATADESC()

//=========================================================
//=========================================================
class CDODObj : public CINSObjective
{
public:
	DECLARE_CLASS(CDODObj, CINSObjective);
	DECLARE_DATADESC();

protected:
	void Init(void);

private:
	string_t m_iszControlPointName;
};

//=========================================================
//=========================================================
LINK_ENTITY_TO_CLASS(dod_capture_area, CDODObj);

BEGIN_DATADESC(CDODObj)

	DEFINE_KEYFIELD(m_iszControlPointName, FIELD_STRING, "area_cap_point")

END_DATADESC()

//=========================================================
//=========================================================
void CDODObj::Init(void)
{
	CDODControlPoint *pControlPoint = (CDODControlPoint*)gEntList.FindEntityByName(NULL, m_iszControlPointName, NULL);

	if(pControlPoint)
		m_iID = pControlPoint->GetPointID() + 1;

	BaseClass::Init();
}