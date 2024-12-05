#include <stdio.h>
#include <string.h>
#include <stdlib.h>
extern "C"
{
#include "ssh.h"

	static void no_progress(void *param, int action, int phase, int iprogress)
	{
		static int _iprogress=0,_action=0,_phase=0;
		
		if (_iprogress!=iprogress)
		{
			putchar('.');
			_iprogress=iprogress;
		}
		if (_action!=action)
		{
			putchar('+');
			_action=action;
		}
		if (_phase!=phase)
		{
			putchar('#');
			_phase=phase;
		}
	}
}

int main(void)
{
	puts("Generating...");
	struct RSAKey *rsakey = snew(struct RSAKey);
	time_t t;
	srand((unsigned int)time(&t));
	rsa_generate(rsakey, 1024, no_progress, NULL);
	FILE* fp;
	fp=fopen("rsakey.n","w+b");
	if (fp)
	{
		for (int i=0;i<128;i++)
			fputc(bignum_byte(rsakey->modulus,127-i),fp);
		fclose(fp);
	}
	fp=fopen("rsakey.d","w+b");
	if (fp)
	{
		for (int i=0;i<128;i++)
			fputc(bignum_byte(rsakey->private_exponent,127-i),fp);
		fclose(fp);
	}
	fp=fopen("rsakey.e","w+b");
	if (fp)
	{
		for (int i=0;i<128;i++)
			fputc(bignum_byte(rsakey->exponent,127-i),fp);
		fclose(fp);
	}
	sfree(rsakey);
	puts("Done... outputs (128 bytes unsigned big-endian integers):\nrsakey.n rsakey.d rsakey.e");
}