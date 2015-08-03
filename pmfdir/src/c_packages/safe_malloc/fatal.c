/*  fatal.c
 *	-- a "fatal" error function that can be used with safe_malloc etc.
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 20, 1991
 */

#include <stdio.h>
#include "safe_malloc.h"

extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;

fatal(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    fflush(stdout);
    fprintf(stderr, "\n");
    fprintf(stderr, "\007");
    fprintf(stderr, "FATAL ERROR: ");
    fprintf(stderr, fmt, a1, a2, a3, a4, a5);
    fprintf(stderr, "\n");
    fprintf(stderr, "    Unix error number: %d (%s).\n",
	    errno, sys_errlist[errno]);
    fprintf(stderr, "    Goodbye. I am sorry that this happened. Exiting.\n");
    fflush(stderr);
    exit(1);
} /* fatal */
