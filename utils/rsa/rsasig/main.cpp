#include <stdio.h>
#include <string.h>
#include <sha512.h>
extern "C"
{
#include <bignum.h>
}

#define CHUNK_SIZE 4096
#define SIGNATURE_SIZE 256
#define ENCRYPTED_HASH_SIZE (256-64-16)

static char g_szTag[4][4]= { "INS", "RSA", "SHA", "512" };

#pragma pack(push,1)
int main(int argc, char**argv)
{
	if (argc<2)
	{
		puts("Supply a filename to sign");
		return 1;
	}
	FILE *fp,*fp2;
	fp2=fopen(argv[1],"rb");
	if (!fp2)
	{
		puts("Supply a valid filename to sign");
		return 1;
	}

	SHA512_State shaState;
	unsigned char abChunk[CHUNK_SIZE];
	size_t ulBytes;
	struct {
		char abTag[16]; //INS\0RSA\0SHA\0512\0
		unsigned char abHash[64];
		unsigned char abEncryptedHash[ENCRYPTED_HASH_SIZE];
	} abSignature;
	memcpy(abSignature.abTag,g_szTag,16);

	SHA512_Init(&shaState);
	while (!feof(fp2))
	{
		ulBytes=fread(abChunk,1,CHUNK_SIZE,fp2);
		SHA512_Bytes(&shaState, abChunk, ulBytes);
	}
	SHA512_Final(&shaState, abSignature.abHash);
	memset(&shaState,0,sizeof(shaState));

	unsigned char abPQ[128];
	unsigned char abD[128];

	fp=fopen("rsakey.n","rb");
	if (!fp)
	{
		fclose(fp2);
		puts("Can't find rsakey.n");
		return 1;
	}
	fread(abPQ,1,128,fp);
	fclose(fp);

	fp=fopen("rsakey.d","rb");
	if (!fp)
	{
		fclose(fp2);
		puts("Can't find rsakey.d");
		return 1;
	}
	fread(abD,1,128,fp);
	fclose(fp);

	Bignum mpPQ=NULL, mpD=NULL, mpHash=NULL, mpEncryptedHash=NULL;
	mpPQ=bignum_from_bytes(abPQ,128);
	mpD=bignum_from_bytes(abD,128);
	mpHash=bignum_from_bytes(abSignature.abHash,64);
	if (!(mpPQ&&mpD&&mpHash))
	{
		fclose(fp2);
		puts("Out of memory");
		return 1;
	}

	mpEncryptedHash=modpow(mpHash,mpD,mpPQ);
	if (!mpEncryptedHash)
	{
		fclose(fp2);
		puts("Out of memory");
		return 1;
	}

	memset(abSignature.abEncryptedHash,0,ENCRYPTED_HASH_SIZE);
	int iLen=bignum_bitcount(mpEncryptedHash);
	if (iLen>(ENCRYPTED_HASH_SIZE*8))
	{
		fclose(fp2);
		printf("Something went wrong, encrypted hash is more than %d bytes, shouldn't be more than 128 bytes",ENCRYPTED_HASH_SIZE);
		freebn(mpPQ);
		freebn(mpD);
		freebn(mpHash);
		freebn(mpEncryptedHash);
		return 1;
	}
	bignum_to_bytes(mpEncryptedHash,abSignature.abEncryptedHash,ENCRYPTED_HASH_SIZE);

	freebn(mpPQ);
	freebn(mpD);
	freebn(mpHash);
	freebn(mpEncryptedHash);

	char szPath[260];
	sprintf(szPath,"%s+s",argv[1]);
	fp=fopen(szPath,"w+b");
	if (!fp)
	{
		fclose(fp2);
		puts("Can't open output file");
		return 1;
	}

	fwrite(&abSignature,1,SIGNATURE_SIZE,fp);
	rewind(fp2);

	while (!feof(fp2))
	{
		ulBytes=fread(abChunk,1,CHUNK_SIZE,fp2);
		fwrite(abChunk,1,ulBytes,fp);
	}

	fclose(fp);
	fclose(fp2);

	puts("Succesfully signed!");
	return 0;
}
#pragma pack(pop)