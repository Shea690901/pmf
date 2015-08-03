/*
 *  signal.c -- handle Unix signals
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <signal.h>
#include <setjmp.h>
#include "config.h"
#include "pmf.h"

extern jmp_buf home_sweet_home;

/*---------------------------------------------------------------------------*/

catch_signal(nr)
int nr;
{
    fatal("Arrrrgh! pmf was killed by signal %d!", nr);
} /* catch_signal */

catch_sigint(nr)
int nr;
{
    ASSERT(nr == SIGINT);
    ldisplay("\n");
    longjmp(home_sweet_home, 1);	/* 1.11: added the 0 arg */
} /* catch_signal */

setup_signals()
{

    signal(SIGINT, catch_sigint);
} /* setup_signals */
