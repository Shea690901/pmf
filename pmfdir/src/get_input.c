/*
 *  get_input.c -- get an input line (with editing, if from a terminal)
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <strings.h>
#include <stdio.h>
#include "config.h"
#include "pmf.h"
#include "globals.h"

#ifdef GNU_READLINE
  extern char *readline();
#endif

/*---------------------------------------------------------------------------*/

char *get_input_line(infile)
FILE *infile;
{
    static char line_buffer[MAX_LINE_LENGTH + 1];
    char *readline_result;
    int len, this_char;

#ifdef GNU_READLINE
    if (infile == stdin && isatty(fileno(infile))) {

	readline_prompt_first_char = readline_prompt[0];
	readline_prompt[0] = '\0';
	/* readline_result = readline(readline_prompt); */
	readline_result = readline(NULL);

	/*  readline() calls communicate_with_mud(),
	 *  that checks and handles output from mud and also
	 *  sets back the readline_prompt!
	 */

	if (readline_result)
	    { USER_DEBUG(("readline_result: '%s'", readline_result)); }
	else
	    { USER_DEBUG(("readline_result == NULL")); }

	if (readline_result == NULL)
	    was_feof = 1;
	user_newline();

	return readline_result;
    }
#endif

    if (infile == stdin && !input_from_player_is_available())
	return NULL;

    line_buffer[MAX_LINE_LENGTH] = '\0';
    if (fgets(line_buffer, MAX_LINE_LENGTH + 1, infile) == NULL) {
	return NULL;
    }
    len = strlen(line_buffer);

    if (line_buffer[len - 1] == '\n') {
	line_buffer[len - 1] = '\0';
    }
    else {
	/* Too long line. Ignore it and skip to after the next newline. */
	error("Line too long (more than %d characters). The line is ignored.",
	      MAX_LINE_LENGTH);
	while ((this_char = getc(infile)) != '\n' && this_char != EOF)
	    ;
	return NULL;
    }

    if (infile == stdin && isatty(fileno(infile)))
	user_newline();

    return line_buffer;
} /* get_input_line */
