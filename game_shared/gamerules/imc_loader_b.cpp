//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// $NoKeywords: $
//
// Notes:
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "imc_config.h"
#include "imc_config.h"
#include "team_lookup.h"
#include "imc_utils.h"
#include "ins_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
/*class CLoadIMCb : public CLoadIMC
{
public:
	CLoadIMCb() { }

	virtual const char *GetFileSuffix(void) const { return "imc"; }

	virtual void Init(CIMCData *pIMCData) { }
	virtual int LoadIMC(void) { return 1; }

private:
	int GetTeamOverLoadOffset(int iTeam);

private:
	FileHandle_t m_hFile;
};

DECLARE_IMCLOADER(CLoadIMCb);

//=========================================================
//=========================================================
void CLoadIMCb::Load(void)
{
	char szIMCPath[64];
	GetIMCPath(szIMCPath, sizeof(szIMCPath));

	// use encryption one day
	m_hFile = filesystem->Open(szIMCPath, "rb", "GAME");
}

//=========================================================
//=========================================================
int CLoadIMCb::CheckIMC(void)
{
	// header check
	SIMCHeader Header;
	SIMCConfig Config;

	filesystem->Read(&Header, sizeof(SIMCHeader), m_hFile);

	if(Header.m_iSignature != IMC_SIGNATURE || Header.m_iVersion != IMC_VERSION)
		return GRWARN_IMC_INVALIDFORMAT;

	// read config and store whats needed
	filesystem->Read(&Config, sizeof(SIMCConfig), m_hFile);

	if(Config.iMapName > MAX_MAPNAME_LENGTH ||
		Config.iMapOverview > MAX_MAPOVERVIEW_LENGTH ||
		Config.iNumObjectives > MAX_OBJECTIVES ||
		Config.iNumProfiles > MAX_IMC_PROFILES)
		return GRWARN_IMC_INVALIDFORMAT;

	if(g_iCurrentProfile >= Config.iNumProfiles)
	{
		Warning("*** Warning: An Invalid Profile was Chosen, Attempting Another");
		g_iCurrentProfile = 0;
	}

	// now load objectives
	CIMCGlobalObj GlobalObjs[MAX_OBJECTIVES];
	SIMCGlobalObj GlobalObj;

	for(int i = 0; i < Config.iNumObjectives; i++)
	{
		filesystem->Read(&GlobalObj, sizeof(SIMCGlobalObj), m_hFile);

		if(GlobalObj.iName > MAX_OBJNAME_LENGTH
			|| GlobalObj.iPhonetic > ALPHABET_SIZE)
			return GRWARN_IMC_INVALIDFORMAT;
	}

	// before setting up teams, check that squads are valid
	SIMCTeam Team;

	for(int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		filesystem->Read(&Team, sizeof(SIMCTeam), m_hFile);

		if(Team.iTeam > MAX_LOOKUP_TEAMS ||
			(Team.iNumWaves < MIN_WAVES && Team.iNumWaves != UNLIMITED_SUPPLIES) || Team.iNumWaves > MAX_WAVES ||
			Team.iTimeWave < MIN_TIMEWAVE || Team.iTimeWave > MAX_TIMEWAVE)
			return GRWARN_IMC_BOUNDS;

		int iSquadCount = 0;

		int iPlatoonCommanderID = g_pTeamLookup[Team.iTeam]->FindCommanderSlot();
		bool bFoundCommander = false;

		for(int j = 0; j < MAX_SQUADS; j++)
		{
			if(Team.iSquads[j] == DISABLED_SQUAD)
				continue;

			if(Team.iSquads[j] > MAX_SQUADNAME_LENGTH)
				return GRWARN_IMC_INVALIDFORMAT;

			filesystem->Seek(m_hFile, Team.iSquads[j] + 1, FILESYSTEM_SEEK_CURRENT);

			signed char iSlots[MAX_SQUAD_SLOTS];
			filesystem->Read(iSlots, sizeof(iSlots), m_hFile);

			int iSlotCount = 0;
			
			for(int k = 0; k < MAX_SQUAD_SLOTS; k++)
			{
				int iSlotID = iSlots[k];

				if(iSlotID == DISABLED_SLOT)
					continue;

				if(iSlotID == iPlatoonCommanderID)
				{
					if(bFoundCommander)
						return GRWARN_IMC_COMMANDERS;
					else
						bFoundCommander = true;
				}

				iSlotCount++;
			}

			if(iSlotCount == 0)
				return GRWARN_IMC_NOSLOTS;

			iSquadCount++;
		}

		if(iSquadCount == 0)
			return GRWARN_IMC_NOSQUADS;
	}

	// skip past all profiles before the required
	for(int i = 0; i < g_iCurrentProfile; i++)
	{
		SIMCProfile Profile;
		filesystem->Read(&Profile, sizeof(SIMCProfile), m_hFile);

		filesystem->Seek(m_hFile, Profile.iName + 1, FILESYSTEM_SEEK_CURRENT);

		for(int j = TEAM_ONE; j <= TEAM_TWO; j++)
		{
			if(Profile.iFlags & (1<<(GetTeamOverLoadOffset(j) + PROFILE_TEAM_REINFORCEMENTS)))
				filesystem->Seek(m_hFile, sizeof(char) * 2, FILESYSTEM_SEEK_CURRENT);
		}

		filesystem->Seek(m_hFile, Profile.iNumObjectives * sizeof(SIMCProfileObj), FILESYSTEM_SEEK_CURRENT);
	}

	SIMCProfile Profile;
	filesystem->Read(&Profile, sizeof(SIMCProfile), m_hFile);

	// ... and that the profiles valid too!
	if(Profile.iMagicNumber != PROFILE_MAGICNUMBER)
		return GRWARN_IMC_INVALIDFORMAT;

	if(Profile.iGameType >= MAX_GAMETYPES ||
		Profile.iName > MAX_PROFILENAME_LENGTH ||
		Profile.iRoundLength < MIN_ROUNDLENGTH || Profile.iRoundLength > MAX_ROUNDLENGTH ||
		*//*Profile.iNumObjectives < MIN_NUMOBJECTIVES || *//*Profile.iNumObjectives > MAX_NUMOBJECTIVES )
		return GRWARN_IMC_BOUNDS;

	filesystem->Seek(m_hFile, Profile.iName + 1, FILESYSTEM_SEEK_CURRENT);

	for(int i = TEAM_ONE; i <= TEAM_TWO; i++)
	{
		if(Profile.iFlags & (1<<(GetTeamOverLoadOffset(i) + PROFILE_TEAM_REINFORCEMENTS)))
			filesystem->Seek(m_hFile, sizeof(char) * 2, FILESYSTEM_SEEK_CURRENT);
	}

	// ... and the objectives!
	CUtlVector<int> m_UsedIDs;

	for(int i = 0; i < Profile.iNumObjectives; i++)
	{
		SIMCProfileObj ProfileObj;
		filesystem->Read(&ProfileObj, sizeof(SIMCProfileObj), m_hFile);

		if(ProfileObj.iMagicNumber != PROFILE_OBJ_MAGICNUMER)
			return GRWARN_IMC_INVALIDFORMAT;

		if(ProfileObj.iID > MAX_OBJECTIVES)
			return GRWARN_IMC_INVALIDFORMAT;

		if(m_UsedIDs.HasElement(ProfileObj.iID))
			return GRWARN_IMC_INVALIDFORMAT;

		m_UsedIDs.AddToTail(ProfileObj.iID);

		if((ProfileObj.iFlags & PROFILE_OBJ_TEAMSTART_ONE) && (ProfileObj.iFlags & PROFILE_OBJ_TEAMSTART_TWO))
			return GRWARN_IMC_INVALIDFORMAT;

		if(((ProfileObj.iFlags & PROFILE_OBJ_HIDDEN) == 0) && (
			*//*ProfileObj.iReqPercent < MIN_OBJ_REQPERCENT || *//*ProfileObj.iReqPercent > MAX_OBJ_REQPERCENT ||
			ProfileObj.iCapTime < MIN_OBJ_CAPTIME || ProfileObj.iCapTime > MAX_OBJ_CAPTIME))
			return GRWARN_IMC_BOUNDS;
	}

	return 0;
}

//=========================================================
//=========================================================
void CLoadIMCb::LoadIMC(void)
{
}

//=========================================================
//=========================================================
int CLoadIMCb::GetTeamOverLoadOffset(int iTeam)
{
	Assert(iTeam == TEAM_ONE || iTeam == TEAM_TWO);

	switch(iTeam)
	{
		case TEAM_ONE: return 0; break;
		case TEAM_TWO: return PROFILE_TEAM_TEAMTWO; break;
	}

	return -1;
}*/