#include <stdio.h>
#include <string.h>
#include "rsaread.h"

#define CHUNK_SIZE 4096

int main(int argc, char**argv)
{
	if (argc<2)
	{
		puts("Supply a filename to test");
		return 1;
	}
	FILE *fp;
	fp=fopen(argv[1],"rb");
	if (!fp)
	{
		puts("Supply a valid filename to test");
		return 1;
	}

	unsigned char abChunk[CHUNK_SIZE];
	rewind(fp);
	unsigned int ulBytes=0;
	while (!feof(fp))
	{
		ulBytes+=fread(abChunk,1,CHUNK_SIZE,fp);
	}
	rewind(fp);

	unsigned char *pbData=new unsigned char[ulBytes];
	fread(pbData,1,ulBytes,fp);
	fclose(fp);

	unsigned char abPQ[128];
	unsigned char abE[128];

	fp=fopen("rsakey.n","rb");
	if (!fp)
	{
		puts("Can't find rsakey.n");
		return 1;
	}
	fread(abPQ,1,128,fp);
	fclose(fp);

	fp=fopen("rsakey.e","rb");
	if (!fp)
	{
		puts("Can't find rsakey.e");
		return 1;
	}
	fread(abE,1,128,fp);
	fclose(fp);

	unsigned char *pbFile;
	char *pszError;

	pbFile=read_file_check_signature(pbData,ulBytes,abPQ,abE,&pszError);
	if (!pbFile)
	{
		puts("Signature is invalid, the error generated was:");
		puts(pszError);
		delete [] pbData;
		return 1;
	}

	puts("Signature is valid!");

	delete [] pbData;
	return 0;
}