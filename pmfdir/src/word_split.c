/*
 *  word_split.c -- split a string into words (or lines)
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

/*  This function splits the string the_line into words OR LINES.
 *  The string the_line is supposed to end with '\0' and contain no newlines.
 *  If the string contains an un-quoted $n (i. e. outside a double-quoted
 *  string and not quoted as $$n), the string is split into lines.
 *  If there is no un-quoted $n in the string linecp, it is split into words.
 *  
 *  The words or lines themselves are put in the buffer pointed to by
 *  the_buffer, and the_array will contain pointers into that buffer.
 *  
 *  The contents of the orignal line is not changed.
 *  
 *  If the string the_line was split into words, the number of words extracted
 *  from the string is returned. If it was split into lines, the NEGATIVE
 *  number of lines is returned.
 *  If there was an error, or there the line was blank, 0 is returned.
 *  
 */
int word_split(the_line, the_array, the_buffer)
char *the_line, *the_buffer;
char **the_array;
{
    int wordnr, inside_quotes;
    register char *line_cp, *buffer_cp;

    line_cp = the_line;
    buffer_cp = the_buffer;
    wordnr = 0;
    inside_quotes = 0;

    while (*line_cp) {
	/* Skip Botha */
	while (*line_cp && isspace(*line_cp))
	    ++line_cp;

	/* Start reading a word, if there is one. */
	if (!*line_cp)
	    return wordnr;	/* End of string. Finished. */
	else if (inside_quotes)
	    error("This cannot happen: a word started inside a string: %s", line_cp);
	else if (*line_cp == '$') {
	    /* A character quoted by a $ character. Check next character. */
	    ++line_cp;
	    if (*line_cp == 'n') {
		/* Found an un-quoted $n. Split by lines instead of words! */
		return -line_split(the_line, the_array, the_buffer);
	    }
	    else if (*line_cp == '\0') {
		/* Last character in the line was a dollar. */
		message("Malformed line: Tried to quote newline (last char on line was $).");
		return 0;
	    }
	    else {
		/* A $-quoted character, except for 'n' and '\0'. Just copy it. */
		the_array[wordnr++] = buffer_cp;	/* A word has started. */
		*buffer_cp++ = *line_cp++;		/* First character of that word. */
	    }
	}
	else if (*line_cp == '"') {
	    /* Found an un-quoted double quote. This means that a string starts here. */
	    inside_quotes = 1;
	    ++line_cp;
	    the_array[wordnr++] = buffer_cp;	/* A word has started. */
	}
	else {
	    /* Found a character other than dollar or double quote. */
	    the_array[wordnr++] = buffer_cp;	/* A word has started. */
	}

	/* Read the word. If it started with " or $-something, those are already copied. */
	while (*line_cp && *line_cp != '"' && (inside_quotes || !isspace(*line_cp))) {
	    if (*line_cp == '$') {
		/* A character quoted by a $ character. Check next character. */
		++line_cp;
		if (*line_cp == 'n' && !inside_quotes) {
		    /* Found an un-quoted $n. Split by lines instead of words! */
		    return line_split(the_line, the_array, the_buffer);
		}
		else if (*line_cp == '\0') {
		    /* Last character in the line was a dollar. */
		    message("Malformed line: Tried to quote newline (last char on line was $).");
		    return 0;
		}
		else if (*line_cp == '"') {
		    /* A $ plus a ". */
		    *buffer_cp++ = *line_cp++;
		}
		else {
		    /* A $ plus a character. */
		    if (inside_quotes) {
			*buffer_cp++ = '$';
			*buffer_cp++ = *line_cp++;
		    }
		    else
			*buffer_cp++ = *line_cp++;
		}
	    }
	    else {
		/* Found a character other than dollar or double quote. */
		*buffer_cp++ = *line_cp++;
	    }
	} /* Loop until end of line, end of this word, or end of this double-quoted string. */

	if (!*line_cp && inside_quotes) {
	    message("Malformed line: String does not end (mismatched \" on input line: %s).", the_line);
	    return 0;
	}
	else if (*line_cp == '"') {
	    /* Found an un-quoted double quote. This means that this string ends here. */
	    inside_quotes = 0;
	    ++line_cp;
	}

	/* Put the '\0' at the end of this word. */
	*buffer_cp++ = '\0';

    } /* while not end of line */

    if (inside_quotes)
	error("Ooops: Take care of this, Padrone!");

    return wordnr;
} /* word_split */
