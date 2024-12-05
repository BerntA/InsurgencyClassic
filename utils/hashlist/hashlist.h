#ifndef HASHLIST_H
#define HASHLIST_H

namespace hashlist
{
	enum {
		FF_IGNORE_MISSING=0x0001,
	};

	#pragma pack(push,1)
	typedef struct filesignature_s {
		unsigned __int32 iFileSize;
		unsigned __int32 iAdler32;
		unsigned __int8  abHash[64];
	} filesignature_t;

	typedef struct fileinfo_s {
		unsigned __int16 iFlags;
		unsigned __int8  iPurity;
		unsigned __int8  iSignatures;
		unsigned __int16 iFilenameLength;
		filesignature_t* pSignatures;
		char*            pszFilename;
		char*            pBuffer;
	} fileinfo_t;
	#pragma pack(pop)

	void AllocFile(fileinfo_t*& pFileinfo);
	void ResetFile(char *pszFilename, int iFlags, int iPurity, fileinfo_t* pFileinfo);
	bool ReadNextFile(char*& pBuffer, unsigned int& iLen, fileinfo_t* pFileinfo);
	void WriteFile(char*& pBuffer, unsigned int& iLen, fileinfo_t* pFileinfo);
	void FreeFile(fileinfo_t* pFileinfo);

	bool CheckFile(char *pszFile, fileinfo_t* pFileinfo);
	void AddFile(char *pszFile, fileinfo_t* pFileinfo);
	void AddSignature(filesignature_t *pSignature, fileinfo_t* pFileinfo);
};

#endif //HASHLIST_H