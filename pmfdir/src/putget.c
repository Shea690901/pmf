/*
 *  putget.c -- the PMF commands /putfile and /getfile, for LPmud
 *
 *	NOTE: The code in this file is heavily dependent on the exact
 *	      format of input and output to the built-in LPmud editor "ed"!
 *	      If they are changed, this file will probably
 *	      have to be changed too!
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include "safe_malloc.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

extern char *get_input_line(), *expand_alias(),
    *find_alias_string(),
    *expand_history(), *get_statstring(), *getmud(),
    *expand_variables(),
    *find_robot_action_string(),
    *return_last();

/*  This is a temporary buffer of text received from the MUD game.
 *  If the flag "getfiling" is true, the routines that receive the MUD
 *  output will put it here too, using the function getfile_got_text().
 *  We use this to remember the lines received when doing a getfile,
 *  since we cannot simply write all the output to the file
 *  -- it contains the editor prompt ":", and maybe some "Harry says:" etc.
 */
static char *received_text = NULL;
static int received_text_length = 0;

/*---------------------------------------------------------------------------*/

/* Send a file to LPmud using the built-in editor */
cmd_putfile(filename, optional_filename)
char *filename, *optional_filename;
{
    char *local_filename, *remote_filename, cmd_buffer[MAX_LINE_LENGTH + 1];
    struct stat dummy_statbuf;

    local_filename = filename;
    if (optional_filename)
	remote_filename = optional_filename;
    else
	remote_filename = filename;

    if (stat(local_filename, &dummy_statbuf) != 0) {
	uerror("No such local file: \"%s\".", local_filename);
	return;
    }

    ldisplay("Putting local \"%s\" (%s) to remote \"%s\".\n",
	     local_filename, get_statstring(local_filename), remote_filename);

    sprintf(cmd_buffer, "ed %s", remote_filename);
    queue_mudline(cmd_buffer, ":");

/* -- We do NOT write a backup file on the MUD server!
    sprintf(cmd_buffer, "w %s.bak", remote_filename);
    queue_mudline(cmd_buffer, ":");
*/
    queue_mudline("1,$d", ":");
    queue_mudline("i", "*");
    while (!ok_to_send())
	communicate_with_mud();
    cmd_send(local_filename, "*", 1);
    while (!ok_to_send())
	communicate_with_mud();
    sprintf(cmd_buffer, "w %s", remote_filename);
    queue_mudline(".", ":");
    queue_mudline(cmd_buffer, ":");
    queue_mudline("Q", ">");
    while (!ok_to_send())
	communicate_with_mud();
    ldisplay("\n");
    ldisplay("Finished putting local \"%s\".\n", local_filename);

} /* cmd_putfile */

/*---------------------------------------------------------------------------*/

static void reset_received_text()
{

    if (received_text != NULL)
	safe_free(received_text);
    received_text = NULL;
    received_text_length = 0;
} /* reset_received_text */

/*---------------------------------------------------------------------------*/

/* Get a file from LPmud using the built-in editor */
cmd_getfile(filename, optional_filename)
char *filename, *optional_filename;
{
    char *local_filename, *remote_filename, cmd_buffer[MAX_LINE_LENGTH + 1];
    int nr_lines, linenr, chunkstart, chunkend, got_lines, lines_to_get;

    if (sending || receiving || getfiling) {
	message("Cannot do more than one send or receive command at the same time.");
	return;
    }

    /* Must be done before communicate_with_mud() is called: */
    reset_received_text();

    remote_filename = filename;
    if (optional_filename)
	local_filename = optional_filename;
    else
	local_filename = filename;

    if (backup_file(local_filename) == -1)
	uerror("Couldn't backup.");
    else if ((the_open_file = fopen(local_filename, "w")) == NULL) {
	uerror("Couldn't open local file \"%s\".", local_filename);
    }

    /* Remember the file and its name so we can close it if an error occurs: */
    /* We already set "the_open_file". */
    the_open_file_name = strcpy(safe_malloc(strlen(local_filename) + 1),
				local_filename);
    getfiling = 1;

    ldisplay("Getting remote \"%s\" into local \"%s\".\n", remote_filename, local_filename);

    /* First: Enter the editor. */
    sprintf(cmd_buffer, "ed %s", remote_filename);
    queue_mudline(cmd_buffer, ":");
    while (!ok_to_send())
	communicate_with_mud();

    /* Get the number of lines in the remote file. */
    do {
	reset_received_text();
	queue_mudline("$=", ":");
	while (!ok_to_send())
	    communicate_with_mud();
    } while (nr_lines_in_text(received_text) != 1 || received_text[received_text_length - 1] != ':');

    nr_lines = atoi(received_text);
    ldisplay("\nThe remote file \"%s\" seems to contain %d lines.\n", remote_filename, nr_lines);

    /* Get the lines. */
    for (linenr = 1; linenr < nr_lines + 1; linenr += GETFILE_CHUNKSIZE) {
	while (!ok_to_send())
	    communicate_with_mud();
	chunkstart = linenr;
	chunkend = chunkstart + GETFILE_CHUNKSIZE - 1;
	if (chunkend > nr_lines)
	    chunkend = nr_lines;
	lines_to_get = chunkend - chunkstart + 1;
	sprintf(cmd_buffer, "%d,%dp", chunkstart, chunkend);
	do {
	    USER_DEBUG(("Trying to get lines %d..%d from remote file \"%s\"", chunkstart, chunkend, remote_filename));
	    reset_received_text();
	    queue_mudline(cmd_buffer, ":");
	    while (!ok_to_send())
		communicate_with_mud();
	    got_lines = nr_lines_in_text(received_text);
	    if (got_lines != lines_to_get || received_text[received_text_length - 1] != ':') {
		ldisplay("\n");
		message("Failed to receive lines %d..%d. Trying again.", chunkstart, chunkend);
	    }
	} while (got_lines != lines_to_get || received_text[received_text_length - 1] != ':');
	USER_DEBUG(("Got lines %d..%d from remote file \"%s\"", chunkstart, chunkend, remote_filename));
	received_text[received_text_length - 1] = '\0';	/* Remove the ed prompt ":" */
	fprintf(the_open_file, "%s", received_text);	/* No '\n' here. */
	fflush(the_open_file);
    }

    ldisplay("\n");

    /* Close the file, free the file name buffer, and restore all states. */
    fclose(the_open_file);
    safe_free(the_open_file_name);
    the_open_file = NULL;
    the_open_file_name = NULL;
    getfiling = 0;

    /* And exit the LPmud editor. */
    while (!ok_to_send())
	communicate_with_mud();
    queue_mudline("Q", ">");
    while (!ok_to_send())
	communicate_with_mud();

    ldisplay("Finished getting remote \"%s\" into local \"%s\" (%s).\n",
	     remote_filename, local_filename, get_statstring(local_filename));
} /* cmd_getfile */

/*---------------------------------------------------------------------------*/

/*  This function should be called from the routines that receive text
 *  from MUD, if and only if "getfiling" is true.
 */
char *getfile_got_text(text_from_mud)
char *text_from_mud;
{

    ASSERT(getfiling);
    ASSERT(text_from_mud != NULL);

    if (received_text == NULL) {
	received_text_length = strlen(text_from_mud);
	    /* Without the null byte! */
	received_text = safe_malloc(received_text_length + 1);
	    /* The null byte too! */
	strcpy(received_text, text_from_mud);
    }
    else {
	received_text =
	    safe_realloc(received_text,
			 (received_text_length += strlen(text_from_mud)) + 1);
	             /* The null byte here too, idiot! */
	strcat(received_text, text_from_mud);
    }
    return received_text;
} /* getfile_got_text */
