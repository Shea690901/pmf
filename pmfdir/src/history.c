/*
 *  history.c -- handling the history list
 *
 *	This is the history list implementation to be used if
 *	we are NOT using the GNU readline library!
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 22, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include <ctype.h>
#include "safe_malloc.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

/* The array "history_list" contains the history list. */
struct event_struct {
    int number;
    char *the_command;
};
static int max_history = 0;
static struct event_struct *history_list = NULL;
static int nr_events = 0;
static int latest_event_nr = 0;

/*---------------------------------------------------------------------------*/

void set_max_history(n)
{
    max_history = n;
    if (nr_events > max_history) {

	copy_bytes_up(&history_list[0], &history_list[nr_events - max_history],
		      max_history * sizeof(struct event_struct));

	nr_events = max_history;
    }
    if (!history_list)
	history_list = (struct event_struct *)
	    safe_malloc(max_history * sizeof(struct event_struct));
    else
	history_list = (struct event_struct *)
	    safe_realloc(history_list, max_history * sizeof(struct event_struct));
} /* set_max_history */

/* Get an event. If not on the list, return NULL. */
char *find_event(number)
  int number;
{

    if (!history_list)
	set_max_history(DEFAULT_MAX_HISTORY);

    if (latest_event_nr - number >= nr_events || number > latest_event_nr)
	return NULL;
    return history_list[number - history_list[0].number].the_command;
} /* find_event */

int search_event(str)
char *str;
{
    int i, len;

    len = strlen(str);
    for (i = nr_events - 1; i >= 0; --i)
	if (!strncmp(str, history_list[i].the_command, len))
	    return history_list[i].number;
    return 0;
} /* search_event */

void print_history(fp, nr_lines)
int nr_lines;
{
    int i;

    if (nr_lines <= 0 || nr_lines > nr_events)
	i = 0;
    else
	i = nr_events - nr_lines;

    for (; i < nr_events; ++i)
	fprintf(fp, "%5d   %s\n",
		history_list[i].number, history_list[i].the_command);
} /* print_history */

/*  Add an event to the history list.
 *  This function should be called after history expansion of the current command!
 */
void add_mud_history(command_line)
  char *command_line;
{

    USER_DEBUG(("add_mud_history(\"%s\")", command_line));

    ++latest_event_nr;
    if (!history_list) {
	set_max_history(DEFAULT_MAX_HISTORY);
    }
    if (nr_events >= max_history) {
	safe_free(history_list[0].the_command);
	/* Move the list one step up. */
	copy_bytes_up(&history_list[0], &history_list[1],
		      (max_history - 1) * sizeof(struct event_struct));
    }
    else
	++nr_events;
    history_list[nr_events - 1].number = latest_event_nr;
    history_list[nr_events - 1].the_command = 
	strcpy(safe_malloc(strlen(command_line) + 1), command_line);

    /* This is a kludge to hide the password! It will probably work most of the time. */
    if (echo_is_off)
	history_list[nr_events - 1].the_command[0] = '\0';

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
    char first_word[MAX_WORD_LENGTH + 1];
    int first_length;
    char *this_expansion, *expanded_line;
    int event_nr;

    USER_DEBUG(("expand_history(\"%s\")", command_line));

    while (isspace(*command_line))
	++command_line;

    /* Get length of the first word in the string (up to space, tab or null). */
    first_length = 0;
    while (command_line[first_length] && !isspace(command_line[first_length]))
	++first_length;
    if (first_length > MAX_WORD_LENGTH)
	return NULL;	/* A very long first word. Surely not a history ref! */

    strncpy(first_word, command_line, first_length);
    first_word[first_length] = '\0';
    if (!strcmp(first_word, "!!"))
	event_nr = latest_event_nr;
    else if (sscanf(first_word, "!-%d", &event_nr) == 1)
	event_nr = latest_event_nr - event_nr + 1;
    else if (sscanf(first_word, "!%d", &event_nr) == 1)
	;
    else if (first_word[0] == '!') {
	event_nr = search_event(first_word + 1);
	if (!event_nr)
	    return NULL;
    }
    else
	return NULL;
    this_expansion = find_event(event_nr);
    if (!this_expansion)
	error("No such event (\"!%d\") in the history list.", event_nr);
    expanded_line = safe_malloc(strlen(this_expansion) + strlen(command_line) + 1);
	/* Yes, some wasted space at the end. */
    strcpy(expanded_line, this_expansion);
    strcat(expanded_line, " ");
    if (command_line[first_length])
	strcat(expanded_line, command_line + first_length + 1);
    USER_DEBUG(("expanded_line == '%s'", expanded_line));
    return expanded_line;
} /* expand_history */

query_max_history() {
    return max_history;
}

query_nr_events() {
    return nr_events;
}

query_latest_event_nr() {
    return latest_event_nr;
}
