//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_playerresource.h"
#include "c_team.h"
#include "gamestringpool.h"
#include "play_team_shared.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

const float PLAYER_RESOURCE_THINK_INTERVAL = 0.2f;

IMPLEMENT_CLIENTCLASS_DT_NOBASE(C_PlayerResource, DT_PlayerResource, CPlayerResource)
RecvPropArray3(RECVINFO_ARRAY(m_bConnected), RecvPropBool(RECVINFO(m_bConnected[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_iPing), RecvPropInt(RECVINFO(m_iPing[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_iTeam), RecvPropInt(RECVINFO(m_iTeam[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_bAlive), RecvPropBool(RECVINFO(m_bAlive[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_iHealth), RecvPropInt(RECVINFO(m_iHealth[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_iMorale), RecvPropInt(RECVINFO(m_iMorale[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_iKills), RecvPropInt(RECVINFO(m_iKills[0]))),
RecvPropArray3(RECVINFO_ARRAY(m_iDeaths), RecvPropInt(RECVINFO(m_iDeaths[0]))),
END_RECV_TABLE()

BEGIN_PREDICTION_DATA(C_PlayerResource)

DEFINE_PRED_ARRAY(m_szName, FIELD_STRING, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_bConnected, FIELD_BOOLEAN, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_iPing, FIELD_INTEGER, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_iTeam, FIELD_INTEGER, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_bAlive, FIELD_BOOLEAN, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_iHealth, FIELD_INTEGER, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_iMorale, FIELD_INTEGER, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_iKills, FIELD_INTEGER, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),
DEFINE_PRED_ARRAY(m_iDeaths, FIELD_INTEGER, MAX_PLAYERS + 1, FTYPEDESC_PRIVATE),

END_PREDICTION_DATA()

C_PlayerResource *g_PR;

IGameResources * GameResources(void) { return g_PR; }

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_PlayerResource::C_PlayerResource()
{
	memset(m_bConnected, 0, sizeof(m_bConnected));
	memset(m_iPing, 0, sizeof(m_iPing));	
	memset(m_iTeam, 0, sizeof(m_iTeam));
	memset(m_bAlive, 0, sizeof(m_bAlive));
	memset(m_iMorale, 0, sizeof(m_iMorale));
	memset(m_iKills, 0, sizeof(m_iKills));
	memset(m_iDeaths, 0, sizeof(m_iDeaths));
	memset(m_iHealth, 0, sizeof(m_iHealth));
	m_szUnconnectedName = 0;
	g_PR = this;
}

C_PlayerResource::~C_PlayerResource()
{
	g_PR = NULL;
}

void C_PlayerResource::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(gpGlobals->curtime + PLAYER_RESOURCE_THINK_INTERVAL);
	}
}

void C_PlayerResource::UpdatePlayerName(int slot)
{
	if (slot < 1 || slot > MAX_PLAYERS)
	{
		Error("UpdatePlayerName with bogus slot %d\n", slot);
		return;
	}
	if (!m_szUnconnectedName)
		m_szUnconnectedName = AllocPooledString(PLAYER_UNCONNECTED_NAME);

	player_info_t sPlayerInfo;
	if (IsConnected(slot) && engine->GetPlayerInfo(slot, &sPlayerInfo))
	{
		m_szName[slot] = AllocPooledString(sPlayerInfo.name);
	}
	else
	{
		m_szName[slot] = m_szUnconnectedName;
	}
}

void C_PlayerResource::ClientThink()
{
	BaseClass::ClientThink();

	for (int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		UpdatePlayerName(i);
	}

	SetNextClientThink(gpGlobals->curtime + PLAYER_RESOURCE_THINK_INTERVAL);
}

const char *C_PlayerResource::GetPlayerName(int iIndex)
{
	if (iIndex < 1 || iIndex > MAX_PLAYERS)
	{
		Assert(false);
		return PLAYER_ERROR_NAME;
	}

	if (!IsConnected(iIndex))
		return PLAYER_UNCONNECTED_NAME;

	// X360TBD: Network - figure out why the name isn't set
	if (!m_szName[iIndex] || !Q_stricmp(m_szName[iIndex], PLAYER_UNCONNECTED_NAME))
	{
		// If you get a full "reset" uncompressed update from server, then you can have NULLNAME show up in the scoreboard
		UpdatePlayerName(iIndex);
	}

	// This gets updated in ClientThink, so it could be up to 1 second out of date, oh well.
	return m_szName[iIndex];
}

bool C_PlayerResource::IsAlive(int iIndex)
{
	return m_bAlive[iIndex];
}

int C_PlayerResource::GetTeam(int iIndex)
{
	if (iIndex < 1 || iIndex > MAX_PLAYERS)
	{
		Assert(false);
		return 0;
	}
	else
	{
		return m_iTeam[iIndex];
	}
}

const char * C_PlayerResource::GetTeamName(int index)
{
	C_Team *team = GetGlobalTeam(index);
	return (team ? team->Get_Name() : "Unknown");
}

int C_PlayerResource::GetTeamScore(int index)
{
	C_Team *team = GetGlobalTeam(index);
	return (team ? team->GetTotalPlayerScore() : 0);
}

int C_PlayerResource::GetFrags(int index)
{
	return (IsConnected(index) ? m_iKills[index] : 0);
}

int C_PlayerResource::GetDeaths(int index)
{
	return (IsConnected(index) ? m_iDeaths[index] : 0);
}

int	C_PlayerResource::GetMorale(int index)
{
	return (IsConnected(index) ? m_iMorale[index] : 0);
}

bool C_PlayerResource::IsLocalPlayer(int index)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	return (index == pPlayer->entindex());
}

bool C_PlayerResource::IsHLTV(int index)
{
	if (!IsConnected(index))
		return false;

	player_info_t sPlayerInfo;

	if (engine->GetPlayerInfo(index, &sPlayerInfo))
	{
		return sPlayerInfo.ishltv;
	}

	return false;
}

bool C_PlayerResource::IsReplay(int index)
{
	return false;
}

bool C_PlayerResource::IsFakePlayer(int iIndex)
{
	if (!IsConnected(iIndex))
		return false;

	// Yuck, make sure it's up to date
	player_info_t sPlayerInfo;
	if (engine->GetPlayerInfo(iIndex, &sPlayerInfo))
	{
		return sPlayerInfo.fakeplayer;
	}

	return false;
}

int	C_PlayerResource::GetPing(int iIndex)
{
	return (IsConnected(iIndex) ? m_iPing[iIndex] : 0);
}

int	C_PlayerResource::GetHealth(int iIndex)
{
	if (!IsConnected(iIndex))
		return 0;

	return m_iHealth[iIndex];
}

const Color &C_PlayerResource::GetTeamColor(int index)
{
	if (index < 0 || index >= MAX_TEAMS)
	{
		Assert(false);
		static Color blah;
		return blah;
	}
	else
	{
		return INSRules()->TeamColor(index);
	}
}

bool C_PlayerResource::IsConnected(int iIndex)
{
	if (iIndex < 1 || iIndex > MAX_PLAYERS)
		return false;
	else
		return m_bConnected[iIndex];
}

bool C_PlayerResource::GetSquadData(int index, SquadData_t& SquadData)
{
	int iTeamID = GetTeamID(index);
	C_PlayTeam* pTeam = GetGlobalPlayTeam(iTeamID);
	return (pTeam ? pTeam->GetSquadData(index, SquadData) : false);
}

int C_PlayerResource::GetSquadID(int iID)
{
	SquadData_t SquadData;

	if (!GetSquadData(iID, SquadData))
		return INVALID_SQUAD;

	return SquadData.GetSquadID();
}

int C_PlayerResource::GetSlotID(int iID)
{
	SquadData_t SquadData;

	if (!GetSquadData(iID, SquadData))
		return INVALID_SQUAD;

	return SquadData.GetSlotID();
}