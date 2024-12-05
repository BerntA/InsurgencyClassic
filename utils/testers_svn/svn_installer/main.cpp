#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <atlbase.h>
#include <atlapp.h>
#include <atldlgs.h>
#include <atlstr.h>
#include <shellapi.h>

class CPathFolderDialog : public CFolderDialogImpl<CPathFolderDialog>
{
public:
	CPathFolderDialog(LPCTSTR szPath = NULL, HWND hWndParent = NULL, LPCTSTR szTitle = NULL, UINT uFlags = BIF_RETURNONLYFSDIRS ) : CFolderDialogImpl<CPathFolderDialog>(hWndParent, szTitle, uFlags)
	{
		SetInitialFolder(szPath);
	};

	LPCTSTR Browse()
	{
		if (DoModal()==IDOK)
			return GetFolderPath();
		return NULL;
	};
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HKEY keyPath;
	DWORD nSize=0;
	RegOpenKey(HKEY_CURRENT_USER,_T("Software\\Valve\\Steam"),&keyPath);
	RegQueryValueEx(keyPath,_T("SourceModInstallPath"),NULL,NULL,NULL,&nSize);
	LPCTSTR pszInitDir=(LPCTSTR)new char[nSize];
	RegQueryValueEx(keyPath,_T("SourceModInstallPath"),NULL,NULL,(LPBYTE)pszInitDir,&nSize);
	RegCloseKey(keyPath);
	CPathFolderDialog dlgFolder(pszInitDir,NULL,_T("Where do you want to install Insurgency?"),BIF_USENEWUI|BIF_VALIDATE|BIF_BROWSEINCLUDEFILES|BIF_RETURNONLYFSDIRS);
	CString pszDir=dlgFolder.Browse();
	delete [] pszInitDir;
	if (!pszDir.IsEmpty())
	{
		pszDir+=_T("\\Insurgency\\updater");
		SHCreateDirectory(NULL,pszDir);
		SHFILEOPSTRUCT fos={0};
		fos.wFunc=FO_COPY;
		fos.fFlags=FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SIMPLEPROGRESS;
		fos.pFrom=_T("data\\updater\\*");
		fos.pTo=pszDir;
		SHFileOperation(&fos);
		ShellExecute(NULL,_T("open"),pszDir+_T("\\setup.bat"),NULL,pszDir,SW_SHOWNORMAL);
	}
	return 0;
}

