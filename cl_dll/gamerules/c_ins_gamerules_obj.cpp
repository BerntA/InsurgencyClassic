//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//
#include "cbase.h"
#include "ins_gamerules.h"
#include "c_ins_gamerules_obj.h"
#include "c_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
IMPLEMENT_CLIENTCLASS_DT(C_INSObjective, DT_INSObjective, CINSObjective)

	RecvPropString(RECVINFO(m_szName)),
	RecvPropInt(RECVINFO(m_iID)),
	RecvPropInt(RECVINFO(m_iCapturedTeam)),
	RecvPropInt(RECVINFO(m_iCapturingTeam)),
	RecvPropInt(RECVINFO(m_clrColor)),
	RecvPropInt(RECVINFO(m_bHidden)),
	RecvPropVector(RECVINFO(m_vOrigin))

END_RECV_TABLE()

CUtlVector<C_INSObjective*> g_Objectives;

//=========================================================
//=========================================================
C_INSObjective::C_INSObjective()
{
	g_Objectives.AddToTail(this);

	memset(m_szName, 0, sizeof(m_szName));
}

//=========================================================
//=========================================================
C_INSObjective::~C_INSObjective()
{
	g_Objectives.FindAndRemove(this);
}

//=========================================================
//=========================================================
const char *C_INSObjective::GetPhonetischName(void) const
{
	Assert(m_iID <= ALPHABET_SIZE);

	return g_szPhoneticAlphabet[m_iID];
}