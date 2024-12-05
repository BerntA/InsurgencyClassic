//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_INS_GAMERULES_OBJ_H
#define C_INS_GAMERULES_OBJ_H
#ifdef _WIN32
#pragma once
#endif

#include "client_thinklist.h"

// ------------------------------------------------------------------------------------ //
// Client Version of the Insurgency Objective
// ------------------------------------------------------------------------------------ //
class C_INSObjective : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_INSObjective, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	C_INSObjective();
	virtual ~C_INSObjective();

	void SetCapturedTeam(int iCapturedTeam) { m_iCapturedTeam = iCapturedTeam; }
	void SetCapturingTeam(int iCapturingTeam) { m_iCapturingTeam = iCapturingTeam; }

	const char *GetName(void) { return m_szName; }
	const char *GetPhonetischName(void) const;
	int GetCapturedTeam(void) const { return m_iCapturedTeam; }
	int GetCapturingTeam(void) const { return m_iCapturingTeam; }
	color32 GetColor(void) { return m_clrColor; }
	bool IsHidden(void) const { return m_bHidden; }
	Vector& GetMarker(void) { return m_vOrigin; }

private:
	char m_szName[MAX_OBJECTIVE_NAME_LENGTH];
	int m_iID;
	int m_iCapturedTeam;
	int m_iCapturingTeam;
	color32 m_clrColor;
	bool m_bHidden;
	Vector m_vOrigin;
};

extern CUtlVector<C_INSObjective*> g_Objectives;

#endif // C_INS_GAMERULES_OBJ_H
