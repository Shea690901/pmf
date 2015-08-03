/*
 *  apply.c -- the function apply_definition
 * 
 *  PMF -- Padrone's MudFrontend, a frontend for (maybe mostly LP-)mud
 *  Thomas Padron-McCarthy (Email: padrone@lysator.liu.se), 1990, 1991
 *  Share and enjoy, but be nice: don't steal my program! Hugo is watching!
 *  This file latest updated: Sept 21, 1991
 *
 */

#include <stdio.h>
#include <strings.h>
#include <ctype.h>
#include "safe_malloc.h"
#include "config.h"
#include "pmf.h"
#include "globals.h"

/*---------------------------------------------------------------------------*/

/*  This function is called with a string containing a macro definition
 *  (for example an alias definition), an integer containing the number
 *  of arguments, an a pointer to an array, containing those arguments.
 *  Note that argument 0 is the command itself, and normally not used.
 *  A malloced string is returned, containg the evaluated form.
 *  In the pattern, these substitutions are performed:
 *	$N, where N is a number, is changed to argument N
 *	$* is changed to the list of all arguments (except argument 0)
 *	$LN, as $N but lower-cased argument
 *	$L*, as $* but lower-cased arguments
 *	$$ is changed to $
 *	$anything-else (including $n) is not replaced
 *  If append_if_none_inserted is true, and no arguments were inserted,
 *  all arguments (except argument 0) are appended to the resulting string.
 */
char *apply_definition(definition, nr_arguments, arguments, append_if_none_inserted)
char *definition, **arguments;
int nr_arguments;
int append_if_none_inserted;
{
    register char *result, *defcp, *rescp;
    int argnr;
    int arg_was_inserted;
    int lower_flag;	/* Also used to remember if 'L' or 'l' */

    USER_DEBUG(("apply_definition(\"%s\", %d, ...)", definition, nr_arguments));

    result = safe_malloc(10 * MAX_LINE_LENGTH + 1);
    rescp =  result;
    defcp = definition;
    arg_was_inserted = 0;

    while (*defcp) {
	if (*defcp == '$') {
	    ++defcp;
	    if ((*defcp == 'L' || *defcp == 'l') && (defcp[1] == '*' || isdigit(defcp[1]))) {
		/* $Lsomething or $lsomething */
		lower_flag = *defcp;
		++defcp;
	    }
	    else
		lower_flag = 0;
	    if (*defcp == '$')
		/* $$ */
		*rescp++ = *defcp++;
	    else if (*defcp == '*') {
		/* $* */
		for (argnr = 1; argnr < nr_arguments; ++argnr) {
		    if (argnr > 1)
			*rescp++ = ' ';
		    strcpy(rescp, arguments[argnr]);
		    if (lower_flag)
			lower_string(rescp);
		    rescp += strlen(arguments[argnr]);
		    arg_was_inserted = 1;
		}
		defcp++;
	    }
	    else if (isdigit(*defcp)) {
		/* $number */
		argnr = atoi(defcp);
		if (argnr >= 0 && argnr < nr_arguments) {
		    strcpy(rescp, arguments[argnr]);
		    if (lower_flag)
			lower_string(rescp);
		    rescp += strlen(arguments[argnr]);
		    arg_was_inserted = 1;
		}
		else
		    error("Bad argument reference ($%d) when applying definition \"%s\".",
			  argnr, definition);
		while (isdigit(*defcp))
		    ++defcp;
	    }
	    else {
		/* $something-else */
		*rescp++ = '$';
		if (lower_flag)
		    *rescp++ = lower_flag;
	    }
	}
	else {
	    *rescp++ = *defcp++;
	}
    } /* while not end of definition string */

    /* If no arguments were inserted with the $N or $* syntax, just append them. */
    if (append_if_none_inserted && !arg_was_inserted) {
	for (argnr = 1; argnr < nr_arguments; ++argnr) {
	    *rescp++ = ' ';
	    strcpy(rescp, arguments[argnr]);
	    rescp += strlen(arguments[argnr]);
	}
    }
    *rescp++ = '\0';
    return safe_realloc(result, strlen(result) + 1);
} /* apply_definition */
