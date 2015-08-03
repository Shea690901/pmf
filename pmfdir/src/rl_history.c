/*
 *  rl_history.c -- handling the history list using GNU readline
 *
 *	This is the interface to the GNU readline library's
 *	history functions. When installing PMF with GNU readline,
 *	this file is used instead of my own history implementation
 *	in history.c.
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 23, 1993
 *
 */

#include <stdio.h>
#include "safe_malloc.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

#include "readline/history.h"

/* Gnu history library: */
extern int history_expand();
extern HIST_ENTRY **history_list();
extern int history_base;

/*---------------------------------------------------------------------------*/

void set_max_history(n)
{
    stifle_history(n);
} /* set_max_history */

/* There must be a better way to do this! But I'm too tired now. And hungry. */
int get_history_length()
{
    int i;
    HIST_ENTRY **hpp;

    hpp = history_list();
    i = 0;
    while (*hpp) {
	++i;
	++hpp;
    }
    return i;
} /* get_history_length */

void print_history(fp, nr_lines)
FILE *fp;
int nr_lines;
{
    int i, history_length, offset;
    HIST_ENTRY **hpp;

    history_length = get_history_length();
    hpp = history_list();

    if (nr_lines <= 0 || nr_lines > history_length) {
	offset = 0;
	i = history_base;
    }
    else {
	offset = history_length - nr_lines;
	i = history_base + offset;
    }

    hpp += offset;
    while (*hpp) {
	fprintf(fp, "%5d   %s\n", i, (*hpp)->line);
	++i;
	++hpp;
    }
} /* print_history */

/*  Add an event to the history list.
 *  This function should be called after history expansion of the current command!
 */
void add_mud_history(command_line)
  char *command_line;
{

    INTERNAL_DEBUG(("add_mud_history(\"%s\")", command_line));

    /* This is a kludge to hide the password! It will probably work most of the time. */
    if (echo_is_off)
	add_history("");
    else
	add_history(command_line);
} /* add_mud_history */

/*  Expand history refrences of the type "!N" (where N is a number)
 *  or "!!" in a command line. It will return a a malloc'ed string.
 *  If it returns NULL, no history references were expanded.
 *  This function should be called before add_mud_history of the
 *  current command!
 *  Else, it returns a malloc'ed string.
 */
char *expand_history(command_line)
  char *command_line;
{
    static char *expanded_line = NULL;
    char *temp;
    int expand_res;

    USER_DEBUG(("expand_history(\"%s\")", command_line));

    if (!expanded_line)
	expanded_line = safe_malloc(2 * MAX_LINE_LENGTH + 1);
    expand_res = history_expand(command_line, &expanded_line);

    USER_DEBUG(("GNU history_expand returned %d", expand_res));

    if (expand_res == 0)
	return NULL;
    else if (expand_res == -1)
	return command_line;
    else {
	INTERNAL_DEBUG(("expanded_line == '%s'", expanded_line));
	temp = expanded_line;
	expanded_line = NULL;
	return temp;
    }
} /* expand_history */

query_max_history() {
    int the_max;

    the_max = -unstifle_history();
    stifle_history(the_max);
    return the_max;
} /* query_max_history */

query_latest_event_nr() {
    return where_history() + 1;
} /* query_latest_event_nr */
