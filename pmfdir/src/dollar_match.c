/*
 *  dollar_match.c -- matching patterns containing "$" place holders
 *
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <stdio.h>
#include <ctype.h>
#include "pmf.h"
#include "config.h"
#include "globals.h"

/*---------------------------------------------------------------------------*/

/*  This function is called with a pattern, a string, an array of char
 *  pointers, and a buffer. It matches the pattern and the string, and
 *  returns the number of words (0 or more) that was stored in the buffer
 *  and pointed to by the pointers in the array.
 *  If the match failed, -1 is returned.
 *  No checks are done to see if the array and the buffer sizes are enough.
 */
int dollar_match(pattern_cp, string_cp, words, buffer_cp)
  register char *pattern_cp, *string_cp, *buffer_cp;
  char *words[];
{
    int match_result, word_length;

    INTERNAL_DEBUG(("dollar_match(\"%s\", \"%s\")", pattern_cp, string_cp));

    while (*pattern_cp != '$')
	if (!*pattern_cp)	
	    return 0;	/* Finished. An exact match. */
	else if (*pattern_cp++ != *string_cp++)
	    return -1;	/* Match failed. */
    
    /* Now we have a $ character in the pattern. Skip it. */
    ++pattern_cp;
    
    if (*pattern_cp == '$') {
	/* There was a $$ in the pattern. */

	++pattern_cp;
	if (*string_cp++ == '$')
	    return dollar_match(pattern_cp + 1, string_cp + 1, words, buffer_cp);
	else
	    return -1;
    }

    /* Now we have either $number or $Lnumber in pattern. */
    if (*pattern_cp == 'L' || *pattern_cp == 'l')
	;

    /* Read past the digits in $number or $Lnumber. */
    while (isdigit(*pattern_cp))
	++pattern_cp;

    /*  Now, copy from string into a word, and match against the rest
     *  Start by copying as long a word as possible, so for example
     *    dollar_match("Guest says: $1", Guest says: Hi there!", ...)
     *  will put "Hi there!" in $1, not the empty string
     *  (which would be the shortest possibility in this case).
     */
    words[0] = buffer_cp;
    word_length = strlen(string_cp);
    while ((match_result = dollar_match(pattern_cp, string_cp + word_length, words + 1, buffer_cp + word_length + 1)) == -1) {

	if (word_length < 0)
	    return -1;	/* No possible match. */
	--word_length;

    } /* while */

    strncpy(buffer_cp, string_cp, word_length);
    buffer_cp[word_length] = '\0';
    return match_result + 1;
} /* dollar_match */
