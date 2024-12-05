#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <stdlib.h>
#include <direct.h>
#include <shlwapi.h>

#include "hashlist.h"
extern "C"
{
#include "glob.h"
}

#pragma comment(lib,"shlwapi.lib")

using namespace hashlist;

#define BUFFER_INC 4096

bool iswhitespace(int c,bool bEscapeState)
{
	switch (c)
	{
		case ' ':
			return !bEscapeState;
		case EOF:
		case '\t':
		case '\n':
		case '\r':
			return true;
	}
	return false;
}

inline void addchar(char*& pBuffer,size_t& iSize,size_t& iLen,int c)
{
	if (iLen+1>=iSize)
	{
		pBuffer=(char*)realloc(pBuffer,iSize+=BUFFER_INC);
	}
	pBuffer[iLen++]=c;
}

static unsigned char acHexLo[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static unsigned char acHexHi[256]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,16,32,48,64,80,96,112,128,144,0,0,0,0,0,0,0,160,176,192,208,224,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,160,176,192,208,224,240,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

char* nextarg(FILE* fp)
{
	static size_t iBufferSize=BUFFER_INC;
	static char *pszBuffer=(char*)malloc(BUFFER_INC);

	if (!fp)
		return pszBuffer;

	size_t iLen=0;

	int c;
	int iQuoteState=0;
	bool bEscapeNext=false;

	while (!feof(fp)&&iswhitespace(c=fgetc(fp),false));
	
	if (!feof(fp))
	{
		do {
			switch (c)
			{
				case '"':
				case '\'':
					if (iQuoteState==c)
					{
						if (bEscapeNext)
						{
							addchar(pszBuffer,iBufferSize,iLen,c);
							bEscapeNext=false;
						}
						else
						{
							iQuoteState=0;
						}
					}
					else if (!iQuoteState&&!bEscapeNext)
					{
						iQuoteState=c;
					}
					else
					{
						if (bEscapeNext)
							bEscapeNext=false;
						addchar(pszBuffer,iBufferSize,iLen,c);
					}
					break;
				case '\\':
					if (bEscapeNext)
					{
						addchar(pszBuffer,iBufferSize,iLen,c);
						bEscapeNext=false;
					}
					else
					{
						bEscapeNext=true;
					}
					break;
				default:
					if (bEscapeNext)
					{
						if (!iswhitespace(c,iQuoteState?true:false))
							addchar(pszBuffer,iBufferSize,iLen,'\\');
						bEscapeNext=false;
					}
					addchar(pszBuffer,iBufferSize,iLen,c);
			}
		} while (!feof(fp)&&!iswhitespace(c=fgetc(fp),iQuoteState||bEscapeNext));
	}

	pszBuffer[iLen]=0;
	return pszBuffer;
}

int main(int argc, char**argv)
{
	if (argc<3)
	{
		puts("Usage: hashlist.exe <input> <output>");
		return 1;
	}

	fileinfo_t* pInfo;
	char *pBuffer, *pszCur;
	int iFlags=0;
	int iPurity=1;
	unsigned int iLen;
	bool bInFile=false;
	bool bSubDirs=true;
	FILE *fIn=fopen(argv[1],"rb"),*fOut=fopen(argv[2],"w+b");
	AllocFile(pInfo);
	while ((pszCur=nextarg(fIn))&&*pszCur)
	{
		if (stricmp("Base",pszCur)==0)
		{
			pszCur=nextarg(fIn);
			if (pszCur)
			{
				chdir(pszCur);
			}
		}
		else if (stricmp("File",pszCur)==0)
		{
			if (bInFile)
			{
				WriteFile(pBuffer,iLen,pInfo);
				fwrite(pBuffer,iLen,1,fOut);
				bInFile=false;
			}
			pszCur=nextarg(fIn);
			if (pszCur)
			{
				ResetFile(pszCur,iFlags,iPurity,pInfo);
				AddFile(pszCur,pInfo);
				printf("File added: %s\n",pszCur);
				bInFile=true;
			}
		}
		else if (stricmp("Global",pszCur)==0)
		{
			if (bInFile)
			{
				WriteFile(pBuffer,iLen,pInfo);
				fwrite(pBuffer,iLen,1,fOut);
				bInFile=false;
			}
		}
		else if (stricmp("Files",pszCur)==0)
		{
			if (bInFile)
			{
				WriteFile(pBuffer,iLen,pInfo);
				fwrite(pBuffer,iLen,1,fOut);
				bInFile=false;
			}
			pszCur=nextarg(fIn);
			if (pszCur)
			{
				glob_t Glob={0};
				printf("Processing %s (%scluding subdirectories)...\n",pszCur,bSubDirs?"in":"ex");
				glob(pszCur,GLOB_NOSORT,NULL,&Glob);
				if (Glob.gl_pathv)
				{
					int i=0;
					for (char *pszFile=Glob.gl_pathv[i];pszFile;pszFile=Glob.gl_pathv[++i])
					{
						if (PathIsDirectory(pszFile))
						{
							if (bSubDirs)
							{
								pszCur=new char[strlen(pszFile)+4];
								strcpy(pszCur,pszFile);
								strcat(pszCur,"/*");
								printf("Processing %s (including subdirectories)...\n",pszCur);
								glob(pszCur,GLOB_NOSORT|GLOB_APPEND,NULL,&Glob);
								delete [] pszCur;
							}
						}
						else
						{
							printf("File added: %s\n",pszFile);
							ResetFile(pszFile,iFlags,iPurity,pInfo);
							AddFile(pszFile,pInfo);
							WriteFile(pBuffer,iLen,pInfo);
							fwrite(pBuffer,iLen,1,fOut);
						}
					}
				}
				globfree(&Glob);
			}
		}
		else if (stricmp("AlsoHash",pszCur)==0)
		{
			pszCur=nextarg(fIn);
			if (bInFile)
			{
				if (pszCur)
				{
					if (strlen(pszCur)==144)
					{
						filesignature_t Sig;
						unsigned char* pSig=(unsigned char*)&Sig;
						for (int i=0;i<144;i+=2)
						{
							pSig[i>>1]=acHexHi[pszCur[i]]|acHexLo[pszCur[i+1]];
						}
						AddSignature(&Sig,pInfo);
					}
					else
					{
						printf("Hash should be 72 hex characters\n");
					}
				}
			}
			else
			{
				printf("AlsoHash only allowed in File context\n");
			}
		}
		else if (stricmp("AlsoFile",pszCur)==0)
		{
			pszCur=nextarg(fIn);
			if (bInFile)
			{
				if (pszCur)
				{
					AddFile(pszCur,pInfo);
				}
			}
			else
			{
				printf("AlsoFile only allowed in File context\n");
			}
		}
		else if (stricmp("IgnoreMissing",pszCur)==0)
		{
			if (bInFile)
			{
				pInfo->iFlags|=FF_IGNORE_MISSING;
			}
			else
			{
				iFlags|=FF_IGNORE_MISSING;
			}
		}
		else if (stricmp("Required",pszCur)==0)
		{
			if (bInFile)
			{
				pInfo->iFlags&=~FF_IGNORE_MISSING;
			}
			else
			{
				iFlags&=~FF_IGNORE_MISSING;
			}
		}
		else if (stricmp("Purity",pszCur)==0)
		{
			pszCur=nextarg(fIn);
			if (pszCur)
			{
				if (bInFile)
				{
					pInfo->iPurity=atoi(pszCur);
				}
				else
				{
					iPurity=atoi(pszCur);
				}
			}
		}
		else if (stricmp("Recursive",pszCur)==0)
		{
			if (bInFile)
			{
				printf("Recursive only allowed in Global context\n");
			}
			else
			{
				bSubDirs=true;
			}
		}
		else if (stricmp("FilesOnly",pszCur)==0)
		{
			if (bInFile)
			{
				printf("FilesOnly only allowed in Global context\n");
			}
			else
			{
				bSubDirs=false;
			}
		}
	}
	fclose(fOut);
	FreeFile(pInfo);

	free(nextarg(NULL));
}