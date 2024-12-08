//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "script_check_shared.h"
#include "filesystem.h"
#include "checksum_crc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=========================================================
//=========================================================
START_SCRIPTCHECK_TABLE()
	DEFINE_DIRECTORY("weapons"),
	DEFINE_DIRECTORY("pclasses"),
	//DEFINE_DIRECTORY("items"),
END_SCRIPTCHECK_TABLE()

//=========================================================
//=========================================================
CScriptCheckShared g_ScriptCheckShared;

CScriptCheckShared::CScriptCheckShared()
{
	m_iScriptCRC32 = 0;
}

//=========================================================
//=========================================================

// NOTE: one day this needs expanding so that you can have "custom script" checks
//		and also when we use encrypted files

void CScriptCheckShared::Calculate()
{
	// find all the scripts needed
	for(int i = 0;; i++)
	{
		const char *pszDirectory = g_pszScriptCheckTable[i];

		if(!pszDirectory)
			break;

		char szDirectoryPath[256];
		Q_snprintf(szDirectoryPath, sizeof(szDirectoryPath), "scripts/%s", pszDirectory);

		FindDirectory(szDirectoryPath);
	}

	// now create a buffer that contains all the checksums
	CRC32_t CRC32Value;
	CRC32_Init(&CRC32Value);

	for(int i = 0; i < m_ScriptsCRC32.Count(); i++)
	{
		CRC32_ProcessBuffer(&CRC32Value, &(m_ScriptsCRC32[i]), sizeof(unsigned int));
	}

	// now do a CRC32 checksum of the buffer
	CRC32_Final(&CRC32Value);
}

//=========================================================
//=========================================================
void CScriptCheckShared::FindDirectory(const char *pszPath)
{
	FileFindHandle_t hFile;
	const char *pszFile;

	char szFindString[256];
	szFindString[0] = '\0';

	Q_strncpy(szFindString, pszPath, sizeof(szFindString));
	FormFindString(szFindString, sizeof(szFindString));

	pszFile = filesystem->FindFirst(szFindString, &hFile);

	while(pszFile)
	{
		char szCorrectedPath[256];
		Q_snprintf(szCorrectedPath, sizeof(szCorrectedPath), "%s/%s", pszPath, pszFile);

		if(filesystem->FindIsDirectory(hFile))
			FindDirectory(szCorrectedPath);
		else
			AddFile(szCorrectedPath);

		pszFile = filesystem->FindNext(hFile);
	}

	filesystem->FindClose(hFile);
}

//=========================================================
//=========================================================
void CScriptCheckShared::AddFile(const char *pszPath)
{
	unsigned int iScriptCRC32;

	if(!UTIL_GetCRC32File(pszPath, iScriptCRC32))
		return;

	m_ScriptsCRC32.AddToTail(iScriptCRC32);
}

//=========================================================
//=========================================================
void CScriptCheckShared::FormFindString(char *pszBuffer, int iLength)
{
	Q_strncat(pszBuffer, "/*.txt", iLength, COPY_ALL_CHARACTERS);
}