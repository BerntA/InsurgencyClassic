#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <malloc.h>

void Run(LPTSTR pszCmd)
{
	STARTUPINFO StartupInfo;
	PROCESS_INFORMATION ProcessInformation ;

	memset(&StartupInfo,0,sizeof(STARTUPINFO));
	StartupInfo.cb=sizeof(STARTUPINFO) ;

	if(CreateProcess(NULL, pszCmd, NULL, NULL, FALSE, DETACHED_PROCESS, NULL, NULL, &StartupInfo, &ProcessInformation))
	{
		WaitForSingleObject(ProcessInformation.hProcess, INFINITE);
		CloseHandle(ProcessInformation.hProcess);
		CloseHandle(ProcessInformation.hThread);
	}
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	TCHAR pszStrInstMsiW[]=_T("InstMsiW.exe");
	TCHAR pszStrInstMsiA[]=_T("InstMsiA.exe");
	TCHAR pszStrMsiExec[]=_T("msiexec.exe /i INSinst.msi");

	DWORD nSize,nTmp;
	nSize=GetFileVersionInfoSize(_T("msi.dll"),&nTmp);

	void* pBuffer=malloc(nSize);
	GetFileVersionInfo(_T("msi.dll"),0,nSize,pBuffer);

	VS_FIXEDFILEINFO* pFileVersion;
	VerQueryValue(pBuffer,_T("\\"),(void**)&pFileVersion,(PUINT)&nTmp);
	if (HIWORD(pFileVersion->dwProductVersionMS)<2)
	{
		OSVERSIONINFO OSversion;

		OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
		GetVersionEx(&OSversion);

		Run((OSversion.dwPlatformId==VER_PLATFORM_WIN32_NT)?pszStrInstMsiW:pszStrInstMsiA);
	}

	Run(pszStrMsiExec);

	free(pBuffer);
}
