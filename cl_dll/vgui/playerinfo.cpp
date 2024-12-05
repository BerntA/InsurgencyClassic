//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "playerinfo.h"

#include "filesystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
#define INPUT_BUFFER_LENGTH 8192
#define OUTPUT_BUFFER_LENGTH INPUT_BUFFER_LENGTH
#define TEMP_WRITE 8

//=========================================================
//=========================================================
#define PARSETYPEPATHS_SUFFIX materials/VGUI/resources/scoreboard

class CTagSetup
{
	enum TagTypes_t
	{
		TAGTYPE_INVALID = 0,
		TAGTYPE_INT,
		TAGTYPE_PERCENT,
		TAGTYPE_STRING,
		TAGTYPE_INCLUDE
	};

public:
	//=========================================================
	//=========================================================
	CTagSetup(CParseTagHelper *pParseTagHelper, const char *pszName)
	{
		m_pParseTagHelper = pParseTagHelper;

		Init(pszName);
	}

	//=========================================================
	//=========================================================
	~CTagSetup()
	{
		if(m_iType != TAGTYPE_STRING)
			return;

		delete []m_Types.m_pszString;

	}

	//=========================================================
	//=========================================================
	void SetInt(int iInt)
	{
		m_iType = TAGTYPE_INT;

		m_Types.m_iInt = iInt;
	}

	//=========================================================
	//=========================================================
	void SetPercent(float flPercentage)
	{
		m_iType = TAGTYPE_PERCENT;

		Assert(flPercentage >= 0 && flPercentage <= 100.0f);
		m_Types.m_flPercentage = flPercentage;
	}

	//=========================================================
	//=========================================================
	void SetString(const char *pszString)
	{
		m_iType = TAGTYPE_STRING;

		m_Types.m_pszString = new char[Q_strlen(pszString) + 1];
		Q_strcpy(m_Types.m_pszString, pszString);
	}

	//=========================================================
	//=========================================================
	void SetInclude(CParseTagHelper *pParser)
	{
		m_iType = TAGTYPE_INCLUDE;

		m_Types.pParser = pParser;
	}

	//=========================================================
	//=========================================================
	void Write(char *pszBuffer)
	{
		if(m_iType == TAGTYPE_INVALID)
			return;

		char szExtString[128];
		szExtString[0] = '\0';

		switch(m_iType)
		{
			case TAGTYPE_INT:
			{
				Q_snprintf(szExtString, sizeof(szExtString), "%i", m_Types.m_iInt);
				break;
			}
			case TAGTYPE_PERCENT:
			{
				Q_snprintf(szExtString, sizeof(szExtString), "%2.2f%", m_Types.m_iInt);
				break;
			}
			case TAGTYPE_STRING:
			{
				Q_strncpy(pszBuffer, m_Types.m_pszString, sizeof(szExtString));
				break;
			}
			case TAGTYPE_INCLUDE:
			{
				m_Types.pParser->HandleInclude(pszBuffer);
				break;
			}
		}

		if(m_iType != TAGTYPE_INCLUDE)
			Q_strncat(pszBuffer, szExtString, OUTPUT_BUFFER_LENGTH, COPY_ALL_CHARACTERS);
	}

	//=========================================================
	//=========================================================
	const char *GetName(void) const
	{
		return m_pszName;
    }

private:
	//=========================================================
	//=========================================================
	void Init(const char *pszName)
	{
		m_iType = INVALID_STATSTYPE;
		m_pszName = pszName;
	}

private:
	CParseTagHelper *m_pParseTagHelper;

	const char *m_pszName;
	int m_iType;

	union {
		int m_iInt;
		float m_flPercentage;
		char *m_pszString;
		CParseTagHelper *pParser;
	} m_Types;
};

//=========================================================
//=========================================================
#define TAGSIZE_LENGTH 32

//=========================================================
//=========================================================
CParseTagHelper::CParseTagHelper(CPlayerInfo *pPlayerInfo, const char *pszPath)
{
	m_pPlayerInfo = pPlayerInfo;
	m_pszPath = pszPath;
}

//=========================================================
//=========================================================
CParseTagHelper::~CParseTagHelper()
{
	m_SetupTags.PurgeAndDeleteElements();
}

//=========================================================
//=========================================================
void CParseTagHelper::AddTag(const char *pszName, int iInt)
{
	CTagSetup *pTag = new CTagSetup(this, pszName);
	pTag->SetInt(iInt);

	m_SetupTags.AddToTail(pTag);
}

//=========================================================
//=========================================================
void CParseTagHelper::AddTag(const char *pszName, float flPercent)
{
	CTagSetup *pTag = new CTagSetup(this, pszName);
	pTag->SetPercent(flPercent);

	m_SetupTags.AddToTail(pTag);
}

//=========================================================
//=========================================================
void CParseTagHelper::AddTag(const char *pszName, char *pszString)
{
	CTagSetup *pTag = new CTagSetup(this, pszName);
	pTag->SetString(pszString);

	m_SetupTags.AddToTail(pTag);
}

//=========================================================
//=========================================================
void CParseTagHelper::AddParser(const char *pszName, CParseTagHelper *pParseTagHelper)
{
	CTagSetup *pTag = new CTagSetup(this, pszName);
	pTag->SetInclude(pParseTagHelper);

	m_SetupTags.AddToTail(pTag);
}

/*//=========================================================
//=========================================================
void CParseTagHelper::SetupParse(InfoStats_t &Stats)
{
	m_pStats = &Stats;
}*/

//=========================================================
//=========================================================
void CParseTagHelper::ParseTag(char *pszBuffer, const char *pszTag)
{
	// find the tag in the list
	for(int i = 0; i < m_SetupTags.Count(); i++)
	{
		CTagSetup *pTag = m_SetupTags[i];

		if(Q_strcmp(pTag->GetName(), pszTag) != 0)
			continue;

		// ... and write
		pTag->Write(pszBuffer);

		break;
	}
}

//=========================================================
//=========================================================
void CParseTagHelper::HandleInclude(char *pszBuffer)
{
	//m_pPlayerInfo->Create(pszBuffer, GetPath(), *m_pStats, this);
}

//=========================================================
//=========================================================
const char *CParseTagHelper::GetPath(void) const
{
	Assert(m_pszPath);
	return m_pszPath;
}

//=========================================================
//=========================================================
class CParseRootTag : public CParseTagHelper
{
public:
	//=========================================================
	//=========================================================
	CParseRootTag(CPlayerInfo *pPlayerInfo)
		: CParseTagHelper(pPlayerInfo, NULL)
	{
	}

	//=========================================================
	//=========================================================
	/*virtual void SetupParse(InfoStats_t &Stats)
	{
		PlayerStats_t *pPlayerStats = (PlayerStats_t*)&Stats;

		AddTag("score", pPlayerStats->GetValue(PLAYERSTATS_KILLPTS));
	}*/
};

//=========================================================
//=========================================================
CPlayerInfo::CPlayerInfo(vgui::Panel *pParent)
	: CHTMLWindow(pParent, "PlayerInfo")
{
	m_pRootParser = new CParseRootTag(this);
}

//=========================================================
//=========================================================
CPlayerInfo::~CPlayerInfo()
{
	Reset();
}

//=========================================================
//=========================================================
void CPlayerInfo::Reset(void)
{
	for(int i = 0; i < MAX_PLAYERINFO_HANDLES; i++)
	{
		if(IsValidInfoURL(i))
			vgui::filesystem()->RemoveFile(m_PlayerInfo[i], "GAME");
	}
}

//=========================================================
//=========================================================
/*HPlayerInfo CPlayerInfo::Create(const char *pszPath, PlayerStats_t &PlayerStats)
{
	// find a free handle
	HPlayerInfo hFreeID = INVALID_PLAYERINFO_HANDLE;

	for(int i = 0; i < MAX_PLAYERINFO_HANDLES; i++)
	{
		if(!IsValidInfoURL(i))
			hFreeID = i;
	}

	if(hFreeID != INVALID_PLAYERINFO_HANDLE)
		return INVALID_PLAYERINFO_HANDLE;

	// create from path
	char szOutput[OUTPUT_BUFFER_LENGTH];
	Create(szOutput, pszPath, PlayerStats, m_pRootParser);

	// now create a temporary file for writing output
	char szRandomName[TEMP_WRITE+1];
	UTIL_RandomString(szRandomName, TEMP_WRITE);

	return hFreeID;
}*/

//=========================================================
//=========================================================
#define INVALID_TAG -1

/*bool CPlayerInfo::Create(char *pszBuffer, const char *pszPath, InfoStats_t &Stats, CParseTagHelper *pParseTagHelper)
{
	char szTag[TAGSIZE_LENGTH];

	// read input
	char szParsed[INPUT_BUFFER_LENGTH];
	szParsed[0] = '\0';

	FileHandle_t hFile = vgui::filesystem()->Open(pszPath, "r", "GAME");

	if(hFile == FILESYSTEM_INVALID_HANDLE)
		return false;

	vgui::filesystem()->Read(szParsed, INPUT_BUFFER_LENGTH, hFile);
	vgui::filesystem()->Close(hFile);

	// setup parsing
	pParseTagHelper->SetupParse(Stats);

	// parse
	for(const char *pPathOffset = pszPath; pPathOffset != '\0'; pPathOffset++)
	{
		int iTagLength = FindTag(szTag, pPathOffset);

		if(iTagLength == INVALID_TAG)
			continue;

		pParseTagHelper->ParseTag(pszBuffer, szTag);
		pPathOffset += iTagLength;
	}

	return true;
}*/

//=========================================================
//=========================================================
#define TAG_INDICATOR '#'

int CPlayerInfo::FindTag(char *pszBuffer, const char *pszSrc)
{
	// find tags
	bool bFoundPossibleTag = false;

	for(const char *pSrcOffset = pszSrc; pSrcOffset != '\0'; pSrcOffset++)
	{
		if(*pSrcOffset != TAG_INDICATOR)
			continue;

		if(!bFoundPossibleTag)
			bFoundPossibleTag = true;
		else
			goto FoundTag;
	}

	return INVALID_TAG;

FoundTag:
	int iBufferElementID = 0;

	for(const char *pSrcOffset = pszSrc; pSrcOffset != '\0'; pSrcOffset++)
	{
		if(*pSrcOffset == TAG_INDICATOR)
			break;

		if(pSrcOffset - pszSrc >= TAGSIZE_LENGTH)
			break;

		pszBuffer[iBufferElementID] = *pSrcOffset;
		iBufferElementID++;
	}

	if(iBufferElementID == 0)
		return INVALID_TAG;

	return iBufferElementID + 1;
}