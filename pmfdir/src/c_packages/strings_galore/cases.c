/*  cases.c -- some case conversions
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 */

#include <string.h>
#include <ctype.h>
#include "safe_malloc.h"
#include "sg_config.h"
#include "str_galore.h"

char *lower_string_copy(str)
  char *str;
{
    register char *start_cp, *cp;

    cp = start_cp = copy_string(str);
    while (*cp) {
	if (isupper(*cp))
	    *cp = tolower(*cp);
	++cp;	/* Never trust a macro! */
    }
    return start_cp;
} /* lower_string_copy */

char *upper_string_copy(str)
  char *str;
{
    register char *start_cp, *cp;

    cp = start_cp = copy_string(str);
    while (*cp) {
	if (islower(*cp))
	    *cp = toupper(*cp);
	++cp;	/* Never trust a macro! */
    }
    return start_cp;
} /* upper_string_copy */
