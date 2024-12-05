//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_PLAYERINFO_H
#define VGUI_PLAYERINFO_H
#ifdef _WIN32
#pragma once
#endif

#include "htmlwindow.h"
#include "playerstats.h"

//=========================================================
//=========================================================
#define MAX_PLAYERINFO_HANDLES MAX_PLAYERS
#define MAX_PLAYERINFO_URL 256

#define INVALID_PLAYERINFO_HANDLE -1

typedef int HPlayerInfo;
typedef char PlayerInfoURL[MAX_PLAYERINFO_URL];

class CParseTagHelper;
class CParseRootTag;

//=========================================================
//=========================================================
class CPlayerInfo : public CHTMLWindow
{
	DECLARE_CLASS_SIMPLE(CPlayerInfo, CHTMLWindow);
	friend CParseTagHelper;

public:
	CPlayerInfo(vgui::Panel *pParent);
	virtual ~CPlayerInfo();

	void Reset(void);

	HPlayerInfo Create(const char *pszPath, PlayerStats_t &PlayerStats);

private:
	//bool Create(char *pszBuffer, const char *pszPath, InfoStats_t &Stats, CParseTagHelper *pParseTagHelper);

	int FindTag(char *pszBuffer, const char *pszSrc);

	inline bool IsValidInfoURL(int iID) const { return m_PlayerInfo[iID] != '\0'; }

private:
	PlayerInfoURL m_PlayerInfo[MAX_PLAYERINFO_HANDLES];

	CParseRootTag *m_pRootParser;
};

//=========================================================
//=========================================================
class InfoStats_t;
class CTagSetup;

class CParseTagHelper
{
public:
	CParseTagHelper(CPlayerInfo *pPlayerInfo, const char *pszPath);
	~CParseTagHelper();

	void AddTag(const char *pszName, int iInt);
	void AddTag(const char *pszName, float flPercent);
	void AddTag(const char *pszName, char *pszString);

	void AddParser(const char *pszName, CParseTagHelper *pParseTagHelper);

	//virtual void SetupParse(InfoStats_t &Stats);
	void ParseTag(char *pszBuffer, const char *pszTag);
	void HandleInclude(char *pszBuffer);

	const char *GetPath(void) const;

private:
	CPlayerInfo *m_pPlayerInfo;

	const char *m_pszPath;
	//InfoStats_t *m_pStats;

	CUtlVector<CTagSetup*> m_SetupTags;
};


#endif // VGUI_PLAYERINFO_H
