/*  build_string.c
 *	-- a package for dynamic building of strings
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 21, 1991
 *
 *  These functions are used to build strings of arbitrary lengths.
 *  The result is an ordinary malloc'ed NUL-terminated string,
 *  that can be realloc'ed or free'd in the usual way.
 *  No space is wasted in the resulting string, since it is
 *  realloc'ed to its final length (plus the NUL byte).
 *  This package uses the safe_malloc package.
 */

#include <string.h>
#include "safe_malloc.h"
#include "sg_config.h"
#include "str_galore.h"

#ifndef NULL
#	define NULL	0
#endif

/*-----------------------------------------------------------------*/

struct string_being_built *begin_string()
{
    register struct string_being_built *sbb;

    sbb = (struct string_being_built *)
	safe_malloc(sizeof(struct string_being_built));
    sbb->string_buffer = safe_malloc(INITIAL_STRING_LENGTH + 1);
    sbb->buffer_length = INITIAL_STRING_LENGTH + 1;
    sbb->string_length = 0;
    return sbb;
} /* begin_string */

void bs_add_char(sbb, c)
  register struct string_being_built *sbb;
  int c;
{
    sbb->string_buffer[sbb->string_length++] = c;
    if (sbb->string_length >= sbb->buffer_length) {
	sbb->buffer_length += STRING_INCREMENT;
	sbb->string_buffer =
	    safe_realloc(sbb->string_buffer, sbb->buffer_length);
    }
} /* bs_add_char */

char *end_string(sbb)
  register struct string_being_built *sbb;
{
    register char *temp;

    if (sbb->string_length + 1 != sbb->buffer_length)
	sbb->string_buffer =
	    safe_realloc(sbb->string_buffer, sbb->string_length + 1);
    temp = sbb->string_buffer;
    temp[sbb->string_length] = '\0';
    free(sbb);
    return temp;
} /* end_string */
