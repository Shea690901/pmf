/*
 *  help.c -- the help function
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <stdio.h>
#include "safe_malloc.h"
#include "str_galore.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

extern char compile_time[], version[];

/*---------------------------------------------------------------------------*/

cmd_help(orig_arg)
char *orig_arg;
{
    char *arg, *lc = NULL, *try2 = NULL, *try3 = NULL, *try4 = NULL;

    if (!orig_arg)
	orig_arg = "pmf";
    arg = copy_string(orig_arg);

    if (arg && !strcmp(try2 = lower_string_copy(arg), "version")) {
	ldisplay("\n");
	ldisplay("        This is PMF (Padrone's MudFrontend) version %s,\n", version);
	ldisplay("        by Thomas Padron-McCarthy (Email: padrone@lysator.liu.se).\n");
	ldisplay("        Compile time: %s.\n", compile_time);
#ifdef GNU_READLINE
	ldisplay("        Compiled with GNU readline.\n");
#endif
#ifdef SOUND
	ldisplay("        Compiled for using sounds on Sun SPARCstations.\n");
#endif
	ldisplay("\n");
	return;
    }
    if (lc == NULL)
	safe_free(lc);

    if (arg[0] == '/')
	arg[0] = '+';	/* Since file names cannot contain '/'! */
    else if (arg[0] == '.')
	error("Insane help request: %s", arg);

    if (   cat_help_file(arg) == -1
	&& cat_help_file(try2 = lower_string_copy(arg)) == -1
	&& cat_help_file(try3 = upper_string_copy(arg)) == -1
	&& cat_help_file(try4 = add_strings("+", arg)) == -1)
   	message("Sorry, no help available in PMF about \"%s\".\n", orig_arg);

    safe_free(arg);
    if (try2) {
	safe_free(try2);
	if (try3) {
	    safe_free(try3);
	    if (try4)
		safe_free(try4);
	}
    }
} /* cmd_help */

int cat_help_file(helpname)
char *helpname;
{
    static char *helpdir;
    char *the_path;
    FILE *fp;
    int c;

    if (helpdir == NULL)
	helpdir = make_path(SYSTEM_DIR, SYSTEM_HELP_DIR);
    the_path = make_path(helpdir, helpname);

    fp = fopen(the_path, "r");
    safe_free(the_path);

    if (fp == NULL) {
	return -1;
    }
    else {
	ldisplay("\n");
	while ((c = getc(fp)) != EOF)
	    ldisplay("%c", c);	/* Too slow? */
	ldisplay("\n");
	return 0;
    }
} /* cat_help_file */
