#include <stddef.h>		       /* for size_t */

#define smalloc(z) safemalloc(z,1)
#define snmalloc safemalloc
#define sfree safefree

void *safemalloc(size_t, size_t);
void safefree(void *);

/*
 * Direct use of smalloc within the code should be avoided where
 * possible, in favour of these type-casting macros which ensure
 * you don't mistakenly allocate enough space for one sort of
 * structure and assign it to a different sort of pointer.
 */
#define snew(type) ((type *)snmalloc(1, sizeof(type)))
#define snewn(n, type) ((type *)snmalloc((n), sizeof(type)))

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#ifndef min
#define min(x,y) ( (x) < (y) ? (x) : (y) )
#endif
#ifndef max
#define max(x,y) ( (x) > (y) ? (x) : (y) )
#endif


#ifndef BIGNUM_INTERNAL
typedef void *Bignum;
#endif

typedef unsigned int word32;
typedef unsigned int uint32;

void freebn(Bignum b);
Bignum modpow(Bignum base, Bignum exp, Bignum mod);
extern Bignum Zero, One;
Bignum bignum_from_bytes(const unsigned char *data, int nbytes);
void bignum_to_bytes(Bignum bn, unsigned char *buffer, int nbytes);
int bignum_bitcount(Bignum bn);
int bignum_byte(Bignum bn, int i);
Bignum bigmod(Bignum a, Bignum b);
