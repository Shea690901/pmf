/*  safe_malloc.c
 *	-- a package for "safe" memory allocation
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 20, 1991
 *
 *  These functions are used as an interface to the standard
 *  malloc calls. If a malloc (or realloc) call fails,
 *  the function "fatal" -- which should be defined by the
 *  application -- is called.
 *
 *  The smart thing is not only that you don't have to check
 *  every malloc, but also that you can replace this package
 *  with the debug_malloc package (debug_malloc.h and debug_malloc.c)!
 *
 */

#include "safe_malloc.h"

#ifndef NULL
#	define NULL	0
#endif

extern char *malloc(), *realloc();
extern void free();

char *safe_malloc(s)
int s;
{
    char *temp;

    temp = malloc(s);
    if (temp == NULL)
	fatal("malloc: Couldn't allocate %d bytes.", s);
    return temp;
} /* safe_malloc */

char *safe_realloc(p, s)
char *p;
int s;
{
    char *temp;

    temp = realloc(p, s);
    if (temp == NULL)
	fatal("realloc: Couldn't allocate %d bytes.", s);
    return temp;
} /* safe_realloc */

void safe_free(p)
char *p;
{
    free(p);
} /* safe_free */
