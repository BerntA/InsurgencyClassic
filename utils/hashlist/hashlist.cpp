#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#include <adler32.h>
#include <sha512.h>

#include "hashlist.h"

namespace hashlist
{
	void AllocFile(fileinfo_t*& pFileinfo)
	{
		pFileinfo=new fileinfo_t;
		pFileinfo->iFilenameLength=0;
		pFileinfo->iFlags=0;
		pFileinfo->iPurity=0;
		pFileinfo->iSignatures=0;
		pFileinfo->pSignatures=new filesignature_t[0];
		pFileinfo->pszFilename=new char[0];
		pFileinfo->pBuffer=new char[0];
	}

	void ResetFile(char *pszFilename, int iFlags, int iPurity, fileinfo_t* pFileinfo)
	{
		pFileinfo->iFilenameLength=strlen(pszFilename);
		pFileinfo->iFlags=iFlags;
		pFileinfo->iPurity=iPurity;
		pFileinfo->iSignatures=0;
		delete [] pFileinfo->pszFilename;
		pFileinfo->pszFilename=new char[pFileinfo->iFilenameLength+1];
		strcpy(pFileinfo->pszFilename,pszFilename);
	}

	bool ReadNextFile(char*& pBuffer, unsigned int& iLen, fileinfo_t* pFileinfo)
	{
		unsigned int iSize;
		
		iSize=2+1+1+2;
		if (iLen<iSize)
			return false;
		memcpy(pFileinfo,pBuffer,iSize);
		pBuffer+=iSize;
		iLen-=iSize;

		iSize=sizeof(filesignature_t)*pFileinfo->iSignatures;
		if (iLen<iSize)
			return false;
		delete [] pFileinfo->pSignatures;
		pFileinfo->pSignatures=new filesignature_t[pFileinfo->iSignatures];
		memcpy(pFileinfo->pSignatures,pBuffer,iSize);
		pBuffer+=iSize;
		iLen-=iSize;

		iSize=pFileinfo->iFilenameLength;
		if (iLen<iSize)
			return false;
		delete [] pFileinfo->pszFilename;
		pFileinfo->pszFilename=new char[iSize+1];
		memcpy(pFileinfo->pszFilename,pBuffer,iSize);
		pFileinfo->pszFilename[iSize]=0;
		pBuffer+=iSize;
		iLen-=iSize;

		return true;
	}


	void WriteFile(char*& pBuffer, unsigned int& iLen, fileinfo_t* pFileinfo)
	{
		unsigned int iSize;

		iLen=2+1+1+2+sizeof(filesignature_t)*pFileinfo->iSignatures+pFileinfo->iFilenameLength;
		delete [] pFileinfo->pBuffer;
		pFileinfo->pBuffer=pBuffer=new char[iLen];

		iSize=2+1+1+2;
		memcpy(pBuffer,pFileinfo,iSize);
		pBuffer+=iSize;

		iSize=sizeof(filesignature_t)*pFileinfo->iSignatures;
		memcpy(pBuffer,pFileinfo->pSignatures,iSize);
		pBuffer+=iSize;

		memcpy(pBuffer,pFileinfo->pszFilename,pFileinfo->iFilenameLength);

		pBuffer=pFileinfo->pBuffer;
	}

	void FreeFile(fileinfo_t* pFileinfo)
	{
		if (!pFileinfo)
			return;
		delete [] pFileinfo->pSignatures;
		delete [] pFileinfo->pszFilename;
		delete [] pFileinfo->pBuffer;
		delete [] pFileinfo;
	}

	#define CHUNK_SIZE 65536

	bool CheckFile(char *pszFile, fileinfo_t* pFileinfo)
	{
		struct stat sb;
		if (stat(pszFile,&sb)==-1)
			return (pFileinfo->iFlags&FF_IGNORE_MISSING)?true:false;
		FILE *fp=fopen(pszFile,"rb");
		if (!fp)
			return (pFileinfo->iFlags&FF_IGNORE_MISSING)?true:false;

		bool bOK=false;
		bool bGotAdler=false;
		unsigned __int32 adler=1;
		bool bGotSHA512=false;
		unsigned char szSHA512[64];
		for (int i=0;i<pFileinfo->iSignatures;i++)
		{
			if (pFileinfo->pSignatures[i].iFileSize==sb.st_size)
			{
				size_t iLen;
				unsigned char szBuffer[CHUNK_SIZE];
				if (!bGotAdler)
				{
					while (!feof(fp))
					{
						iLen=fread(szBuffer,1,CHUNK_SIZE,fp);
						adler=adler32(adler,szBuffer,iLen);
					}
					bGotAdler=true;
				}
				if (adler!=pFileinfo->pSignatures[i].iAdler32)
				{
					continue;
				}
				if (!bGotSHA512)
				{
					rewind(fp);
					SHA512_State shaState;
					SHA512_Init(&shaState);
					while (!feof(fp))
					{
						iLen=fread(szBuffer,1,CHUNK_SIZE,fp);
						SHA512_Bytes(&shaState, szBuffer, iLen);
					}
					SHA512_Final(&shaState, szSHA512);
					bGotSHA512=true;
				}
				if (memcmp(szSHA512,pFileinfo->pSignatures[i].abHash,64))
				{
					continue;
				}
				else
				{
					bOK=true;
					break;
				}
			}
		}

		fclose(fp);
		return bOK;
	}

	void AddFile(char *pszFile, fileinfo_t* pFileinfo)
	{
		struct stat sb;
		if (stat(pszFile,&sb)==-1)
			return;

		FILE *fp=fopen(pszFile,"rb");
		if (!fp)
			return;

		int idx=pFileinfo->iSignatures++;
		filesignature_t* pTmp=pFileinfo->pSignatures;
		pFileinfo->pSignatures=new filesignature_t[pFileinfo->iSignatures];
		memcpy(pFileinfo->pSignatures,pTmp,sizeof(filesignature_t)*(idx));
		delete [] pTmp;

		pFileinfo->pSignatures[idx].iFileSize=sb.st_size;

		size_t iLen;
		unsigned char szBuffer[CHUNK_SIZE];
		SHA512_State shaState;
		pFileinfo->pSignatures[idx].iAdler32=1;
		SHA512_Init(&shaState);
		while (!feof(fp))
		{
			iLen=fread(szBuffer,1,CHUNK_SIZE,fp);
			pFileinfo->pSignatures[idx].iAdler32=adler32(pFileinfo->pSignatures[idx].iAdler32,szBuffer,iLen);
			SHA512_Bytes(&shaState, szBuffer, iLen);
		}
		SHA512_Final(&shaState, pFileinfo->pSignatures[idx].abHash);

		fclose(fp);
	}

	void AddSignature(filesignature_t *pSignature, fileinfo_t* pFileinfo)
	{
		int idx=pFileinfo->iSignatures++;
		filesignature_t* pTmp=pFileinfo->pSignatures;
		pFileinfo->pSignatures=new filesignature_t[pFileinfo->iSignatures];
		memcpy(pFileinfo->pSignatures,pTmp,sizeof(filesignature_t)*(idx));
		delete [] pTmp;
		memcpy(pFileinfo->pSignatures+idx,pSignature,sizeof(filesignature_t));
	}
};