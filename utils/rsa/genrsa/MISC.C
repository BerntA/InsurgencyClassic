/*
 * Platform-independent routines shared between all PuTTY programs.
 */

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include "ssh.h"

/* ----------------------------------------------------------------------
 * My own versions of malloc, realloc and free. Because I want
 * malloc and realloc to bomb out and exit the program if they run
 * out of memory, realloc to reliably call malloc if passed a NULL
 * pointer, and free to reliably do nothing if passed a NULL
 * pointer. We can also put trace printouts in, if we need to; and
 * we can also replace the allocator with an ElectricFence-like
 * one.
 */

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
		puts("Out of memory!\n");
		exit(1);
    }
    return p;
}

void *saferealloc(void *ptr, size_t n, size_t size)
{
    void *p;

    if (n > INT_MAX / size)
	p = NULL;
	else
	{
		size *= n;
		if (!ptr)
			p = malloc(size);
		else
			p = realloc(ptr, size);
    }

    if (!p)
	{
		puts("Out of memory!\n");
		exit(1);
    }
    return p;
}

void safefree(void *ptr)
{
    if (ptr)
		free(ptr);
}
