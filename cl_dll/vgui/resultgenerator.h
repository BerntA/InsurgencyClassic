//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUI_RESULTGENERATOR_H
#define VGUI_RESULTGENERATOR_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/richtext.h>

class CSentanceManager;
class CStringManager;

struct Result_t;
struct TagBlock_t;
struct TagTypeID_t;

#define MAX_SENTANCE_LENGTH 1024
#define MAX_STRING_LENGTH 256

#define MAX_SENTANCES 32
#define MAX_SENTANCE_TYPES 4

#define	TAGFLAGS_START					(1<<0)
#define	TAGFLAGS_MIDDLE					(1<<1)
#define TAGFLAGS_END					(1<<2)
#define TAGFLAGS_TEAM_NEUTRAL			(1<<3)
#define TAGFLAGS_HINT_TEAM				(1<<4) // HINT

#define MAX_TAG_NAME_LENGTH 32
#define MAX_TAG_DEFAULT_LENGTH MAX_TAG_NAME_LENGTH
#define MAX_TAG_BLOCKS 4
#define MAX_TAG_FLAG_LENGTH 128

#define MAX_RANDOMSTRING_LENGTH 32

enum TagTypes_t
{
	TAGTYPE_BALANCE = 0,
	TAGTYPE_WTEAM,
	TAGTYPE_LTEAM,
	TAGTYPE_TIME,
	TAGTYPE_COUNT
};

// ------------------------------------------------------------------------------------ //
// End Game Panel
// ------------------------------------------------------------------------------------ //
class CResultGenerator : public vgui::RichText
{
	DECLARE_CLASS_SIMPLE(CResultGenerator, vgui::RichText);

public:
	CResultGenerator(Panel *pParent, const char *pszPanelName);

	void Reset(void);
	void Generate(const Result_t &Result);

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	void BrowseSentance(const Result_t &Result, const char *pszSentance);
	void SetupTags(const Result_t &Result, const char *pszSentance, CUtlVector<TagBlock_t> &TagBlocks);
	void FinalParse(const Result_t &Result, const char *pszSentance, const CUtlVector<TagBlock_t> &TagBlocks, TagTypeID_t *pSelectedTagLookups[TAGTYPE_COUNT]);

private:
	CSentanceManager *m_pSentanceManager;
	CStringManager *m_pStringManager;

	Color m_TextColor;
};

#endif // VGUI_RESULTGENERATOR_H