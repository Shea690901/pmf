/*
 *  error.c -- error handling
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include <setjmp.h>
#include "config.h"
#include "pmf.h"

extern int errno;
extern char *sys_errlist[];
extern int sys_nerr;

/*---------------------------------------------------------------------------*/

/* Just a friendly message, with a nice beep. No jumping. */
message(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{

    ldisplay("\007");
    ldisplay("*** ");
    ldisplay(fmt, a1, a2, a3, a4, a5);
    ldisplay("\n");
} /* message */

/*---------------------------------------------------------------------------*/

static do_error(uflag, fmt, a1, a2, a3, a4, a5)
  int uflag;
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{

    ldisplay("\n");
    ldisplay("\007");
    ldisplay("ERROR in PMF: ");
    ldisplay(fmt, a1, a2, a3, a4, a5);
    ldisplay("\n");
    if (uflag)
	ldisplay("    Current Unix error number: %d (%s).\n",
		 errno, sys_errlist[errno]);
    goto_home_sweet_home(1);
} /* error */

/*  Print an error message, and longjmp home.
 *  Active source/send/receive commands are killed, all lines on the queue
 *  to be send to mud are removed, and we jump back to command mode.
 */
error(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    do_error(0, fmt, a1, a2, a3, a4, a5);
} /* error */

/* As above, but also print the latest Unix error. */
uerror(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    do_error(1, fmt, a1, a2, a3, a4, a5);
} /* uerror */

/*---------------------------------------------------------------------------*/

/* Fatal error. Scream and exit. */
fatal(fmt, a1, a2, a3, a4, a5)
  char *fmt, *a1, *a2, *a3, *a4, *a5;
{
    static int fatal_done = 0;

    fflush(stdout);
    fflush(stderr);

    if (fatal_done)
	exit(2);
    else
	fatal_done = 1;
    
    fprintf(stderr, "\n");
    fprintf(stderr, "\007");
    fprintf(stderr, "FATAL ERROR in PMF: ");
    fprintf(stderr, fmt, a1, a2, a3, a4, a5);
    fprintf(stderr, "\n");
    fprintf(stderr, "    Current Unix error number: %d (%s).\n",
	    errno, sys_errlist[errno]);
    fprintf(stderr, "    Goodbye. I am sorry that this happened. Exiting from PMF.\n");

    fflush(stderr);
    say_goodbye_and_exit(1);
} /* fatal */
