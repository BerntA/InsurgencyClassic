/*
 * Bignum routines for RSA and DH and stuff.
 */

#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#if defined __GNUC__ && defined __i386__
typedef unsigned long BignumInt;
typedef unsigned long long BignumDblInt;
#define BIGNUM_INT_MASK  0xFFFFFFFFUL
#define BIGNUM_TOP_BIT   0x80000000UL
#define BIGNUM_INT_BITS  32
#define MUL_WORD(w1, w2) ((BignumDblInt)w1 * w2)
#define DIVMOD_WORD(q, r, hi, lo, w) \
    __asm__("div %2" : \
	    "=d" (r), "=a" (q) : \
	    "r" (w), "d" (hi), "a" (lo))
#else
typedef unsigned short BignumInt;
typedef unsigned long BignumDblInt;
#define BIGNUM_INT_MASK  0xFFFFU
#define BIGNUM_TOP_BIT   0x8000U
#define BIGNUM_INT_BITS  16
#define MUL_WORD(w1, w2) ((BignumDblInt)w1 * w2)
#define DIVMOD_WORD(q, r, hi, lo, w) do { \
    BignumDblInt n = (((BignumDblInt)hi) << BIGNUM_INT_BITS) | lo; \
    q = n / w; \
    r = n % w; \
} while (0)
#endif

#define BIGNUM_INT_BYTES (BIGNUM_INT_BITS / 8)

typedef BignumInt *Bignum;

#define BIGNUM_INTERNAL
#include "bignum.h"

BignumInt bnZero[1] = { 0 };
BignumInt bnOne[2] = { 1, 1 };

/*
 * The Bignum format is an array of `BignumInt'. The first
 * element of the array counts the remaining elements. The
 * remaining elements express the actual number, base 2^BIGNUM_INT_BITS, _least_
 * significant digit first. (So it's trivial to extract the bit
 * with value 2^n for any n.)
 *
 * All Bignums in this module are positive. Negative numbers must
 * be dealt with outside it.
 *
 * INVARIANT: the most significant word of any Bignum must be
 * nonzero.
 */

Bignum Zero = bnZero, One = bnOne;

static Bignum newbn(int length)
{
    Bignum b = snewn(length + 1, BignumInt);
    if (!b)
		return NULL;
    memset(b, 0, (length + 1) * sizeof(*b));
    b[0] = length;
    return b;
}

void freebn(Bignum b)
{
    /*
     * Burn the evidence, just in case.
     */
	if (!b)
		return;
    memset(b, 0, sizeof(b[0]) * (b[0] + 1));
    sfree(b);
}

/*
 * Compute c = a * b.
 * Input is in the first len words of a and b.
 * Result is returned in the first 2*len words of c.
 */
static void internal_mul(BignumInt *a, BignumInt *b,
			 BignumInt *c, int len)
{
    int i, j;
    BignumDblInt t;

    for (j = 0; j < 2 * len; j++)
	c[j] = 0;

    for (i = len - 1; i >= 0; i--) {
	t = 0;
	for (j = len - 1; j >= 0; j--) {
	    t += MUL_WORD(a[i], (BignumDblInt) b[j]);
	    t += (BignumDblInt) c[i + j + 1];
	    c[i + j + 1] = (BignumInt) t;
	    t = t >> BIGNUM_INT_BITS;
	}
	c[i] = (BignumInt) t;
    }
}

static void internal_add_shifted(BignumInt *number,
				 unsigned n, int shift)
{
    int word = 1 + (shift / BIGNUM_INT_BITS);
    int bshift = shift % BIGNUM_INT_BITS;
    BignumDblInt addend;

    addend = (BignumDblInt)n << bshift;

    while (addend) {
	addend += number[word];
	number[word] = (BignumInt) addend & BIGNUM_INT_MASK;
	addend >>= BIGNUM_INT_BITS;
	word++;
    }
}

/*
 * Compute a = a % m.
 * Input in first alen words of a and first mlen words of m.
 * Output in first alen words of a
 * (of which first alen-mlen words will be zero).
 * The MSW of m MUST have its high bit set.
 * Quotient is accumulated in the `quotient' array, which is a Bignum
 * rather than the internal bigendian format. Quotient parts are shifted
 * left by `qshift' before adding into quot.
 */
static void internal_mod(BignumInt *a, int alen,
			 BignumInt *m, int mlen,
			 BignumInt *quot, int qshift)
{
    BignumInt m0, m1;
    unsigned int h;
    int i, k;

    m0 = m[0];
    if (mlen > 1)
	m1 = m[1];
    else
	m1 = 0;

    for (i = 0; i <= alen - mlen; i++) {
	BignumDblInt t;
	unsigned int q, r, c, ai1;

	if (i == 0) {
	    h = 0;
	} else {
	    h = a[i - 1];
	    a[i - 1] = 0;
	}

	if (i == alen - 1)
	    ai1 = 0;
	else
	    ai1 = a[i + 1];

	/* Find q = h:a[i] / m0 */
	if (h >= m0) {
	    /*
	     * Special case.
	     * 
	     * To illustrate it, suppose a BignumInt is 8 bits, and
	     * we are dividing (say) A1:23:45:67 by A1:B2:C3. Then
	     * our initial division will be 0xA123 / 0xA1, which
	     * will give a quotient of 0x100 and a divide overflow.
	     * However, the invariants in this division algorithm
	     * are not violated, since the full number A1:23:... is
	     * _less_ than the quotient prefix A1:B2:... and so the
	     * following correction loop would have sorted it out.
	     * 
	     * In this situation we set q to be the largest
	     * quotient we _can_ stomach (0xFF, of course).
	     */
	    q = BIGNUM_INT_MASK;
	} else {
	    DIVMOD_WORD(q, r, h, a[i], m0);

	    /* Refine our estimate of q by looking at
	     h:a[i]:a[i+1] / m0:m1 */
	    t = MUL_WORD(m1, q);
	    if (t > ((BignumDblInt) r << BIGNUM_INT_BITS) + ai1) {
		q--;
		t -= m1;
		r = (r + m0) & BIGNUM_INT_MASK;     /* overflow? */
		if (r >= (BignumDblInt) m0 &&
		    t > ((BignumDblInt) r << BIGNUM_INT_BITS) + ai1) q--;
	    }
	}

	/* Subtract q * m from a[i...] */
	c = 0;
	for (k = mlen - 1; k >= 0; k--) {
	    t = MUL_WORD(q, m[k]);
	    t += c;
	    c = t >> BIGNUM_INT_BITS;
	    if ((BignumInt) t > a[i + k])
		c++;
	    a[i + k] -= (BignumInt) t;
	}

	/* Add back m in case of borrow */
	if (c != h) {
	    t = 0;
	    for (k = mlen - 1; k >= 0; k--) {
		t += m[k];
		t += a[i + k];
		a[i + k] = (BignumInt) t;
		t = t >> BIGNUM_INT_BITS;
	    }
	    q--;
	}
	if (quot)
	    internal_add_shifted(quot, q, qshift + BIGNUM_INT_BITS * (alen - mlen - i));
    }
}

/*
 * Compute (base ^ exp) % mod.
 */
Bignum modpow(Bignum base_in, Bignum exp, Bignum mod)
{
    BignumInt *a, *b, *n, *m;
    int mshift;
    int mlen, i, j;
    Bignum base, result;

    /*
     * The most significant word of mod needs to be non-zero. It
     * should already be, but let's make sure.
     */
    assert(mod[mod[0]] != 0);

    /*
     * Make sure the base is smaller than the modulus, by reducing
     * it modulo the modulus if not.
     */
    base = bigmod(base_in, mod);
	if (!base)
		return NULL;

    /* Allocate m of size mlen, copy mod to m */
    /* We use big endian internally */
    mlen = mod[0];
    m = snewn(mlen, BignumInt);
    for (j = 0; j < mlen; j++)
	m[j] = mod[mod[0] - j];

    /* Shift m left to make msb bit set */
    for (mshift = 0; mshift < BIGNUM_INT_BITS-1; mshift++)
	if ((m[0] << mshift) & BIGNUM_TOP_BIT)
	    break;
    if (mshift) {
	for (i = 0; i < mlen - 1; i++)
	    m[i] = (m[i] << mshift) | (m[i + 1] >> (BIGNUM_INT_BITS - mshift));
	m[mlen - 1] = m[mlen - 1] << mshift;
    }

    /* Allocate n of size mlen, copy base to n */
    n = snewn(mlen, BignumInt);
    i = mlen - base[0];
    for (j = 0; j < i; j++)
	n[j] = 0;
    for (j = 0; j < base[0]; j++)
	n[i + j] = base[base[0] - j];

    /* Allocate a and b of size 2*mlen. Set a = 1 */
    a = snewn(2 * mlen, BignumInt);
    b = snewn(2 * mlen, BignumInt);
    for (i = 0; i < 2 * mlen; i++)
	a[i] = 0;
    a[2 * mlen - 1] = 1;

    /* Skip leading zero bits of exp. */
    i = 0;
    j = BIGNUM_INT_BITS-1;
    while (i < exp[0] && (exp[exp[0] - i] & (1 << j)) == 0) {
	j--;
	if (j < 0) {
	    i++;
	    j = BIGNUM_INT_BITS-1;
	}
    }

    /* Main computation */
    while (i < exp[0]) {
	while (j >= 0) {
	    internal_mul(a + mlen, a + mlen, b, mlen);
	    internal_mod(b, mlen * 2, m, mlen, NULL, 0);
	    if ((exp[exp[0] - i] & (1 << j)) != 0) {
		internal_mul(b + mlen, n, a, mlen);
		internal_mod(a, mlen * 2, m, mlen, NULL, 0);
	    } else {
		BignumInt *t;
		t = a;
		a = b;
		b = t;
	    }
	    j--;
	}
	i++;
	j = BIGNUM_INT_BITS-1;
    }

    /* Fixup result in case the modulus was shifted */
    if (mshift) {
	for (i = mlen - 1; i < 2 * mlen - 1; i++)
	    a[i] = (a[i] << mshift) | (a[i + 1] >> (BIGNUM_INT_BITS - mshift));
	a[2 * mlen - 1] = a[2 * mlen - 1] << mshift;
	internal_mod(a, mlen * 2, m, mlen, NULL, 0);
	for (i = 2 * mlen - 1; i >= mlen; i--)
	    a[i] = (a[i] >> mshift) | (a[i - 1] << (BIGNUM_INT_BITS - mshift));
    }

    /* Copy result to buffer */
    result = newbn(mod[0]);
	if (!result)
		return NULL;
    for (i = 0; i < mlen; i++)
	result[result[0] - i] = a[i + mlen];
    while (result[0] > 1 && result[result[0]] == 0)
	result[0]--;

    /* Free temporary arrays */
    for (i = 0; i < 2 * mlen; i++)
	a[i] = 0;
    sfree(a);
    for (i = 0; i < 2 * mlen; i++)
	b[i] = 0;
    sfree(b);
    for (i = 0; i < mlen; i++)
	m[i] = 0;
    sfree(m);
    for (i = 0; i < mlen; i++)
	n[i] = 0;
    sfree(n);

    freebn(base);

    return result;
}

/*
 * Compute p % mod.
 * The most significant word of mod MUST be non-zero.
 * We assume that the result array is the same size as the mod array.
 * We optionally write out a quotient if `quotient' is non-NULL.
 * We can avoid writing out the result if `result' is NULL.
 */
static void bigdivmod(Bignum p, Bignum mod, Bignum result, Bignum quotient)
{
    BignumInt *n, *m;
    int mshift;
    int plen, mlen, i, j;

    /* Allocate m of size mlen, copy mod to m */
    /* We use big endian internally */
    mlen = mod[0];
    m = snewn(mlen, BignumInt);
    for (j = 0; j < mlen; j++)
	m[j] = mod[mod[0] - j];

    /* Shift m left to make msb bit set */
    for (mshift = 0; mshift < BIGNUM_INT_BITS-1; mshift++)
	if ((m[0] << mshift) & BIGNUM_TOP_BIT)
	    break;
    if (mshift) {
	for (i = 0; i < mlen - 1; i++)
	    m[i] = (m[i] << mshift) | (m[i + 1] >> (BIGNUM_INT_BITS - mshift));
	m[mlen - 1] = m[mlen - 1] << mshift;
    }

    plen = p[0];
    /* Ensure plen > mlen */
    if (plen <= mlen)
	plen = mlen + 1;

    /* Allocate n of size plen, copy p to n */
    n = snewn(plen, BignumInt);
    for (j = 0; j < plen; j++)
	n[j] = 0;
    for (j = 1; j <= p[0]; j++)
	n[plen - j] = p[j];

    /* Main computation */
    internal_mod(n, plen, m, mlen, quotient, mshift);

    /* Fixup result in case the modulus was shifted */
    if (mshift) {
	for (i = plen - mlen - 1; i < plen - 1; i++)
	    n[i] = (n[i] << mshift) | (n[i + 1] >> (BIGNUM_INT_BITS - mshift));
	n[plen - 1] = n[plen - 1] << mshift;
	internal_mod(n, plen, m, mlen, quotient, 0);
	for (i = plen - 1; i >= plen - mlen; i--)
	    n[i] = (n[i] >> mshift) | (n[i - 1] << (BIGNUM_INT_BITS - mshift));
    }

    /* Copy result to buffer */
    if (result) {
	for (i = 1; i <= result[0]; i++) {
	    int j = plen - i;
	    result[i] = j >= 0 ? n[j] : 0;
	}
    }

    /* Free temporary arrays */
    for (i = 0; i < mlen; i++)
	m[i] = 0;
    sfree(m);
    for (i = 0; i < plen; i++)
	n[i] = 0;
    sfree(n);
}

Bignum bignum_from_bytes(const unsigned char *data, int nbytes)
{
    Bignum result;
    int w, i;

    w = (nbytes + BIGNUM_INT_BYTES - 1) / BIGNUM_INT_BYTES; /* bytes->words */

    result = newbn(w);
	if (!result)
		return NULL;
    for (i = 1; i <= w; i++)
	result[i] = 0;
    for (i = nbytes; i--;) {
	unsigned char byte = *data++;
	result[1 + i / BIGNUM_INT_BYTES] |= byte << (8*i % BIGNUM_INT_BITS);
    }

    while (result[0] > 1 && result[result[0]] == 0)
	result[0]--;
    return result;
}


void bignum_to_bytes(Bignum bn, unsigned char *buffer, int nbytes)
{
	int i;
	unsigned char *p = buffer+nbytes;

	for (i=0;i<nbytes;i++)
		*--p=bignum_byte(bn, i);
}


/*
 * Return the bit count of a bignum, for SSH-1 encoding.
 */
int bignum_bitcount(Bignum bn)
{
    int bitcount = bn[0] * BIGNUM_INT_BITS - 1;
    while (bitcount >= 0
	   && (bn[bitcount / BIGNUM_INT_BITS + 1] >> (bitcount % BIGNUM_INT_BITS)) == 0) bitcount--;
    return bitcount + 1;
}

/*
 * Return a byte from a bignum; 0 is least significant, etc.
 */
int bignum_byte(Bignum bn, int i)
{
    if (i >= BIGNUM_INT_BYTES * bn[0])
	return 0;		       /* beyond the end */
    else
	return (bn[i / BIGNUM_INT_BYTES + 1] >>
		((i % BIGNUM_INT_BYTES)*8)) & 0xFF;
}

/*
 * Simple remainder.
 */
Bignum bigmod(Bignum a, Bignum b)
{
    Bignum r = newbn(b[0]);
	if (!r)
		return NULL;
    bigdivmod(a, b, r, NULL);
    return r;
}

void *safemalloc(size_t n, size_t size)
{
    void *p;

    if (n > INT_MAX / size)
		p = NULL;
    else
	{
		size *= n;
		p = malloc(size);
    }

    if (!p)
	{
		return NULL;
    }
    return p;
}


void safefree(void *ptr)
{
    if (ptr)
		free(ptr);
}
