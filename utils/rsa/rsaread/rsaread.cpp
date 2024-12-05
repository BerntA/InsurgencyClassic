#include <stdio.h>
#include <string.h>
#include <sha512.h>
#include "rsaread.h"
extern "C"
{
#include <bignum.h>
}

#define SIGNATURE_SIZE 256
#define ENCRYPTED_HASH_SIZE 256-64-16

static char g_szTag[4][4]= { "INS", "RSA", "SHA", "512" };

#pragma pack(push,1)
unsigned char* read_file_check_signature(unsigned char* abData, unsigned int ulLength, const unsigned char* abPQ, const unsigned char* abE, char **pszError)
{
	*pszError=NULL;

	if (ulLength<SIGNATURE_SIZE)
	{
		*pszError="Invalid filelength";
		return NULL;
	}

	unsigned char abCheckHash[64];
	struct {
		char abTag[16]; //INS\0RSA\0SHA\0512\0
		unsigned char abHash[64];
		unsigned char abEncryptedHash[ENCRYPTED_HASH_SIZE];
	} abSignature;
	memcpy(&abSignature,abData,SIGNATURE_SIZE);
	if (memcmp(abSignature.abTag,g_szTag,16))
	{
		*pszError="Invalid tag";
		return NULL;
	}

	SHA512_Simple(abData+SIGNATURE_SIZE,ulLength-SIGNATURE_SIZE,abCheckHash);
	if (memcmp(abSignature.abHash,abCheckHash,64))
	{
		*pszError="Invalid checksum";
		return NULL;
	}

	Bignum mpPQ=NULL, mpE=NULL, mpHash=NULL, mpEncryptedHash=NULL;
	mpPQ=bignum_from_bytes(abPQ,128);
	mpE=bignum_from_bytes(abE,128);
	mpEncryptedHash=bignum_from_bytes(abSignature.abEncryptedHash,ENCRYPTED_HASH_SIZE);
	if (!(mpPQ&&mpE&&mpEncryptedHash))
	{
		*pszError="Out of memory";
		return NULL;
	}

	mpHash=modpow(mpEncryptedHash,mpE,mpPQ);

	if (!mpHash)
	{
		*pszError="Out of memory";
		return NULL;
	}

	memset(abCheckHash,0,64);
	int iLen=bignum_bitcount(mpHash);
	if (iLen>(64*8))
	{
		*pszError="Invalid signature, decryption failure";
		freebn(mpPQ);
		freebn(mpE);
		freebn(mpHash);
		freebn(mpEncryptedHash);
		return NULL;
	}
	bignum_to_bytes(mpHash,abCheckHash,64);

	freebn(mpPQ);
	freebn(mpE);
	freebn(mpHash);
	freebn(mpEncryptedHash);
	
	if (memcmp(abSignature.abHash,abCheckHash,64))
	{
		*pszError="Invalid signature, checksum failure";
		return NULL;
	}

	return abData+SIGNATURE_SIZE;
}
#pragma pack(pop)