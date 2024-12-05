#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MsiQuery.h>

TCHAR* GetP(MSIHANDLE hInstall, LPCTSTR szName)
{
	TCHAR* szValueBuf=NULL;
	DWORD cchValueBuf=0;
	UINT uiStat=MsiGetProperty(hInstall, szName, TEXT(""), &cchValueBuf);
	szValueBuf=(TCHAR*)LocalAlloc(LMEM_FIXED,(++cchValueBuf)*sizeof(TCHAR));
	*szValueBuf=0;
	if (ERROR_MORE_DATA==uiStat)
	{
		MsiGetProperty(hInstall, szName, szValueBuf, &cchValueBuf);
	}
	return szValueBuf;
}

void DoReplaceSlashes(TCHAR* szText)
{
	for (size_t i=0;szText[i];i++)
	{
		if (szText[i]==L'/')
			szText[i]=L'\\';
	}
}

UINT __stdcall ReplaceSlashes(MSIHANDLE hInstall)
{
	TCHAR* pszT;
	pszT=GetP(hInstall,TEXT("STEAMEXE"));
	DoReplaceSlashes(pszT);
	MsiSetProperty(hInstall,TEXT("STEAMEXE"),pszT);
	LocalFree(pszT);
	pszT=GetP(hInstall,TEXT("STEAMDIR"));
	DoReplaceSlashes(pszT);
	MsiSetProperty(hInstall,TEXT("STEAMDIR"),pszT);
	LocalFree(pszT);
	pszT=GetP(hInstall,TEXT("INSTALLDIR"));
	DoReplaceSlashes(pszT);
	MsiSetProperty(hInstall,TEXT("INSTALLDIR"),pszT);
	LocalFree(pszT);
	return ERROR_SUCCESS;
}

