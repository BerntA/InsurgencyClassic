#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	SHFILEOPSTRUCT fos={0};
	fos.wFunc=FO_DELETE;
	fos.fFlags=FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SIMPLEPROGRESS;
	fos.pFrom=__TEXT(".\\updater");
	SHFileOperation(&fos);
	return 0;
}

