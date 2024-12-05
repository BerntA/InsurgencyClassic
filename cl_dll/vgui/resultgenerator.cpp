//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "filesystem.h"
#include <vgui/ischeme.h>
#include <vgui/isurface.h>

#include "resultgenerator.h"
#include "endgame.h"
#include "ins_gamerules.h"
#include "play_team_shared.h"
#include "team_lookup.h"
#include "basic_colors.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//=========================================================
//=========================================================
struct ResultString_t
{
	bool m_bValid;

	const char *m_pszString;
	int m_iFlags;
};

struct TagInfo_t
{
	bool m_bValid;

	short m_iType;

	short m_iOffset;
	short m_iLength;

	bool m_bUseDefault;
	char m_szDefault[MAX_TAG_DEFAULT_LENGTH];

	bool m_bUseFlags;
	char m_szFlags[MAX_TAG_FLAG_LENGTH];
};

struct TagBlock_t
{
	TagBlock_t()
	{
		for(int i = 0; i < MAX_TAG_BLOCKS; i++)
			m_Tags[i].m_bValid = false;
	}

	TagInfo_t m_Tags[MAX_TAG_BLOCKS];
};

struct TagTypeID_t
{
	TagTypeID_t()
	{
		m_bValid = false;
	}

	bool m_bValid;

	int m_iOffset;

	int m_iTagBlock;
	int m_iTagID;
};

//=========================================================
//=========================================================
class CSentanceManager
{
public:
	CSentanceManager()
	{
		int iCurrentID;

		// ENDGAME_WINCONDITION_OBJ
		iCurrentID = ENDGAME_WINCONDITION_OBJ;

		AddSentance(iCurrentID, "[balance|The](START) [wteam] [balance](MIDDLE) Secured the 'Area'%'Position'");
	}

	const char *GetSentance(int iID)
	{
		Assert(m_Sentances[iID].Count() != 0);
		return m_Sentances[iID][random->RandomInt(0, m_Sentances[iID].Count() - 1)];
	}

private:
	void AddSentance(int iID, const char *pszSentance)
	{
		Assert(Q_strlen(pszSentance) <= MAX_SENTANCE_LENGTH);
		m_Sentances[iID].AddToTail(pszSentance);
	}

private:
	CUtlVector<const char*> m_Sentances[ENDGAME_WINCONDITION_COUNT];
};

//=========================================================
//=========================================================
class CStringManager
{
private:
	struct InternalString_t
	{
		int m_iID;
		int m_iFlags;
	};

public:
	CStringManager()
	{
		int iCurrentType, iCurrentGroup;

		// ENDGAME_STRINGTYPES_ROUNDLENGTH
		iCurrentType = ENDGAME_STRINGTYPES_ROUNDLENGTH;

		// ENDGAME_ROUNDLENGTH_QUICK
		iCurrentGroup = ENDGAME_ROUNDLENGTH_QUICK;

		AddString("Quickly", TAGFLAGS_START, iCurrentType, iCurrentGroup);
		AddString("Speedily", TAGFLAGS_START, iCurrentType, iCurrentGroup);
		AddString("Rapidly", TAGFLAGS_START, iCurrentType, iCurrentGroup);

		// ENDGAME_ROUNDLENGTH_MEDIUM
		iCurrentGroup = ENDGAME_ROUNDLENGTH_MEDIUM;

		// ENDGAME_ROUNDLENGTH_LONG
		iCurrentGroup = ENDGAME_ROUNDLENGTH_LONG;

		AddString("Eventually", TAGFLAGS_START, iCurrentType, iCurrentGroup);
		AddString("Finally", (TAGFLAGS_START | TAGFLAGS_END), iCurrentType, iCurrentGroup);

		// ENDGAME_BALANCE_SUPERIOR
		iCurrentType = ENDGAME_STRINGTYPES_BALANCE;

		// ENDGAME_BALANCE_BALANCED
		iCurrentGroup = ENDGAME_BALANCE_BALANCED;

		AddString("Despite Facing Fierce Resistance", (TAGFLAGS_START | TAGFLAGS_END | TAGFLAGS_HINT_TEAM), iCurrentType, iCurrentGroup);
		AddString("After many Tough Fire-Fights", (TAGFLAGS_START | TAGFLAGS_END | TAGFLAGS_HINT_TEAM | TAGFLAGS_TEAM_NEUTRAL), iCurrentType, iCurrentGroup);
		AddString("Eventually the Stalemate was Ended with a Crushing Blow from", (TAGFLAGS_START | TAGFLAGS_HINT_TEAM), iCurrentType, iCurrentGroup);

		// ENDGAME_BALANCE_SUPERIOR
		iCurrentGroup = ENDGAME_BALANCE_SUPERIOR;

		AddString("The Superior", TAGFLAGS_START, iCurrentType, iCurrentGroup);
		AddString("Slightly Outclassed and", TAGFLAGS_MIDDLE, iCurrentType, iCurrentGroup);
		AddString("Outmaneuvered the Enemy and", TAGFLAGS_MIDDLE, iCurrentType, iCurrentGroup);

		// ENDGAME_BALANCE_OWNED
		iCurrentGroup = ENDGAME_BALANCE_OWNED;

		AddString("Crushed the Opposing Team and ", TAGFLAGS_MIDDLE, iCurrentType, iCurrentGroup);
		AddString("Owned the Opposing Team and ", TAGFLAGS_MIDDLE, iCurrentType, iCurrentGroup);
		AddString("who Owned", TAGFLAGS_END, iCurrentType, iCurrentGroup);
	}

	ResultString_t GetString(int iStringType, int iGroup, int iID)
	{
		CUtlVector<InternalString_t> &StringLookup = m_StringLookup[iStringType][iGroup];

		InternalString_t &InternalString = StringLookup[iID];

		ResultString_t String = { true, m_Strings[InternalString.m_iID], InternalString.m_iFlags };
		return String;
	}

	int GetStringCount(int iStringType, int iGroup)
	{
		CUtlVector<InternalString_t> &StringLookup = m_StringLookup[iStringType][iGroup];
		return StringLookup.Count();
	}

private:
	void AddString(const char *pszString, int iFlags, int iStringType, int iGroup)
	{
		Assert(Q_strlen(pszString) <= MAX_STRING_LENGTH);

		CUtlVector<InternalString_t> &StringLookup = m_StringLookup[iStringType][iGroup];

		InternalString_t String = { m_Strings.AddToTail(pszString), iFlags };
		StringLookup.AddToTail(String);
	}

private:
	CUtlVector<InternalString_t> m_StringLookup[ENDGAME_STRINGTYPES_COUNT][ENDGAME_STRINGTYPES_GROUP_COUNT];
	CUtlVector<const char*> m_Strings;
};

//=========================================================
//=========================================================
CResultGenerator::CResultGenerator(Panel *pParent, const char *pszPanelName)
: RichText(pParent, pszPanelName)
{
}

//=========================================================
//=========================================================
void CResultGenerator::Reset(void)
{
	ClearText();
}

//=========================================================
//=========================================================
void CResultGenerator::Generate(const Result_t &Result)
{
	// first of all, find a sentance from the manager
	//const char *pszSentance = m_pSentanceManager->GetSentance(Result.m_iType);

	// three passes, browse setup and parse
	//BrowseSentance(Result, pszSentance);

	InsertString("CResultGenerator is not Stable Enough Yet!!");
}

//=========================================================
//=========================================================
void CResultGenerator::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_TextColor = GetSchemeColor("Results.TextColor", GetFgColor(), pScheme);
}

//=========================================================
//=========================================================
void CResultGenerator::BrowseSentance(const Result_t &Result, const char *pszSentance)
{
	CUtlVector<TagBlock_t> TagBlocks;

	// now find the tags
	for(const char *i = pszSentance; *i != '\0'; i++)
	{
		if(*i == '[')
		{
			int iTagOffset = i - pszSentance;

			i++;

			TagBlock_t TagBlock;
			bool bFirstTag = true;
			bool bFinishedTagFind = false;
			int iTagID = 0;

			int iOffset = 0;

			while(!bFinishedTagFind)
			{
				// continue to find tags until they stop
				if(!bFirstTag && *(i + iOffset) != '[')
				{
					bFinishedTagFind = true;
					continue;
				}

				// .. if there are too many
				// HACKHACK: instead of "skipping" the remaining tags, just Assert
				Assert(iTagID < MAX_TAG_BLOCKS);

				bFirstTag = false;

				// sort out the name
				int iNameLength = 0;
				bool bFoundDefault = false;

				for(const char *j = i + iOffset; *j != '\0'; j++)
				{
					iNameLength++;

					if(*j == '|')
					{
						bFoundDefault= true;
						break;
					}

					if(*j == ']')
						break;
				}

				Assert(iNameLength <= MAX_TAG_NAME_LENGTH);

				char szName[MAX_TAG_NAME_LENGTH];
				Q_strncpy(szName, i + iOffset, iNameLength);

				iOffset += iNameLength;

				// use default if specified
				char szDefault[MAX_TAG_DEFAULT_LENGTH];
				szDefault[0] = '\0';

				if(bFoundDefault)
				{
					int iDefaultLength = 0;

					for(j = i + iOffset; *j != '\0'; j++)
					{
						iDefaultLength++;
						Assert(iDefaultLength <= MAX_TAG_DEFAULT_LENGTH);

						if(*j == ']')
							break;
					}

					Q_strncpy(szDefault, i + iOffset, iDefaultLength);

					iOffset += iDefaultLength;
				}

				// see if there are any flags
				bool bFoundFlags = false;
				char szFlags[MAX_TAG_FLAG_LENGTH];
				szFlags[0] = '\0';

				if(*(i + iOffset) == '(')
				{
					bFoundFlags = true;

					int iFlagLength = 0;

					for(const char *j = i + iOffset; *j != '\0'; j++)
					{
						if(*j == ')')
							break;

						iFlagLength++;
					}

					Q_strncpy(szFlags, i + iOffset + 1, iFlagLength);

					iOffset += iFlagLength;
				}

				// setup temp info for tag
				int iTagType = 0;

				if(Q_strcmp(szName, "balance") == 0)
					iTagType = TAGTYPE_BALANCE;
				else if(Q_strcmp(szName, "wteam") == 0)
					iTagType = TAGTYPE_WTEAM;
				else if(Q_strcmp(szName, "lteam") == 0)
					iTagType = TAGTYPE_LTEAM;
				else if(Q_strcmp(szName, "time") == 0)
					iTagType = TAGTYPE_TIME;
				else
					Assert(false);

				TagInfo_t CurrentTag;
				CurrentTag.m_bValid = true;
				CurrentTag.m_iType = iTagType;
				CurrentTag.m_iOffset = iTagOffset;
				CurrentTag.m_iLength = iOffset + (bFoundDefault ? 1 : 0) + (bFoundFlags ? 1 : 0);
				CurrentTag.m_bUseDefault = bFoundDefault;
				Q_strcpy(CurrentTag.m_szDefault, szDefault);
				CurrentTag.m_bUseFlags = bFoundFlags;
				Q_strcpy(CurrentTag.m_szFlags, szFlags);

				memcpy(&TagBlock.m_Tags[iTagID], &CurrentTag, sizeof(TagInfo_t));

				iTagID++;
			}

			TagBlocks.AddToTail(TagBlock);

			i += iOffset;
		}
	}

	SetupTags(Result, pszSentance, TagBlocks);
}


void CResultGenerator::SetupTags(const Result_t &Result, const char *pszSentance, CUtlVector<TagBlock_t> &TagBlocks)
{
	// now find the best tags
	CUtlVector<TagTypeID_t> TagLookups[TAGTYPE_COUNT];

	for(int i = 0; i < TagBlocks.Count(); i++)
	{
		TagBlock_t &TagBlock = TagBlocks[i];

		for(int j = 0; j < MAX_TAG_BLOCKS; j++)
		{
			if(!TagBlock.m_Tags[j].m_bValid)
				continue;

			TagTypeID_t TagTypeID;
			TagTypeID.m_bValid = true;
			TagTypeID.m_iOffset = TagBlock.m_Tags[j].m_iOffset;
			TagTypeID.m_iTagBlock = i;
			TagTypeID.m_iTagID = j;

			TagLookups[TagBlock.m_Tags[j].m_iType].AddToTail(TagTypeID);
		}
	}

	// now just select random ones *sigh*
	TagTypeID_t SelectedTagLookups[TAGTYPE_COUNT];

	for(int i = 0; i < TAGTYPE_COUNT; i++)
	{
		CUtlVector<TagTypeID_t> &TagLookupList = TagLookups[i];
		int iTagLookupListCount = TagLookupList.Count();

		if(iTagLookupListCount == 0)
			continue;

		SelectedTagLookups[i] = TagLookups[i][random->RandomInt(0, iTagLookupListCount - 1)];
	}

	TagTypeID_t *pSelectedTagLookups[TAGTYPE_COUNT];

	for(int i = 0; i < TAGTYPE_COUNT; i++)
		pSelectedTagLookups[i] = &SelectedTagLookups[i];

	// now we have our chosen tags send them off to FinalParse
	FinalParse(Result, pszSentance, TagBlocks, pSelectedTagLookups);
}

struct RandomString_t
{
	int m_iOffset;
	int m_iLength;
};

void CResultGenerator::FinalParse(const Result_t &Result, const char *pszSentance, const CUtlVector<TagBlock_t> &TagBlocks, TagTypeID_t *pSelectedTagLookups[TAGTYPE_COUNT])
{
	char szSentanceBuffer[MAX_SENTANCE_LENGTH];
	szSentanceBuffer[0] = '\0';
	int iSentanceBufferOffset = 0;

	InsertColorChange(m_TextColor);

	for(const char *i = pszSentance; *i != '\0'; i++)
	{
		if(*i == '[')
		{
			// we've found random elements, empty sentance buffer
			InsertString(szSentanceBuffer);
			szSentanceBuffer[0] = '\0';
			iSentanceBufferOffset = 0;

			int iTagOffset = i - pszSentance;

			const TagTypeID_t *pTagTypeID = NULL;

			for(int j = 0; j < TAGTYPE_COUNT; j++)
			{
				const TagTypeID_t *pCurrentTagTypeID = pSelectedTagLookups[j];

				if(!pCurrentTagTypeID->m_bValid)
					continue;

				if(iTagOffset == pCurrentTagTypeID->m_iOffset)
				{
					pTagTypeID = pCurrentTagTypeID;
					break;
				}
			}

			const TagInfo_t *pTagInfo = NULL;

			if(pTagTypeID)
			{
				pTagInfo = &TagBlocks[pTagTypeID->m_iTagBlock].m_Tags[pTagTypeID->m_iTagID];
				//char szBuffer[MAX_SENTANCE_LENGTH];

				if(pTagInfo->m_iType == TAGTYPE_WTEAM || pTagInfo->m_iType == TAGTYPE_LTEAM)
				{
					int iTeam;
					Color TeamColor;

					if(pTagInfo->m_iType == TAGTYPE_WTEAM)
					{
						iTeam = Result.m_iTeam;
						TeamColor = COLOR_GREEN;
					}
					else // TAGTYPE_LTEAM
					{
						iTeam = ((Result.m_iTeam == TEAM_ONE) ? TEAM_TWO : TEAM_ONE);
						TeamColor = COLOR_RED;
					}

					InsertColorChange(TeamColor);
					InsertString(GetGlobalPlayTeam(iTeam)->GetTeamLookup()->GetRandomName());
					InsertColorChange(m_TextColor);
				}
				else
				{
					// find a list of all the possible tags (match flags)
					int iTagFlags = 0;
					int iCurrentPos = 0;
					int iLastTag = 0;

					const char *j = pTagInfo->m_szFlags;

					for(const char *k = j; ; k++)
					{
						if(*k == '|' || *k == '\0')
						{
							char szTag[MAX_TAG_FLAG_LENGTH];
							Q_strncpy(szTag, j + iLastTag, iCurrentPos - iLastTag + 1);

							iLastTag = iCurrentPos;

							if(Q_strcmp(szTag, "START") == 0)
								iTagFlags |= TAGFLAGS_START;
							else if(Q_strcmp(szTag, "MIDDLE") == 0)
								iTagFlags |= TAGFLAGS_MIDDLE;
							else if(Q_strcmp(szTag, "END") == 0)
								iTagFlags |= TAGFLAGS_END;
							else if(Q_strcmp(szTag, "TEAM_NEUTRAL") == 0)
								iTagFlags |= TAGFLAGS_TEAM_NEUTRAL;
							else if(Q_strcmp(szTag, "HINT_TEAM") == 0)
								iTagFlags |= TAGFLAGS_HINT_TEAM;
							else
								Assert(false);

							if(*k == '\0')
								break;
						}

						iCurrentPos++;
					}

					CUtlVector<ResultString_t> m_PossibleStrings;

					int iStringType, iStringGroup;

					if(pTagInfo->m_iType == TAGTYPE_BALANCE)
					{
						iStringType = ENDGAME_STRINGTYPES_BALANCE;
						iStringGroup = Result.m_iBalance;
					}
					else // TAGTYPE_TIME
					{
						iStringType = ENDGAME_STRINGTYPES_ROUNDLENGTH;
						iStringGroup = Result.m_iLength;
					}

					int iStringCount = m_pStringManager->GetStringCount(iStringType, iStringGroup);

					bool bUseDefault = true;

					if(iStringCount != 0)
					{
						// propigate the list
						for(int j = 0; j < iStringCount; j++)
							m_PossibleStrings.AddToTail(m_pStringManager->GetString(iStringType, iStringGroup, j));

						// ensure the validity of each element
						for(int j = 0; j < iStringCount; j++)
						{
							ResultString_t &String = m_PossibleStrings[j];

							bool bValidString = true;

							while(bValidString)
							{
								if(iTagFlags & TAGFLAGS_START && !(String.m_iFlags & TAGFLAGS_START))
								{
									bValidString = false;
									continue;
								}

								if(iTagFlags & TAGFLAGS_MIDDLE && !(String.m_iFlags & TAGFLAGS_MIDDLE))
								{
									bValidString = false;
									continue;
								}

								if(iTagFlags & TAGFLAGS_END && !(String.m_iFlags & TAGFLAGS_END))
								{
									bValidString = false;
									continue;
								}

								if(iTagFlags & TAGFLAGS_TEAM_NEUTRAL && !(String.m_iFlags & TAGFLAGS_TEAM_NEUTRAL))
								{
									bValidString = false;
									continue;
								}

								break;
							}

							if(!bValidString)
								String.m_bValid = false;
						}

						CUtlVector<int> ValidPossibleStrings;

						for(int j = 0; j < iStringCount; j++)
						{
							ResultString_t &String = m_PossibleStrings[j];

							if(String.m_bValid)
								ValidPossibleStrings.AddToTail(j);
						}

						int iValidPossibleStringsCount = ValidPossibleStrings.Count();

						if(iValidPossibleStringsCount != 0)
						{
							InsertString(m_PossibleStrings[ValidPossibleStrings[random->RandomInt(0, iValidPossibleStringsCount)]].m_pszString);
						}
						else
						{
							bUseDefault = true;
						}
					}

					if(bUseDefault && pTagInfo->m_bUseDefault)
						InsertString(pTagInfo->m_szDefault);
				}
			}
			else
			{
				for(int j = 0; j < TagBlocks.Count() && !pTagInfo; j++)
				{
					const TagBlock_t &CurrentTagBlock = TagBlocks[j];

					for(int k = 0; k < MAX_TAG_BLOCKS && !pTagInfo; k++)
					{
						const TagInfo_t &CurrentTagInfo = CurrentTagBlock.m_Tags[k];

						if(iTagOffset == CurrentTagInfo.m_iOffset)
							pTagInfo = &CurrentTagInfo;
					}
				}

				Assert(pTagInfo && pTagInfo->m_bValid);
			}

			i += pTagInfo->m_iLength;

			continue;
		}
		else if(*i == '\'')
		{
			// we've found random elements, empty sentance buffer
			InsertString(szSentanceBuffer);
			szSentanceBuffer[0] = '\0';
			iSentanceBufferOffset = 0;

			// make a list of random sentances
			CUtlVector<RandomString_t> RandomStrings;
			bool bFinishedStringFind = false;
			bool bFirstString = true;

			int iOffset = 0;

			while(!bFinishedStringFind)
			{
				// continue to find tags until they stop
				if(!bFirstString && *(i + iOffset) != '%')
				{
					bFinishedStringFind = true;
					continue;
				}

				iOffset++;

				if(!bFirstString)
					iOffset++;

				bFirstString = false;

				int iLength = 0;

				for(const char *j = i + iOffset; *j != '\0'; j++)
				{
					iLength++;

					if(*j == '\'')
						break;
				}

				RandomString_t RandomString;
				RandomString.m_iOffset = iOffset;
				RandomString.m_iLength = iLength;

				RandomStrings.AddToTail(RandomString);

				iOffset += iLength;
			}

			int iRandomString = random->RandomInt(0, RandomStrings.Count() - 1);
			char szString[MAX_RANDOMSTRING_LENGTH];

			Q_strncpy(szString, i + RandomStrings[iRandomString].m_iOffset, RandomStrings[iRandomString].m_iLength);

			InsertString(szString);

			i += iOffset;

			continue;
		}

		szSentanceBuffer[iSentanceBufferOffset] = *i;
		iSentanceBufferOffset++;
	}

	// print out remaining sentance
	InsertString(szSentanceBuffer);
}