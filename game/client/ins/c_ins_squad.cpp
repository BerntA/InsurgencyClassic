//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ins_squad_shared.h"
#include "c_playerresource.h"
#include "ins_player_shared.h"
#include "clientmode_shared.h"
#include "inshud.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
const char *g_pszDefaultNames[MAX_SQUADS] = {
	"Squad 1",
	"Squad 2",
};

const char *C_INSSquad::GetDefaultName(int iSquad)
{
	if(!IsValidSquad(iSquad))
		return NULL;

	return g_pszDefaultNames[iSquad];
}

//=========================================================
//=========================================================
int C_INSSquad::GetPlayerID(int iSlotID) const
{
	if(!IsValidSlotID(iSlotID))
		return 0;

	return m_iPlayerSlots[iSlotID];
}

//=========================================================
//=========================================================
C_INSPlayer *C_INSSquad::GetPlayer(int iSlotID) const
{
	if(!IsValidSlotID(iSlotID))
		return 0;

	return ToINSPlayer(ClientEntityList().GetEnt(m_iPlayerSlots[iSlotID]));
}
