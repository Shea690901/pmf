/*  bcs.c
 *	-- a package for dynamic building of constant strings
 *  Thomas Padron-McCarthy (padrone@lysator.liu.se), 1991
 *  This file latest updated: Sept 22, 1991
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

static char *buffer = NULL, *current_pos, *after_buffer;
static int buffer_size = FIRST_BUFFER_SIZE;

static void new_buffer()
{
    /* Each new buffer is twice as big as the previous one. */
    buffer = safe_malloc(buffer_size);
    current_pos = buffer;
    after_buffer = buffer + buffer_size;
    buffer_size *= 2;
    /*  It's not a good idea to realloc the old buffer
     *  to save some space. There are pointers into it, remember?
     *  We could remember the extra bytes at the end in some way,
     *  but I don't think it's worth the trouble.
     */
} /* new_buffer */

/*-----------------------------------------------------------------*/

struct const_string_being_built *begin_const_string()
{
    register struct const_string_being_built *csbb;

    if (buffer == NULL)
	new_buffer();
    csbb = (struct const_string_being_built *)
	safe_malloc(sizeof(struct const_string_being_built));
    csbb->start_cp = current_pos;
    csbb->cp = current_pos;
    csbb->length = 0;
    return csbb;
} /* begin_const_string */

void bcs_add_char(csbb, c)
  register struct const_string_being_built *csbb;
  int c;
{
    if (current_pos == after_buffer) {
	new_buffer();
	strncpy(buffer, csbb->start_cp, csbb->length);
	csbb->start_cp = buffer;
	current_pos += csbb->length;
	csbb->cp = current_pos;
    }
    *current_pos++ = c;
    csbb->cp = current_pos;
    csbb->length += 1;
} /* bcs_add_char */

char *end_const_string(csbb)
  register struct const_string_being_built *csbb;
{
    register char *temp;

    bcs_add_char(csbb, '\0');
    temp = csbb->start_cp;
    free(csbb);
    return temp;
} /* end_const_string */
