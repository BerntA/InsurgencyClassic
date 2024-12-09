//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "player_resource.h"
#include "ins_gamerules.h"
#include "ins_player.h"
#include <coordsize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CPlayerResource* g_pPlayerResource = NULL;

// Datatable
IMPLEMENT_SERVERCLASS_ST_NOBASE(CPlayerResource, DT_PlayerResource)
SendPropArray3(SENDINFO_ARRAY3(m_bConnected), SendPropInt(SENDINFO_ARRAY(m_bConnected), 1, SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_iPing), SendPropInt(SENDINFO_ARRAY(m_iPing), 10, SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_iTeam), SendPropInt(SENDINFO_ARRAY(m_iTeam), 2, SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_bAlive), SendPropInt(SENDINFO_ARRAY(m_bAlive), 1, SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_iHealth), SendPropInt(SENDINFO_ARRAY(m_iHealth), -1, SPROP_VARINT | SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_iMorale), SendPropInt(SENDINFO_ARRAY(m_iMorale), 13)),
SendPropArray3(SENDINFO_ARRAY3(m_iKills), SendPropInt(SENDINFO_ARRAY(m_iKills), 12, SPROP_UNSIGNED)),
SendPropArray3(SENDINFO_ARRAY3(m_iDeaths), SendPropInt(SENDINFO_ARRAY(m_iDeaths), 12, SPROP_UNSIGNED)),
END_SEND_TABLE()

BEGIN_DATADESC(CPlayerResource)
DEFINE_FUNCTION(ResourceThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(player_manager, CPlayerResource);

void CPlayerResource::Create(void)
{
	g_pPlayerResource = (CPlayerResource*)CBaseEntity::Create("player_manager", vec3_origin, vec3_angle);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::Spawn(void)
{
	for (int i = 0; i < MAX_PLAYERS + 1; i++)
	{
		m_iPing.Set(i, 0);
		m_bConnected.Set(i, 0);
		m_iTeam.Set(i, 0);
		m_bAlive.Set(i, 0);
		m_iHealth.Set(i, 100);
		m_iMorale.Set(i, 0);
		m_iKills.Set(i, 0);
		m_iDeaths.Set(i, 0);
	}

	SetThink(&CPlayerResource::ResourceThink);
	SetNextThink(gpGlobals->curtime);
	m_nUpdateCounter = 0;
}

//-----------------------------------------------------------------------------
// Purpose: The Player resource is always transmitted to clients
//-----------------------------------------------------------------------------
int CPlayerResource::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState(FL_EDICT_ALWAYS);
}

//-----------------------------------------------------------------------------
// Purpose: Wrapper for the virtual GrabPlayerData Think function
//-----------------------------------------------------------------------------
void CPlayerResource::ResourceThink(void)
{	
	UpdatePlayerData();
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::UpdatePlayerData(void)
{
	m_nUpdateCounter++;
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CINSPlayer* pPlayer = ToINSPlayer(UTIL_PlayerByIndex(i));
		if (pPlayer && pPlayer->IsConnected())
		{
			m_bConnected.Set(i, 1);
			m_iTeam.Set(i, pPlayer->GetTeamNumber());
			m_bAlive.Set(i, pPlayer->IsAlive() ? 1 : 0);
			m_iHealth.Set(i, MAX(0, pPlayer->GetHealth()));

			const CGamePlayerStats& PlayerStats = pPlayer->GetUpdateStats();
			m_iMorale.Set(i, pPlayer->GetMorale());
			m_iKills.Set(i, PlayerStats.m_iKills);
			m_iDeaths.Set(i, PlayerStats.m_iDeaths);

			// Don't update ping / packetloss everytime
			if (!(m_nUpdateCounter % 20))
			{
				// update ping all 20 think ticks = (20*0.1=2seconds)
				int ping, packetloss;
				UTIL_GetPlayerConnectionInfo(i, ping, packetloss);

				// calc avg for scoreboard so it's not so jittery
				ping = 0.8f * m_iPing.Get(i) + 0.2f * ping;

				m_iPing.Set(i, ping);
			}
		}
		else
		{
			m_bConnected.Set(i, 0);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::UpdatePlayerTeamID(int index, int iTeamID)
{
	m_iTeam.Set(index, iTeamID);
}