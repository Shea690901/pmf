/*
 *  use_ipc.c -- interface to the inter-process communication
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 22, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include <sys/param.h>
#include <ctype.h>
#include "safe_malloc.h"
#include "str_galore.h"
#include "pmf.h"
#include "config.h"
#include "globals.h"

#include "generic_fifo.h"
#include "generic_list.h"

static generic_fifo queue_to_mud = NULL;
struct queue_to_mud_entry {
    char *line_to_send, *confirm_string;
};

static int waiting_for_response;

extern char *get_line_from_mud(), *cryptsay_handle_line();
extern char compile_time[], version[];

/*---------------------------------------------------------------------------*/

init_queue_to_mud()
{
    queue_to_mud = create_generic_fifo();
    set_generic_fifo_increment(queue_to_mud, QUEUE_TO_MUD_INCREMENT);
} /* init_queue_to_mud */


struct queue_to_mud_entry *create_queue_entry(line_to_send, confirm_string)
char *line_to_send, *confirm_string;
{
    struct queue_to_mud_entry *this_queue_entry;

    this_queue_entry = (struct queue_to_mud_entry *)
	safe_malloc(sizeof(struct queue_to_mud_entry));
    this_queue_entry->line_to_send = copy_string(line_to_send);
    if (confirm_string)
	this_queue_entry->confirm_string = copy_string(confirm_string);
    else
	this_queue_entry->confirm_string = NULL;
    return this_queue_entry;
} /* create_queue_entry */


/*  Put a line on the queue of lines to be sent to the game.
 *  confirm_string is the string we will wait for as a confirmation from
 *  the game that it has received the line sent. If no confirm_string is NULL,
 *  we will not wait for confirmation.
 *  An empty string means accept any output from the mud game as confirmation.
 */
queue_mudline(line_to_send, confirm_string)
char *line_to_send, *confirm_string;
{
    struct queue_to_mud_entry *this_queue_entry;

    ASSERT(line_to_send);
    USER_DEBUG(("Queuing: \"%s\", \"%s\"\n", line_to_send, confirm_string ? confirm_string : "NULL"));

    this_queue_entry = create_queue_entry(line_to_send, confirm_string);
    queue_on_generic_fifo(queue_to_mud, this_queue_entry);
} /* queue_mudline */


/*  Like queue_mudline, but put the command FIRST in the queue!
 */
express_queue_mudline(line_to_send, confirm_string)
char *line_to_send, *confirm_string;
{
    struct queue_to_mud_entry *this_queue_entry;

    ASSERT(line_to_send);
    USER_DEBUG(("Express-queuing: \"%s\", \"%s\"\n", line_to_send, confirm_string ? confirm_string : "NULL"));

    this_queue_entry = create_queue_entry(line_to_send, confirm_string);
    add_first_to_generic_list(get_list_inside_generic_fifo(queue_to_mud),
			      this_queue_entry);
} /* express_queue_mudline */


/*  This function checks if the (probably just received) line_from_mud
 *  matches the (probably the current, global) confirm_string.
 *  Returns 1 or 0.
 */
int match_confirm_string(line_from_mud, confirm_string, confirm_string_length)
char *line_from_mud, *confirm_string;
int confirm_string_length;
{
    /* The words returned by dollar_match are never used. */
    char *mudline_words[MAX_WORDS_PER_LINE];
    char mudline_words_buffer[2 * MAX_LINE_LENGTH];

    INTERNAL_DEBUG(("match_confirm_string('%s', '%s', %d)",
		    line_from_mud, confirm_string, confirm_string_length));
    return    (!strncmp(confirm_string, line_from_mud, confirm_string_length))
           || (dollar_match(confirm_string, line_from_mud,
			mudline_words, mudline_words_buffer) != -1);
} /* match_confirm_string */


/*  This function should be called periodically (at least a few times each
 *  second). It takes care of all the communication with the mud process.
 *  Never do any read or write calls or any other operations on the socket
 *  except from this function! Returns 0 if nothing happened.
 *  Also, if nothing happened we sleep for a while.
 */
int communicate_with_mud()
{
    struct queue_to_mud_entry *to_head;
    char *line_from_mud, *line_to_mud;
    /* These variables are used to save the state between calls: */
    /* static int waiting_for_response = 0; -- moved outside! */
    static char *confirm_string[MAX_CONFIRM_STRING_LENGTH + 1];
    static int confirm_string_length;
    int retval, was_gag;
    char *nl_p;

#ifdef GNU_READLINE
    readline_prompt[0] = readline_prompt_first_char;
#endif

    retval = 0;

    line_from_mud = get_line_from_mud();
    /* It is possible that the "line_from_mud" is just a part of a line. */
    if (line_from_mud) {
	USER_DEBUG(("communicate_with_mud: got a line: \"%s\"", line_from_mud));
	if (   waiting_for_response
	    && match_confirm_string(line_from_mud, confirm_string, confirm_string_length)) {
	    USER_DEBUG(("communicate_with_mud: got confirm_string \"%s\" in line \"%s\"", confirm_string, line_from_mud));
	    waiting_for_response = 0;
	}

	/* We use the telnet codes too, but do this kludge too: */
	if (!strcmp(line_from_mud, "Password: "))
	    echo_off();

	IPC_DEBUG(("line = \"%s\", strcmp(line, \"Password: \") = %d",
		   line_from_mud, strcmp(line_from_mud, "Password: ")));

	if (receiving) {
	    if (match_confirm_string(line_from_mud, receive_stop_string, strlen(receive_stop_string))) {
		USER_DEBUG(("communicate_with_mud: got receive_stop_string \"%s\" in line \"%s\"",
			    receive_stop_string, line_from_mud));
		stop_receiving();
	    }
	    else {
		fprintf(the_open_file, "%s", line_from_mud);
	    }
	}

	/* If doing a "getfile": save the text, newlines NOT stripped! */
	if (getfiling)
	    getfile_got_text(line_from_mud);

	/* Is there a newline character at the end of the line? Remove it! */
	nl_p = &line_from_mud[strlen(line_from_mud) - 1];
	if (*nl_p == '\n') {
	    *nl_p = '\0';
	    ++nr_received_lines;
	}
	else
	    nl_p = NULL;

	/* Checking for gagging: newlines stripped! */
	was_gag = is_gag_line(line_from_mud) && !fighting_line(line_from_mud);

	/* The secret stuff! */
	if (cryptkey) {
	    char *temp;
	    temp = cryptsay_handle_line(line_from_mud);
	    /* safe_free(line_from_mud); -- this is done in cryptsay_handle_line! */
	    line_from_mud = temp;
	}

	if ((getfiling || receiving) && !show_receive) {
	    if (nl_p) {
		/* Lars has no line buffering on the sockets, even when inside ed. */
		ldisplay(".");
		fflush(stdout);
	    }
	}
	else if (!was_gag) {
	    rdisplay("%s", line_from_mud);	/* The line can contain %'s! */
	    if (nl_p)
		rdisplay("\n");
	}
	retval += 1;

	/*  You can remove these three automatic actions,
	 *  but try to avoid doing that.
	 *  At least keep the version query! /Padrone
	 */

	/* Very hard-coded: if PMF_QUERY_VERSION is sent, we reply! */
	if (!strcmp(line_from_mud, "PMF_QUERY_VERSION")) {
	    char buf[MAX_LINE_LENGTH + 1];
	    sprintf(buf, "say pmf version \"%s\" compiled %s\n",
		    version, compile_time);
	    message("Your PMF just answered a PMF_QUERY_VERSION message.");
	    queue_mudline(buf, "");
	}
	/* And another one: if PMF_QUERY_HOST is sent, we reply! */
	if (!strcmp(line_from_mud, "PMF_QUERY_HOST")) {
	    char namebuf[MAXHOSTNAMELEN + 1], buf[MAX_LINE_LENGTH + 1];
	    extern char *gethostname();
	    if (gethostname(namebuf, MAXHOSTNAMELEN) == 0)
		sprintf(buf, "say host name \"%s\"\n",  namebuf);
	    else
		sprintf(buf, "say host name unknown\n");
	    message("Your PMF just answered a PMF_QUERY_HOST message.");
	    queue_mudline(buf, "");
	}
	/* And another one: if PMF_QUERY_TERM is sent, we reply! */
	if (!strcmp(line_from_mud, "PMF_QUERY_TERM")) {
	    char *termtype, buf[MAX_LINE_LENGTH + 1];
	    extern char *getenv();
	    termtype = getenv("TERM");
	    if (termtype)
		sprintf(buf, "say terminal type \"%s\"\n",  termtype);
	    else
		sprintf(buf, "say terminal type unknown\n");
	    message("Your PMF just answered a PMF_QUERY_TERM message.");
	    queue_mudline(buf, "");
	}

	if (robot_mode && !sending && !receiving && !getfiling && !was_gag) {
	    robot_handle_line(line_from_mud);
	}

#ifdef SOUND
	if (sound_mode && !sending && !receiving && !getfiling && !was_gag) {
	    sound_handle_line(line_from_mud);
	}
#endif

    }

    if (!waiting_for_response) {
	to_head = (struct queue_to_mud_entry *)get_from_generic_fifo(queue_to_mud);
	if (to_head) {

	    USER_DEBUG(("Got from queue: \"%s\", \"%s\"\n", to_head->line_to_send, to_head->confirm_string ? to_head->confirm_string : "NULL"));

	    line_to_mud = to_head->line_to_send;
	    if (to_head->confirm_string) {
		confirm_string_length = strlen(to_head->confirm_string);
		if (confirm_string_length > MAX_CONFIRM_STRING_LENGTH)
		    error("Ridicolously long confirmation string (%d chars): \"%s\".",
			  confirm_string_length, confirm_string);
		strcpy(confirm_string, to_head->confirm_string);
		waiting_for_response = 1;
		USER_DEBUG(("communicate_with_mud: confirm_string set to \"%s\"", confirm_string));
	    }
	    safe_free(to_head);
	    if (line_to_mud) {
		USER_DEBUG(("communicate_with_mud: sending line \"%s\"", line_to_mud));
		send_to_mud(line_to_mud);
		safe_free(line_to_mud);
	    }
	    else {
		USER_DEBUG(("communicate_with_mud: line was null, not sending"));
	    }
	    retval += 2;
	}
    }

    if (!retval)
	usleep(100000);

/*
#ifdef GNU_READLINE
    else
	pmf_rl_redisplay_line(1, 0);
#endif
*/

    return retval;
} /* communicate_with_mud */


int ok_to_send()
{

    return query_generic_fifo_size(queue_to_mud) == 0 && !waiting_for_response;
} /* ok_to_send */


int query_queue_to_mud_size()
{

    return query_generic_fifo_size(queue_to_mud);
} /* query_queue_to_mud_size */


void flush_queue_to_mud()
{
    struct queue_to_mud_entry *to_head;
    int i;

    to_head = (struct queue_to_mud_entry *)get_from_generic_fifo(queue_to_mud);
    if (to_head) {
	ldisplay("Throwing away the lines on the queue to MUD:\n");
	i = 0;
	while (to_head) {
	    ++i;
	    ldisplay("    %d:\t%s\n", i, to_head->line_to_send);
	    to_head = (struct queue_to_mud_entry *)get_from_generic_fifo(queue_to_mud);
	}
    }
    waiting_for_response = 0;
} /* flush_queue_to_mud */

int query_waiting_for_response()
{
    return waiting_for_response;
} /* query_waiting_for_response */
