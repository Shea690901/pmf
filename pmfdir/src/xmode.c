/*
 *  xmode.c -- the two-X-windows mode
 *  Thanks to Markus Baumeister for doing this solution with a named pipe.
 *  And to Illuvatar for helping me fix it for HP/UX.
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "pmf.h"
#include "config.h"
#include "globals.h"

FILE *popen_x_out()
{
    FILE *xfp;
    int fo;

    make_npname(named_pipe_name);
    unlink(named_pipe_name);
    mknod(named_pipe_name, S_IFIFO|0600, 0);
    /* If you are worried, check here... */
    if ((fo = fork()) == 0) {
	/* -u is needed on hp/ux */
	execlp("xterm", "xterm", "-title", "Output from MUD", "-e",
	       X_OUTPUT_PROGRAM, "-u", named_pipe_name, NULL);
	/* This should never be reached: */
	fatal("Couldn't find the xterm program. Sorry.");
	kill(getppid(), 9);
	exit(1);
    }
    else if (fo == -1) {
	fatal("Fork failed. How horrible!");
	exit(2);
    }
    else {
	sleep(5);   /* Make sure that the reader is at the other end of the pipe! */
	xfp = fopen(named_pipe_name, "r+");
	setbuf(xfp, NULL);
	return xfp;
    }
} /* popen_x_out */
