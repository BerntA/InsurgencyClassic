//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef SCRIPT_CHECK_SHARED_H
#define SCRIPT_CHECK_SHARED_H
#ifdef _WIN32
#pragma once
#endif

//=========================================================
//=========================================================
//#define USING_SCRIPTCHECK

#define START_SCRIPTCHECK_TABLE()  const char *g_pszScriptCheckTable[] = {
#define DEFINE_DIRECTORY(dirctory) dirctory
#define END_SCRIPTCHECK_TABLE()  \
							NULL }; \

//=========================================================
//=========================================================
class CScriptCheckShared
{
public:
	CScriptCheckShared();

	void Calculate(void);
	
	unsigned int GetScriptCRC32(void) const { return m_iScriptCRC32; }

private:
	void FindDirectory(const char *pszPath);
	void AddFile(const char *pszPath);

	void FormFindString(char *pszBuffer, int iLength);

private:
	CUtlVector<unsigned int> m_ScriptsCRC32;
	unsigned int m_iScriptCRC32;
};

extern CScriptCheckShared g_ScriptCheckShared;

#endif // SCRIPT_CHECK_SHARED_H