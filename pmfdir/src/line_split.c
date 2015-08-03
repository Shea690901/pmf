/*
 *  line_split.c -- split a string into lines
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <ctype.h>
#include "pmf.h"

/*---------------------------------------------------------------------------*/

/*  This function is similar to split_line(), but it splits the_line
 *  into strings splitting at every unquoted $n in the string the_line.
 *  When this function is called, we know that the line contains at least
 *  one $n, meaning that it will be split into at least two lines.
 *  It returns the (NOT NEGATIVE) number of lines extracted.
 */
int line_split(the_line, the_array, the_buffer)
char *the_line, *the_buffer;
char **the_array;
{
    int linenr, inside_quotes, copying;
    register char *line_cp, *buffer_cp;

    line_cp = the_line;
    buffer_cp = the_buffer;
    linenr = 0;
    inside_quotes = 0;
    copying = 0;

    while (*line_cp) {
	/* Skip Botha */
	while (*line_cp && isspace(*line_cp))
	    ++line_cp;

	/* Start reading a line, if there is one. */
	if (!*line_cp)
	    return linenr;	/* End of string. Finished. */
	else if (inside_quotes)
	    error("This cannot happen: a line started inside a string: %s", line_cp);
	else if (*line_cp == '$') {
	    /* A character quoted by a $ character. Check next character. */
	    ++line_cp;
	    if (*line_cp == 'n') {
		/* Found an un-quoted $n. End of this line! */
		++line_cp;
		the_array[linenr++] = buffer_cp;	/* Start this line first. */
		*buffer_cp++ = '\0';
		skip_trailing_space(the_array[linenr - 1]);
		the_array[linenr++] = buffer_cp;	/* Start of the next line. */
	    }
	    else if (*line_cp == '\0') {
		/* Last character in the line was a dollar. */
		error("Tried to quote newline (last char on line was $).");
	    }
	    else {
		/* A $-quoted character, except for 'n' and '\0'. Just copy it. WITH the $! */
		the_array[linenr++] = buffer_cp;	/* A line has started. */
		*buffer_cp++ = '$';
		*buffer_cp++ = *line_cp++;		/* First character of that line. */
	    }
	}
	else if (*line_cp == '"') {
	    /* Found an un-quoted double quote. This means that a string starts here. */
	    inside_quotes = 1;
	    the_array[linenr++] = buffer_cp;	/* A line has started. */
	    *buffer_cp++ = *line_cp++;		/* First character of that line. */
	}
	else {
	    /* Found a character other than dollar or double quote. */
	    the_array[linenr++] = buffer_cp;	/* A line has started. */
	}

	copying = 1;

	/* Read the line. If it started with " or $-something, those are already copied. */
	while (copying) {
	    if (*line_cp == '$') {
		/* A character quoted by a $ character. Check next character. */
		++line_cp;
		if (*line_cp == 'n' && !inside_quotes) {
		    /* Found an un-quoted $n. End of this line. */
		    ++line_cp;
		    *buffer_cp++ = '\0';
		    skip_trailing_space(the_array[linenr - 1]);
		    copying = 0;
		}
		else if (*line_cp == '\0') {
		    /* Last character in the line was a dollar. */
		    error("Tried to quote newline (last char on line was $).");
		}
		else {
		    *buffer_cp++ = '$';
		}
	    }
	    else if (*line_cp == '"') {
		inside_quotes = !inside_quotes;
	    }
	    else if (!*line_cp) {
		/* Put the '\0' at the end of this line. */
		*buffer_cp++ = '\0';
		skip_trailing_space(the_array[linenr - 1]);
		copying = 0;
	    }
	    else {
		/* Found a character other than dollar or double quote. */
	    }
	    if (copying)
		*buffer_cp++ = *line_cp++;
	} /* Loop until end of line or end of this line */

	if (!*line_cp && inside_quotes)
	    error("String does not end (mismatched \" on input line: %s).", the_line);

    } /* while not end of line */

    if (inside_quotes)
	error("This cannot happen. Inside line_split.");

    return linenr;
} /* line_split */
