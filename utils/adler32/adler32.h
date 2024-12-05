#ifndef ADLER32_H
#define ADLER32_H

//Begin with adler=1
inline unsigned __int32 adler32(unsigned __int32 adler, unsigned char *pBuffer, unsigned int iSize)
{
	const unsigned __int32 BASE=65521;
	unsigned __int32 s1 = adler&0xffff;
	unsigned __int32 s2 = (adler>>16)&0xffff;
	unsigned int n;
	
	for (n=0;n<iSize;n++)
	{
		s1=(s1+pBuffer[n])%BASE;
		s2=(s2+s1)        %BASE;
	}
	return (s2<<16)+s1;
}

#endif //ADLER32_H