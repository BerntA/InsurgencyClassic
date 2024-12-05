#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <shellapi.h>
#include <shlobj.h>
#include <list>

using namespace std;

struct gdata_s {
	char szSourceFile[MAX_PATH];
	char szSourcesDir[MAX_PATH];
	char szTargetDir[MAX_PATH];
} g_Config;

typedef list<char*> StringList;

typedef void(*DirHandler)(const char*,StringList&);

typedef struct dirdata_s {
	const char* pszDirName;
	size_t		uiDirNameLen;
	DirHandler  fpDirHandler;
	bool		bExtension;
} dirdata_t;

typedef enum {
	FT_CANCEL=0,
	FT_INVALID,
	FT_OK,
} fileselect_e;

bool strsort(char* a,char* b)
{
	return stricmp(a,b)==-1;
}

inline void error_msg(const char* msg)
{
	MessageBox(0,msg,"vmflist",MB_ICONEXCLAMATION);
}

void error_quit(const char* msg)
{
	error_msg(msg);
	exit(1);
}

fileselect_e get_file(const char* pszTitle,char* pszPath)
{
	OPENFILENAME ofn={0};
	ofn.lStructSize=sizeof(OPENFILENAME);
	pszPath[0]='\0';
	ofn.lpstrFile=pszPath;
	ofn.nMaxFile=MAX_PATH;
	ofn.lpstrTitle=pszTitle;
	ofn.lpstrFilter="Valve Map File\0*.vmf\0";
	ofn.nFilterIndex=1;
	ofn.Flags=OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST|OFN_READONLY;
	fileselect_e bRet=FT_CANCEL;
	if (GetOpenFileName(&ofn))
		bRet=FT_OK;
	return bRet;
}

int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
{
	if (uMsg==BFFM_INITIALIZED)
	{
		PostMessage(hwnd,BFFM_SETSELECTION,TRUE,lpData);
	}
	return FALSE;
}

fileselect_e get_directory(const char* pszTitle,char* pszPath,bool bNoEdit=false)
{
	BROWSEINFO bi={0};
	bi.lpszTitle=pszTitle;
	if (pszPath[0])
	{
		bi.lpfn=BrowseCallbackProc;
		bi.lParam=(LPARAM)pszPath;
	}
	bi.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_VALIDATE;
	if (bNoEdit)
		bi.ulFlags|=BIF_NONEWFOLDERBUTTON;
	LPITEMIDLIST pidl=SHBrowseForFolder(&bi);
	fileselect_e bRet=FT_CANCEL;
	if (pidl!=0)
	{
		// get the name of the folder
		if (SHGetPathFromIDList(pidl, pszPath))
			bRet=FT_OK;
		else
			bRet=FT_INVALID;

		// free memory used
		IMalloc *imalloc=0;
		if (SUCCEEDED(SHGetMalloc(&imalloc)))
		{
			imalloc->Free(pidl);
			imalloc->Release();
		}
	}
	return bRet;
}

void strreplchr(char* str,char search,char replace)
{
	char* psz;
	while (psz=strchr(str,search))
		*psz=replace;
}

void MaterialsHandler(const char* pszFile,StringList& aszFilesToCopy)
{
	static char* aszParams[]={
		"$basetexture",
		"$basetexture2",
		"$bumpmap",
		"$bumpmap2",
		"$parallaxmap",
		"$detail",
		"$envmapmask",
		"$iris",
	};
#define PARAMSTABLE_LENGTH (sizeof(aszParams)/sizeof(aszParams[0]))
	char* pszVMT=new char[strlen(pszFile)+1+4];
	strcat(strcpy(pszVMT,pszFile),".vmt");
	aszFilesToCopy.push_back(pszVMT);
	char szPath[MAX_PATH];
	strcat(strcat(strcpy(szPath,g_Config.szSourcesDir),"\\"),pszVMT);
	FILE *fp=fopen(szPath,"rt");
	if (!fp)
		return;
	char szLine[2048],szKey[2048],szValue[2048],*pszTmp,*pszAlloc;
	int i;
	while (!feof(fp))
	{
		fgets(szLine,2048,fp);
		if (strchr(szLine,'/'))
		{
			sscanf(szLine,"%s %s",szKey,szValue);
			pszTmp=szKey+1;
			szKey[strlen(pszTmp)]='\0';
			szLine[0]='\0';
			for (i=0;i<PARAMSTABLE_LENGTH;i++)
			{
				if (!stricmp(pszTmp,aszParams[i]))
				{
					pszTmp=szValue+1;
					szValue[strlen(pszTmp)]='\0';
					strcat(szLine,"materials\\");
					strcat(szLine,pszTmp);
					strcat(szLine,".vtf");
					pszAlloc=new char[strlen(szLine)+1];
					strcpy(pszAlloc,szLine);
					strreplchr(pszAlloc,'/','\\');
					aszFilesToCopy.push_back(pszAlloc);
					break;
				}
			}
		}
	}
	fclose(fp);
}

void ModelsHandler(const char* pszFile,StringList& aszFilesToCopy)
{
	char* pszAlloc;
	pszAlloc=new char[strlen(pszFile)+1+4];
	strcat(strcpy(pszAlloc,pszFile),".mdl");
	aszFilesToCopy.push_back(pszAlloc);
	pszAlloc=new char[strlen(pszFile)+1+4];
	strcat(strcpy(pszAlloc,pszFile),".phy");
	aszFilesToCopy.push_back(pszAlloc);
	pszAlloc=new char[strlen(pszFile)+1+4];
	strcat(strcpy(pszAlloc,pszFile),".vvd");
	aszFilesToCopy.push_back(pszAlloc);
	pszAlloc=new char[strlen(pszFile)+1+9];
	strcat(strcpy(pszAlloc,pszFile),".dx80.vtx");
	aszFilesToCopy.push_back(pszAlloc);
	pszAlloc=new char[strlen(pszFile)+1+9];
	strcat(strcpy(pszAlloc,pszFile),".dx90.vtx");
	aszFilesToCopy.push_back(pszAlloc);
	pszAlloc=new char[strlen(pszFile)+1+7];
	strcat(strcpy(pszAlloc,pszFile),".sw.vtx");
	aszFilesToCopy.push_back(pszAlloc);
}

void ScriptsHandler(const char* pszFile,StringList& aszFilesToCopy)
{
	char* pszAlloc=new char[strlen(pszFile)+1];
	strcpy(pszAlloc,pszFile);
	aszFilesToCopy.push_back(pszAlloc);
}

dirdata_t g_HandlerTable[] = {
	{ "materials",	9, MaterialsHandler,	false },
	{ "models",		6, ModelsHandler,		false },
	{ "scripts",	7, ScriptsHandler,		true },
};

#define HANDLERTABLE_LENGTH (sizeof(g_HandlerTable)/sizeof(g_HandlerTable[0]))

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	//initialization
	memset(&g_Config,0,sizeof(g_Config));
	fileselect_e bRet;
	//source file
	bRet=FT_INVALID;
	while (bRet==FT_INVALID)
	{
		bRet=get_file("Select source file",g_Config.szSourceFile);
		if (bRet==FT_INVALID)
			error_msg("You must select an existing file!");
	}
	if (bRet==FT_CANCEL)
		return 1;
	//sources dir
	GetEnvironmentVariable("VPROJECT",g_Config.szSourcesDir,MAX_PATH);
	bRet=FT_INVALID;
	while (bRet==FT_INVALID)
	{
		bRet=get_directory("Select sources directory",g_Config.szSourcesDir,true);
		if (bRet==FT_INVALID)
			error_msg("You must select an existing directory!");
	}
	if (bRet==FT_CANCEL)
		return 1;
	//target dir
	strcpy(g_Config.szTargetDir,g_Config.szSourceFile);
	*strrchr(g_Config.szTargetDir,'\\')='\0';
	bRet=FT_INVALID;
	while (bRet==FT_INVALID)
	{
		bRet=get_directory("Select target directory",g_Config.szTargetDir);
		if (bRet==FT_INVALID)
			error_msg("You must select an existing directory!");
	}
	if (bRet==FT_CANCEL)
		return 1;
	//read the vmf
	FILE *fp=fopen(g_Config.szSourceFile,"rt");
	if (!fp)
		error_quit("Unable to open file");
	char szLine[2048],szKey[2048],szValue[2048],*pszTmp,*pszAlloc;
	StringList aszFiles;
	while (!feof(fp))
	{
		fgets(szLine,2048,fp);
		if (strchr(szLine,'/'))
		{
			sscanf(szLine,"%s %s",szKey,szValue);
			pszTmp=szValue+1;
			szValue[strlen(pszTmp)]='\0';
			szLine[0]='\0';
			if (strstr(szKey,"material"))
				strcat(szLine,"materials\\");
			strcat(szLine,pszTmp);
			pszAlloc=new char[strlen(szLine)+1];
			strcpy(pszAlloc,szLine);
			strreplchr(pszAlloc,'/','\\');
			aszFiles.push_back(pszAlloc);
		}
	}
	fclose(fp);
	//sort, to omit duplicates
	aszFiles.sort(strsort);
	StringList::iterator iIt,iEnd=aszFiles.end();
	szLine[0]='\0';
	DirHandler fpHandler;
	StringList aszFilesToCopy;
	int i;
	for (iIt=aszFiles.begin();iIt!=iEnd;iIt++)
	{
		pszTmp=*iIt;
		if (stricmp(szLine,pszTmp))
		{
			strcpy(szLine,pszTmp);
			fpHandler=NULL;
			for (i=0;i<HANDLERTABLE_LENGTH;i++)
				if (0==strnicmp(pszTmp,g_HandlerTable[i].pszDirName,g_HandlerTable[i].uiDirNameLen))
				{
					fpHandler=g_HandlerTable[0].fpDirHandler;
					break;
				}
			if (fpHandler)
			{
				//strip extension
				if (!g_HandlerTable[i].bExtension)
				{
					pszAlloc=strchr(pszTmp,'.');
					if (pszAlloc)
						pszAlloc[0]='\0';
				}
				fpHandler(pszTmp,aszFilesToCopy);
			}
			else
			{
				pszAlloc=new char[strlen(pszTmp)+1];
				strcpy(pszAlloc,pszTmp);
				aszFilesToCopy.push_back(pszAlloc);
			}
		}
		delete [] pszTmp;
	}
	iEnd=aszFilesToCopy.end();
	SHFILEOPSTRUCT fos={0};
	fos.wFunc=FO_COPY;
	fos.fFlags=FOF_NOCONFIRMATION|FOF_NOERRORUI|FOF_SILENT;
	strcat(strcpy(szKey,g_Config.szSourcesDir),"\\");
	pszTmp=szKey+strlen(szKey);
	strcat(strcpy(szValue,g_Config.szTargetDir),"\\");
	pszAlloc=szValue+strlen(szValue);
	fos.pFrom=szKey;
	fos.pTo=szValue;
	wchar_t wszPath[MAX_PATH];
	for (iIt=aszFilesToCopy.begin();iIt!=iEnd;iIt++)
	{
		strcpy(pszTmp,*iIt);
		pszTmp[strlen(pszTmp)+1]='\0';
		strcpy(pszAlloc,pszTmp);
		pszTmp[strlen(pszAlloc)+1]='\0';
		strcpy(szLine,szValue);
		*strrchr(szLine,'\\')='\0';
		mbstowcs(wszPath,szLine,MAX_PATH);
		SHCreateDirectory(0,wszPath);
		SHFileOperation(&fos);
		delete [] *iIt;
	}
	return 0;
}