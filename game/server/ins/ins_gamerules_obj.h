//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef INS_GAMERULES_OBJ_H
#define INS_GAMERULES_OBJ_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "team.h"

#define TEAM_NEUTRAL TEAM_UNASSIGNED

class CINSObjMarker; 

// ------------------------------------------------------------------------------------ //
// Insurgency Objective
// ------------------------------------------------------------------------------------ //
class CINSObjective : public CBaseEntity
{
public:
	DECLARE_CLASS(CINSObjective, CBaseEntity);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CINSObjective();

	// Entity Basic's
	int UpdateTransmitState() {	return SetTransmitState( FL_EDICT_ALWAYS );	}

	void Spawn(void);
	void Reset(void);

	virtual bool KeyValue(const char *szKeyName, const char *szValue);

	// Touching
	virtual void StartTouch(CBaseEntity *pOther);
	virtual void EndTouch(CBaseEntity *pOther);

	void PlayerHasLeft(CBasePlayer *pPlayer);

	// Capturing
	void CaptureThink(void);

	// General
	void ResetObjective(void);

	void SetMarker(CINSObjMarker *pMarker);

	bool GetSpawnPoints(void);
	CBaseEntity* GetPlayerSpawnPoint(CBasePlayer *pPlayer);

	void SetMapID(int iID) { m_iID = iID; }
	void SetStartSpawn(bool bState) { m_bStartSpawn = bState; }
	void SetHidden(bool bState) { m_bHidden = bState; }
	void SetCapturedTeam(int iCapturedTeam) { m_iCapturedTeam = iCapturedTeam; }
	void SetCapturingTeam(int iCapturingTeam) { m_iCapturingTeam = iCapturingTeam; }
	void SetCutUp(bool bState) { m_bCutUp = bState; }

	const char *GetName(void) const { return m_szName; }
	int GetMapID(void) const { return m_iID; }
	int GetCapturedTeam(void) const { return m_iCapturedTeam; }

	bool IsStartSpawn(void) const { return m_bStartSpawn; }
	bool IsHidden(void) const { return m_bHidden; }
	bool IsCaptured(void) const { return m_iCapturedTeam != TEAM_NEUTRAL; }
	bool IsCutUp(void) const { return m_bCutUp; }
	bool IsBeingCaptured(void) const { return m_flCaptureTimer != 0.0f; }

private:
	// Touching
	void TouchHook(CBaseEntity *pOther, void (CINSObjective::*TouchCall)(CBasePlayer*));
	void StartPlayerTouch(CBasePlayer *pPlayer);
	void EndPlayerTouch(CBasePlayer *pPlayer);

	void FinishPlayerEndTouch(CBasePlayer *pPlayer);

	void SendOutsideMessage(CBasePlayer *pPlayer, bool bLeft);

	// Capturing
	void StartCapture(void);
	bool CheckCapture(CUtlVector<CBasePlayer*> &PlayerList);
	void StopCapturing(void);

	// General
	inline void UpdateObjective(void) { NetworkStateChanged(); };

private:
	static int m_iObjectives;

	CNetworkString(m_szName, MAX_OBJECTIVE_NAME_LENGTH);
	CNetworkVar(int, m_iID);
	bool m_bUseMixedSpawns;
	bool m_bStartSpawn;
	int m_iInitialTeam;
	int m_iCapTime;
	int m_iReqPercent;
	CNetworkColor32(m_clrColor);

	CNetworkVar(bool, m_bHidden);
	bool m_bCutUp;

	CNetworkVar(int, m_iCapturedTeam);
	CNetworkVar(int, m_iCapturingTeam);
	float m_flCaptureTimer;
	CUtlVector<CBasePlayer*> m_CapturingPlayers;
	CUtlVector<CBasePlayer*> m_EnemyPlayers;

	// note: shove this in a union?
	CUtlVector<int> m_T1SpawnPoints;
	CUtlVector<int> m_T2SpawnPoints;
	CUtlVector<int> m_MixedSpawnPoints;

	CNetworkVector(m_vOrigin);
};

// ------------------------------------------------------------------------------------ //
// RecipientFilter for the Capturing Team
// ------------------------------------------------------------------------------------ //
class CObjCapturingRecipientFilter : public CRecipientFilter
{
public:
	CObjCapturingRecipientFilter(CUtlVector<CBasePlayer*> &CapturingPlayers)
	{
		MakeReliable();

		for(int i = 0; i < CapturingPlayers.Size(); i++)
		{
			AddRecipient(CapturingPlayers[i]);
		}
	}
};

#endif // INS_GAMERULES_OBJ_H
